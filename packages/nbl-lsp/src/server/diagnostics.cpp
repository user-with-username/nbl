#include "nbl/lsp/server/server.h"

#include <algorithm>
#include <iterator>
#include <utility>

#include "nbl/lsp/server/constants.h"
#include "nbl/lsp/server/file_uri.h"
#include "nbl/lsp/text/lsp_diagnostic.h"
#include "nbl/utils/files.h"
#include "nbl/utils/sources.h"

namespace nbl::lsp {

namespace {

using Files = nbl::utils::OverlaySources::Files;

/// Text of every module that has diagnostics: from the open-document overlay
/// when present, else from disk. Modules that cannot be read are omitted.
std::map<std::string, std::string>
load_module_texts(const DiagnosticsByModule &diagnostics, const Files &files) {
  std::map<std::string, std::string> texts;

  for (const auto &[module, list] : diagnostics) {
    (void)list;
    if (auto it = files.find(module); it != files.end()) {
      texts.emplace(module, it->second);
      continue;
    }
    auto text = nbl::utils::read_file_opt(module);
    if (text)
      texts.emplace(module, std::move(*text));
  }

  return texts;
}

std::vector<::lsp::Diagnostic>
convert_diagnostics(const std::vector<nbl::analysis::Diagnostic> &list,
                    std::string_view source) {
  std::vector<::lsp::Diagnostic> converted;
  converted.reserve(list.size());

  for (const nbl::analysis::Diagnostic &diagnostic : list)
    converted.push_back(to_diagnostic(diagnostic, source));

  return converted;
}

} // namespace

void Server::schedule_check(const std::string &uri) {
  const auto found = file_path(uri);
  if (!found)
    return;
  const std::string path = *found;

  // Classification happens here on the LSP thread: by the time the job is
  // posted to a worker, both the current text and the require index are
  // up to date.
  std::string text;
  if (const std::optional<Document> doc = document(uri)) {
    text = doc->text;
  } else {
    auto from_disk = nbl::utils::read_file_opt(path);
    if (!from_disk)
      return;
    text = std::move(*from_disk);
  }

  const nbl::analysis::ModuleKind kind = module_kind(path, text);

  workers_->forKey(path).post(
      path, kDiagnosticsJob, [this, uri, path, kind] {
        const std::optional<Document> current = document(uri);
        if (!current)
          return;

        const unsigned index = workers_->indexForKey(path);
        const Files files = snapshot();
        const nbl::utils::OverlaySources sources{files};

        publish(uri, current->version,
                checkers_[index]->check(path, sources, kind), files);
      });
}

void Server::recheck_all_open() {
  std::vector<std::string> uris;
  {
    const std::lock_guard lock(mutex_);
    uris.reserve(documents_.size());
    for (const auto &[uri, _] : documents_)
      uris.push_back(uri);
  }
  for (const std::string &uri : uris)
    schedule_check(uri);
}

void Server::publish(const std::string &uri, int version,
                     const DiagnosticsByModule &diagnostics,
                     const Files &files) {
  // The document moved on while the checker ran: drop the stale result.
  const std::optional<Document> current = document(uri);
  if (!current || current->version != version)
    return;

  const std::map<std::string, std::string> texts =
      load_module_texts(diagnostics, files);

  std::set<std::string> reported;
  for (const auto &[module, list] : diagnostics) {
    const auto text = texts.find(module);
    const std::string_view source =
        text == texts.end() ? std::string_view{} : text->second;

    send_diagnostics(module, convert_diagnostics(list, source));
    reported.insert(module);
  }

  // Modules that had diagnostics last time but none now must be cleared.
  std::set<std::string> stale;
  {
    const std::lock_guard lock(mutex_);
    std::set<std::string> &previous = published_[uri];
    std::set_difference(previous.begin(), previous.end(), reported.begin(),
                        reported.end(), std::inserter(stale, stale.begin()));
    previous = reported;
  }

  for (const std::string &module : stale)
    send_diagnostics(module, {});
}

void Server::send_diagnostics(const std::string &module,
                              std::vector<::lsp::Diagnostic> diagnostics) {
  endpoint_.textDocumentPublishDiagnostics(
      ::lsp::PublishDiagnosticsParams{
          .uri = ::lsp::Uri::parse(uri_for(module)),
          .diagnostics = std::move(diagnostics),
      });
}

} // namespace nbl::lsp

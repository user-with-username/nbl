#include "nbl/lsp/server/server.h"

#include <utility>

#include "nbl/lsp/server/file_uri.h"
#include "nbl/utils/files.h"
#include "nbl/utils/sources.h"

namespace nbl::lsp {

void Server::open(std::string uri, std::string text, int version) {
  {
    const std::lock_guard lock(mutex_);
    documents_[uri] = Document{std::move(text), version};
  }

  index_document(uri);
  schedule_check(uri);
}

void Server::change_document(const std::string &uri, const std::string &text,
                             int version) {
  {
    const std::lock_guard lock(mutex_);
    auto it = documents_.find(uri);
    if (it == documents_.end())
      return;

    it->second.text = text;
    it->second.version = version;
  }

  index_document(uri);
  schedule_check(uri);
}

void Server::close(const std::string &uri) {
  bool known = false;
  {
    const std::lock_guard lock(mutex_);
    known = documents_.erase(uri) > 0;
    published_.erase(uri);
  }

  // The document is gone from the overlay: index it from disk again.
  if (const auto path = file_path(uri)) {
    static const nbl::utils::FileSystemSources kFiles;
    const std::lock_guard lock(mutex_);
    require_index_.update(*path, kFiles);
  }

  if (known)
    endpoint_.textDocumentPublishDiagnostics(
        ::lsp::PublishDiagnosticsParams{
            .uri = ::lsp::Uri::parse(uri),
            .diagnostics = {},
        });
}

void Server::index_document(const std::string &uri) {
  if (const auto path = file_path(uri)) {
    const nbl::utils::OverlaySources sources{snapshot()};
    const std::lock_guard lock(mutex_);
    require_index_.update(*path, sources);
  }
}

nbl::utils::OverlaySources::Files Server::snapshot() const {
  const std::lock_guard lock(mutex_);

  nbl::utils::OverlaySources::Files files;
  for (const auto &[uri, document] : documents_) {
    if (const auto path = file_path(uri))
      files.emplace(*path, document.text);
  }

  return files;
}

std::optional<Server::Document>
Server::document(const std::string &uri) const {
  const std::lock_guard lock(mutex_);

  auto it = documents_.find(uri);
  if (it == documents_.end())
    return std::nullopt;

  return it->second;
}

std::string Server::uri_for(const std::string &path) const {
  const std::lock_guard lock(mutex_);

  for (const auto &[uri, document] : documents_) {
    const auto candidate = file_path(uri);
    if (candidate && *candidate == path)
      return uri;
  }

  return ::lsp::Uri::fileUriFromPath(path).toString();
}

} // namespace nbl::lsp

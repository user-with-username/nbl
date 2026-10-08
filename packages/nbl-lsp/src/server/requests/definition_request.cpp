#include "nbl/lsp/server/server.h"

#include <utility>

#include "nbl/lsp/server/file_uri.h"
#include "nbl/lsp/server/resolved_text.h"
#include "nbl/lsp/text/positions.h"
#include "nbl/utils/sources.h"

namespace nbl::lsp {

::lsp::RequestResult<::lsp::TextDocumentDefinitionResult>
Server::definition(::lsp::DefinitionParams &&params) {
  const std::string uri = params.textDocument.uri.toString();
  const unsigned line = static_cast<unsigned>(params.position.line);
  const unsigned character = static_cast<unsigned>(params.position.character);

  const auto found_path = file_path(uri);
  if (!found_path)
    return ::lsp::TextDocumentDefinitionResult{};

  const std::string path = *found_path;
  const unsigned index = workers_->indexForKey(path);

  // std::future<Result> becomes RequestResult automatically via
  // RequestResult(FutureType&&). The framework calls .get() on a worker.
  auto future = workers_->forKey(path).submit(
      path,
      [this, index, path, line,
       character]() -> ::lsp::TextDocumentDefinitionResult {
        Checker &checker = *checkers_[index];

        const nbl::utils::OverlaySources::Files files = snapshot();
        const nbl::utils::OverlaySources sources{files};

        const ResolvedText source(files, path);
        const Luau::Position position{
            line, static_cast<unsigned>(
                      to_byte_column(source.view(), line, character))};

        const std::optional<Definition> found =
            checker.definition(path, position, sources);
        if (!found)
          return ::lsp::TextDocumentDefinitionResult{};

        // The range is expressed in the target's text: the definitions file
        // lives in memory, anything else is an open document or on disk.
        std::optional<ResolvedText> target_file;
        std::string_view target_text;
        if (found->path == checker.definitions_path()) {
          target_text = checker.definitions();
        } else {
          target_file.emplace(files, found->path);
          target_text = target_file->view();
        }

        return ::lsp::TextDocumentDefinitionResult{::lsp::Location{
            ::lsp::Uri::parse(uri_for(found->path)),
            to_range(target_text, found->location),
        }};
      });

  return ::lsp::RequestResult<::lsp::TextDocumentDefinitionResult>(
      std::move(future));
}

} // namespace nbl::lsp

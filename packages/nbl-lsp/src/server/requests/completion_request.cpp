#include "nbl/lsp/server/server.h"

#include <utility>

#include "nbl/lsp/completion/cursor.h"
#include "nbl/lsp/completion/lsp_items.h"
#include "nbl/lsp/server/file_uri.h"
#include "nbl/lsp/server/resolved_text.h"
#include "nbl/utils/sources.h"

namespace nbl::lsp {

::lsp::RequestResult<::lsp::TextDocumentCompletionResult>
Server::completion(::lsp::CompletionParams &&params) {
  const std::string uri = params.textDocument.uri.toString();
  const unsigned line = static_cast<unsigned>(params.position.line);
  const unsigned character = static_cast<unsigned>(params.position.character);

  const auto found_path = file_path(uri);
  if (!found_path)
    return ::lsp::TextDocumentCompletionResult{};

  const std::string path = *found_path;
  const unsigned index = workers_->indexForKey(path);

  auto future = workers_->forKey(path).submit(
      path,
      [this, index, path, line,
       character]() -> ::lsp::TextDocumentCompletionResult {
        const nbl::utils::OverlaySources::Files files = snapshot();
        const nbl::utils::OverlaySources sources{files};

        const ResolvedText source(files, path);
        const CompletionCursor cursor =
            make_completion_cursor(source.view(), line, character);

        std::vector<CompletionItem> items =
            checkers_[index]->complete(path, cursor.position, sources);

        return ::lsp::TextDocumentCompletionResult{::lsp::CompletionList{
            .items = to_lsp_items(std::move(items), cursor.replace_range),
        }};
      });

  return ::lsp::RequestResult<::lsp::TextDocumentCompletionResult>(
      std::move(future));
}

} // namespace nbl::lsp

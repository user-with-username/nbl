#include "nbl/lsp/server/capabilities.h"

#include <string>
#include <vector>

namespace nbl::lsp {

::lsp::InitializeResult make_initialize_result() {
  // Field order in ServerCapabilities is dictated by lsp/types.h:
  // completionProvider is declared before definitionProvider.
  return {
      .capabilities =
          {
              .positionEncoding = ::lsp::PositionEncodingKind::UTF16,
              .textDocumentSync =
                  ::lsp::TextDocumentSyncOptions{
                      .openClose = true,
                      .change = ::lsp::TextDocumentSyncKind::Full,
                  },
              .completionProvider =
                  ::lsp::CompletionOptions{
                      .triggerCharacters =
                          std::vector<std::string>{".", ":", "\"", "/"},
                      .resolveProvider = false,
                  },
              .definitionProvider = true,
          },
      .serverInfo =
          ::lsp::ServerInfo{
              .name = "nbl-lsp",
              .version = "0.1.0",
          },
  };
}

} // namespace nbl::lsp

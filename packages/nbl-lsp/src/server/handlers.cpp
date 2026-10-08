#include "nbl/lsp/server/server.h"

#include <iostream>
#include <utility>
#include <variant>

namespace nbl::lsp {

void Server::install_handlers() {
  endpoint_.setLogHook([](std::string_view level, std::string_view message) {
    std::cerr << "[lsp] " << level << ": " << message << '\n';
  });

  endpoint_
      .onInitialize([this](::lsp::InitializeParams &&params) {
        return initialize(params);
      })
      .onInitialized([this](auto &&) {
        if (load_error_) {
          log(::lsp::MessageType::Error, *load_error_);
          return;
        }
        start_index_build();
      })
      .onTextDocumentDidOpen([this](auto &&params) {
        open(params.textDocument.uri.toString(),
             std::move(params.textDocument.text),
             static_cast<int>(params.textDocument.version));
      })
      .onTextDocumentDidChange([this](auto &&params) {
        const std::string uri = params.textDocument.uri.toString();
        const int version = static_cast<int>(params.textDocument.version);

        // Full sync: the client sends the whole buffer in one change event.
        for (auto &change : params.contentChanges) {
          if (auto *whole =
                  std::get_if<::lsp::TextDocumentContentChangeWholeDocument>(
                      &change))
            change_document(uri, whole->text, version);
        }
      })
      .onTextDocumentDidClose([this](auto &&params) {
        close(params.textDocument.uri.toString());
      })
      .onTextDocumentDefinition(
          [this](::lsp::DefinitionParams &&params)
              -> ::lsp::RequestResult<::lsp::TextDocumentDefinitionResult> {
            return definition(std::move(params));
          })
      .onTextDocumentCompletion(
          [this](::lsp::CompletionParams &&params)
              -> ::lsp::RequestResult<::lsp::TextDocumentCompletionResult> {
            return completion(std::move(params));
          })
      .onShutdown([]() -> ::lsp::ShutdownResult { return {}; })
      .onExit([]() {});
}

} // namespace nbl::lsp

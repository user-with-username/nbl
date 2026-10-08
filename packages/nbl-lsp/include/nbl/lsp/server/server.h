#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include <lsp/io/stream.h>
#include <lsp/messages.h>
#include <lsp/request_result.h>
#include <lsp/server_endpoint.h>
#include <lsp/types.h>
#include <lsp/uri.h>

#include "nbl/analysis/module_kind.h"
#include "nbl/analysis/require_index.h"
#include "nbl/lsp/checker/checker.h"
#include "nbl/lsp/checker/diagnostics_by_module.h"
#include "nbl/lsp/server/server_options.h"
#include "nbl/lsp/worker/worker_pool.h"
#include "nbl/utils/sources.h"

namespace nbl::lsp {

/// The language server. Owns the LSP endpoint, the worker pool with one
/// `Checker` per worker, and the state of open documents.
///
/// The implementation is split by concern (src/server/):
///   server.cpp                    - construction, run loop, logging
///   handlers.cpp                  - wiring LSP callbacks to members
///   initialize.cpp                - `initialize`
///   documents.cpp                 - open / change / close, snapshots, URIs
///   index.cpp                     - require index, module classification
///   diagnostics.cpp               - scheduling checks, publishing results
///   requests/definition_request.cpp
///   requests/completion_request.cpp
class Server {
public:
  Server(::lsp::io::Stream &io, ServerOptions options);
  ~Server();

  void run();

private:
  struct Document {
    std::string text;
    int version = 0;
  };

  // server.cpp
  void start_workers();
  void log(::lsp::MessageType type, const std::string &message);

  // handlers.cpp
  void install_handlers();

  // initialize.cpp
  ::lsp::InitializeResult initialize(const ::lsp::InitializeParams &params);

  // documents.cpp
  void open(std::string uri, std::string text, int version);
  void change_document(const std::string &uri, const std::string &text,
                       int version);
  void close(const std::string &uri);
  /// Re-indexes `uri` (if it is a file) using the open-document overlay.
  void index_document(const std::string &uri);
  nbl::utils::OverlaySources::Files snapshot() const;
  std::optional<Document> document(const std::string &uri) const;
  std::string uri_for(const std::string &path) const;

  // index.cpp
  void start_index_build();
  void on_watched_files_changed(
      const ::lsp::DidChangeWatchedFilesParams &params);
  nbl::analysis::ModuleKind module_kind(const std::string &path,
                                        std::string_view text) const;

  // diagnostics.cpp
  void schedule_check(const std::string &uri);
  void recheck_all_open();
  void publish(const std::string &uri, int version,
               const DiagnosticsByModule &diagnostics,
               const nbl::utils::OverlaySources::Files &files);
  void send_diagnostics(const std::string &module,
                        std::vector<::lsp::Diagnostic> diagnostics);

  // requests/
  ::lsp::RequestResult<::lsp::TextDocumentDefinitionResult>
  definition(::lsp::DefinitionParams &&params);
  ::lsp::RequestResult<::lsp::TextDocumentCompletionResult>
  completion(::lsp::CompletionParams &&params);

  ::lsp::ServerEndpoint endpoint_;
  ServerOptions options_;

  std::unique_ptr<WorkerPool> workers_;
  std::vector<std::unique_ptr<Checker>> checkers_;
  std::optional<std::string> load_error_;

  std::string workspace_root_;
  bool index_ready_ = false;
  nbl::analysis::RequireIndex require_index_;

  mutable std::mutex mutex_;
  std::map<std::string, Document> documents_;
  std::map<std::string, std::set<std::string>> published_;
};

} // namespace nbl::lsp

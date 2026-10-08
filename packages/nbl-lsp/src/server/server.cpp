#include "nbl/lsp/server/server.h"

#include <algorithm>
#include <thread>
#include <utility>

namespace nbl::lsp {

Server::Server(::lsp::io::Stream &io, ServerOptions options)
    : endpoint_(io), options_(std::move(options)) {
  start_workers();
  install_handlers();
}

Server::~Server() = default;

void Server::run() { endpoint_.runMessageLoop(); }

void Server::start_workers() {
  unsigned count = options_.workers;
  if (count == 0) {
    const unsigned hardware = std::thread::hardware_concurrency();
    count = std::clamp(hardware > 1 ? hardware - 1 : 1u, 1u, 4u);
  }

  workers_ = std::make_unique<WorkerPool>(count);
  for (unsigned i = 0; i < workers_->size(); ++i) {
    checkers_.push_back(std::make_unique<Checker>(options_.definitions,
                                                  options_.definitions_path));
    if (!load_error_)
      load_error_ = checkers_.back()->load();
  }
}

void Server::log(::lsp::MessageType type, const std::string &message) {
  endpoint_.windowLogMessage(::lsp::LogMessageParams{
      .type = type,
      .message = "nbl-lsp: " + message,
  });
}

} // namespace nbl::lsp

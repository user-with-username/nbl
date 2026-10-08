#include "nbl/lsp/server/server.h"

#include <utility>

#include "nbl/lsp/server/capabilities.h"
#include "nbl/lsp/server/workspace_root.h"

namespace nbl::lsp {

::lsp::InitializeResult
Server::initialize(const ::lsp::InitializeParams &params) {
  if (auto root = workspace_root_from(params))
    workspace_root_ = std::move(*root);

  return make_initialize_result();
}

} // namespace nbl::lsp

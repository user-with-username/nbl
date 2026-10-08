#pragma once

#include <lsp/messages.h>

namespace nbl::lsp {

/// The `initialize` response: what this server can do.
::lsp::InitializeResult make_initialize_result();

} // namespace nbl::lsp

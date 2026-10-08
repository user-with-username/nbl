#pragma once

#include <optional>
#include <string>

#include <lsp/types.h>

namespace nbl::lsp {

/// Canonical workspace root from `initialize` params: the first workspace
/// folder, else `rootUri`. `nullopt` when neither is a `file:` URI.
std::optional<std::string>
workspace_root_from(const ::lsp::InitializeParams &params);

} // namespace nbl::lsp

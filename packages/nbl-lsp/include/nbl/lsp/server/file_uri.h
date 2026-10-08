#pragma once

#include <optional>
#include <string>

namespace nbl::lsp {

/// Canonical filesystem path of `uri`, or `nullopt` if it is not a `file:` URI.
std::optional<std::string> file_path(const std::string &uri);

} // namespace nbl::lsp

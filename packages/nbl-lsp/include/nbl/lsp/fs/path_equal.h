#pragma once

#include <string>

namespace nbl::lsp {

/// Case-insensitive equality of two canonical paths. On Windows LSP URIs
/// arrive as `c%3A/...` and `canonicalize()` normalizes the drive letter
/// differently, so a plain `==` is not reliable.
bool path_equal(const std::string &a, const std::string &b);

} // namespace nbl::lsp

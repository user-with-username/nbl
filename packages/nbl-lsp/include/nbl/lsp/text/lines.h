#pragma once

#include <cstddef>
#include <string_view>

namespace nbl::lsp {

/// Byte offset of the beginning of `line` (zero-based).
size_t line_begin(std::string_view text, unsigned line);

/// Contents of `line` without the trailing newline.
std::string_view line_of(std::string_view text, unsigned line);

} // namespace nbl::lsp

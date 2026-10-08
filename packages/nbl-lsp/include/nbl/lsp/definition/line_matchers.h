#pragma once

#include <string_view>

namespace nbl::lsp {

/// True if the line is a `declare name ...`, `declare function name ...`,
/// or `declare extern type name ...`.
bool declares(std::string_view line, std::string_view name);

/// True if the line declares a field `name: ...` (used inside
/// `declare extern type` bodies).
bool is_property(std::string_view line, std::string_view name);

/// True if the line is a global definition (`function name(...)`,
/// `name = ...`, or `name(...)`) and not a comment.
bool defines_global(std::string_view line, std::string_view name);

} // namespace nbl::lsp

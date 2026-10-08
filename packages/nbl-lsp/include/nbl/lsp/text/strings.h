#pragma once

#include <string_view>

namespace nbl::lsp {

/// True for `[A-Za-z0-9_]`.
bool is_identifier_char(char c);

/// Trims ASCII whitespace from both ends.
std::string_view trim(std::string_view text);

/// True when `text` begins with `word` followed by a non-identifier char
/// (or `word` is the whole string).
bool starts_with_word(std::string_view text, std::string_view word);

} // namespace nbl::lsp

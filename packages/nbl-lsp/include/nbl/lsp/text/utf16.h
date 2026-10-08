#pragma once

#include <cstddef>
#include <string_view>

namespace nbl::lsp {

/// One step through UTF-8 text: how many bytes the sequence that starts with
/// a given lead byte occupies, and how many UTF-16 code units it yields.
/// Invalid lead bytes count as a single byte / single unit.
struct Utf8Step {
  size_t bytes;
  unsigned utf16_units;
};

Utf8Step utf8_step(unsigned char lead_byte);

/// Number of UTF-16 code units in `text`.
unsigned utf16_units(std::string_view text);

} // namespace nbl::lsp

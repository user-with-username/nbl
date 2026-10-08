#include "nbl/lsp/text/utf16.h"

namespace nbl::lsp {

Utf8Step utf8_step(unsigned char lead_byte) {
  if (lead_byte < 0x80)
    return {1, 1};
  if ((lead_byte >> 5) == 0x6)
    return {2, 1};
  if ((lead_byte >> 4) == 0xE)
    return {3, 1};
  if ((lead_byte >> 3) == 0x1E)
    return {4, 2}; // surrogate pair
  return {1, 1};
}

unsigned utf16_units(std::string_view text) {
  unsigned units = 0;

  for (size_t i = 0; i < text.size();) {
    const Utf8Step step = utf8_step(static_cast<unsigned char>(text[i]));
    units += step.utf16_units;
    i += step.bytes;
  }

  return units;
}

} // namespace nbl::lsp

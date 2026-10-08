#include "nbl/lsp/text/positions.h"

#include <algorithm>

#include "nbl/lsp/text/lines.h"
#include "nbl/lsp/text/utf16.h"

namespace nbl::lsp {

::lsp::Position to_position(std::string_view text, unsigned line,
                            size_t byte_column) {
  const std::string_view content = line_of(text, line);
  const size_t clamped = std::min(byte_column, content.size());

  return ::lsp::Position{
      static_cast<unsigned>(line),
      static_cast<unsigned>(utf16_units(content.substr(0, clamped)))};
}

::lsp::Position to_position(std::string_view text,
                            const Luau::Position &position) {
  return to_position(text, position.line, position.column);
}

::lsp::Range to_range(std::string_view text, const Luau::Location &location) {
  return ::lsp::Range{to_position(text, location.begin),
                      to_position(text, location.end)};
}

size_t to_byte_column(std::string_view text, unsigned line, unsigned character) {
  const std::string_view content = line_of(text, line);

  unsigned units = 0;
  for (size_t i = 0; i < content.size();) {
    if (units >= character)
      return i;

    const Utf8Step step = utf8_step(static_cast<unsigned char>(content[i]));
    units += step.utf16_units;
    i += step.bytes;
  }

  return content.size();
}

} // namespace nbl::lsp

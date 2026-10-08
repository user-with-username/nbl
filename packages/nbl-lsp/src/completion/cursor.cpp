#include "nbl/lsp/completion/cursor.h"

#include <algorithm>

#include "nbl/lsp/text/lines.h"
#include "nbl/lsp/text/positions.h"
#include "nbl/lsp/text/strings.h"
#include "nbl/lsp/text/utf16.h"

namespace nbl::lsp {

namespace {

/// Byte offset where the identifier ending at `column` begins.
size_t identifier_prefix_begin(std::string_view line, size_t column) {
  size_t begin = std::min<size_t>(column, line.size());
  while (begin > 0 && is_identifier_char(line[begin - 1]))
    --begin;
  return begin;
}

} // namespace

CompletionCursor make_completion_cursor(std::string_view text, unsigned line,
                                        unsigned character) {
  const unsigned byte_col =
      static_cast<unsigned>(to_byte_column(text, line, character));

  const std::string_view line_view = line_of(text, line);
  const size_t prefix_begin = identifier_prefix_begin(line_view, byte_col);

  const unsigned start_utf16 = static_cast<unsigned>(
      utf16_units(line_view.substr(0, prefix_begin)));

  return CompletionCursor{
      Luau::Position{line, byte_col},
      ::lsp::Range{
          ::lsp::Position{line, start_utf16},
          ::lsp::Position{line, character},
      },
  };
}

} // namespace nbl::lsp

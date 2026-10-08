#include "nbl/lsp/text/lines.h"

namespace nbl::lsp {

size_t line_begin(std::string_view text, unsigned line) {
  size_t offset = 0;

  for (unsigned current = 0; current < line && offset < text.size(); ++current) {
    const size_t next = text.find('\n', offset);
    if (next == std::string_view::npos)
      return text.size();

    offset = next + 1;
  }

  return offset;
}

std::string_view line_of(std::string_view text, unsigned line) {
  const size_t begin = line_begin(text, line);
  const size_t end = text.find('\n', begin);

  return text.substr(begin, end == std::string_view::npos ? text.size() - begin
                                                         : end - begin);
}

} // namespace nbl::lsp

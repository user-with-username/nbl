#include "nbl/lsp/text/strings.h"

#include <cctype>

namespace nbl::lsp {

bool is_identifier_char(char c) {
  return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

std::string_view trim(std::string_view text) {
  size_t begin = 0;
  while (begin < text.size() &&
         std::isspace(static_cast<unsigned char>(text[begin])))
    ++begin;

  size_t end = text.size();
  while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1])))
    --end;

  return text.substr(begin, end - begin);
}

bool starts_with_word(std::string_view text, std::string_view word) {
  if (text.size() < word.size() || text.compare(0, word.size(), word) != 0)
    return false;

  return text.size() == word.size() || !is_identifier_char(text[word.size()]);
}

} // namespace nbl::lsp

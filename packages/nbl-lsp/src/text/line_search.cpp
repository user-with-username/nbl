#include "nbl/lsp/text/line_search.h"

#include <sstream>
#include <string>

namespace nbl::lsp {

std::optional<Luau::Location> find_line(
    std::string_view text,
    const std::function<bool(std::string_view)> &match) {
  std::istringstream stream{std::string(text)};
  std::string line;
  unsigned int number = 0;
  while (std::getline(stream, line)) {
    if (match(line)) {
      const Luau::Position position{number, 0};
      return Luau::Location{position, position};
    }
    ++number;
  }
  return std::nullopt;
}

size_t count_matches(std::string_view text,
                     const std::function<bool(std::string_view)> &match) {
  std::istringstream stream{std::string(text)};
  std::string line;
  size_t count = 0;
  while (std::getline(stream, line))
    if (match(line))
      ++count;
  return count;
}

} // namespace nbl::lsp

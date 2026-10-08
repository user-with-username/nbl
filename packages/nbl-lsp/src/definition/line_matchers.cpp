#include "nbl/lsp/definition/line_matchers.h"

#include <cstring>

#include "nbl/lsp/text/strings.h"

namespace nbl::lsp {

bool declares(std::string_view line, std::string_view name) {
  constexpr std::string_view kDeclare = "declare";

  line = trim(line);
  if (line.rfind(kDeclare, 0) != 0)
    return false;

  line = trim(line.substr(kDeclare.size()));
  if (line.rfind("function", 0) == 0)
    line = trim(line.substr(std::strlen("function")));
  else if (line.rfind("extern", 0) == 0) {
    line = trim(line.substr(std::strlen("extern")));
    if (line.rfind("type", 0) == 0)
      line = trim(line.substr(std::strlen("type")));
  }

  return starts_with_word(line, name);
}

bool is_property(std::string_view line, std::string_view name) {
  line = trim(line);
  if (!starts_with_word(line, name))
    return false;

  line = trim(line.substr(name.size()));
  return !line.empty() && line.front() == ':';
}

bool defines_global(std::string_view line, std::string_view name) {
  line = trim(line);
  if (!line.empty() && line.front() == '-' && line.size() > 1 &&
      line[1] == '-')
    return false;

  if (line.rfind("function", 0) == 0)
    return starts_with_word(trim(line.substr(std::strlen("function"))), name);

  if (!starts_with_word(line, name))
    return false;

  line = trim(line.substr(name.size()));
  return !line.empty() && (line.front() == '=' || line.front() == '(');
}

} // namespace nbl::lsp

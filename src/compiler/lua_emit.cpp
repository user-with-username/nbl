#include "compiler/lua_emit.h"

#include <sstream>

namespace compiler {

const char *const PRELUDE = R"(local __modules = {}
local __loaded = {}
local function __require(id)
    local cached = __loaded[id]
    if cached ~= nil then return cached end
    local loader = __modules[id]
    if not loader then error('module not found: ' .. tostring(id)) end
    local result = loader()
    __loaded[id] = result
    return result
end
)";

const char *const BUNDLE_HEADER = "-- bundled by NBL. Do not edit by paws.\n";

std::string quote(const std::string &s) {
  std::string out = "\"";
  for (char c : s) {
    if (c == '\\' || c == '"')
      out += '\\';
    out += c;
  }
  out += '"';
  return out;
}

void indent(std::ostream &out, const std::string &body,
            const std::string &pad) {
  std::istringstream ss(body);
  std::string line;
  while (std::getline(ss, line))
    out << pad << line << "\n";
}

void emit_module(std::ostream &out, const std::string &id,
                 const std::string &body) {
  out << "__modules[" << quote(id) << "] = function()\n";
  out << "    local require = __require\n";
  indent(out, body, "    ");
  out << "end\n\n";
}

void emit_entry(std::ostream &out, const std::string &body) {
  out << "local require = __require\n";
  out << body << "\n";
}

} // namespace compiler

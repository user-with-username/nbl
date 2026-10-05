#include "nbl/compiler/lua_emit.h"

#include <sstream>

namespace nbl::compiler {

const char *const PRELUDE = R"(local __modules = {}
local __loaded = {}
local __done = {}
local __loading = {}
local function __require(id)
    if __done[id] then return __loaded[id] end
    if __loading[id] then error('circular require: ' .. tostring(id)) end
    local loader = __modules[id]
    if not loader then error('module not found: ' .. tostring(id)) end
    __loading[id] = true
    local ok, result = pcall(loader)
    __loading[id] = nil
    if not ok then error(result, 0) end
    __loaded[id] = result
    __done[id] = true
    return result
end
)";

const char *const BUNDLE_HEADER = "-- bundled by NBL. Do not edit.\n";

std::string quote(const std::string &s) {
  std::string out = "\"";
  out.reserve(s.size() + 2);
  for (char c : s) {
    switch (c) {
    case '\\': out += "\\\\"; break;
    case '"':  out += "\\\""; break;
    case '\n': out += "\\n";  break;
    case '\r': out += "\\r";  break;
    case '\t': out += "\\t";  break;
    default:   out += c;      break;
    }
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

} // namespace nbl::compiler
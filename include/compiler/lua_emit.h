#pragma once

#include <ostream>
#include <string>

namespace compiler {

extern const char* const PRELUDE;
extern const char* const BUNDLE_HEADER;

// wrap a string in double quotes, escaping `\` and `"`
std::string quote(const std::string& s);

// write `body` to `out`, prefixing every line with `pad`
void indent(std::ostream& out, const std::string& body, const std::string& pad);

// emit `__modules["<id>"] = function() ... end`
void emit_module(std::ostream& out, const std::string& id,
                 const std::string& body);

// emit the top-level entry chunk
void emit_entry(std::ostream& out, const std::string& body);

} // namespace compiler
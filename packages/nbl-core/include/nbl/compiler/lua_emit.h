#pragma once

#include <ostream>
#include <string>

namespace nbl::compiler {

extern const char *const PRELUDE;
extern const char *const BUNDLE_HEADER;

std::string quote(const std::string &s);

void indent(std::ostream &out, const std::string &body,
            const std::string &pad);

void emit_module(std::ostream &out, const std::string &id,
                 const std::string &body);

void emit_entry(std::ostream &out, const std::string &body);

} // namespace nbl::compiler
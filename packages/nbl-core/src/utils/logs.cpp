#include "nbl/utils/logs.h"

#include <iostream>

#include "Luau/Frontend.h"
#include "Luau/ToString.h"

namespace nbl::utils {

namespace {

constexpr const char *kErrorColor = "\x1b[38;2;244;113;116m";
constexpr const char *kWarningColor = "\x1b[38;2;250;204;21m";
constexpr const char *kInfoColor = "\x1b[38;2;120;180;255m";
constexpr const char *kResetColor = "\x1b[0m";

bool is_default_location(const Luau::Location &loc) {
  return loc.begin.line == 0 && loc.begin.column == 0 &&
         loc.end.line == 0 && loc.end.column == 0;
}

} // namespace

std::string format_location(const std::string &module_name,
                            const Luau::Location &location) {
  if (is_default_location(location))
    return module_name;

  std::string text = module_name + ":" +
                     std::to_string(location.begin.line + 1) + ":" +
                     std::to_string(location.begin.column + 1);

  if (location.end.line != location.begin.line) {
    text += "-" + std::to_string(location.end.line + 1) + ":" +
            std::to_string(location.end.column + 1);
  } else if (location.end.column != location.begin.column) {
    text += "-" + std::to_string(location.end.column + 1);
  }

  return text;
}

Diagnostics::Diagnostics() : out_(std::cout) {}
Diagnostics::Diagnostics(std::ostream &out) : out_(out) {}

void Diagnostics::error(const std::string &message) {
  out_ << kErrorColor << "error:" << kResetColor << ' ' << message << '\n';
  ++error_count;
}

void Diagnostics::warning(const std::string &message) {
  out_ << kWarningColor << "warning:" << kResetColor << ' ' << message << '\n';
  ++warning_count;
}

void Diagnostics::info(const std::string &message) {
  out_ << kInfoColor << "info:" << kResetColor << ' ' << message << '\n';
}

void Diagnostics::add_definition(const Luau::LoadDefinitionFileResult &result,
                                 const std::string &types_name) {
  const int before = error_count;

  for (const Luau::ParseError &parse_error : result.parseResult.errors)
    error(format_location(types_name, parse_error.getLocation()) + ": " +
          parse_error.getMessage());

  if (result.module)
    for (const Luau::TypeError &type_error : result.module->errors)
      error(format_location(types_name, type_error.location) + ": " +
            Luau::toString(type_error));

  if (error_count == before)
    error(types_name + ": type definitions could not be loaded");
}

void Diagnostics::print_summary() const {
  if (error_count + warning_count > 0)
    out_ << '\n';
  out_ << error_count << " error(s), " << warning_count << " warning(s)\n";
}

bool Diagnostics::has_errors() const { return error_count != 0; }

} // namespace nbl::utils
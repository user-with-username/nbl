#include "nbl/utils/logs.h"

#include <iostream>

#include "Luau/Frontend.h"
#include "Luau/Linter.h"
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

bool is_entrypoint_unused(const Luau::LintWarning &lint) {
  return lint.code == Luau::LintWarning::Code_FunctionUnused &&
         lint.text.find("'tick'") != std::string::npos;
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

void Diagnostics::add(const Luau::CheckResult &result,
                      const std::string &module_name) {
  for (const Luau::TypeError &type_error : result.errors)
    error(format_location(type_error.moduleName, type_error.location) + ": " +
          Luau::toString(type_error));

  for (const Luau::LintWarning &lint : result.lintResult.errors)
    if (!is_entrypoint_unused(lint))
      error(format_location(module_name, lint.location) + ": " + lint.text);

  for (const Luau::LintWarning &lint : result.lintResult.warnings)
    if (!is_entrypoint_unused(lint))
      warning(format_location(module_name, lint.location) + ": " + lint.text);
}

void Diagnostics::print_summary() const {
  if (error_count + warning_count > 0)
    out_ << '\n';
  out_ << error_count << " error(s), " << warning_count << " warning(s)\n";
}

bool Diagnostics::has_errors() const { return error_count != 0; }

} // namespace nbl::utils
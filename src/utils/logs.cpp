#include "utils/logs.h"

#include <iostream>

#include "Luau/Linter.h"
#include "Luau/ToString.h"

static constexpr const char *error_color = "\x1b[38;2;244;113;116m";
static constexpr const char *warning_color = "\x1b[38;2;250;204;21m";
static constexpr const char *reset_color = "\x1b[0m";

std::string format_location(const std::string &module_name,
                            const Luau::Location &location) {
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

// `tick` is the entrypoint func. luau's typechecker will always throw warn
// about the unused function. so we need to shut him up
static bool is_entrypoint_unused(const Luau::LintWarning &lint) {
  return lint.code == Luau::LintWarning::Code_FunctionUnused &&
         lint.text.find("'tick'") != std::string::npos;
}

void Diagnostics::error(const std::string &message) {
  std::cout << error_color << "error:" << reset_color << ' ' << message << '\n';
  ++error_count;
}

void Diagnostics::warning(const std::string &message) {
  std::cout << warning_color << "warning:" << reset_color << ' ' << message
            << '\n';
  ++warning_count;
}

void Diagnostics::add(const Luau::CheckResult &result,
                      const std::string &script) {
  for (const Luau::TypeError &type_error : result.errors) {
    error(format_location(type_error.moduleName, type_error.location) + ": " +
          Luau::toString(type_error));
  }

  for (const Luau::LintWarning &lint : result.lintResult.errors)
    if (!is_entrypoint_unused(lint))
      error(format_location(script, lint.location) + ": " + lint.text);

  for (const Luau::LintWarning &lint : result.lintResult.warnings)
    if (!is_entrypoint_unused(lint))
      warning(format_location(script, lint.location) + ": " + lint.text);
}

void Diagnostics::print_summary() const {
  if (error_count + warning_count > 0)
    std::cout << '\n';
  std::cout << error_count << " error(s), " << warning_count << " warning(s)\n";
}

bool Diagnostics::has_errors() const { return error_count != 0; }

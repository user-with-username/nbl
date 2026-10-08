#include "nbl/analysis/checker.h"

#include <algorithm>
#include <string>
#include <vector>

#include "Luau/Frontend.h"
#include "Luau/Linter.h"

#include "nbl/analysis/dep_graph.h"
#include "nbl/utils/files.h"

namespace nbl::analysis {

namespace {

/// `tick()` is called by the game, so Luau's "unused function" warning about it
/// is always wrong.
bool is_entrypoint_unused(const Luau::LintWarning &lint) {
  return lint.code == Luau::LintWarning::Code_FunctionUnused &&
         lint.text.find("'tick'") != std::string::npos;
}

std::vector<std::string> findTickModules(const DepGraph &graph) {
  std::vector<std::string> result;
  for (const auto &[name, node] : graph.nodes())
    if (node.tick.has_tick)
      result.push_back(name);
  std::sort(result.begin(), result.end());
  return result;
}

void addModuleDiagnostics(LintResult &result, const Luau::CheckResult &check,
                          const std::string &module) {
  for (const Luau::TypeError &type_error : check.errors)
    result.diagnostics.push_back({Severity::Error, type_error.moduleName,
                                  type_error.location,
                                  Luau::toString(type_error)});

  for (const Luau::LintWarning &lint : check.lintResult.errors)
    if (!is_entrypoint_unused(lint))
      result.diagnostics.push_back(
          {Severity::Error, module, lint.location, lint.text});

  for (const Luau::LintWarning &lint : check.lintResult.warnings)
    if (!is_entrypoint_unused(lint))
      result.diagnostics.push_back(
          {Severity::Warning, module, lint.location, lint.text});
}

} // namespace

bool LintResult::has_errors() const {
  return std::any_of(diagnostics.begin(), diagnostics.end(),
                     [](const Diagnostic &diagnostic) {
                       return diagnostic.severity == Severity::Error;
                     });
}

LintResult lint(Luau::Frontend &frontend, const std::string &script,
                bool require_entrypoint,
                const nbl::utils::SourceProvider &sources) {
  LintResult result;

  if (nbl::utils::is_bundle_file(script)) {
    result.diagnostics.push_back(
        {Severity::Error, script, {},
         "refusing to type-check a .bundle.luau file"});
    return result;
  }

  result.graph.build(script, sources);

  for (const DepError &error : result.graph.errors())
    result.diagnostics.push_back(
        {Severity::Error, error.module, error.loc, error.message});

  if (!result.graph.errors().empty())
    return result;

  const std::vector<std::string> tick_modules = findTickModules(result.graph);

  if (require_entrypoint && tick_modules.empty()) {
    result.diagnostics.push_back(
        {Severity::Error, script, {},
         "no module defines entrypoint `function tick()`"});
    return result;
  }

  if (tick_modules.size() > 1) {
    std::string message = "multiple modules define `function tick()`:";
    for (const std::string &module : tick_modules)
      message += "\n  " + module;

    result.diagnostics.push_back(
        {Severity::Error, script, {}, std::move(message)});
    return result;
  }

  // Every module in the graph is (re)checked from its current source, so the
  // frontend has to forget what it cached for them.
  for (const auto &[name, node] : result.graph.nodes())
    frontend.markDirty(name);

  for (const std::string &name : result.graph.topo_order())
    addModuleDiagnostics(result, frontend.check(name), name);

  return result;
}

LintResult lint(Luau::Frontend &frontend, const std::string &script,
                bool require_entrypoint) {
  static const nbl::utils::FileSystemSources kFiles;
  return lint(frontend, script, require_entrypoint, kFiles);
}

} // namespace nbl::analysis

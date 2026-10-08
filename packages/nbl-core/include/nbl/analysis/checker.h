#pragma once

#include <string>
#include <vector>

#include "Luau/Location.h"

#include "nbl/analysis/dep_graph.h"
#include "nbl/utils/sources.h"

namespace Luau {
class Frontend;
}

namespace nbl::analysis {

enum class Severity { Error, Warning };

/// One problem found while checking a module graph, as data: the CLI prints it,
/// the language server turns it into an LSP diagnostic.
struct Diagnostic {
  Severity severity = Severity::Error;
  /// Module (file) the diagnostic belongs to.
  std::string module;
  /// `Location{}` means the whole module.
  Luau::Location location;
  std::string message;
};

struct LintResult {
  DepGraph graph;
  std::vector<Diagnostic> diagnostics;

  bool has_errors() const;
};

/// Builds the module graph rooted at `script` and type-checks every module in
/// it, collecting the dependency, `tick()` and Luau diagnostics.
///
/// With `require_entrypoint` the graph must also define exactly one
/// `function tick()`: the CLI checks complete scripts, while an editor also
/// checks library modules that are only required by someone else.
LintResult lint(Luau::Frontend &frontend, const std::string &script,
                bool require_entrypoint,
                const nbl::utils::SourceProvider &sources);

/// Same as above, reading the modules from disk.
LintResult lint(Luau::Frontend &frontend, const std::string &script,
                bool require_entrypoint);

} // namespace nbl::analysis

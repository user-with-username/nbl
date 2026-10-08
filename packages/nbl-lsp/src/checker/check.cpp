#include "nbl/lsp/checker/checker.h"

#include "Luau/Frontend.h"

namespace nbl::lsp {

DiagnosticsByModule Checker::check(const std::string &path,
                                   const nbl::utils::SourceProvider &sources,
                                   nbl::analysis::ModuleKind kind) {
  DiagnosticsByModule grouped;

  // definitions.d.luau never goes through the regular lint path; its errors
  // were collected by `load()` from `loadDefinitionFile`.
  if (is_definitions(path)) {
    if (!definitions_diagnostics_.empty())
      grouped[definitions_path_] = definitions_diagnostics_;
    return grouped;
  }

  if (!frontend_)
    return grouped;

  sources_.set(sources);

  const bool require_entrypoint =
      kind == nbl::analysis::ModuleKind::Entrypoint;

  nbl::analysis::LintResult result =
      nbl::analysis::lint(*frontend_, path, require_entrypoint, sources);

  for (const nbl::analysis::Diagnostic &diagnostic : result.diagnostics)
    grouped[diagnostic.module].push_back(diagnostic);

  return grouped;
}

} // namespace nbl::lsp

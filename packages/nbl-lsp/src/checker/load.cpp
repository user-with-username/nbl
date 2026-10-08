#include "nbl/lsp/checker/checker.h"

#include <sstream>

#include "Luau/Frontend.h"
#include "Luau/ToString.h"

namespace nbl::lsp {

std::optional<std::string> Checker::load() {
  definitions_diagnostics_.clear();
  frontend_ = std::make_unique<Luau::Frontend>(&files_, &config_);

  // (1) globals for diagnostics.
  Luau::LoadDefinitionFileResult loaded = frontend_->loadDefinitionFile(
      frontend_->globals, frontend_->globals.globalScope, definitions_,
      definitions_path_, /*captureComments*/ false,
      /*typeCheckForAutocomplete*/ false);

  // Collect parse and type errors as diagnostics on the definitions file
  // itself. `loadDefinitionFile` runs the parser in .d.luau mode, so
  // `declare` / `extern type ... with ... end` are valid syntax here.
  for (const Luau::ParseError &error : loaded.parseResult.errors) {
    definitions_diagnostics_.push_back(nbl::analysis::Diagnostic{
        nbl::analysis::Severity::Error,
        definitions_path_,
        error.getLocation(),
        error.getMessage(),
    });
  }

  if (!loaded.success) {
    std::ostringstream reason;
    reason << definitions_path_ << ": definitions failed to load";
    return reason.str();
  }

  if (loaded.module) {
    for (const Luau::TypeError &error : loaded.module->errors) {
      definitions_diagnostics_.push_back(nbl::analysis::Diagnostic{
          nbl::analysis::Severity::Error,
          definitions_path_,
          error.location,
          Luau::toString(error),
      });
    }
  }

  // (2) Separate globals for autocomplete. In solver v1
  // `Luau::autocomplete` reads `globalsForAutocomplete`, not `globals`.
  (void)frontend_->loadDefinitionFile(
      frontend_->globalsForAutocomplete,
      frontend_->globalsForAutocomplete.globalScope, definitions_,
      definitions_path_, /*captureComments*/ false,
      /*typeCheckForAutocomplete*/ true);

  return std::nullopt;
}

} // namespace nbl::lsp

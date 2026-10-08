#include "nbl/lsp/text/lsp_diagnostic.h"

#include "nbl/lsp/text/positions.h"

namespace nbl::lsp {

::lsp::Diagnostic to_diagnostic(const nbl::analysis::Diagnostic &diagnostic,
                                std::string_view text) {
  ::lsp::Diagnostic result;
  result.range = to_range(text, diagnostic.location);
  result.severity = diagnostic.severity == nbl::analysis::Severity::Error
                        ? ::lsp::DiagnosticSeverity::Error
                        : ::lsp::DiagnosticSeverity::Warning;
  result.message = diagnostic.message;
  result.source = "nbl";
  return result;
}

} // namespace nbl::lsp

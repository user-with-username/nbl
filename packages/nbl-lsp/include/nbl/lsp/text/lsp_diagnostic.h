#pragma once

#include <string_view>

#include "lsp/types.h"

#include "nbl/analysis/checker.h"

namespace nbl::lsp {

/// Converts a nbl-core diagnostic into an LSP diagnostic, using `text`
/// to resolve byte-based locations into UTF-16 positions.
::lsp::Diagnostic to_diagnostic(const nbl::analysis::Diagnostic &diagnostic,
                                std::string_view text);

} // namespace nbl::lsp

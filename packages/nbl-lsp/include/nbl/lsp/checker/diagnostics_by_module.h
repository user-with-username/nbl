#pragma once

#include <map>
#include <string>
#include <vector>

#include "nbl/analysis/checker.h"

namespace nbl::lsp {

/// Diagnostics grouped by the module (canonical path) they belong to.
using DiagnosticsByModule =
    std::map<std::string, std::vector<nbl::analysis::Diagnostic>>;

} // namespace nbl::lsp

#pragma once

#include <string>

#include "nbl/types/resolve.h"

namespace nbl::lsp::app {

/// Definitions that do not live in a single file on disk (the embedded ones, or
/// globals plus types downloaded into ~/.nbl) are written out, so that "go to
/// definition" can point at a real file. Returns an empty string and fills in
/// `reason` when that is not possible.
std::string materialize_definitions(const nbl::types::Resolved &resolved,
                                    std::string &reason);

} // namespace nbl::lsp::app

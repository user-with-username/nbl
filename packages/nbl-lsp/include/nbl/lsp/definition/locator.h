#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "Luau/Location.h"

#include "nbl/lsp/definition/definition.h"

namespace nbl::lsp {

/// Finds where the symbol under `position` in `module_text` is defined.
///
/// `module` is the canonical path of the document, `definitions` /
/// `definitions_path` describe the loaded `.d.luau` type definitions. Purely
/// syntactic: parses the document and searches text, no type checking.
std::optional<Definition>
locate_definition(const std::string &module, std::string_view module_text,
                  Luau::Position position, std::string_view definitions,
                  const std::string &definitions_path);

} // namespace nbl::lsp

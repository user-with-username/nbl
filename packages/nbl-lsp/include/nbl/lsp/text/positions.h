#pragma once

#include <cstddef>
#include <string_view>

#include "Luau/Location.h"

#include "lsp/types.h"

namespace nbl::lsp {

/// Converts a byte column on `line` into an LSP position (UTF-16 units).
::lsp::Position to_position(std::string_view text, unsigned line,
                            size_t byte_column);

/// Converts a Luau position into an LSP position.
::lsp::Position to_position(std::string_view text,
                            const Luau::Position &position);

/// Converts a Luau location into an LSP range.
::lsp::Range to_range(std::string_view text, const Luau::Location &location);

/// Converts an LSP UTF-16 column on `line` into a byte column.
size_t to_byte_column(std::string_view text, unsigned line, unsigned character);

} // namespace nbl::lsp

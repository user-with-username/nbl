#pragma once

#include <string_view>

#include "Luau/Location.h"

#include "lsp/types.h"

namespace nbl::lsp {

/// Where completion was requested, in both coordinate systems.
struct CompletionCursor {
  /// Cursor as a Luau position (byte column).
  Luau::Position position;
  /// LSP range covering the identifier prefix right before the cursor, so the
  /// popup can replace it (`foo.ba|` replaces `ba`).
  ::lsp::Range replace_range;
};

/// Builds the cursor for an LSP position (`line`, UTF-16 `character`).
CompletionCursor make_completion_cursor(std::string_view text, unsigned line,
                                        unsigned character);

} // namespace nbl::lsp

#pragma once

#include "Luau/Autocomplete.h"

namespace nbl::lsp {

/// Maps a Luau autocomplete category to an LSP `CompletionItemKind` number.
int to_completion_kind(Luau::AutocompleteEntryKind kind);

} // namespace nbl::lsp

#pragma once

#include <vector>

#include "lsp/types.h"

#include "nbl/lsp/completion/completion_item.h"

namespace nbl::lsp {

/// Turns checker completion items into protocol items. Every item replaces
/// `replace_range` with its insert text. Consumes `items`.
std::vector<::lsp::CompletionItem>
to_lsp_items(std::vector<CompletionItem> &&items,
             const ::lsp::Range &replace_range);

} // namespace nbl::lsp

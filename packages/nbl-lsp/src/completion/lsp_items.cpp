#include "nbl/lsp/completion/lsp_items.h"

#include <utility>

namespace nbl::lsp {

std::vector<::lsp::CompletionItem>
to_lsp_items(std::vector<CompletionItem> &&items,
             const ::lsp::Range &replace_range) {
  std::vector<::lsp::CompletionItem> lsp_items;
  lsp_items.reserve(items.size());

  for (auto &item : items) {
    ::lsp::CompletionItem lsp_item;
    lsp_item.label = std::move(item.label);
    lsp_item.kind = static_cast<::lsp::CompletionItemKind>(item.kind);
    if (!item.detail.empty())
      lsp_item.detail = std::move(item.detail);
    lsp_item.textEdit = ::lsp::TextEdit{
        .range = replace_range,
        .newText = std::move(item.insert_text),
    };
    lsp_items.push_back(std::move(lsp_item));
  }

  return lsp_items;
}

} // namespace nbl::lsp

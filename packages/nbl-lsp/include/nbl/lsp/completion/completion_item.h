#pragma once

#include <string>

namespace nbl::lsp {

/// One entry the completion popup shows. Kept deliberately minimal: the LSP
/// layer turns `kind` (a numeric LSP `CompletionItemKind`) into whatever
/// wrapper type the protocol generator expects.
struct CompletionItem {
  std::string label;
  std::string detail;      ///< human-readable type, e.g. "(DataModel) -> string"
  std::string insert_text; ///< defaults to `label`
  int kind = 1;            ///< LSP `CompletionItemKind`
};

} // namespace nbl::lsp

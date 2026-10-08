#include "nbl/lsp/completion/completion_kind.h"

namespace nbl::lsp {

int to_completion_kind(Luau::AutocompleteEntryKind kind) {
  switch (kind) {
  case Luau::AutocompleteEntryKind::Property:          return 10; // Property
  case Luau::AutocompleteEntryKind::Binding:           return 6;  // Variable
  case Luau::AutocompleteEntryKind::Keyword:           return 14; // Keyword
  case Luau::AutocompleteEntryKind::String:            return 15; // Snippet
  case Luau::AutocompleteEntryKind::Type:              return 25; // TypeParameter
  case Luau::AutocompleteEntryKind::Module:            return 9;  // Module
  case Luau::AutocompleteEntryKind::GeneratedFunction: return 3;  // Function
  case Luau::AutocompleteEntryKind::RequirePath:       return 17; // File
  case Luau::AutocompleteEntryKind::HotComment:        return 14; // Keyword
  }
  return 1; // Text
}

} // namespace nbl::lsp

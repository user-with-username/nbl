#include "nbl/lsp/checker/checker.h"

#include <iostream>
#include <utility>

#include "Luau/Autocomplete.h"
#include "Luau/Frontend.h"
#include "Luau/ToString.h"

#include "nbl/lsp/completion/completion_kind.h"
#include "nbl/utils/files.h"

namespace nbl::lsp {

std::vector<CompletionItem>
Checker::complete(const std::string &path, Luau::Position position,
                  const nbl::utils::SourceProvider &sources) {
  std::vector<CompletionItem> out;

  // Nothing to complete inside the definitions file itself.
  if (is_definitions(path))
    return out;

  if (!frontend_)
    return out;

  sources_.set(sources);
  const std::string module = nbl::utils::canonicalize(path);

  auto text = sources.read(module);
  if (!text)
    return out;

  // Reset the module cache so the Frontend re-reads the source from the
  // overlay.
  frontend_->markDirty(module);

  // (1) Regular check - fills `moduleResolver` (used for diagnostics).
  (void)frontend_->check(module);

  // (2) The critical step for solver v1: `check` with `forAutocomplete`
  // fills `moduleResolverForAutocomplete`, which is what
  // `Luau::autocomplete` reads in Old mode.
  Luau::FrontendOptions opts;
  opts.forAutocomplete = true;
  (void)frontend_->check(module, opts);

  try {
    Luau::AutocompleteResult result =
        Luau::autocomplete(*frontend_, module, position, /*callback*/ nullptr);

    out.reserve(result.entryMap.size());
    for (const auto &[name, entry] : result.entryMap) {
      CompletionItem item;
      item.label = name;
      item.insert_text = item.label;
      item.kind = to_completion_kind(entry.kind);
      if (entry.type)
        item.detail = Luau::toString(*entry.type);
      out.push_back(std::move(item));
    }
  } catch (const std::exception &e) {
    std::cerr << "nbl-lsp: autocomplete failed: " << e.what() << "\n";
  }

  return out;
}

} // namespace nbl::lsp

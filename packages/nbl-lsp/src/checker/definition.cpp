#include "nbl/lsp/checker/checker.h"

#include "nbl/lsp/definition/locator.h"
#include "nbl/utils/files.h"

namespace nbl::lsp {

std::optional<Definition>
Checker::definition(const std::string &path, Luau::Position position,
                    const nbl::utils::SourceProvider &sources) {
  // Nothing to go to inside the definitions file itself.
  if (is_definitions(path))
    return std::nullopt;

  sources_.set(sources);
  const std::string module = nbl::utils::canonicalize(path);

  auto module_text = sources.read(module);
  if (!module_text)
    return std::nullopt;

  return locate_definition(module, *module_text, position, definitions_,
                           definitions_path_);
}

} // namespace nbl::lsp

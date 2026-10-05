#include "nbl/types/resolve.h"

#include "nbl/types/dir.h"
#include "nbl/utils/files.h"

namespace nbl::types {

std::optional<Resolved> resolve(const std::string &explicit_path) {
  if (explicit_path == "embedded:") {
    return Resolved{Source::Embedded, std::string(embedded_types()), ""};
  }

  if (!explicit_path.empty()) {
    auto content = nbl::utils::read_file_opt(explicit_path);
    if (!content)
      return std::nullopt;
    return Resolved{Source::Explicit, std::move(*content), explicit_path};
  }

  if (auto dir = nbl_dir()) {
    auto path = (*dir / kTypesFilename).string();
    if (auto content = nbl::utils::read_file_opt(path))
      return Resolved{Source::Override, std::move(*content), path};
  }

  return Resolved{Source::Embedded, std::string(embedded_types()), ""};
}

std::string load(const std::string &explicit_path) {
  auto r = resolve(explicit_path);
  return r ? std::move(r->content) : std::string{};
}

} // namespace nbl::types
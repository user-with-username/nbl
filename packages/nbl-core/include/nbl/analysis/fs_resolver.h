#pragma once

#include <optional>
#include <string>

#include "Luau/FileResolver.h"

#include "nbl/utils/sources.h"

namespace nbl::analysis {

std::optional<std::string> validate_require_string(const std::string &req);

std::optional<std::string> resolve_lua_path(const std::string &from_module,
                                            const std::string &require_path);

class FsResolver : public Luau::FileResolver {
public:
  /// Reads module sources from disk.
  FsResolver() = default;

  /// Reads module sources through `sources`, which must outlive the resolver.
  explicit FsResolver(const nbl::utils::SourceProvider &sources)
      : sources_(&sources) {}

  std::optional<Luau::SourceCode>
  readSource(const Luau::ModuleName &name) override;

  std::optional<Luau::ModuleInfo>
  resolveModule(const Luau::ModuleInfo *context, Luau::AstExpr *node,
                const Luau::TypeCheckLimits &limits) override;

  std::string
  getHumanReadableModuleName(const Luau::ModuleName &name) const override;

  std::optional<std::string>
  getEnvironmentForModule(const Luau::ModuleName &name) const override;

private:
  const nbl::utils::SourceProvider *sources_ = nullptr;
};

} // namespace nbl::analysis
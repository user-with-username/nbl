#pragma once

#include <optional>
#include <string>

#include "Luau/FileResolver.h"

namespace nbl::analysis {

std::optional<std::string> validate_require_string(const std::string &req);

std::optional<std::string> resolve_lua_path(const std::string &from_module,
                                            const std::string &require_path);

class FsResolver : public Luau::FileResolver {
public:
  std::optional<Luau::SourceCode>
  readSource(const Luau::ModuleName &name) override;

  std::optional<Luau::ModuleInfo>
  resolveModule(const Luau::ModuleInfo *context, Luau::AstExpr *node,
                const Luau::TypeCheckLimits &limits) override;

  std::string
  getHumanReadableModuleName(const Luau::ModuleName &name) const override;

  std::optional<std::string>
  getEnvironmentForModule(const Luau::ModuleName &name) const override;
};

} // namespace nbl::analysis
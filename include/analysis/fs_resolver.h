#pragma once
#include <optional>
#include <string>
#include "Luau/FileResolver.h"

std::optional<std::string> resolve_lua_path(const std::string& from_module,
                                            const std::string& require_path);

std::optional<std::string> validate_require_string(const std::string& req);

struct FsResolver : Luau::FileResolver
{
    std::optional<Luau::SourceCode> readSource(const Luau::ModuleName& name) override;
    std::optional<Luau::ModuleInfo> resolveModule(
        const Luau::ModuleInfo*, Luau::AstExpr*, const Luau::TypeCheckLimits&) override;
    std::string getHumanReadableModuleName(const Luau::ModuleName& name) const override;
    std::optional<std::string> getEnvironmentForModule(const Luau::ModuleName&) const override;
};
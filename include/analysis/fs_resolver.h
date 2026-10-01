#pragma once
#include <optional>
#include <string>
#include "Luau/FileResolver.h"

struct FsResolver : Luau::FileResolver
{
    std::optional<Luau::SourceCode> readSource(const Luau::ModuleName& name) override;
    std::optional<Luau::ModuleInfo> resolveModule(
        const Luau::ModuleInfo*, Luau::AstExpr*, const Luau::TypeCheckLimits&) override;
    std::string getHumanReadableModuleName(const Luau::ModuleName& name) const override;
    std::optional<std::string> getEnvironmentForModule(const Luau::ModuleName&) const override;
};
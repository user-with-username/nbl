#include "analysis/fs_resolver.h"

#include <fstream>
#include <sstream>

std::optional<Luau::SourceCode> FsResolver::readSource(const Luau::ModuleName& name)
{
    std::ifstream file(name, std::ios::binary);
    if (!file)
        return std::nullopt;

    std::stringstream ss;
    ss << file.rdbuf();

    return Luau::SourceCode{ss.str(), Luau::SourceCode::Type::Module};
}

std::optional<Luau::ModuleInfo> FsResolver::resolveModule(
    const Luau::ModuleInfo*,
    Luau::AstExpr*,
    const Luau::TypeCheckLimits&)
{
    return std::nullopt;
}

std::string FsResolver::getHumanReadableModuleName(const Luau::ModuleName& name) const
{
    return name;
}

std::optional<std::string> FsResolver::getEnvironmentForModule(const Luau::ModuleName&) const
{
    return std::nullopt;
}
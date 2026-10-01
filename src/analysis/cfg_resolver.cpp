#include "analysis/cfg_resolver.h"

CfgResolver::CfgResolver()
{
    config.mode = Luau::Mode::Strict;
    config.enabledLint.setDefaults();
}

const Luau::Config& CfgResolver::getConfig(const Luau::ModuleName&, const Luau::TypeCheckLimits&) const
{
    return config;
}
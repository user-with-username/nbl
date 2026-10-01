#pragma once

#include "Luau/Config.h"
#include "Luau/ConfigResolver.h"
#include "Luau/TypeCheckLimits.h"

struct CfgResolver : Luau::ConfigResolver
{
    Luau::Config config;

    CfgResolver();

    const Luau::Config& getConfig(const Luau::ModuleName&, const Luau::TypeCheckLimits&) const override;
};
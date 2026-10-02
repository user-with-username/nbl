#include "analysis/cfg_resolver.h"
#include "utils/flags.h"

CfgResolver::CfgResolver() {
  setFlag("LuauSolverV2", true);
  config.mode = Luau::Mode::Strict;
  config.enabledLint.setDefaults();
}

const Luau::Config &
CfgResolver::getConfig(const Luau::ModuleName &,
                       const Luau::TypeCheckLimits &) const {
  return config;
}

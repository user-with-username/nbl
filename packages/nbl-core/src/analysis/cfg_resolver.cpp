#include "nbl/analysis/cfg_resolver.h"

#include "Luau/ParseOptions.h"

namespace nbl::analysis {

CfgResolver::CfgResolver() {
  config.mode = Luau::Mode::Strict;
  config.enabledLint.setDefaults();
}

const Luau::Config &
CfgResolver::getConfig(const Luau::ModuleName &,
                       const Luau::TypeCheckLimits &) const {
  return config;
}

} // namespace nbl::analysis
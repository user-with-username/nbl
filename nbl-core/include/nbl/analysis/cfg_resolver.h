#pragma once

#include "Luau/ConfigResolver.h"
#include "Luau/FileResolver.h"

namespace nbl::analysis {

class CfgResolver : public Luau::ConfigResolver {
public:
  CfgResolver();

  const Luau::Config &
  getConfig(const Luau::ModuleName &name,
            const Luau::TypeCheckLimits &limits) const override;

private:
  Luau::Config config;
};

} // namespace nbl::analysis
#pragma once

#include <optional>
#include <string>

#include "nbl/utils/logs.h"

namespace Luau {
class Frontend;
}

namespace nbl::analysis {

std::optional<std::string> check_script(Luau::Frontend &frontend,
                                        const std::string &script,
                                        nbl::utils::Diagnostics &diagnostics);

} // namespace nbl::analysis
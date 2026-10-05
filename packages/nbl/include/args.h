#pragma once

#include <optional>
#include <string>

#include "nbl/utils/logs.h"

namespace nbl::cli {

enum class Command { Check, Update };

struct Args {
  Command command = Command::Check;
  std::string script;
  std::string output;
  std::string types;
};

std::optional<Args> parse_args(int argc, char *argv[],
                               nbl::utils::Diagnostics &diagnostics);

} // namespace nbl::cli
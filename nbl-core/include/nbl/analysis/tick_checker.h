#pragma once

#include <string>
#include <vector>

#include "Luau/Ast.h"
#include "Luau/Location.h"

namespace nbl::analysis {

struct TickInfo {
  struct Error {
    Luau::Location loc;
    std::string message;
  };

  bool has_tick = false;
  Luau::Location tick_loc;
  std::vector<Error> errors;
};

TickInfo analyzeTick(Luau::AstStatBlock *root);

} // namespace nbl::analysis
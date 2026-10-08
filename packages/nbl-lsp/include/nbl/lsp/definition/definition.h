#pragma once

#include <string>

#include "Luau/Location.h"

namespace nbl::lsp {

/// A place a symbol is defined at.
struct Definition {
  std::string path;
  /// `Location{}` points at the start of the file.
  Luau::Location location;
};

} // namespace nbl::lsp

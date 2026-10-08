#pragma once

#include <string>

namespace nbl::lsp {

struct ServerOptions {
  std::string definitions;
  std::string definitions_path;
  unsigned workers = 0;
};

} // namespace nbl::lsp

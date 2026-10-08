#pragma once

#include <optional>
#include <string>

namespace nbl::lsp::app {

struct CliOptions {
  std::string types;
  unsigned workers = 0;
};

/// Parses the command line into `options`. Returns an exit code when the
/// process should stop right away (`--help`, a usage error); `nullopt` to go on.
std::optional<int> parse_cli(int argc, char *argv[], CliOptions &options);

} // namespace nbl::lsp::app

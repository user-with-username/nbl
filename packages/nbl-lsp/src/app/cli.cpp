#include "nbl/lsp/app/cli.h"

#include <CLI/CLI.hpp>

#include "nbl/types/resolve.h"

namespace nbl::lsp::app {

std::optional<int> parse_cli(int argc, char *argv[], CliOptions &options) {
  bool stdio = false;

  CLI::App cli{"Language server for NB scripts"};
  cli.add_option("--types", options.types, "type definitions to use")
      ->type_name("FILE|embedded:[globals|types]")
      ->default_str("~/.nbl/types.d.luau, else embedded:")
      ->check(nbl::types::validate_source);
  cli.add_option("--workers", options.workers, "checker threads")
      ->type_name("N")
      ->default_str("one per core, at most four");
  cli.add_flag("--stdio", stdio, "speak LSP over stdio (always on)");

  try {
    cli.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    return cli.exit(e);
  }

  (void)stdio;
  return std::nullopt;
}

} // namespace nbl::lsp::app

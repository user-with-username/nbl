#include <iostream>
#include <string>
#include <utility>

#include <lsp/io/standard_io.h>

#include "nbl/lsp/app/cli.h"
#include "nbl/lsp/app/definitions_file.h"
#include "nbl/lsp/server/server.h"
#include "nbl/types/resolve.h"

int main(int argc, char *argv[]) {
  nbl::lsp::app::CliOptions cli;
  if (const auto exit_code = nbl::lsp::app::parse_cli(argc, argv, cli))
    return *exit_code;

  auto resolved = nbl::types::resolve(cli.types);
  if (!resolved) {
    std::cerr << "nbl-lsp: cannot read the type definitions: " << cli.types
              << '\n';
    return 1;
  }

  std::string reason;
  const std::string definitions_path =
      nbl::lsp::app::materialize_definitions(*resolved, reason);
  if (definitions_path.empty()) {
    std::cerr << "nbl-lsp: cannot write the type definitions to a file: "
              << reason << '\n';
    return 1;
  }

  nbl::lsp::ServerOptions options;
  options.definitions = std::move(resolved->content);
  options.definitions_path = definitions_path;
  options.workers = cli.workers;

  ::lsp::io::Stream &io = ::lsp::io::standardIO();
  nbl::lsp::Server server(io, std::move(options));
  server.run();

  return 0;
}

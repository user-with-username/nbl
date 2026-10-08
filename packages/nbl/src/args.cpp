#include "args.h"

#include <string>

#include <CLI/CLI.hpp>

#include "nbl/types/resolve.h"

namespace nbl::cli {

namespace {

constexpr const char *kTypesDefault = "~/.nbl/types.d.luau, else embedded:";

void add_check_options(CLI::App *command, Args &args) {
  command->add_option("script", args.script, "script to check")
      ->required()
      ->check(CLI::ExistingFile);

  command->add_option("--types", args.types, "type definitions to use")
      ->type_name("FILE|embedded:[globals|types]")
      ->default_str(kTypesDefault)
      ->check(nbl::types::validate_source);
}

} // namespace

void configure_cli(CLI::App &app, Args &args) {
  app.require_subcommand(1);

  CLI::App *lint =
      app.add_subcommand("lint", "type-check a script and print diagnostics");
  add_check_options(lint, args);
  lint->callback([&args] { args.command = Command::Lint; });

  CLI::App *run =
      app.add_subcommand("run", "type-check a script, then bundle it");
  add_check_options(run, args);
  run->add_option("-o,--output", args.output, "where to write the bundle")
      ->type_name("FILE")
      ->default_str("<script>.bundle.luau");
  run->callback([&args] { args.command = Command::Run; });

  app.add_subcommand("update",
                     "download the latest definitions into ~/.nbl")
      ->callback([&args] { args.command = Command::Update; });
}

} // namespace nbl::cli

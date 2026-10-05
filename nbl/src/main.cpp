#include <optional>
#include <string>

#include "Luau/Frontend.h"

#include "args.h"
#include "update.h"

#include "nbl/analysis/cfg_resolver.h"
#include "nbl/analysis/checker.h"
#include "nbl/analysis/fs_resolver.h"
#include "nbl/types/resolve.h"
#include "nbl/utils/files.h"
#include "nbl/utils/logs.h"

namespace {

using nbl::utils::Diagnostics;

int run_check(const nbl::cli::Args &args, Diagnostics &diagnostics) {
  if (nbl::utils::is_bundle_file(args.script)) {
    diagnostics.error(args.script + ": input is already a bundle");
    return 1;
  }

  auto resolved = nbl::types::resolve(args.types);
  if (!resolved) {
    diagnostics.error(args.types + ": cannot read types file");
    return 1;
  }

  nbl::analysis::FsResolver files;
  nbl::analysis::CfgResolver config;

  Luau::FrontendOptions opts;
  opts.runLintChecks = true;
  Luau::Frontend frontend(&files, &config, opts);

  frontend.loadDefinitionFile(frontend.globals, frontend.globals.globalScope,
                              std::move(resolved->content), "script",
                              /*captureComments*/ false,
                              /*typeCheckForAutocomplete*/ false);

  std::optional<std::string> bundle;
  try {
    bundle = nbl::analysis::check_script(frontend, args.script, diagnostics);
  } catch (const std::exception &e) {
    diagnostics.error(std::string("internal error: ") + e.what());
  }

  if (bundle && !nbl::utils::write_file(args.output, *bundle))
    diagnostics.error(args.output + ": failed to write bundle");

  return diagnostics.has_errors() ? 1 : 0;
}

int run_update(Diagnostics &diagnostics) {
  return nbl::cli::update_types(diagnostics) ? 0 : 1;
}

} // namespace

int main(int argc, char *argv[]) {
  Diagnostics diagnostics;

  auto args = nbl::cli::parse_args(argc, argv, diagnostics);
  if (!args) {
    diagnostics.print_summary();
    return 1;
  }

  int status = 0;
  switch (args->command) {
  case nbl::cli::Command::Check:
    status = run_check(*args, diagnostics);
    break;
  case nbl::cli::Command::Update:
    status = run_update(diagnostics);
    break;
  }

  diagnostics.print_summary();
  return status;
}
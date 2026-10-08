#include <optional>
#include <string>

#include <CLI/CLI.hpp>

#include "Luau/Frontend.h"

#include "args.h"
#include "update.h"

#include "nbl/analysis/cfg_resolver.h"
#include "nbl/analysis/checker.h"
#include "nbl/analysis/dep_graph.h"
#include "nbl/analysis/fs_resolver.h"
#include "nbl/compiler/bundler.h"
#include "nbl/types/resolve.h"
#include "nbl/utils/files.h"
#include "nbl/utils/logs.h"

namespace {

using nbl::utils::Diagnostics;

constexpr const char *kTypesPackageName = "script";

/// Loads the type definitions and checks the script. Returns the check result
/// when the definitions could be loaded, so the caller can decide whether to
/// bundle the module graph.
std::optional<nbl::analysis::LintResult> check(const nbl::cli::Args &args,
                                               Diagnostics &diagnostics) {
  auto resolved = nbl::types::resolve(args.types);
  if (!resolved) {
    diagnostics.error(args.types + ": cannot read types file");
    return std::nullopt;
  }

  nbl::analysis::FsResolver files;
  nbl::analysis::CfgResolver config;

  Luau::FrontendOptions opts;
  opts.runLintChecks = true;
  Luau::Frontend frontend(&files, &config, opts);

  const std::string types_name =
      resolved->path.empty() ? "<embedded types>" : resolved->path;

  Luau::LoadDefinitionFileResult loaded = frontend.loadDefinitionFile(
      frontend.globals, frontend.globals.globalScope, resolved->content,
      kTypesPackageName,
      /*captureComments*/ false,
      /*typeCheckForAutocomplete*/ false);

  if (!loaded.success) {
    diagnostics.add_definition(loaded, types_name);
    return std::nullopt;
  }

  try {
    return nbl::analysis::lint(frontend, args.script,
                               /*require_entrypoint*/ true);
  } catch (const std::exception &e) {
    diagnostics.error(std::string("internal error: ") + e.what());
    return std::nullopt;
  }
}

void report(const nbl::analysis::LintResult &result, Diagnostics &diagnostics) {
  for (const nbl::analysis::Diagnostic &diagnostic : result.diagnostics) {
    const std::string text =
        nbl::utils::format_location(diagnostic.module, diagnostic.location) +
        ": " + diagnostic.message;

    if (diagnostic.severity == nbl::analysis::Severity::Error)
      diagnostics.error(text);
    else
      diagnostics.warning(text);
  }
}

int run_lint(const nbl::cli::Args &args, Diagnostics &diagnostics) {
  if (auto result = check(args, diagnostics))
    report(*result, diagnostics);

  return diagnostics.has_errors() ? 1 : 0;
}

int run_bundle(const nbl::cli::Args &args, Diagnostics &diagnostics) {
  std::optional<nbl::analysis::LintResult> result = check(args, diagnostics);
  if (!result)
    return 1;

  report(*result, diagnostics);
  if (result->has_errors())
    return 1;

  const std::string output = args.output.empty()
                                 ? nbl::utils::bundle_output_path(args.script)
                                 : args.output;

  if (!nbl::utils::write_file(output, nbl::compiler::bundle(result->graph))) {
    diagnostics.error(output + ": failed to write bundle");
    return 1;
  }

  diagnostics.info("wrote " + output);
  return 0;
}

int run_update(Diagnostics &diagnostics) {
  return nbl::cli::update_types(diagnostics) ? 0 : 1;
}

} // namespace

int main(int argc, char *argv[]) {
  Diagnostics diagnostics;

  nbl::cli::Args args;
  CLI::App app{"Tool for linting NB scripts"};
  nbl::cli::configure_cli(app, args);

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    return app.exit(e);
  }

  int status = 0;
  switch (args.command) {
  case nbl::cli::Command::Lint:
    status = run_lint(args, diagnostics);
    break;
  case nbl::cli::Command::Run:
    status = run_bundle(args, diagnostics);
    break;
  case nbl::cli::Command::Update:
    status = run_update(diagnostics);
    break;
  }

  diagnostics.print_summary();
  return status;
}

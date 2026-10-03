#include <string>

#include "Luau/Frontend.h"

#include "analysis/cfg_resolver.h"
#include "analysis/checker.h"
#include "analysis/fs_resolver.h"
#include "utils/files.h"
#include "utils/logs.h"

int main(int argc, char *argv[]) {
  Diagnostics diagnostics;

  if (argc < 2 || argc > 3) {
    diagnostics.error("usage: nbl <file> [script.bundle.luau]");
    return 1;
  }

  const std::string script = argv[1];
  const std::string output = (argc == 3) ? argv[2] : bundle_output_path(script);

  FsResolver files;
  CfgResolver config;

  Luau::FrontendOptions options;
  options.runLintChecks = true;
  Luau::Frontend frontend(&files, &config, options);

  frontend.loadDefinitionFile(frontend.globals, frontend.globals.globalScope,
                              read_file("types.d.luau"), "script",
                              /*captureComments*/ false,
                              /*typeCheckForAutocomplete*/ false);

  CheckOutput result = check_script(frontend, script, diagnostics);
  diagnostics.print_summary();

  if (result.ok)
    write_file(output, result.bundle);

  return diagnostics.has_errors() ? 1 : 0;
}

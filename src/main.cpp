#include <string>

#include "Luau/Frontend.h"

#include "analysis/cfg_resolver.h"
#include "analysis/checker.h"
#include "analysis/fs_resolver.h"
#include "utils/files.h"
#include "utils/logs.h"

int main(int argc, char* argv[])
{
    Diagnostics diagnostics;

    if (argc != 2)
    {
        diagnostics.error("No input file");
        return 1;
    }

    const std::string script = argv[1];

    FsResolver files;
    CfgResolver config;

    Luau::FrontendOptions options;
    options.runLintChecks = true;
    Luau::Frontend frontend(&files, &config, options);

    frontend.loadDefinitionFile(
        frontend.globals,
        frontend.globals.globalScope,
        read_file("types.d.luau"),
        "script",
        /*captureComments*/ false,
        /*typeCheckForAutocomplete*/ false
    );

    check_script(frontend, script, diagnostics);
    diagnostics.print_summary();

    return diagnostics.has_errors() ? 1 : 0;
}
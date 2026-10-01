#include "analysis/checker.h"

#include "analysis/visitor.h"
#include "utils/logs.h"

void check_script(Luau::Frontend& frontend, const std::string& script, Diagnostics& diagnostics)
{
    frontend.parse(script);
    Luau::SourceModule* source_module = frontend.getSourceModule(script);

    if (!source_module)
    {
        diagnostics.error(script + ": cannot read file");
        return;
    }

    if (source_module->parseErrors.empty() && !definesTick(source_module->root))
    {
        diagnostics.error(script + ": no entrypoint `function tick()`");
        return;
    }

    diagnostics.add(frontend.check(script), script);
}
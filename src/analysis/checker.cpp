#include "analysis/checker.h"

#include <vector>

#include "compiler/bundler.h"
#include "analysis/dep_graph.h"
#include "utils/files.h"

namespace {

std::vector<std::string> findTickModules(const DepGraph& graph) {
    std::vector<std::string> result;
    for (const auto& name : graph.topo_order()) {
        if (is_bundle_file(name))
            continue;
        if (graph.nodes().at(name).tick.has_tick)
            result.push_back(name);
    }
    return result;
}

} // namespace

CheckOutput check_script(Luau::Frontend& frontend, const std::string& script,
                         Diagnostics& diagnostics) {
    CheckOutput output;

    if (is_bundle_file(script)) {
        diagnostics.error(
            script + ": refusing to type-check a .bundle.luau file");
        return output;
    }

    DepGraph graph;
    if (!graph.build(script)) {
        diagnostics.error(script + ": cannot read file or its dependencies");
        return output;
    }

    for (const auto& e : graph.errors())
        diagnostics.error(": " + format_location(e.module, e.loc) + e.message);

    if (!graph.errors().empty())
        return output;

    std::vector<std::string> tick_modules = findTickModules(graph);

    if (tick_modules.empty()) {
        diagnostics.error(
            script + ": no module defines entrypoint `function tick()`");
        return output;
    }
    if (tick_modules.size() > 1) {
        std::string msg = "multiple modules define `function tick()`:";
        for (const auto& m : tick_modules)
            msg += "\n  " + m;
        diagnostics.error(msg);
        return output;
    }

    for (const auto& name : graph.topo_order()) {
        if (is_bundle_file(name))
            continue;
        diagnostics.add(frontend.check(name), name);
    }

    output.bundle = compiler::bundle(graph);
    output.ok = true;
    return output;
}
#include "nbl/analysis/checker.h"

#include <algorithm>
#include <vector>

#include "Luau/Frontend.h"

#include "nbl/analysis/dep_graph.h"
#include "nbl/utils/files.h"

namespace nbl::analysis {

namespace {

std::vector<std::string> findTickModules(const DepGraph &graph) {
  std::vector<std::string> result;
  for (const auto &[name, node] : graph.nodes())
    if (node.tick.has_tick)
      result.push_back(name);
  std::sort(result.begin(), result.end());
  return result;
}

} // namespace

std::optional<DepGraph> check_script(Luau::Frontend &frontend,
                                     const std::string &script,
                                     nbl::utils::Diagnostics &diagnostics) {
  if (nbl::utils::is_bundle_file(script)) {
    diagnostics.error(script + ": refusing to type-check a .bundle.luau file");
    return std::nullopt;
  }

  DepGraph graph;
  graph.build(script);

  if (!graph.errors().empty()) {
    for (const auto &e : graph.errors())
      diagnostics.error(nbl::utils::format_location(e.module, e.loc) + ": " +
                        e.message);
    return std::nullopt;
  }

  std::vector<std::string> tick_modules = findTickModules(graph);

  if (tick_modules.empty()) {
    diagnostics.error(script +
                      ": no module defines entrypoint `function tick()`");
    return std::nullopt;
  }
  if (tick_modules.size() > 1) {
    diagnostics.error("multiple modules define `function tick()`:");
    for (const auto &m : tick_modules)
      diagnostics.error("  " + m);
    return std::nullopt;
  }

  for (const auto &name : graph.topo_order())
    diagnostics.add(frontend.check(name), name);

  if (diagnostics.has_errors())
    return std::nullopt;

  return graph;
}

} // namespace nbl::analysis
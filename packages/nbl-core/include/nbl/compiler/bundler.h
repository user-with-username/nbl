#pragma once

#include <string>

#include "nbl/analysis/dep_graph.h"

namespace nbl::compiler {

std::string bundle(const nbl::analysis::DepGraph &graph);

} // namespace nbl::compiler
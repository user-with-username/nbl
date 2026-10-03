#pragma once

#include <string>

#include "analysis/dep_graph.h"

namespace compiler {

std::string bundle(const DepGraph& graph);

} // namespace compiler
#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Luau/Location.h"

#include "nbl/analysis/tick_checker.h"

namespace nbl::analysis {

struct DepEdge {
  std::string raw;
  std::string resolved;
  Luau::Location loc;
};

struct DepError {
  std::string module;
  Luau::Location loc;
  std::string message;
};

struct DepNode {
  std::string path;
  std::string source;
  TickInfo tick;
  std::vector<DepEdge> deps;
};

class DepGraph {
public:
  void build(const std::string &entry);

  const std::string &entry() const { return entry_; }
  const std::vector<std::string> &topo_order() const { return topo_; }
  const std::unordered_map<std::string, DepNode> &nodes() const {
    return nodes_;
  }
  const std::vector<DepError> &errors() const { return errors_; }

private:
  std::optional<DepNode> loadNode(const std::string &path,
                                  const Luau::Location &from_loc);

  void dfs(const std::string &path, const Luau::Location &from_loc);
  std::string formatCycle(const std::string &start) const;

  std::string entry_;
  std::vector<std::string> stack_;
  std::vector<std::string> topo_;
  std::unordered_set<std::string> done_;
  std::unordered_map<std::string, DepNode> nodes_;
  std::vector<DepError> errors_;
};

} // namespace nbl::analysis
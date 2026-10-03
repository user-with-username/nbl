#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Luau/Location.h"

#include "analysis/visitor.h"

struct DepEdge {
    std::string raw;
    std::string resolved;
    Luau::Location loc;
};

struct DepNode {
    std::string path;
    std::string source;
    std::vector<DepEdge> deps;    
    TickInfo tick;
};

struct DepError {
    std::string module;
    Luau::Location loc;
    std::string message;
};

class DepGraph {
public:
    bool build(const std::string& entry);

    const std::string& entry() const { return entry_; }
    const std::unordered_map<std::string, DepNode>& nodes() const { return nodes_; }
    const std::vector<std::string>& topo_order() const { return topo_; }
    const std::vector<DepError>& errors() const { return errors_; }

private:
    bool visit(const std::string& path, const Luau::Location& from_loc);
    std::string formatCycle(const std::string& start) const;

    std::string entry_;
    std::unordered_map<std::string, DepNode> nodes_;
    std::vector<std::string> topo_;
    std::vector<DepError> errors_;
    std::unordered_set<std::string> on_stack_;
    std::vector<std::string> stack_;
    std::unordered_set<std::string> done_;
};
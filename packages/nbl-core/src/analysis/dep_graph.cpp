#include "nbl/analysis/dep_graph.h"

#include <algorithm>

#include "Luau/Ast.h"
#include "Luau/Parser.h"

#include "nbl/analysis/fs_resolver.h"
#include "nbl/utils/files.h"

namespace nbl::analysis {

namespace {

class RequireCollector : public Luau::AstVisitor {
public:
  RequireCollector(const std::string &path, std::vector<DepEdge> &edges,
                   std::vector<DepError> &errors)
      : path_(path), edges_(edges), errors_(errors) {}

  bool visit(Luau::AstExprCall *call) override {
    auto *global = call->func->as<Luau::AstExprGlobal>();
    if (!global || global->name != "require")
      return true;
    if (call->args.size != 1)
      return true;
    auto *str = call->args.data[0]->as<Luau::AstExprConstantString>();
    if (!str)
      return true;

    std::string raw(str->value.data, str->value.size);

    if (auto reason = validate_require_string(raw)) {
      errors_.push_back({path_, str->location, *reason});
      return true;
    }
    auto resolved = resolve_lua_path(path_, raw);
    if (!resolved) {
      errors_.push_back(
          {path_, str->location, "cannot resolve require(\"" + raw + "\")"});
      return true;
    }
    edges_.push_back({std::move(raw), std::move(*resolved), str->location});
    return true;
  }

private:
  const std::string &path_;
  std::vector<DepEdge> &edges_;
  std::vector<DepError> &errors_;
};

} // namespace

void DepGraph::build(const std::string &entry) {
  *this = DepGraph{};
  entry_ = nbl::utils::canonicalize(entry);
  dfs(entry_, {});
}

std::optional<DepNode> DepGraph::loadNode(const std::string &path,
                                          const Luau::Location &from_loc) {
  auto source = nbl::utils::read_file_opt(path);
  if (!source) {
    errors_.push_back({path, from_loc, "cannot read file"});
    return std::nullopt;
  }

  DepNode node;
  node.path = path;
  node.source = std::move(*source);

  Luau::Allocator allocator;
  Luau::AstNameTable names(allocator);
  Luau::ParseResult result = Luau::Parser::parse(
      node.source.data(), node.source.size(), names, allocator, {});

  if (!result.root)
    return node;

  node.tick = analyzeTick(result.root);
  for (const auto &e : node.tick.errors)
    errors_.push_back({path, e.loc, e.message});

  RequireCollector collector(path, node.deps, errors_);
  result.root->visit(&collector);

  return node;
}

void DepGraph::dfs(const std::string &path, const Luau::Location &from_loc) {
  if (nbl::utils::is_bundle_file(path) || done_.count(path))
    return;

  if (std::find(stack_.begin(), stack_.end(), path) != stack_.end()) {
    errors_.push_back(
        {path, from_loc, "circular dependency: " + formatCycle(path)});
    return;
  }

  auto node = loadNode(path, from_loc);
  if (!node) {
    done_.insert(path);
    return;
  }

  stack_.push_back(path);
  for (const auto &edge : node->deps)
    dfs(edge.resolved, edge.loc);
  stack_.pop_back();

  nodes_.emplace(path, std::move(*node));
  topo_.push_back(path);
  done_.insert(path);
}

std::string DepGraph::formatCycle(const std::string &start) const {
  std::string out;
  bool in_cycle = false;
  for (const auto &p : stack_) {
    if (p == start)
      in_cycle = true;
    if (in_cycle) {
      if (!out.empty())
        out += " -> ";
      out += p;
    }
  }
  if (!out.empty())
    out += " -> " + start;
  return out;
}

} // namespace nbl::analysis
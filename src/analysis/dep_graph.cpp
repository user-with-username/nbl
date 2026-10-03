#include "analysis/dep_graph.h"

#include <filesystem>

#include "Luau/Ast.h"
#include "Luau/Parser.h"

#include "analysis/fs_resolver.h"
#include "analysis/visitor.h"
#include "utils/files.h"

namespace {

class RequireCollector : public Luau::AstVisitor {
public:
    struct Entry {
        std::string raw;
        Luau::Location loc;
    };
    std::vector<Entry> entries;

    bool visit(Luau::AstExprCall* call) override {
        auto* global = call->func->as<Luau::AstExprGlobal>();
        if (!global || global->name != "require")
            return true;
        if (call->args.size != 1)
            return true;
        auto* str = call->args.data[0]->as<Luau::AstExprConstantString>();
        if (!str)
            return true;
        entries.push_back({std::string(str->value.data, str->value.size),
                           str->location});
        return true;
    }
};

std::vector<DepEdge> collectRequires(const std::string& path,
                                     Luau::AstStatBlock* root,
                                     std::vector<DepError>& errors) {
    RequireCollector collector;
    root->visit(&collector);

    std::vector<DepEdge> edges;
    edges.reserve(collector.entries.size());

    for (const auto& e : collector.entries) {
        if (auto reason = validate_require_string(e.raw)) {
            errors.push_back({path, e.loc, *reason});
            continue;
        }
        auto resolved = resolve_lua_path(path, e.raw);
        if (!resolved) {
            errors.push_back(
                {path, e.loc, "cannot resolve require(\"" + e.raw + "\")"});
            continue;
        }
        edges.push_back({e.raw, *resolved, e.loc});
    }
    return edges;
}

} // namespace

bool DepGraph::build(const std::string& entry) {
    namespace fs = std::filesystem;
    std::error_code ec;
    fs::path p = fs::weakly_canonical(entry, ec);
    entry_ = (ec ? fs::absolute(entry) : p).string();
    return visit(entry_, {});
}

std::string DepGraph::formatCycle(const std::string& start) const {
    std::string out;
    bool in_cycle = false;
    for (const auto& p : stack_) {
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

bool DepGraph::visit(const std::string& path, const Luau::Location& from_loc) {
    // Bundle outputs are never part of the graph: skip entirely.
    if (is_bundle_file(path))
        return true;

    if (done_.count(path))
        return true;

    if (on_stack_.count(path)) {
        std::string cycle = formatCycle(path);
        errors_.push_back({path, from_loc,
                           "circular dependency: " + cycle});
        return true;
    }

    std::error_code ec;
    if (!std::filesystem::is_regular_file(path, ec))
        return false;

    DepNode node;
    node.path = path;
    node.source = read_file(path);

    Luau::Allocator allocator;
    Luau::AstNameTable names(allocator);
    Luau::ParseOptions parseOptions;
    Luau::ParseResult result = Luau::Parser::parse(
        node.source.data(), node.source.size(), names, allocator, parseOptions);

    if (result.root) {
        node.tick = analyzeTick(result.root);
        for (const auto& e : node.tick.errors)
            errors_.push_back({path, e.loc, e.message});

        node.deps = collectRequires(path, result.root, errors_);
    }

    on_stack_.insert(path);
    stack_.push_back(path);
    for (const auto& edge : node.deps)
        if (!visit(edge.resolved, edge.loc))
            return false;
    stack_.pop_back();
    on_stack_.erase(path);

    nodes_.emplace(path, std::move(node));
    topo_.push_back(path);
    done_.insert(path);
    return true;
}
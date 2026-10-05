#include "nbl/compiler/bundler.h"

#include <filesystem>
#include <sstream>
#include <unordered_map>

#include "nbl/compiler/lua_emit.h"
#include "nbl/compiler/source_editor.h"
#include "nbl/utils/files.h"

namespace fs = std::filesystem;

namespace nbl::compiler {

namespace {

std::string make_id(const std::string &entry_dir, const std::string &abs) {
  std::error_code ec;
  auto rel = fs::relative(abs, entry_dir, ec);
  return ec ? abs : rel.generic_string();
}

std::string
rewrite_requires(
    const nbl::analysis::DepNode &node,
    const std::unordered_map<std::string, std::string> &id_map) {
  SourceEditor editor(node.source);
  for (const auto &r : node.deps) {
    auto it = id_map.find(r.resolved);
    const std::string &id = (it != id_map.end()) ? it->second : r.resolved;
    editor.replace(r.loc, quote(id));
  }
  return editor.apply();
}

std::unordered_map<std::string, std::string>
build_id_map(const nbl::analysis::DepGraph &graph,
             const std::string &entry_dir) {
  std::unordered_map<std::string, std::string> ids;
  for (const auto &kv : graph.nodes()) {
    if (nbl::utils::is_bundle_file(kv.first))
      continue;
    ids[kv.first] = make_id(entry_dir, kv.first);
  }
  return ids;
}

} // namespace

std::string bundle(const nbl::analysis::DepGraph &graph) {
  const std::string entry = graph.entry();
  const std::string entry_dir = fs::path(entry).parent_path().string();
  auto id_map = build_id_map(graph, entry_dir);

  std::ostringstream out;
  out << BUNDLE_HEADER;
  out << PRELUDE << "\n";

  for (const auto &name : graph.topo_order()) {
    if (name == entry)
      continue;
    if (nbl::utils::is_bundle_file(name))
      continue;
    const nbl::analysis::DepNode &node = graph.nodes().at(name);
    emit_module(out, id_map[name], rewrite_requires(node, id_map));
  }

  const nbl::analysis::DepNode &entry_node = graph.nodes().at(entry);
  emit_entry(out, rewrite_requires(entry_node, id_map));

  return out.str();
}

} // namespace nbl::compiler
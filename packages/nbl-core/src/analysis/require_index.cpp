#include "nbl/analysis/require_index.h"

#include <filesystem>
#include <string>

#include "Luau/Ast.h"
#include "Luau/Parser.h"

#include "nbl/analysis/fs_resolver.h"
#include "nbl/utils/sources.h"
#include "nbl/utils/files.h"

namespace fs = std::filesystem;

namespace nbl::analysis {

namespace {

bool is_luau_source(const std::string &path) {
  const fs::path p{path};
  if (p.extension() != ".luau")
    return false;
  const std::string name = p.filename().string();
  return name.find(".bundle.") == std::string::npos;
}

bool is_ignored_dir(const fs::path &p) {
  const std::string name = p.filename().string();
  return name == ".git" || name == ".svn" || name == ".hg" ||
         name == "node_modules" || name == "build" || name == "dist" ||
         name == "out" || name == ".cache" || name == ".vscode" ||
         name == ".nbl-cache";
}

class RequireTargets : public Luau::AstVisitor {
public:
  RequireTargets(const std::string &path, std::vector<std::string> &out)
      : path_(path), out_(out) {}

  bool visit(Luau::AstExprCall *call) override {
    auto *global = call->func->as<Luau::AstExprGlobal>();
    if (!global || global->name != "require" || call->args.size != 1)
      return true;
    auto *str = call->args.data[0]->as<Luau::AstExprConstantString>();
    if (!str)
      return true;

    const std::string raw(str->value.data, str->value.size);
    if (validate_require_string(raw))
      return true;
    if (auto resolved = resolve_lua_path(path_, raw))
      out_.push_back(*resolved);
    return true;
  }

private:
  const std::string &path_;
  std::vector<std::string> &out_;
};

std::vector<std::string> targets_of(const std::string &path,
                                    const std::string &source) {
  std::vector<std::string> targets;

  Luau::Allocator allocator;
  Luau::AstNameTable names(allocator);
  Luau::ParseResult parsed = Luau::Parser::parse(
      source.data(), source.size(), names, allocator, {});
  if (!parsed.root)
    return targets;

  RequireTargets collector(path, targets);
  parsed.root->visit(&collector);
  return targets;
}

} // namespace

void RequireIndex::rebuild(const std::string &root) {
  requirers_.clear();
  requires_.clear();

  if (root.empty())
    return;

  std::error_code ec;
  fs::recursive_directory_iterator it(
      root, fs::directory_options::skip_permission_denied, ec);
  const fs::recursive_directory_iterator end;
  if (ec)
    return;

  static const nbl::utils::FileSystemSources kFiles;

  for (; it != end; it.increment(ec)) {
    if (ec) {
      ec.clear();
      continue;
    }

    if (it->is_directory(ec)) {
      if (is_ignored_dir(it->path()))
        it.disable_recursion_pending();
      continue;
    }

    if (!it->is_regular_file(ec))
      continue;

    const std::string path = it->path().string();
    if (!is_luau_source(path))
      continue;

    const std::string canonical = nbl::utils::canonicalize(path);
    if (auto source = kFiles.read(canonical))
      record(canonical, targets_of(canonical, *source));
  }
}

void RequireIndex::update(const std::string &path,
                          const nbl::utils::SourceProvider &sources) {
  if (!is_luau_source(path)) {
    remove(path);
    return;
  }
  auto source = sources.read(path);
  if (!source) {
    remove(path);
    return;
  }
  record(path, targets_of(path, *source));
}

void RequireIndex::remove(const std::string &path) {
  auto it = requires_.find(path);
  if (it == requires_.end())
    return;

  for (const std::string &target : it->second) {
    auto r = requirers_.find(target);
    if (r == requirers_.end())
      continue;
    r->second.erase(path);
    if (r->second.empty())
      requirers_.erase(r);
  }
  requires_.erase(it);
}

bool RequireIndex::contains(const std::string &path) const {
  return requires_.count(path) > 0;
}

bool RequireIndex::has_requirers(const std::string &path) const {
  auto it = requirers_.find(path);
  return it != requirers_.end() && !it->second.empty();
}

void RequireIndex::record(const std::string &path,
                          const std::vector<std::string> &targets) {
  remove(path);

  std::set<std::string> &set = requires_[path];
  for (const std::string &target : targets) {
    set.insert(target);
    requirers_[target].insert(path);
  }
}

} // namespace nbl::analysis
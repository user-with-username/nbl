#include "analysis/fs_resolver.h"

#include <filesystem>
#include <fstream>
#include <sstream>

#include "Luau/Ast.h"

#include "utils/files.h"

namespace fs = std::filesystem;

namespace {

std::string canonical(const fs::path &p) {
  std::error_code ec;
  auto c = fs::weakly_canonical(p, ec);
  return (ec ? fs::absolute(p) : c).string();
}

} // namespace

std::optional<std::string> validate_require_string(const std::string &req) {
  if (req.empty())
    return "require path is empty";

  auto slash = req.find_last_of("/\\");
  auto dot = req.find_last_of('.');
  if (dot != std::string::npos && (slash == std::string::npos || dot > slash))
    return "require path must not include an extension: \"" + req + "\"";

  if (req.back() == '/' || req.back() == '\\') {
    return "require path must point to a file, not a directory: \"" + req +
           "\"";
  }

  return std::nullopt;
}

std::optional<std::string> resolve_lua_path(const std::string &from_module,
                                            const std::string &require_path) {
  if (validate_require_string(require_path))
    return std::nullopt;

  fs::path base;
  if (require_path.rfind("./", 0) == 0 || require_path.rfind("../", 0) == 0)
    base = fs::path(from_module).parent_path() / require_path;
  else
    base = fs::path(require_path);

  fs::path resolved = base.string() + ".luau";
  std::error_code ec;
  if (!fs::is_regular_file(resolved, ec))
    return std::nullopt;

  std::string canonical_path = ::canonical(resolved);
  if (is_bundle_file(canonical_path))
    return std::nullopt;

  return canonical_path;
}

std::optional<Luau::SourceCode>
FsResolver::readSource(const Luau::ModuleName &name) {
  std::ifstream file(name, std::ios::binary);
  if (!file)
    return std::nullopt;

  std::stringstream ss;
  ss << file.rdbuf();

  return Luau::SourceCode{ss.str(), Luau::SourceCode::Type::Module};
}

std::optional<Luau::ModuleInfo>
FsResolver::resolveModule(const Luau::ModuleInfo *context, Luau::AstExpr *node,
                          const Luau::TypeCheckLimits &) {
  if (!context || !node)
    return std::nullopt;

  auto *str = node->as<Luau::AstExprConstantString>();
  if (!str)
    return std::nullopt;

  std::string req(str->value.data, str->value.size);
  auto resolved = resolve_lua_path(context->name, req);
  if (!resolved)
    return std::nullopt;

  Luau::ModuleInfo info;
  info.name = *resolved;
  return info;
}

std::string
FsResolver::getHumanReadableModuleName(const Luau::ModuleName &name) const {
  return name;
}

std::optional<std::string>
FsResolver::getEnvironmentForModule(const Luau::ModuleName &) const {
  return std::nullopt;
}

#include "nbl/analysis/fs_resolver.h"

#include <filesystem>

#include "Luau/Ast.h"

#include "nbl/utils/files.h"

namespace fs = std::filesystem;

namespace nbl::analysis {

std::optional<std::string> validate_require_string(const std::string &req) {
  if (req.empty())
    return "require path is empty";

  auto slash = req.find_last_of("/\\");
  auto dot = req.find_last_of('.');
  if (dot != std::string::npos && (slash == std::string::npos || dot > slash))
    return "require path must not include an extension: \"" + req + "\"";

  if (req.back() == '/' || req.back() == '\\')
    return "require path must point to a file, not a directory: \"" + req +
           "\"";

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

  std::string c = nbl::utils::canonicalize(resolved.string());
  if (nbl::utils::is_bundle_file(c))
    return std::nullopt;
  return c;
}

std::optional<Luau::SourceCode>
FsResolver::readSource(const Luau::ModuleName &name) {
  static const nbl::utils::FileSystemSources kFiles;
  const nbl::utils::SourceProvider &sources = sources_ ? *sources_ : kFiles;

  if (auto source = sources.read(name))
    return Luau::SourceCode{std::move(*source), Luau::SourceCode::Type::Module};
  return std::nullopt;
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
  info.name = std::move(*resolved);
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

} // namespace nbl::analysis
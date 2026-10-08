#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Luau/Location.h"

#include "nbl/analysis/cfg_resolver.h"
#include "nbl/analysis/checker.h"
#include "nbl/analysis/fs_resolver.h"
#include "nbl/analysis/module_kind.h"
#include "nbl/lsp/checker/diagnostics_by_module.h"
#include "nbl/lsp/completion/completion_item.h"
#include "nbl/lsp/definition/definition.h"
#include "nbl/utils/sources.h"

namespace Luau {
class Frontend;
}

namespace nbl::lsp {

class Checker {
public:
  Checker(std::string definitions, std::string definitions_path);
  ~Checker();

  Checker(const Checker &) = delete;
  Checker &operator=(const Checker &) = delete;

  std::optional<std::string> load();

  DiagnosticsByModule
  check(const std::string &path, const nbl::utils::SourceProvider &sources,
        nbl::analysis::ModuleKind kind = nbl::analysis::ModuleKind::Unknown);

  std::optional<Definition>
  definition(const std::string &path, Luau::Position position,
             const nbl::utils::SourceProvider &sources);

  std::vector<CompletionItem>
  complete(const std::string &path, Luau::Position position,
           const nbl::utils::SourceProvider &sources);

  const std::string &definitions() const { return definitions_; }
  const std::string &definitions_path() const { return definitions_path_; }

  /// Diagnostics collected from the last `load()`. They belong to
  /// `definitions_path_` and are published by the server as-is.
  const std::vector<nbl::analysis::Diagnostic> &definitions_diagnostics() const {
    return definitions_diagnostics_;
  }

private:
  /// True if `path` refers to the definitions file we loaded.
  bool is_definitions(const std::string &path) const;

  std::string definitions_;
  std::string definitions_path_;

  nbl::utils::SwappableSources sources_;
  nbl::analysis::FsResolver files_;
  nbl::analysis::CfgResolver config_;
  std::unique_ptr<Luau::Frontend> frontend_;

  std::vector<nbl::analysis::Diagnostic> definitions_diagnostics_;
};

} // namespace nbl::lsp

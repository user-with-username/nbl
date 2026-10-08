#include "nbl/lsp/server/workspace_root.h"

#include "nbl/utils/files.h"

namespace nbl::lsp {

std::optional<std::string>
workspace_root_from(const ::lsp::InitializeParams &params) {
  // workspaceFolders - std::optional<Nullable<std::vector<WorkspaceFolder>>>:
  // two levels of nullability. Unwrap optional, check Nullable, then use
  // operator-> on Nullable to reach the vector.
  if (params.workspaceFolders.has_value() &&
      !params.workspaceFolders->isNull() &&
      !(*params.workspaceFolders)->empty()) {
    const auto &folder = (*params.workspaceFolders)->front();
    if (folder.uri.isFileUri())
      return nbl::utils::canonicalize(folder.uri.fsPath());
  } else if (!params.rootUri.isNull() && params.rootUri->isFileUri()) {
    return nbl::utils::canonicalize(params.rootUri->fsPath());
  }

  return std::nullopt;
}

} // namespace nbl::lsp

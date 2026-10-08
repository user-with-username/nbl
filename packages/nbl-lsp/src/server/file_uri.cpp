#include "nbl/lsp/server/file_uri.h"

#include <lsp/uri.h>

#include "nbl/utils/files.h"

namespace nbl::lsp {

std::optional<std::string> file_path(const std::string &uri) {
  const ::lsp::Uri parsed = ::lsp::Uri::parse(uri);
  if (!parsed.isFileUri())
    return std::nullopt;

  return nbl::utils::canonicalize(parsed.fsPath());
}

} // namespace nbl::lsp

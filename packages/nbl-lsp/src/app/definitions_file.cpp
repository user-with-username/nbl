#include "nbl/lsp/app/definitions_file.h"

#include <filesystem>

#include "nbl/utils/files.h"

namespace nbl::lsp::app {

std::string materialize_definitions(const nbl::types::Resolved &resolved,
                                    std::string &reason) {
  if (resolved.source == nbl::types::Source::Explicit && !resolved.path.empty())
    return resolved.path;

  std::error_code ec;
  const std::filesystem::path temp = std::filesystem::temp_directory_path(ec);
  if (ec) {
    reason = "cannot find the temporary directory: " + ec.message();
    return {};
  }

  const std::filesystem::path dir = temp / "nbl-lsp";
  std::filesystem::create_directories(dir, ec);
  if (ec) {
    reason = "cannot create " + dir.string() + ": " + ec.message();
    return {};
  }

  const std::filesystem::path path = dir / "definitions.d.luau";
  if (!nbl::utils::write_file_atomic(path.string(), resolved.content)) {
    reason = "cannot write " + path.string();
    return {};
  }

  return path.string();
}

} // namespace nbl::lsp::app

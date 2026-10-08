#include "nbl/lsp/server/resolved_text.h"

#include "nbl/utils/files.h"

namespace nbl::lsp {

ResolvedText::ResolvedText(const nbl::utils::OverlaySources::Files &files,
                           const std::string &path) {
  if (auto it = files.find(path); it != files.end())
    overlay_.emplace(it->second);
  else
    disk_ = nbl::utils::read_file_opt(path);
}

std::string_view ResolvedText::view() const {
  if (overlay_)
    return *overlay_;
  if (disk_)
    return *disk_;
  return {};
}

} // namespace nbl::lsp

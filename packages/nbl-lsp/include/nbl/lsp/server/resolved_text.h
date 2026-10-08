#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "nbl/utils/sources.h"

namespace nbl::lsp {

/// Text of a file as the editor sees it: the open-document overlay if the file
/// is open, the file on disk otherwise, an empty view if neither exists.
class ResolvedText {
public:
  ResolvedText(const nbl::utils::OverlaySources::Files &files,
               const std::string &path);

  std::string_view view() const;

private:
  std::optional<std::string_view> overlay_;
  std::optional<std::string> disk_;
};

} // namespace nbl::lsp

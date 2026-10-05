#pragma once

#include <optional>
#include <string>

namespace nbl::cli {

struct ReleaseAsset {
  std::string url;
  std::string tag;
};

std::optional<ReleaseAsset>
latest_release_asset(const std::string &owner, const std::string &repo,
                     const std::string &asset_name);

} // namespace nbl::cli
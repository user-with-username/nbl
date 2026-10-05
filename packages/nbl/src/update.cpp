#include "update.h"

#include <filesystem>

#include "github.h"
#include "http.h"

#include "nbl/types/dir.h"
#include "nbl/utils/files.h"

namespace fs = std::filesystem;

namespace nbl::cli {

namespace {

constexpr const char *kOwner = "user-with-username";
constexpr const char *kRepo = "nbl";
constexpr const char *kAsset = "types.d.luau";

} // namespace

bool update_types(nbl::utils::Diagnostics &diagnostics) {
  auto dir = nbl::types::nbl_dir();
  if (!dir) {
    diagnostics.error("cannot resolve ~/.nbl (HOME is unset?)");
    return false;
  }

  auto asset = latest_release_asset(kOwner, kRepo, kAsset);
  if (!asset) {
    diagnostics.error(std::string("cannot fetch ") + kAsset + " from " +
                      kOwner + "/" + kRepo + " (network or API error)");
    return false;
  }
  if (asset->url.empty()) {
    diagnostics.error(std::string(kAsset) +
                      ": release asset has no download URL");
    return false;
  }

  HttpRequest req;
  req.url = asset->url;
  req.headers = {"Accept: application/octet-stream"};
  auto resp = http_get(req);
  if (!resp || resp->status != 200) {
    diagnostics.error(std::string("failed to download ") + kAsset);
    return false;
  }

  const fs::path target = *dir / nbl::types::kTypesFilename;
  if (!nbl::utils::write_file_atomic(target.string(), resp->body)) {
    diagnostics.error("cannot write " + target.string());
    return false;
  }

  diagnostics.info(std::string("downloaded ") + kAsset + " (" +
                   std::to_string(resp->body.size()) + " bytes, release " +
                   (asset->tag.empty() ? "<unknown>" : asset->tag) + ") -> " +
                   target.string());
  return true;
}

} // namespace nbl::cli
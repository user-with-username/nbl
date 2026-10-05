#include "github.h"

#include <nlohmann/json.hpp>

#include "http.h"

using json = nlohmann::json;

namespace nbl::cli {

std::optional<ReleaseAsset>
latest_release_asset(const std::string &owner, const std::string &repo,
                     const std::string &asset_name) {
  HttpRequest req;
  req.url = "https://api.github.com/repos/" + owner + "/" + repo +
            "/releases/latest";
  req.headers = {"Accept: application/vnd.github+json",
                 "X-GitHub-Api-Version: 2022-11-28"};

  auto resp = http_get(req);
  if (!resp || resp->status != 200)
    return std::nullopt;

  json doc;
  try {
    doc = json::parse(resp->body);
  } catch (const json::exception &) {
    return std::nullopt;
  }

  const std::string tag = doc.value("tag_name", "");

  if (!doc.contains("assets") || !doc["assets"].is_array())
    return std::nullopt;

  for (const auto &a : doc["assets"]) {
    if (!a.contains("name") || a["name"] != asset_name)
      continue;
    ReleaseAsset out;
    out.url = a.value("browser_download_url", "");
    out.tag = tag;
    return out;
  }
  return std::nullopt;
}

} // namespace nbl::cli
#include "update.h"

#include <cctype>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "http.h"

#include "nbl/types/dir.h"
#include "nbl/utils/files.h"

namespace nbl::cli {

namespace {

constexpr const char *kRepo = "nulls-mods-community/scripting-docs";
constexpr const char *kRef = "main";
constexpr const char *kUserAgent = "nbl-fetch-docs/1.0";
constexpr const char *kFiles[] = {nbl::types::kGlobalsFilename,
                                  nbl::types::kTypesFilename};

std::string raw_url(const std::string &name) {
  return std::string("https://raw.githubusercontent.com/") + kRepo + "/" +
         kRef + "/" + name;
}

bool looks_like_html(std::string_view body) {
  size_t begin = 0;
  while (begin < body.size() &&
         std::isspace(static_cast<unsigned char>(body[begin])))
    ++begin;

  std::string head(body.substr(begin, 64));
  for (char &c : head)
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

  return head.starts_with("<!doctype html") || head.starts_with("<html");
}

std::optional<std::string> fetch(const std::string &url,
                                 nbl::utils::Diagnostics &diagnostics) {
  HttpRequest request;
  request.url = url;
  request.headers = {std::string("User-Agent: ") + kUserAgent,
                     "Accept: text/plain, */*"};

  auto response = http_get(request);
  if (!response || response->status != 200) {
    diagnostics.error(url + ": download failed");
    return std::nullopt;
  }

  if (response->body.empty()) {
    diagnostics.error(url + ": empty response");
    return std::nullopt;
  }

  if (looks_like_html(response->body)) {
    diagnostics.error(url + ": returned HTML, not a raw file");
    return std::nullopt;
  }

  return std::move(response->body);
}

struct Downloaded {
  std::string name;
  std::string path;
  std::string body;
};

} // namespace

bool update_types(nbl::utils::Diagnostics &diagnostics) {
  auto dir = nbl::types::nbl_dir();
  if (!dir) {
    diagnostics.error("cannot resolve ~/.nbl (HOME is unset?)");
    return false;
  }

  std::vector<Downloaded> files;
  for (const char *name : kFiles) {
    auto body = fetch(raw_url(name), diagnostics);
    if (!body)
      return false;

    files.push_back({name, (*dir / name).string(), std::move(*body)});
  }

  for (const Downloaded &file : files) {
    if (auto current = nbl::utils::read_file_opt(file.path);
        current && *current == file.body) {
      diagnostics.info(file.name + " is up to date -> " + file.path);
      continue;
    }

    if (!nbl::utils::write_file_atomic(file.path, file.body)) {
      diagnostics.error(file.path + ": cannot write");
      return false;
    }

    diagnostics.info("downloaded " + file.name + " (" +
                     std::to_string(file.body.size()) + " bytes) -> " +
                     file.path);
  }

  return true;
}

} // namespace nbl::cli

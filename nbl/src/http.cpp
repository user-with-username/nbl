#include "http.h"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

namespace nbl::cli {

namespace {

struct UrlParts {
  std::string scheme;
  std::string host;
  std::string path;
};

std::optional<UrlParts> split_url(const std::string &url) {
  auto scheme_end = url.find("://");
  if (scheme_end == std::string::npos)
    return std::nullopt;

  UrlParts out;
  out.scheme = url.substr(0, scheme_end);
  if (out.scheme != "http" && out.scheme != "https")
    return std::nullopt;

  auto rest = url.substr(scheme_end + 3);
  auto slash = rest.find('/');
  if (slash == std::string::npos) {
    out.host = rest;
    out.path = "/";
  } else {
    out.host = rest.substr(0, slash);
    out.path = rest.substr(slash);
  }
  return out;
}

httplib::Headers build_headers(const std::vector<std::string> &headers) {
  httplib::Headers out;
  for (const auto &line : headers) {
    auto colon = line.find(':');
    if (colon == std::string::npos)
      continue;
    std::string key = line.substr(0, colon);
    std::string value = line.substr(colon + 1);
    while (!value.empty() && value.front() == ' ')
      value.erase(value.begin());
    out.emplace(std::move(key), std::move(value));
  }
  return out;
}

} // namespace

std::optional<HttpResponse> http_get(const HttpRequest &req) {
  auto parts = split_url(req.url);
  if (!parts)
    return std::nullopt;

  auto headers = build_headers(req.headers);

  HttpResponse resp;

  auto do_get = [&](auto &client) -> bool {
    client.set_follow_location(true);
    client.set_connection_timeout(5, 0);
    client.set_read_timeout(15, 0);
    client.set_default_headers(headers);

    auto res = client.Get(parts->path.c_str());
    if (!res)
      return false;

    resp.status = res->status;
    resp.body = std::move(res->body);
    return true;
  };

  if (parts->scheme == "https") {
    httplib::SSLClient client(parts->host);
    if (!do_get(client))
      return std::nullopt;
  } else {
    httplib::Client client(parts->host);
    if (!do_get(client))
      return std::nullopt;
  }

  return resp;
}

} // namespace nbl::cli
#pragma once

#include <optional>
#include <string>
#include <vector>

namespace nbl::cli {

struct HttpRequest {
  std::string url;
  std::vector<std::string> headers;
};

struct HttpResponse {
  long status = 0;
  std::string body;
};

std::optional<HttpResponse> http_get(const HttpRequest &req);

} // namespace nbl::cli
#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace nbl::types {

enum class Source {
  Embedded,
  Override,
  Explicit,
};

struct Resolved {
  Source source;
  std::string content;
  std::string path;
};

std::optional<Resolved> resolve(const std::string &explicit_path = "");

std::string load(const std::string &explicit_path = "");
std::string_view embedded_types();

} // namespace nbl::types
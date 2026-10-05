#pragma once
#include <optional>
#include <string>
#include <string_view>

namespace nbl::types {

enum class Source { Embedded, Explicit, Override };

struct Resolved {
  Source source;
  std::string content;
  std::string path;
};

std::string_view embedded_globals();
std::string_view embedded_types();

std::optional<Resolved> resolve(const std::string &explicit_path);
std::string load(const std::string &explicit_path);

} // namespace nbl::types
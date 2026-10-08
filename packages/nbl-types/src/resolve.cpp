#include "nbl/types/resolve.h"

#include <filesystem>
#include <string>
#include <string_view>

#include "nbl/types/dir.h"
#include "nbl/utils/files.h"

namespace nbl::types {

namespace {

std::string combined(std::string_view globals, std::string_view types) {
  std::string out;
  out.reserve(globals.size() + types.size() + 2);
  out.append(globals);
  out.push_back('\n');
  out.append(types);
  return out;
}

} // namespace

std::optional<Resolved> resolve(const std::string &explicit_path) {
  if (explicit_path == "embedded:") {
    return Resolved{Source::Embedded,
                    combined(embedded_globals(), embedded_types()), ""};
  }
  if (explicit_path == "embedded:globals") {
    return Resolved{Source::Embedded, std::string(embedded_globals()), ""};
  }
  if (explicit_path == "embedded:types") {
    return Resolved{Source::Embedded, std::string(embedded_types()), ""};
  }

  if (!explicit_path.empty()) {
    auto content = nbl::utils::read_file_opt(explicit_path);
    if (!content)
      return std::nullopt;
    return Resolved{Source::Explicit, std::move(*content), explicit_path};
  }

  if (auto dir = nbl_dir()) {
    auto types_path = (*dir / kTypesFilename).string();
    if (auto types = nbl::utils::read_file_opt(types_path)) {
      auto globals_path = (*dir / kGlobalsFilename).string();
      auto globals = nbl::utils::read_file_opt(globals_path);
      std::string text = globals ? combined(*globals, *types)
                                 : combined(embedded_globals(), *types);
      return Resolved{Source::Override, std::move(text), types_path};
    }
  }

  return Resolved{Source::Embedded,
                  combined(embedded_globals(), embedded_types()), ""};
}

std::string load(const std::string &explicit_path) {
  auto r = resolve(explicit_path);
  return r ? std::move(r->content) : std::string{};
}

std::string validate_source(const std::string &value) {
  constexpr std::string_view kEmbedded = "embedded:";

  if (value.rfind(kEmbedded, 0) == 0) {
    if (value == "embedded:" || value == "embedded:globals" ||
        value == "embedded:types")
      return {};
    return value + ": expected embedded:, embedded:globals or embedded:types";
  }

  std::error_code ec;
  if (std::filesystem::is_regular_file(value, ec))
    return {};

  return value + ": no such file";
}

} // namespace nbl::types

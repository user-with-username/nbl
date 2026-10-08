#pragma once

#include <string_view>

namespace nbl::analysis {

enum class ModuleKind {
  Unknown,
  Library,
  Entrypoint,
};

ModuleKind classify_module(std::string_view source, bool has_requirers);

} // namespace nbl::analysis
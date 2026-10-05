#pragma once
#include <filesystem>
#include <optional>

namespace nbl::types {

inline constexpr const char *kTypesFilename   = "types.d.luau";
inline constexpr const char *kGlobalsFilename = "globals.d.luau";

std::optional<std::filesystem::path> home_dir();
std::optional<std::filesystem::path> nbl_dir();

} // namespace nbl::types
#pragma once

#include <filesystem>
#include <optional>

namespace nbl::types {

std::optional<std::filesystem::path> home_dir();
std::optional<std::filesystem::path> nbl_dir();

inline constexpr const char *kTypesFilename = "types.d.luau";

} // namespace nbl::types
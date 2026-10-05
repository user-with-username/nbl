#pragma once

#include <optional>
#include <string>

namespace nbl::utils {

std::optional<std::string> read_file_opt(const std::string &path);
std::string read_file(const std::string &path);
bool write_file(const std::string &path, const std::string &content);
bool write_file_atomic(const std::string &path, const std::string &content);
bool is_bundle_file(const std::string &path);
std::string bundle_output_path(const std::string &script);

std::string canonicalize(const std::string &path);

} // namespace nbl::utils
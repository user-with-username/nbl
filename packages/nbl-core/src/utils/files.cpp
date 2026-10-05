#include "nbl/utils/files.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace nbl::utils {

namespace {

constexpr const char *kBundledSuffix = ".bundle.luau";
constexpr size_t kBundledSuffixLen = 12;
constexpr const char *kLuauExt = ".luau";
constexpr size_t kLuauExtLen = 5;

bool has_suffix(const std::string &s, const char *suffix, size_t n) {
  return s.size() >= n && s.compare(s.size() - n, n, suffix) == 0;
}

} // namespace

std::optional<std::string> read_file_opt(const std::string &path) {
  std::error_code ec;
  if (!fs::is_regular_file(path, ec))
    return std::nullopt;

  std::ifstream f(path, std::ios::binary);
  if (!f)
    return std::nullopt;

  std::stringstream ss;
  ss << f.rdbuf();
  return ss.str();
}

std::string read_file(const std::string &path) {
  return read_file_opt(path).value_or(std::string{});
}

bool write_file(const std::string &path, const std::string &content) {
  std::ofstream f(path, std::ios::binary);
  if (!f)
    return false;
  f << content;
  return f.good();
}

bool write_file_atomic(const std::string &path, const std::string &content) {
  fs::path target = path;
  fs::path tmp = target;
  tmp += ".tmp";

  {
    std::ofstream f(tmp, std::ios::binary | std::ios::trunc);
    if (!f)
      return false;
    f << content;
    if (!f.good())
      return false;
  }

  std::error_code ec;
  fs::rename(tmp, target, ec);
  if (ec) {
    fs::remove(tmp, ec);
    return false;
  }
  return true;
}

bool is_bundle_file(const std::string &path) {
  return has_suffix(path, kBundledSuffix, kBundledSuffixLen);
}

std::string bundle_output_path(const std::string &script) {
  if (is_bundle_file(script))
    return script;
  if (has_suffix(script, kLuauExt, kLuauExtLen))
    return script.substr(0, script.size() - kLuauExtLen) + kBundledSuffix;
  return script + kBundledSuffix;
}

std::string canonicalize(const std::string &path) {
  std::error_code ec;
  auto c = fs::weakly_canonical(path, ec);
  return (ec ? fs::absolute(path) : c).string();
}

} // namespace nbl::utils
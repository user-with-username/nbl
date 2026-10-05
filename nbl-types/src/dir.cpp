#include "nbl/types/dir.h"

#include <cstdlib>

namespace nbl::types {

std::optional<std::filesystem::path> home_dir() {
  const char *h = std::getenv("HOME");
  if (h && *h)
    return std::filesystem::path(h);

#ifdef _WIN32
  const char *p = std::getenv("USERPROFILE");
  if (p && *p)
    return std::filesystem::path(p);
  const char *drive = std::getenv("HOMEDRIVE");
  const char *path = std::getenv("HOMEPATH");
  if (drive && path && *drive && *path)
    return std::filesystem::path(std::string(drive) + path);
#endif

  return std::nullopt;
}

std::optional<std::filesystem::path> nbl_dir() {
  auto h = home_dir();
  if (!h)
    return std::nullopt;

  auto dir = *h / ".nbl";
  std::error_code ec;
  std::filesystem::create_directories(dir, ec);
  if (ec)
    return std::nullopt;
  return dir;
}

} // namespace nbl::types
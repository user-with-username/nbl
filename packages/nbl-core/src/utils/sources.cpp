#include "nbl/utils/sources.h"

#include <utility>

#include "nbl/utils/files.h"

namespace nbl::utils {

std::optional<std::string> FileSystemSources::read(const std::string &path) const {
  return read_file_opt(path);
}

OverlaySources::OverlaySources(Files files) : files_(std::move(files)) {}

std::optional<std::string> OverlaySources::read(const std::string &path) const {
  if (auto it = files_.find(path); it != files_.end())
    return it->second;

  return read_file_opt(path);
}

std::optional<std::string> SwappableSources::read(const std::string &path) const {
  static const FileSystemSources kFiles;
  return (sources_ ? *sources_ : static_cast<const SourceProvider &>(kFiles))
      .read(path);
}

} // namespace nbl::utils

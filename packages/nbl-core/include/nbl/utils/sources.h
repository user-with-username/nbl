#pragma once

#include <map>
#include <optional>
#include <string>

namespace nbl::utils {

/// Where module sources come from. The CLI reads them from disk; the language
/// server overlays the editor's unsaved buffers on top of the filesystem.
class SourceProvider {
public:
  virtual ~SourceProvider() = default;

  /// Contents of `path`, or nullopt when it cannot be read.
  virtual std::optional<std::string> read(const std::string &path) const = 0;
};

/// Reads the files on disk.
class FileSystemSources : public SourceProvider {
public:
  std::optional<std::string> read(const std::string &path) const override;
};

/// In-memory buffers on top of the filesystem.
///
/// A provider is a snapshot: it is handed to another thread as is, so it is
/// immutable on purpose and needs no locking.
class OverlaySources : public SourceProvider {
public:
  using Files = std::map<std::string, std::string>;

  OverlaySources() = default;
  explicit OverlaySources(Files files);

  std::optional<std::string> read(const std::string &path) const override;

private:
  Files files_;
};

/// Forwards to whichever provider was set on it, so a long-lived reader (a
/// Luau frontend, for example) can follow the editor's buffers between checks.
/// Not thread safe: set and read it from one thread.
class SwappableSources : public SourceProvider {
public:
  void set(const SourceProvider &sources) { sources_ = &sources; }

  std::optional<std::string> read(const std::string &path) const override;

private:
  const SourceProvider *sources_ = nullptr;
};

} // namespace nbl::utils

#include "nbl/lsp/server/server.h"

#include <utility>

#include "nbl/analysis/module_kind.h"
#include "nbl/lsp/server/constants.h"
#include "nbl/utils/files.h"
#include "nbl/utils/sources.h"

namespace nbl::lsp {

void Server::start_index_build() {
  if (workspace_root_.empty()) {
    index_ready_ = false;
    return;
  }

  workers_->forKey(kWorkspaceKey).post(
      kWorkspaceKey, Worker::kNoCoalescing, [this] {
        nbl::analysis::RequireIndex built;
        built.rebuild(workspace_root_);

        {
          const std::lock_guard lock(mutex_);
          require_index_ = std::move(built);
          index_ready_ = true;
        }

        // Until the index is ready, open documents were classified as
        // Unknown. Re-check them now.
        recheck_all_open();
      });
}

void Server::on_watched_files_changed(
    const ::lsp::DidChangeWatchedFilesParams &params) {
  bool touched = false;

  for (const auto &change : params.changes) {
    if (!change.uri.isFileUri())
      continue;

    const std::string path = nbl::utils::canonicalize(change.uri.fsPath());
    if (path.size() < 5 || path.compare(path.size() - 5, 5, ".luau") != 0)
      continue;

    // FileChangeType is Enumeration<FileChangeType, unsigned int>, so
    // compare against FileChangeType::Created/Changed/Deleted, not switch.
    if (change.type == ::lsp::FileChangeType::Created ||
        change.type == ::lsp::FileChangeType::Changed) {
      static const nbl::utils::FileSystemSources kFiles;
      const std::lock_guard lock(mutex_);
      require_index_.update(path, kFiles);
      touched = true;
    } else if (change.type == ::lsp::FileChangeType::Deleted) {
      const std::lock_guard lock(mutex_);
      require_index_.remove(path);
      touched = true;
    }
  }

  if (touched)
    recheck_all_open();
}

nbl::analysis::ModuleKind Server::module_kind(const std::string &path,
                                              std::string_view text) const {
  bool has_requirers = false;
  {
    const std::lock_guard lock(mutex_);
    if (index_ready_ && require_index_.contains(path))
      has_requirers = require_index_.has_requirers(path);
  }
  return nbl::analysis::classify_module(text, has_requirers);
}

} // namespace nbl::lsp

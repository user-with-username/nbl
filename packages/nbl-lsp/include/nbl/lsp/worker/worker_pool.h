#pragma once

#include <memory>
#include <string>
#include <vector>

#include "nbl/lsp/worker/worker.h"

namespace nbl::lsp {

/// Spreads documents over a few workers.
class WorkerPool {
public:
  explicit WorkerPool(unsigned count);

  Worker &forKey(const std::string &key);
  unsigned indexForKey(const std::string &key) const;
  unsigned size() const { return static_cast<unsigned>(workers_.size()); }

private:
  std::vector<std::unique_ptr<Worker>> workers_;
};

} // namespace nbl::lsp

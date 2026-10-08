#include "nbl/lsp/worker/worker_pool.h"

#include <algorithm>
#include <functional>

namespace nbl::lsp {

WorkerPool::WorkerPool(unsigned count) {
  count = std::max(1u, count);

  workers_.reserve(count);
  for (unsigned i = 0; i < count; ++i)
    workers_.push_back(std::make_unique<Worker>());
}

unsigned WorkerPool::indexForKey(const std::string &key) const {
  return static_cast<unsigned>(std::hash<std::string>{}(key) % workers_.size());
}

Worker &WorkerPool::forKey(const std::string &key) {
  return *workers_[indexForKey(key)];
}

} // namespace nbl::lsp

#include "nbl/lsp/worker/worker.h"

#include <algorithm>
#include <iostream>
#include <utility>

namespace nbl::lsp {

Worker::Worker() : thread_([this] { main(); }) {}

Worker::~Worker() {
  {
    const std::lock_guard lock(mutex_);
    stopping_ = true;
  }
  event_.notify_all();

  if (thread_.joinable())
    thread_.join();
}

void Worker::post(std::string key, int kind, std::function<void()> job) {
  {
    const std::lock_guard lock(mutex_);
    if (stopping_)
      return;

    if (kind != kNoCoalescing) {
      const auto pending = std::find_if(
          jobs_.begin(), jobs_.end(), [&](const Job &queued) {
            return queued.kind == kind && queued.key == key;
          });
      if (pending != jobs_.end())
        jobs_.erase(pending);
    }

    jobs_.push_back(Job{std::move(key), kind, std::move(job)});
    idle_ = false;
  }

  event_.notify_one();
}

void Worker::drain() {
  std::unique_lock lock(mutex_);
  event_.wait(lock, [this] { return jobs_.empty() && idle_; });
}

bool Worker::take(Job &job) {
  std::unique_lock lock(mutex_);
  event_.wait(lock, [this] { return stopping_ || !jobs_.empty(); });

  if (jobs_.empty())
    return false;

  job = std::move(jobs_.front());
  jobs_.pop_front();
  return true;
}

void Worker::main() {
  for (;;) {
    Job job;
    if (!take(job))
      return;

    try {
      job.run();
    } catch (const std::exception &e) {
      // A failing job must not take the worker down: request jobs report their
      // own errors through their future.
      std::cerr << "nbl-lsp: job failed: " << e.what() << '\n';
    } catch (...) {
      std::cerr << "nbl-lsp: job failed\n";
    }

    {
      const std::lock_guard lock(mutex_);
      if (jobs_.empty()) {
        idle_ = true;
        event_.notify_all();
      }
    }
  }
}

} // namespace nbl::lsp

#pragma once

#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <type_traits>

namespace nbl::lsp {

/// Runs jobs on a thread of its own.
///
/// Luau's frontend is not thread safe, so every worker owns one and a document
/// always lands on the same worker.
class Worker {
public:
  /// Jobs posted with this kind are never coalesced.
  static constexpr int kNoCoalescing = -1;

  Worker();
  ~Worker();

  Worker(const Worker &) = delete;
  Worker &operator=(const Worker &) = delete;

  /// Queues `job`, dropping a still queued job with the same `key` and `kind`:
  /// for a document only the newest state matters.
  void post(std::string key, int kind, std::function<void()> job);

  /// Queues `job` and returns a future with its result.
  template <typename F>
  auto submit(std::string key, F job) -> std::future<std::invoke_result_t<F>> {
    using Result = std::invoke_result_t<F>;

    auto task = std::make_shared<std::packaged_task<Result()>>(std::move(job));
    std::future<Result> future = task->get_future();
    post(std::move(key), kNoCoalescing, [task] { (*task)(); });
    return future;
  }

  /// Waits until every queued job has been run.
  void drain();

private:
  struct Job {
    std::string key;
    int kind = 0;
    std::function<void()> run;
  };

  void main();
  bool take(Job &job);

  std::mutex mutex_;
  std::condition_variable event_;
  std::deque<Job> jobs_;
  bool stopping_ = false;
  bool idle_ = true;
  std::thread thread_;
};

} // namespace nbl::lsp

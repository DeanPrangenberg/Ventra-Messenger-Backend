//
// Created by deanprangenberg on 25.04.25.
//

#include "ThreadPool.h"

namespace Utils {
  // Singleton-Implementierung
  ThreadPool& ThreadPool::getInstance(size_t num_threads) {
    static std::unique_ptr<ThreadPool> instance;
    static std::once_flag init_flag;

    std::call_once(init_flag, [&] {
        if (num_threads == 0) num_threads = 1;
        instance.reset(new ThreadPool(num_threads));
    });

    return *instance;
  }

  // Konstruktor (privat)
  ThreadPool::ThreadPool(size_t num_threads) {
    for (size_t i = 0; i < num_threads; ++i) {
      workers.emplace_back([this](std::stop_token stoken) {
          while (true) {
              std::function<void()> task;
              {
                  std::unique_lock lock(queue_mutex);
                  condition.wait(lock, [&] {
                      return stoken.stop_requested() || !tasks.empty();
                  });
                  if (stoken.stop_requested() && tasks.empty()) return;
                  task = std::move(tasks.front());
                  tasks.pop();
              }
              task();
          }
      });
    }
  }

  // Destruktor (öffentlich)
  ThreadPool::~ThreadPool() {
    stop_source.request_stop();
    condition.notify_all();
  }
} // namespace Utils
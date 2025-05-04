//
// Created by deanprangenberg on 25.04.25.
//

#ifndef THREADPOOL_TPP
#define THREADPOOL_TPP

#include <functional>
#include <memory>

namespace Utils {
  template<typename F, typename... Args>
  auto ThreadPool::addTask(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
    using return_type = std::invoke_result_t<F, Args...>;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind_front(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> result = task->get_future();
    {
      std::unique_lock lock(queue_mutex);
      if (stop_source.stop_requested()) {
        throw std::runtime_error("ThreadPool wurde gestoppt. Keine neuen Aufgaben erlaubt.");
      }
      tasks.emplace([task] { (*task)(); });
    }
    condition.notify_one();
    return result;
  }
} // namespace Utils

#endif // THREADPOOL_TPP
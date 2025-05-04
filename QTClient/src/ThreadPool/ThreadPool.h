//
// Created by deanprangenberg on 25.04.25.
//

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <stop_token>
#include <memory>

namespace Utils {
  class ThreadPool {
  public:
    // Singleton-Zugriff
    static ThreadPool& getInstance(size_t num_threads = std::thread::hardware_concurrency());

    // Destruktor MUSS öffentlich sein, da std::unique_ptr ihn benötigt
    ~ThreadPool();

    template<typename F, typename... Args>
    auto addTask(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>;

    // Verhindere Kopieren und Verschieben
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

  private:
    explicit ThreadPool(size_t num_threads); // Konstruktor bleibt privat

    std::vector<std::jthread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    std::stop_source stop_source;
  };
} // namespace Utils

#include "ThreadPool.tpp"

#endif // THREADPOOL_H

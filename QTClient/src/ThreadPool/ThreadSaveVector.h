//
// Created by deanprangenberg on 25.04.25.
//

#ifndef THREADSAVEVECTOR_H
#define THREADSAVEVECTOR_H

#include <vector>
#include <mutex>
#include <optional>
#include <utility>

namespace Utils {
  template<typename T>
  class ThreadSafeVector {
  public:
    // Vorhandene Methoden (push_back, access, etc.) bleiben unverändert
    void push_back(const T& item) {
      std::lock_guard lock(mutex);
      data.push_back(item);
    }

    template<typename... Args>
    void emplace_back(Args&&... args) {
      std::lock_guard lock(mutex);
      data.emplace_back(std::forward<Args>(args)...);
    }

    std::optional<T> at(size_t index) const {
      std::lock_guard lock(mutex);
      if (index >= data.size()) return std::nullopt;
      return data[index];
    }

    size_t size() const {
      std::lock_guard lock(mutex);
      return data.size();
    }

    template<typename Func>
    auto access(Func&& func) {
      std::lock_guard lock(mutex);
      return func(data);
    }

  private:
    mutable std::mutex mutex;
    std::vector<T> data;
  };

} // namespace Utils

#endif //THREADSAVEVECTOR_H

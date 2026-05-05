#pragma once
#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>

namespace social {

// реализую алгоритм Fixed Window Counter
class RateLimiter {
 public:
  static RateLimiter& Instance();

  // проверяю разрешён ли запрос, если нет, то вызывающий код вернёт 429
  bool IsAllowed(const std::string& key, int limit = 100, int window_sec = 60);

  // сколько запросов осталось, нужно для X-RateLimit-Remaining
  int GetRemaining(const std::string& key, int limit = 100, int window_sec = 60);

  // когда сбросится окно, нужно для X-RateLimit-Reset
  long long GetResetTime(const std::string& key, int window_sec = 60);

  // счётчик заблокированных запросов для Prometheus
  long GetBlocked() const { return blocked_.load(); }

 private:
  RateLimiter() = default;

  // храню счётчик и время начала окна для каждого ключа
  struct WindowData {
    int count = 0;
    std::chrono::steady_clock::time_point window_start;
  };

  mutable std::mutex mutex_;
  std::unordered_map<std::string, WindowData> counters_;

  std::atomic<long> blocked_{0};
};

}  // namespace social

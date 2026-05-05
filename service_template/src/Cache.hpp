#pragma once
#include <atomic>
#include <chrono>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

namespace social {

// простой in-memory кеш с TTL, стратегия Cache-Aside
class Cache {
 public:
  static Cache& Instance();

  void Set(const std::string& key, const std::string& value,
           int ttl_seconds = 60);

  // возвращает nullopt если ключ не найден или TTL истёк
  std::optional<std::string> Get(const std::string& key) const;

  void Invalidate(const std::string& key);
  void InvalidateByPrefix(const std::string& prefix);

  // счётчики для Prometheus - сколько раз попали в кеш и сколько промахнулись
  long GetHits() const { return hits_.load(); }
  long GetMisses() const { return misses_.load(); }

 private:
  Cache() = default;

  struct Entry {
    std::string value;
    std::chrono::steady_clock::time_point expires_at;
  };

  mutable std::mutex mutex_;
  mutable std::unordered_map<std::string, Entry> data_;

  // атомарные счётчики - не нужен mutex, работают из любого потока
  mutable std::atomic<long> hits_{0};
  mutable std::atomic<long> misses_{0};
};

}  // namespace social

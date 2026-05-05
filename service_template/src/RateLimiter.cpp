#include "RateLimiter.hpp"

#include <ctime>

namespace social {

RateLimiter& RateLimiter::Instance() {
  static RateLimiter inst;
  return inst;
}

bool RateLimiter::IsAllowed(const std::string& key, int limit, int window_sec) {
  std::lock_guard lock(mutex_);
  auto now = std::chrono::steady_clock::now();
  auto& w = counters_[key];

  // если окно истекло, начинаю новое и обнуляю счётчик
  if (w.count == 0 ||
      now - w.window_start >= std::chrono::seconds(window_sec)) {
    w.count = 0;
    w.window_start = now;
  }

  // лимит исчерпан — считаю и возвращаю false, вызывающий код вернёт 429
  if (w.count >= limit) {
    ++blocked_;
    return false;
  }

  // запрос разрешён, считаю его
  ++w.count;
  return true;
}

int RateLimiter::GetRemaining(const std::string& key, int limit, int window_sec) {
  std::lock_guard lock(mutex_);
  auto now = std::chrono::steady_clock::now();
  auto it = counters_.find(key);

  // ключ не встречался, то значит запросов ещё не было
  if (it == counters_.end()) return limit;

  auto& w = it->second;

  // окно уже истекло, считаю что лимит полный
  if (now - w.window_start >= std::chrono::seconds(window_sec)) {
    return limit;
  }

  int remaining = limit - w.count;
  return remaining > 0 ? remaining : 0;
}

long long RateLimiter::GetResetTime(const std::string& key, int window_sec) {
  std::lock_guard lock(mutex_);
  auto it = counters_.find(key);

  // ключа нет, окно ещё не начиналось
  if (it == counters_.end()) {
    return std::time(nullptr) + window_sec;
  }

  // считаю сколько секунд осталось до конца окна
  auto& w = it->second;
  auto elapsed = std::chrono::steady_clock::now() - w.window_start;
  auto remaining_sec =
      window_sec -
      std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
  if (remaining_sec < 0) remaining_sec = 0;

  return std::time(nullptr) + remaining_sec;
}

}  // namespace social

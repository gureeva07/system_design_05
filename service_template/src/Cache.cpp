#include "Cache.hpp"

namespace social {

Cache& Cache::Instance() {
  static Cache inst;
  return inst;
}

void Cache::Set(const std::string& key, const std::string& value, int ttl_seconds) {
  std::lock_guard lock(mutex_);
  auto expires = std::chrono::steady_clock::now() + std::chrono::seconds(ttl_seconds);
  data_[key] = {value, expires};
}

std::optional<std::string> Cache::Get(const std::string& key) const {
  std::lock_guard lock(mutex_);
  auto it = data_.find(key);
  if (it == data_.end()) {
    ++misses_;
    return std::nullopt;
  }

  // проверяю TTL - если истёк, удаляю и считаю как промах
  if (std::chrono::steady_clock::now() > it->second.expires_at) {
    data_.erase(it);
    ++misses_;
    return std::nullopt;
  }

  ++hits_;
  return it->second.value;
}

void Cache::Invalidate(const std::string& key) {
  std::lock_guard lock(mutex_);
  data_.erase(key);
}

void Cache::InvalidateByPrefix(const std::string& prefix) {
  std::lock_guard lock(mutex_);
  for (auto it = data_.begin(); it != data_.end();) {
    if (it->first.rfind(prefix, 0) == 0) {
      it = data_.erase(it);
    } else {
      ++it;
    }
  }
}

}  // namespace social

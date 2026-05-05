#include "Storage.hpp"

#include <iomanip>
#include <random>
#include <sstream>

namespace social {

Storage& Storage::Instance() {
  static Storage inst;
  return inst;
}

// Генерация случайного токена — 32 hex символа
std::string Storage::GenerateToken() {
  static std::mt19937_64 rng{std::random_device{}()};
  static std::uniform_int_distribution<uint64_t> dist;
  std::ostringstream oss;
  oss << std::hex << std::setfill('0')
      << std::setw(16) << dist(rng)
      << std::setw(16) << dist(rng);
  return oss.str();
}

std::string Storage::CreateSession(int user_id) {
  std::lock_guard lock(mutex_);
  std::string token = GenerateToken();
  sessions_[token] = user_id;
  return token;
}

std::optional<int> Storage::ValidateSession(const std::string& token) const {
  std::lock_guard lock(mutex_);
  auto it = sessions_.find(token);
  if (it == sessions_.end()) return std::nullopt;
  return it->second;
}

void Storage::DeleteSession(const std::string& token) {
  std::lock_guard lock(mutex_);
  sessions_.erase(token);
}

}  // namespace social

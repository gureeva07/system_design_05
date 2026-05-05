#pragma once
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

namespace social {

// Хранилище сессий (токенов авторизации) — остаётся in-memory
// Всё остальное (пользователи, посты, сообщения) хранится в PostgreSQL
class Storage {
 public:
  static Storage& Instance();

  // Создать сессию для пользователя, вернуть токен
  std::string CreateSession(int user_id);

  // Проверить токен, вернуть user_id если токен валиден
  std::optional<int> ValidateSession(const std::string& token) const;

  // Удалить сессию (выход из системы)
  void DeleteSession(const std::string& token);

 private:
  Storage() = default;

  static std::string GenerateToken();

  mutable std::mutex mutex_;
  std::unordered_map<std::string, int> sessions_;  // токен → user_id
};

}  // namespace social

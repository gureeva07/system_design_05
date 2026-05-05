#pragma once
#include <string>

namespace social {

struct User {
  int id = 0;
  std::string login;
  std::string password_hash;
  std::string first_name;
  std::string last_name;
};

struct WallPost {
  int id = 0;
  int author_id = 0;  // кто написал
  int owner_id = 0;   // чья стена
  std::string content;
  std::string created_at;
};

struct ChatMessage {
  int id = 0;
  int sender_id = 0;
  int receiver_id = 0;
  std::string text;
  std::string created_at;
};

}  // namespace social
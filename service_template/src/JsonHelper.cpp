#include "JsonHelper.hpp"

namespace social {

// перевод пользователя в json
userver::formats::json::Value UserToJson(const User& u) {
  userver::formats::json::ValueBuilder b;
  b["id"] = u.id;
  b["login"] = u.login;
  b["first_name"] = u.first_name;
  b["last_name"] = u.last_name;
  return b.ExtractValue();
}

// перевод пост в json
userver::formats::json::Value WallPostToJson(const WallPost& p) {
  userver::formats::json::ValueBuilder b;
  b["id"] = p.id;
  b["author_id"] = p.author_id;
  b["owner_id"] = p.owner_id;
  b["content"] = p.content;
  b["created_at"] = p.created_at;
  return b.ExtractValue();
}

// перевод сообщениz в json
userver::formats::json::Value ChatMessageToJson(const ChatMessage& m) {
  userver::formats::json::ValueBuilder b;
  b["id"] = m.id;
  b["sender_id"] = m.sender_id;
  b["receiver_id"] = m.receiver_id;
  b["text"] = m.text;
  b["created_at"] = m.created_at;
  return b.ExtractValue();
}

}  // namespace social

#include "ChatHandlers.hpp"
#include "../AuthHelper.hpp"
#include "../MongoHelper.hpp"

#include <mongoc/mongoc.h>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/cluster.hpp>

namespace social {

namespace json = userver::formats::json;

std::string ChatHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& req,
    userver::server::request::RequestContext&) const {

  int sender_id = RequireAuth(req);

  // отправить сообщение
  if (req.GetMethod() == userver::server::http::HttpMethod::kPost) {
    auto body = json::FromString(req.RequestBody());
    int receiver_id = body["receiver_id"].As<int>(0);
    std::string text = body["text"].As<std::string>("");

    if (receiver_id == 0 || text.empty()) {
      req.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
      return R"({"error":"receiver_id and text are required"})";
    }

    // проверка, что получатель существует через PostgreSQL
    auto check = pg_->Execute(
        userver::storages::postgres::ClusterHostType::kSlave,
        "SELECT id FROM users WHERE id = $1", receiver_id);

    if (check.IsEmpty()) {
      req.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
      return R"({"error":"Receiver not found"})";
    }

    std::string now = MongoHelper::NowString();

    bson_t* doc = BCON_NEW(
        "sender_id", BCON_INT32(sender_id),
        "receiver_id", BCON_INT32(receiver_id),
        "text", BCON_UTF8(text.c_str()),
        "created_at", BCON_UTF8(now.c_str()));
    MongoHelper::Instance().InsertOne("chat_messages", doc);
    bson_destroy(doc);

    json::ValueBuilder resp;
    resp["sender_id"] = sender_id;
    resp["receiver_id"] = receiver_id;
    resp["text"] = text;
    resp["created_at"] = now;

    req.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
    return json::ToString(resp.ExtractValue());
  }

  // получить сообщения
  if (req.HasArg("user_id")) {
    int uid = std::stoi(req.GetArg("user_id"));

    // Сообщения где юзер,отправитель или получатель
    bson_t* filter = BCON_NEW(
        "$or", "[",
            "{", "sender_id",   BCON_INT32(uid), "}",
            "{", "receiver_id", BCON_INT32(uid), "}",
        "]");

    auto docs = MongoHelper::Instance().Find("chat_messages", filter);
    bson_destroy(filter);

    json::ValueBuilder arr(json::Type::kArray);
    for (const auto& item : docs) {
      bson_error_t err;
      bson_t* doc = bson_new_from_json(
          reinterpret_cast<const uint8_t*>(item.c_str()), item.size(), &err);

      json::ValueBuilder msg;
      msg["sender_id"] = MongoHelper::GetInt(doc, "sender_id");
      msg["receiver_id"] = MongoHelper::GetInt(doc, "receiver_id");
      msg["text"] = MongoHelper::GetStr(doc, "text");
      msg["created_at"]  = MongoHelper::GetStr(doc, "created_at");
      arr.PushBack(msg.ExtractValue());
      bson_destroy(doc);
    }

    return json::ToString(arr.ExtractValue());
  }

  req.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
  return R"({"error":"Provide 'user_id' query parameter or POST to send a message"})";
}

}  // namespace social

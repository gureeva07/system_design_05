#include "WallHandlers.hpp"
#include "../AuthHelper.hpp"
#include "../Cache.hpp"
#include "../MongoHelper.hpp"

#include <mongoc/mongoc.h>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/cluster.hpp>

namespace social {

namespace json = userver::formats::json;

static constexpr int kWallCacheTtl = 60;  // держу стену в кеше 1 минуту

std::string WallHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& req,
    userver::server::request::RequestContext&) const {

  int author_id = RequireAuth(req);
  int owner_id  = std::stoi(req.GetPathArg("user_id"));

  // проверяю, что пользователь существует, прежде чем лезть в MongoDB
  auto check = pg_->Execute(
      userver::storages::postgres::ClusterHostType::kSlave,
      "SELECT id FROM users WHERE id = $1", owner_id);

  if (check.IsEmpty()) {
    req.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
    return R"({"error":"User not found"})";
  }

  // добавляю новый пост на стену
  if (req.GetMethod() == userver::server::http::HttpMethod::kPost) {
    auto body = json::FromString(req.RequestBody());
    std::string content = body["content"].As<std::string>("");

    if (content.empty()) {
      req.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
      return R"({"error":"content is required"})";
    }

    std::string now = MongoHelper::NowString();

    bson_t* doc = BCON_NEW(
        "owner_id", BCON_INT32(owner_id),
        "author_id", BCON_INT32(author_id),
        "content", BCON_UTF8(content.c_str()),
        "created_at",BCON_UTF8(now.c_str()));
    MongoHelper::Instance().InsertOne("wall_posts", doc);
    bson_destroy(doc);

    // данные изменились, то сбрасываю кеш, чтобы следующий GET получил свежее
    std::string cache_key = "wall:" + std::to_string(owner_id);
    Cache::Instance().Invalidate(cache_key);

    json::ValueBuilder resp;
    resp["owner_id"] = owner_id;
    resp["author_id"] = author_id;
    resp["content"] = content;
    resp["created_at"] = now;

    req.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
    return json::ToString(resp.ExtractValue());
  }

  // загружаю стену
  std::string cache_key = "wall:" + std::to_string(owner_id);

  // сначала смотрю в кеш
  auto cached = Cache::Instance().Get(cache_key);
  if (cached) {
    return *cached;
  }

  // кеша нет, иду в MongoDB
  bson_t* filter = BCON_NEW("owner_id", BCON_INT32(owner_id));
  auto docs = MongoHelper::Instance().Find("wall_posts", filter);
  bson_destroy(filter);

  json::ValueBuilder arr(json::Type::kArray);
  for (const auto& item : docs) {
    bson_error_t err;
    bson_t* doc = bson_new_from_json(
        reinterpret_cast<const uint8_t*>(item.c_str()), item.size(), &err);
    if (!doc) continue;  // битый документ — пропускаю

    json::ValueBuilder post;
    post["owner_id"] = MongoHelper::GetInt(doc, "owner_id");
    post["author_id"] = MongoHelper::GetInt(doc, "author_id");
    post["content"] = MongoHelper::GetStr(doc, "content");
    post["created_at"] = MongoHelper::GetStr(doc, "created_at");
    arr.PushBack(post.ExtractValue());
    bson_destroy(doc);
  }

  std::string result = json::ToString(arr.ExtractValue());

  // кладу результат в кеш на минуту
  Cache::Instance().Set(cache_key, result, kWallCacheTtl);

  return result;
}

}  // namespace social

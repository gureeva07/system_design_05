#include "UserHandlers.hpp"
#include "../AuthHelper.hpp"
#include "../Cache.hpp"
#include "../MongoHelper.hpp"
#include "../RateLimiter.hpp"

#include <mongoc/mongoc.h>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/server/http/http_status.hpp>

namespace social {

namespace json = userver::formats::json;

// поиск по имени, дорогой regex, поэтому ограничиваю 30 запросами в минуту
static constexpr int kSearchLimit = 30;
static constexpr int kSearchWindow = 60;
static constexpr int kUserCacheTtl = 300;  // профиль кеширую на 5 минут

std::string UsersHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& req,
    userver::server::request::RequestContext&) const {

  int auth_user_id = RequireAuth(req);

  // поиск по логину - кеширую, потому что профиль меняется редко
  if (req.HasArg("login")) {
    std::string login = req.GetArg("login");
    std::string cache_key = "user:login:" + login;

    // сначала смотрю в кеш
    auto cached = Cache::Instance().Get(cache_key);
    if (cached) {
      return *cached;
    }

    // кеша нет, иду в MongoDB
    bson_t* filter = BCON_NEW("login", BCON_UTF8(login.c_str()));
    auto doc_json = MongoHelper::Instance().FindOne("users", filter);
    bson_destroy(filter);

    if (!doc_json) {
      req.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
      return R"({"error":"User not found"})";
    }

    bson_error_t err;
    bson_t* doc = bson_new_from_json( reinterpret_cast<const uint8_t*>(doc_json->c_str()), doc_json->size(), &err);

    json::ValueBuilder resp;
    resp["login"] = MongoHelper::GetStr(doc, "login");
    resp["first_name"] = MongoHelper::GetStr(doc, "first_name");
    resp["last_name"] = MongoHelper::GetStr(doc, "last_name");
    bson_destroy(doc);

    std::string result = json::ToString(resp.ExtractValue());

    // кладу в кеш, то следующий запрос с тем же логином не пойдёт в MongoDB
    Cache::Instance().Set(cache_key, result, kUserCacheTtl);

    return result;
  }

  // поиск по имени, здесь ставлю rate limiting
  if (req.HasArg("name")) {
    std::string rl_key = "search:" + std::to_string(auth_user_id);
    auto& rl = RateLimiter::Instance();

    int remaining = rl.GetRemaining(rl_key, kSearchLimit, kSearchWindow);
    long long reset_t = rl.GetResetTime(rl_key, kSearchWindow);

    // добавляю заголовки чтобы клиент видел сколько запросов осталось
    req.GetHttpResponse().SetHeader(std::string("X-RateLimit-Limit"), std::to_string(kSearchLimit));
    req.GetHttpResponse().SetHeader(std::string("X-RateLimit-Remaining"), std::to_string(remaining));
    req.GetHttpResponse().SetHeader(std::string("X-RateLimit-Reset"), std::to_string(reset_t));

    // лимит исчерпан, то возвращаю 429
    if (!rl.IsAllowed(rl_key, kSearchLimit, kSearchWindow)) {
      req.SetResponseStatus(
          userver::server::http::HttpStatus::kTooManyRequests);
      return R"({"error":"Rate limit exceeded. Try again later."})";
    }

    std::string mask = req.GetArg("name");

    // ищу по имени и по фамилии сразу через $or
    bson_t* filter = BCON_NEW("$or", "[",
        "{", "first_name", "{", "$regex", BCON_UTF8(mask.c_str()), "$options", "i", "}", "}",
        "{", "last_name", "{", "$regex", BCON_UTF8(mask.c_str()), "$options", "i", "}", "}",
    "]");

    auto docs = MongoHelper::Instance().Find("users", filter);
    bson_destroy(filter);

    json::ValueBuilder arr(json::Type::kArray);
    for (const auto& item : docs) {
      bson_error_t err;
      bson_t* doc = bson_new_from_json(
          reinterpret_cast<const uint8_t*>(item.c_str()), item.size(), &err);

      json::ValueBuilder user;
      user["login"] = MongoHelper::GetStr(doc, "login");
      user["first_name"] = MongoHelper::GetStr(doc, "first_name");
      user["last_name"] = MongoHelper::GetStr(doc, "last_name");
      arr.PushBack(user.ExtractValue());
      bson_destroy(doc);
    }

    return json::ToString(arr.ExtractValue());
  }

  req.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
  return R"({"error":"Provide 'login' or 'name' query parameter"})";
}

}  // namespace social

#include "MongoHelper.hpp"

#include <ctime>
#include <stdexcept>

namespace social {

MongoHelper& MongoHelper::Instance() {
  static MongoHelper inst;
  return inst;
}

MongoHelper::~MongoHelper() {
  if (pool_) mongoc_client_pool_destroy(pool_);
  mongoc_cleanup();
}

void MongoHelper::Init(const std::string& uri_str, const std::string& db_name) {
  mongoc_init();
  db_name_ = db_name;

  bson_error_t error;
  auto* uri = mongoc_uri_new_with_error(uri_str.c_str(), &error);
  if (!uri)
    throw std::runtime_error(std::string("MongoDB URI error: ") + error.message);

  pool_ = mongoc_client_pool_new(uri);
  mongoc_uri_destroy(uri);
}

void MongoHelper::InsertOne(const std::string& collection, bson_t* doc) {
  auto* client = mongoc_client_pool_pop(pool_);
  auto* coll   = mongoc_client_get_collection(client, db_name_.c_str(), collection.c_str());

  bson_error_t error;
  mongoc_collection_insert_one(coll, doc, nullptr, nullptr, &error);

  mongoc_collection_destroy(coll);
  mongoc_client_pool_push(pool_, client);
}

std::vector<std::string> MongoHelper::Find(const std::string& collection, bson_t* filter) {
  auto* client = mongoc_client_pool_pop(pool_);
  auto* coll   = mongoc_client_get_collection(client, db_name_.c_str(), collection.c_str());
  auto* cursor = mongoc_collection_find_with_opts(coll, filter, nullptr, nullptr);

  std::vector<std::string> results;
  const bson_t* doc;
  while (mongoc_cursor_next(cursor, &doc)) {
    size_t len;
    char* str = bson_as_relaxed_extended_json(doc, &len);
    results.emplace_back(str, len);
    bson_free(str);
  }

  mongoc_cursor_destroy(cursor);
  mongoc_collection_destroy(coll);
  mongoc_client_pool_push(pool_, client);
  return results;
}

std::optional<std::string> MongoHelper::FindOne(const std::string& collection, bson_t* filter) {
  auto results = Find(collection, filter);
  if (results.empty()) return std::nullopt;
  return results[0];
}

std::string MongoHelper::GetStr(const bson_t* doc, const char* key) {
  bson_iter_t iter;
  if (bson_iter_init_find(&iter, doc, key) && BSON_ITER_HOLDS_UTF8(&iter))
    return bson_iter_utf8(&iter, nullptr);
  return "";
}

int32_t MongoHelper::GetInt(const bson_t* doc, const char* key) {
  bson_iter_t iter;
  if (bson_iter_init_find(&iter, doc, key) && BSON_ITER_HOLDS_INT32(&iter))
    return bson_iter_int32(&iter);
  return 0;
}

std::string MongoHelper::NowString() {
  time_t t = time(nullptr);
  struct tm tm_info;
  gmtime_r(&t, &tm_info);
  char buf[32];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_info);
  return std::string(buf);
}

}  // namespace social

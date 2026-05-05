#pragma once
#include <mongoc/mongoc.h>
#include <optional>
#include <string>
#include <vector>

namespace social {

// обёртка над libmongoc для работы с MongoDB
class MongoHelper {
 public:
  static MongoHelper& Instance();

  // Вызвать один раз при старте приложения
  void Init(const std::string& uri_str, const std::string& db_name);

  // Вставить документ в коллекцию
  void InsertOne(const std::string& collection, bson_t* doc);

  // Найти все документы по фильтру, вернуть список JSON-строк
  std::vector<std::string> Find(const std::string& collection, bson_t* filter);

  // Найти один документ по фильтру
  std::optional<std::string> FindOne(const std::string& collection, bson_t* filter);

  // Прочитать строковое поле
  static std::string GetStr(const bson_t* doc, const char* key);

  // Прочитать целочисленное поле
  static int32_t GetInt(const bson_t* doc, const char* key);

  // Текущее время в виде строки "YYYY-MM-DD HH:MM:SS"
  static std::string NowString();

 private:
  MongoHelper() = default;
  ~MongoHelper();

  mongoc_client_pool_t* pool_ = nullptr;
  std::string db_name_;
};

}  // namespace social

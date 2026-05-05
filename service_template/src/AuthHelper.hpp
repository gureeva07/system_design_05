#pragma once
#include "Storage.hpp"

#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/http/http_request.hpp>

namespace social {

/// функция проверки авторизации
// вытаскивает токен из заголовка и проверяет его валидность
inline int RequireAuth(const userver::server::http::HttpRequest& req) {
  const std::string& auth = req.GetHeader("Authorization");
  if (auth.size() <= 7 || auth.substr(0, 7) != "Bearer ") {
    throw userver::server::handlers::CustomHandlerException(
        userver::server::handlers::HandlerErrorCode::kUnauthorized,
        userver::server::handlers::ExternalBody{
            "Missing or invalid Authorization header"});
  }
  std::string token = auth.substr(7);

  // проверка токена в хранилище сессий
  auto uid = Storage::Instance().ValidateSession(token);
  if (!uid) {
    throw userver::server::handlers::CustomHandlerException(
        userver::server::handlers::HandlerErrorCode::kUnauthorized,
        userver::server::handlers::ExternalBody{"Invalid or expired token"});
  }
  return *uid;
}

}  // namespace social

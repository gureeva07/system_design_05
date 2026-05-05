#pragma once
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/component.hpp>

namespace social {

// POST /api/v1/auth/register
class RegisterHandler final
    : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-register";

  // получаем доступ к компоненту PostgreSQL
  RegisterHandler(const userver::components::ComponentConfig& config,
                  const userver::components::ComponentContext& context)
      : HttpHandlerBase(config, context),
        pg_(context.FindComponent<userver::components::Postgres>("postgres-db")
                .GetCluster()) {}

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest& req,
      userver::server::request::RequestContext&) const override;

 private:
  userver::storages::postgres::ClusterPtr pg_;
};

// POST /api/v1/auth/login
class LoginHandler final
    : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-login";

  LoginHandler(const userver::components::ComponentConfig& config,
               const userver::components::ComponentContext& context)
      : HttpHandlerBase(config, context),
        pg_(context.FindComponent<userver::components::Postgres>("postgres-db")
                .GetCluster()) {}

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest& req,
      userver::server::request::RequestContext&) const override;

 private:
  userver::storages::postgres::ClusterPtr pg_;
};

// POST /api/v1/auth/logout
class LogoutHandler final
    : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-logout";
  using HttpHandlerBase::HttpHandlerBase;  // сессии — in-memory, pg не нужен

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest& req,
      userver::server::request::RequestContext&) const override;
};

}  // namespace social

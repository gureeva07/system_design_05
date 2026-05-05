#pragma once
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/components/component_context.hpp>

namespace social {

// POST /api/v1/messages - отправить сообщение (MongoDB)
// GET  /api/v1/messages?user_id= - получить сообщения пользователя (MongoDB)
// проверка существования получателя через PostgreSQL
class ChatHandler final
    : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-messages";

  ChatHandler(const userver::components::ComponentConfig& config,
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

}  // namespace social

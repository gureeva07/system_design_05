#pragma once
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/components/component_context.hpp>

namespace social {

// данные хранятся в MongoDB (коллекция users)
class UsersHandler final
    : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-users";
  using HttpHandlerBase::HttpHandlerBase;

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest& req,
      userver::server::request::RequestContext&) const override;
};

}  // namespace social

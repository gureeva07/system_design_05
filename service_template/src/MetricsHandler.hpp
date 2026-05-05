#pragma once
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/components/component_context.hpp>

namespace social {

// отдаю метрики в формате Prometheus, cache hits/misses и rate limit blocks
class MetricsHandler final
    : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-metrics";

  MetricsHandler(const userver::components::ComponentConfig& config,
                 const userver::components::ComponentContext& context)
      : HttpHandlerBase(config, context) {}

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest& req,
      userver::server::request::RequestContext&) const override;
};

}  // namespace social

#include <cstdlib>

#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>
#include <userver/storages/postgres/component.hpp>

#include "MongoHelper.hpp"
#include "Auth/AuthHandlers.hpp"
#include "Chat/ChatHandlers.hpp"
#include "MetricsHandler.hpp"
#include "User/UserHandlers.hpp"
#include "Wall/WallHandlers.hpp"

int main(int argc, char* argv[]) {
  // Инициализция MongoDB до старта userver
  const char* mongo_dsn = std::getenv("MONGO_DSN");
  social::MongoHelper::Instance().Init(
      mongo_dsn ? mongo_dsn : "mongodb://localhost:27017",
      "social_network");

  auto component_list = userver::components::MinimalServerComponentList()
                            .Append<userver::server::handlers::Ping>()
                            .Append<userver::components::TestsuiteSupport>()
                            .Append<userver::components::HttpClient>()
                            .Append<userver::clients::dns::Component>()
                            .Append<userver::server::handlers::TestsControl>()
                            .Append<userver::components::Postgres>("postgres-db")
                            .Append<social::RegisterHandler>()
                            .Append<social::LoginHandler>()
                            .Append<social::LogoutHandler>()
                            .Append<social::UsersHandler>()
                            .Append<social::WallHandler>()
                            .Append<social::ChatHandler>()
                            .Append<social::MetricsHandler>();

  return userver::utils::DaemonMain(argc, argv, component_list);
}

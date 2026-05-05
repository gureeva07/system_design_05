#include "MetricsHandler.hpp"
#include "Cache.hpp"
#include "RateLimiter.hpp"

#include <sstream>

namespace social {

std::string MetricsHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& req,
    userver::server::request::RequestContext&) const {

  auto& cache = Cache::Instance();
  auto& rl = RateLimiter::Instance();

  long hits = cache.GetHits();
  long misses = cache.GetMisses();
  long total = hits + misses;
  long blocked = rl.GetBlocked();

  std::ostringstream out;
  out << "# HELP cache_hits_total Количество запросов из кеша\n";
  out << "# TYPE cache_hits_total counter\n";
  out << "cache_hits_total " << hits << "\n\n";

  out << "# HELP cache_misses_total Количество промахов кеша (запросов в MongoDB)\n";
  out << "# TYPE cache_misses_total counter\n";
  out << "cache_misses_total " << misses << "\n\n";

  out << "# HELP cache_requests_total Всего запросов через кеш\n";
  out << "# TYPE cache_requests_total counter\n";
  out << "cache_requests_total " << total << "\n\n";

  // hit rate считаю здесь же чтобы Grafana могла показать сразу
  out << "# HELP cache_hit_rate Процент попаданий в кеш (0..1)\n";
  out << "# TYPE cache_hit_rate gauge\n";
  out << "cache_hit_rate " << (total > 0 ? (double)hits / total : 0.0) << "\n\n";

  out << "# HELP rate_limit_blocked_total Количество запросов заблокированных rate limiter\n";
  out << "# TYPE rate_limit_blocked_total counter\n";
  out << "rate_limit_blocked_total " << blocked << "\n";

  req.GetHttpResponse().SetHeader(
      std::string("Content-Type"), std::string("text/plain; version=0.0.4"));

  return out.str();
}

}  // namespace social

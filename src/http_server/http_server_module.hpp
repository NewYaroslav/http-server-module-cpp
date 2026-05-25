#pragma once

#include "http_server/backend/i_http_server_backend.hpp"
#include "http_server/config.hpp"
#include "http_server/route.hpp"

#include <memory>

#if __has_include("event_hub.hpp")
namespace event_hub { class EventBus; }
#endif

namespace http_server {

#if __has_include("event_hub.hpp")
template <typename Command, typename Result>
struct EventRouteAdapter;
#endif

/// Public-facing HTTP server module. Owns a backend via pimpl.
class HttpServerModule {
public:
    HttpServerModule();
    explicit HttpServerModule(HttpServerConfig config);
    ~HttpServerModule();

    HttpServerModule(const HttpServerModule&) = delete;
    HttpServerModule& operator=(const HttpServerModule&) = delete;

    HttpServerModule(HttpServerModule&&) noexcept;
    HttpServerModule& operator=(HttpServerModule&&) noexcept;

    void set_config(HttpServerConfig config);
    const HttpServerConfig& config() const noexcept;

    void add_direct_route(HttpRouteConfig config, DirectHandler handler);
    void add_stream_route(HttpStreamRouteConfig config, StreamHandler handler);

#if __has_include("event_hub.hpp")
    template <typename Command, typename Result>
    void add_event_route(HttpRouteConfig config,
                         event_hub::EventBus* bus,
                         EventRouteAdapter<Command, Result> adapter);
#endif

    void start();
    void stop();

    bool is_running() const noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace http_server

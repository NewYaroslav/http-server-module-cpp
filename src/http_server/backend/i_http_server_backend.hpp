#pragma once

#include "http_server/config.hpp"
#include "http_server/route.hpp"

namespace http_server {

/// Backend interface for the HTTP server implementation.
class IHttpServerBackend {
public:
    virtual ~IHttpServerBackend() = default;

    virtual void set_config(HttpServerConfig config) = 0;

    virtual void add_direct_route(HttpRouteConfig config, DirectHandler handler) = 0;
    virtual void add_stream_route(HttpStreamRouteConfig config, StreamHandler handler) = 0;

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual bool is_running() const noexcept = 0;
};

} // namespace http_server

#pragma once

#include "backend/i_http_server_backend.hpp"
#include "config.hpp"
#include "route.hpp"

#include <memory>

namespace http_server {

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

    void start();
    void stop();

    bool is_running() const noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace http_server

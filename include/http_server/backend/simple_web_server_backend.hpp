#pragma once

#include "../stream_session.hpp"
#include "i_http_server_backend.hpp"

#include <atomic>
#include <cstdint>
#include <exception>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace http_server {

/// Concrete HTTP server backend powered by Simple-Web-Server.
class SimpleWebServerBackend : public IHttpServerBackend {
public:
    SimpleWebServerBackend();
    ~SimpleWebServerBackend() override;

    void set_config(HttpServerConfig config) override;
    void add_direct_route(HttpRouteConfig config, DirectHandler handler) override;
    void add_stream_route(HttpStreamRouteConfig config, StreamHandler handler) override;
    void start() override;
    void stop() override;
    bool is_running() const noexcept override;

    std::exception_ptr last_exception() const noexcept {
        std::lock_guard<std::mutex> lock(exception_mutex_);
        return server_exception_;
    }

private:
    struct SwsServerHolder;

    void register_routes();

    HttpRequestContext make_request_context(const std::shared_ptr<void>& sws_request);

    static int to_sws_status_code(HttpStatus status);

    std::unique_ptr<SwsServerHolder> holder_;
    HttpServerConfig config_;
    std::vector<HttpRoute> direct_routes_;
    std::vector<HttpStreamRoute> stream_routes_;
    std::atomic<bool> running_{false};
    std::atomic<bool> started_{false};
    std::thread server_thread_;
    mutable std::mutex exception_mutex_;
    std::exception_ptr server_exception_;
};

/// Concrete streaming session backed by a SimpleWeb Response.
/// The SimpleWeb Response type is held via an opaque shared_ptr<void>
/// to avoid leaking Simple-Web-Server headers into the public API.
class SimpleWebStreamSession : public HttpStreamSession {
public:
    SimpleWebStreamSession(std::shared_ptr<void> response, std::string stream_id);
    ~SimpleWebStreamSession() override;

    std::string id() const override;
    bool is_closed() const override;

    void send_chunk(std::string chunk) override;
    void send_sse(std::string event_name, std::string data) override;
    void send_sse_data(std::string data) override;
    void send_sse_done() override;
    void close() override;

    void set_on_close(CloseCallback callback) override;

private:
    CloseCallback send_next_locked();
    void finish_close_unlocked();

    std::shared_ptr<void> response_;
    std::string stream_id_;

    mutable std::mutex mutex_;
    std::queue<std::string> queue_;
    bool sending_ = false;
    bool close_requested_ = false;
    bool closed_ = false;
    CloseCallback on_close_;
};

} // namespace http_server

#include "http_server/backend/simple_web_server_backend.hpp"
#include "http_server/enums.hpp"
#include "http_server/response_writer.hpp"
#include "http_server/utils/request_id.hpp"

#include <server_http.hpp>

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace http_server {

// ---------------------------------------------------------------------------
// Opaque holder for the SimpleWeb::Server<HTTP> instance.
// ---------------------------------------------------------------------------

struct SimpleWebServerBackend::SwsServerHolder {
    using SwsServer = SimpleWeb::Server<SimpleWeb::HTTP>;
    std::unique_ptr<SwsServer> server;

    SwsServerHolder() : server(std::make_unique<SwsServer>()) {}
};

// ---------------------------------------------------------------------------
// SimpleWebServerBackend
// ---------------------------------------------------------------------------

SimpleWebServerBackend::SimpleWebServerBackend()
    : holder_(std::make_unique<SwsServerHolder>()) {}

SimpleWebServerBackend::~SimpleWebServerBackend() {
    if (running_.load()) {
        stop();
    }
}

void SimpleWebServerBackend::set_config(HttpServerConfig config) {
    config_ = std::move(config);
}

void SimpleWebServerBackend::add_direct_route(HttpRouteConfig config,
                                               DirectHandler handler) {
    direct_routes_.push_back(HttpRoute{std::move(config), std::move(handler)});
}

void SimpleWebServerBackend::add_stream_route(HttpStreamRouteConfig config,
                                               StreamHandler handler) {
    stream_routes_.push_back(
        HttpStreamRoute{std::move(config), std::move(handler)});
}

void SimpleWebServerBackend::start() {
    auto& server = holder_->server;

    server->config.port = config_.port;
    server->config.thread_pool_size = config_.thread_pool_size;
    server->config.timeout_request =
        static_cast<long>(config_.request_timeout.count());
    server->config.timeout_content =
        static_cast<long>(config_.content_timeout.count());
    server->config.max_request_streambuf_size = config_.max_request_size;
    server->config.address = config_.address;
    server->config.reuse_address = config_.reuse_address;

    register_routes();

    running_.store(true);

    // SimpleWeb::Server::start() blocks the calling thread and runs the
    // io_service.  Launch it in a dedicated thread so the caller retains
    // control.
    server_thread_ = std::thread([this] {
        try {
            holder_->server->start();
        } catch (...) {
        }
        running_.store(false);
    });
}

void SimpleWebServerBackend::stop() {
    holder_->server->stop();
    running_.store(false);
    if (server_thread_.joinable()) {
        server_thread_.join();
    }
}

bool SimpleWebServerBackend::is_running() const noexcept {
    return running_.load();
}

void SimpleWebServerBackend::register_routes() {
    using SwsServer = SwsServerHolder::SwsServer;
    auto& server = holder_->server;

    // Direct routes
    for (const auto& route : direct_routes_) {
        if (!route.config.enabled) continue;

        std::string method_str = to_string(route.config.method);
        auto handler = route.handler;

        server->resource[route.config.path_regex][method_str] =
            [this, handler](const std::shared_ptr<SwsServer::Response>& response,
                            const std::shared_ptr<SwsServer::Request>& request) {
                auto ctx = make_request_context(
                    std::static_pointer_cast<void>(request));

                HttpResponseWriter writer(
                    [response](HttpResponseData data) {
                        SimpleWeb::CaseInsensitiveMultimap headers;
                        for (const auto& hdr : data.headers.items()) {
                            headers.emplace(hdr.first, hdr.second);
                        }
                        auto sws_status =
                            static_cast<SimpleWeb::StatusCode>(
                                to_sws_status_code(data.status));
                        response->write(sws_status, data.body, headers);
                    });

                handler(ctx, writer);
            };
    }

    // Stream routes
    for (const auto& route : stream_routes_) {
        if (!route.config.enabled) continue;

        std::string method_str = to_string(route.config.method);
        auto handler = route.handler;
        bool is_sse = route.config.sse;

        server->resource[route.config.path_regex][method_str] =
            [this, handler, is_sse](
                const std::shared_ptr<SwsServer::Response>& response,
                const std::shared_ptr<SwsServer::Request>& request) {
                auto ctx = make_request_context(
                    std::static_pointer_cast<void>(request));

                if (is_sse) {
                    SimpleWeb::CaseInsensitiveMultimap headers;
                    headers.emplace("Content-Type", "text/event-stream");
                    headers.emplace("Cache-Control", "no-cache");
                    headers.emplace("Connection", "keep-alive");
                    response->write(SimpleWeb::StatusCode::success_ok,
                                    std::string(), headers);
                    response->send();
                }

                auto session = std::make_shared<SimpleWebStreamSession>(
                    std::static_pointer_cast<void>(response),
                    ctx.request_id);

                handler(ctx, session);
            };
    }

    // Default resource -- 404 JSON
    auto methods = {"GET", "POST",  "PUT",    "PATCH",
                    "DELETE", "OPTIONS", "HEAD"};
    for (const auto& m : methods) {
        server->default_resource[m] =
            [](const std::shared_ptr<SwsServer::Response>& response,
               const std::shared_ptr<SwsServer::Request>& /*request*/) {
                SimpleWeb::CaseInsensitiveMultimap headers;
                headers.emplace("Content-Type", "application/json");
                response->write(
                    SimpleWeb::StatusCode::client_error_not_found,
                    R"({"error":"not found"})", headers);
            };
    }
}

HttpRequestContext SimpleWebServerBackend::make_request_context(
    const std::shared_ptr<void>& sws_request) {
    using SwsServer = SwsServerHolder::SwsServer;
    const auto& request =
        *std::static_pointer_cast<SwsServer::Request>(sws_request);

    HttpRequestContext ctx;
    ctx.request_id = utils::generate_request_id();
    ctx.method = http_method_from_string(request.method);
    ctx.path = request.path;
    ctx.target = request.path;
    if (!request.query_string.empty()) {
        ctx.target += "?" + request.query_string;
    }
    ctx.query_string = request.query_string;
    ctx.body = request.content.string();

    for (auto it = request.header.begin(); it != request.header.end(); ++it) {
        ctx.headers.add(it->first, it->second);
    }

    auto remote_ep = request.remote_endpoint();
    ctx.remote_address = remote_ep.address().to_string();
    ctx.remote_port = remote_ep.port();

    auto qparams = request.parse_query_string();
    for (auto it = qparams.begin(); it != qparams.end(); ++it) {
        ctx.query_params[it->first] = it->second;
    }

    return ctx;
}

int SimpleWebServerBackend::to_sws_status_code(HttpStatus status) {
    return static_cast<int>(status);
}

// ---------------------------------------------------------------------------
// SimpleWebStreamSession
// ---------------------------------------------------------------------------

SimpleWebStreamSession::SimpleWebStreamSession(
    std::shared_ptr<void> response, std::string stream_id)
    : response_(std::move(response)), stream_id_(std::move(stream_id)) {}

SimpleWebStreamSession::~SimpleWebStreamSession() {
    if (!closed_) {
        close();
    }
}

std::string SimpleWebStreamSession::id() const {
    return stream_id_;
}

bool SimpleWebStreamSession::is_closed() const {
    return closed_;
}

void SimpleWebStreamSession::send_chunk(std::string chunk) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (closed_) return;

    queue_.push(std::move(chunk));
    if (!sending_) {
        sending_ = true;
        send_next_locked();
    }
}

void SimpleWebStreamSession::send_sse(std::string event_name,
                                      std::string data) {
    std::string frame =
        "event: " + event_name + "\ndata: " + data + "\n\n";
    send_chunk(std::move(frame));
}

void SimpleWebStreamSession::send_sse_data(std::string data) {
    std::string frame = "data: " + data + "\n\n";
    send_chunk(std::move(frame));
}

void SimpleWebStreamSession::send_done() {
    std::string frame = "data: [DONE]\n\n";
    send_chunk(std::move(frame));
}

void SimpleWebStreamSession::close() {
    CloseCallback cb;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (closed_) return;
        closed_ = true;

        if (!sending_) {
            response_.reset();
        }

        cb = std::move(on_close_);
        on_close_ = nullptr;
    }

    if (cb) {
        cb(stream_id_);
    }
}

void SimpleWebStreamSession::set_on_close(CloseCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    on_close_ = std::move(callback);
}

void SimpleWebStreamSession::send_next_locked() {
    using SwsServer = SimpleWeb::Server<SimpleWeb::HTTP>;

    if (queue_.empty()) {
        sending_ = false;
        if (closed_) {
            response_.reset();
            CloseCallback cb = std::move(on_close_);
            on_close_ = nullptr;
            mutex_.unlock();
            if (cb) {
                cb(stream_id_);
            }
            mutex_.lock();
        }
        return;
    }

    std::string chunk = std::move(queue_.front());
    queue_.pop();

    auto& response = *std::static_pointer_cast<SwsServer::Response>(response_);
    response << chunk;

    auto self = std::static_pointer_cast<SimpleWebStreamSession>(
        this->shared_from_this());
    response.send([self](const SimpleWeb::error_code& ec) {
        CloseCallback cb;
        {
            std::lock_guard<std::mutex> lock(self->mutex_);
            if (ec) {
                self->closed_ = true;
                self->sending_ = false;
                self->response_.reset();
                cb = std::move(self->on_close_);
                self->on_close_ = nullptr;
            } else {
                self->send_next_locked();
                return;
            }
        }
        if (cb) {
            cb(self->stream_id_);
        }
    });
}

} // namespace http_server

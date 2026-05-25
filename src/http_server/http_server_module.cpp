#include "http_server/http_server_module.hpp"
#include "http_server/backend/simple_web_server_backend.hpp"

#include <memory>
#include <utility>

namespace http_server {

class HttpServerModule::Impl {
public:
    Impl() : backend_(std::make_unique<SimpleWebServerBackend>()) {}

    explicit Impl(HttpServerConfig config)
        : config_(std::move(config)),
          backend_(std::make_unique<SimpleWebServerBackend>()) {
        backend_->set_config(config_);
    }

    void set_config(HttpServerConfig config) {
        config_ = config;
        backend_->set_config(std::move(config));
    }

    const HttpServerConfig& config() const noexcept {
        return config_;
    }

    void add_direct_route(HttpRouteConfig config, DirectHandler handler) {
        backend_->add_direct_route(std::move(config), std::move(handler));
    }

    void add_stream_route(HttpStreamRouteConfig config, StreamHandler handler) {
        backend_->add_stream_route(std::move(config), std::move(handler));
    }

    void start() {
        backend_->start();
    }

    void stop() {
        backend_->stop();
    }

    bool is_running() const noexcept {
        return backend_->is_running();
    }

private:
    HttpServerConfig config_;
    std::unique_ptr<IHttpServerBackend> backend_;
};

// ---------------------------------------------------------------------------
// HttpServerModule -- forwarding to Impl
// ---------------------------------------------------------------------------

HttpServerModule::HttpServerModule() : impl_(std::make_unique<Impl>()) {}

HttpServerModule::HttpServerModule(HttpServerConfig config)
    : impl_(std::make_unique<Impl>(std::move(config))) {}

HttpServerModule::~HttpServerModule() = default;

HttpServerModule::HttpServerModule(HttpServerModule&&) noexcept = default;
HttpServerModule& HttpServerModule::operator=(HttpServerModule&&) noexcept = default;

void HttpServerModule::set_config(HttpServerConfig config) {
    impl_->set_config(std::move(config));
}

const HttpServerConfig& HttpServerModule::config() const noexcept {
    return impl_->config();
}

void HttpServerModule::add_direct_route(HttpRouteConfig config,
                                        DirectHandler handler) {
    impl_->add_direct_route(std::move(config), std::move(handler));
}

void HttpServerModule::add_stream_route(HttpStreamRouteConfig config,
                                        StreamHandler handler) {
    impl_->add_stream_route(std::move(config), std::move(handler));
}

void HttpServerModule::start() {
    impl_->start();
}

void HttpServerModule::stop() {
    impl_->stop();
}

bool HttpServerModule::is_running() const noexcept {
    return impl_->is_running();
}

} // namespace http_server

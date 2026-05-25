#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace http_server {

/// Configuration parameters for the HTTP server.
struct HttpServerConfig {
    std::string address = "0.0.0.0";
    uint16_t port = 8080;
    std::size_t thread_pool_size = 1;
    std::size_t max_request_size = 1024 * 1024 * 16;   ///< 16 MB
    std::size_t max_response_size = 1024 * 1024 * 64;  ///< 64 MB
    std::chrono::seconds request_timeout{30};
    std::chrono::seconds content_timeout{30};
    std::chrono::seconds response_timeout{30};
    bool reuse_address = true;
};

} // namespace http_server

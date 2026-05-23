#pragma once

#include "enums.hpp"
#include "request.hpp"
#include "response.hpp"

#include <functional>
#include <memory>
#include <string>

namespace http_server {

// Forward declarations
class HttpResponseWriter;
class HttpStreamSession;

/// Handler for direct (non-streaming) routes.
using DirectHandler = std::function<void(const HttpRequestContext&, HttpResponseWriter&)>;

/// Handler for streaming routes.
using StreamHandler = std::function<void(const HttpRequestContext&, std::shared_ptr<HttpStreamSession>)>;

/// Configuration for a direct (non-streaming) route.
struct HttpRouteConfig {
    HttpMethod method = HttpMethod::GET;
    std::string path_regex;
    std::string name;
    bool enabled = true;
};

/// Configuration for a streaming route.
struct HttpStreamRouteConfig {
    HttpMethod method = HttpMethod::GET;
    std::string path_regex;
    std::string name;
    bool enabled = true;
    bool sse = true;
};

/// A direct route entry binding config to handler.
struct HttpRoute {
    HttpRouteConfig config;
    DirectHandler handler;
};

/// A streaming route entry binding config to handler.
struct HttpStreamRoute {
    HttpStreamRouteConfig config;
    StreamHandler handler;
};

} // namespace http_server

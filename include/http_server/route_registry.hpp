#pragma once

#include "enums.hpp"
#include "route.hpp"

#include <optional>
#include <regex>
#include <string>
#include <vector>

namespace http_server {

/// Stores and matches HttpRoute and HttpStreamRoute entries.
class RouteRegistry {
public:
    /// Register a direct (non-streaming) route.
    void add_route(HttpRoute route) {
        direct_routes_.push_back(std::move(route));
    }

    /// Register a streaming route.
    void add_stream_route(HttpStreamRoute route) {
        stream_routes_.push_back(std::move(route));
    }

    /// Find the first direct route matching method and path.
    std::optional<HttpRoute> find_direct_route(HttpMethod method,
                                                const std::string& path) const {
        for (const auto& route : direct_routes_) {
            if (route.config.enabled && route.config.method == method) {
                std::regex re(route.config.path_regex);
                if (std::regex_match(path, re)) {
                    return route;
                }
            }
        }
        return std::nullopt;
    }

    /// Find the first streaming route matching method and path.
    std::optional<HttpStreamRoute> find_stream_route(HttpMethod method,
                                                     const std::string& path) const {
        for (const auto& route : stream_routes_) {
            if (route.config.enabled && route.config.method == method) {
                std::regex re(route.config.path_regex);
                if (std::regex_match(path, re)) {
                    return route;
                }
            }
        }
        return std::nullopt;
    }

private:
    std::vector<HttpRoute> direct_routes_;
    std::vector<HttpStreamRoute> stream_routes_;
};

} // namespace http_server

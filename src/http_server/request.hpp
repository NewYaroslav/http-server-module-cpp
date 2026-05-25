#pragma once

#include "http_server/enums.hpp"
#include "http_server/headers.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace http_server {

/// Parsed HTTP request context passed to handlers.
struct HttpRequestContext {
    std::string request_id;
    HttpMethod method = HttpMethod::GET;
    std::string method_raw;
    std::string path;
    std::string target;
    std::string query_string;
    HttpHeaders headers;
    std::string body;
    std::string remote_address;
    uint16_t remote_port = 0;
    std::unordered_map<std::string, std::string> path_params;
    std::unordered_map<std::string, std::string> query_params;
};

} // namespace http_server

#pragma once

#include "http_server/enums.hpp"
#include "http_server/headers.hpp"

#include <string>

namespace http_server {

/// HTTP response data produced by handlers.
struct HttpResponseData {
    HttpStatus status = HttpStatus::ok;
    HttpHeaders headers;
    std::string body;

    /// Create a plain-text response.
    static HttpResponseData text(HttpStatus status, std::string body) {
        HttpResponseData resp;
        resp.status = status;
        resp.body = std::move(body);
        resp.headers.set("Content-Type", "text/plain; charset=utf-8");
        return resp;
    }

    /// Create a JSON response.
    static HttpResponseData json(HttpStatus status, std::string body) {
        HttpResponseData resp;
        resp.status = status;
        resp.body = std::move(body);
        resp.headers.set("Content-Type", "application/json");
        return resp;
    }

    /// Create an empty response (no body).
    static HttpResponseData empty(HttpStatus status) {
        HttpResponseData resp;
        resp.status = status;
        return resp;
    }
};

} // namespace http_server

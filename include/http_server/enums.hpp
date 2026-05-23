#pragma once

#include <string>
#include <string_view>

namespace http_server {

/// HTTP request methods.
enum class HttpMethod {
    GET,
    POST,
    PUT,
    PATCH,
    DELETE_,
    OPTIONS,
    HEAD
};

/// Convert an HttpMethod to its standard string representation.
inline std::string to_string(HttpMethod method) {
    switch (method) {
        case HttpMethod::GET:     return "GET";
        case HttpMethod::POST:    return "POST";
        case HttpMethod::PUT:     return "PUT";
        case HttpMethod::PATCH:   return "PATCH";
        case HttpMethod::DELETE_: return "DELETE";
        case HttpMethod::OPTIONS: return "OPTIONS";
        case HttpMethod::HEAD:    return "HEAD";
    }
    return "UNKNOWN";
}

/// Parse a string into an HttpMethod. Returns HttpMethod::GET on failure.
inline HttpMethod http_method_from_string(std::string_view str) {
    if (str == "GET")     return HttpMethod::GET;
    if (str == "POST")    return HttpMethod::POST;
    if (str == "PUT")     return HttpMethod::PUT;
    if (str == "PATCH")   return HttpMethod::PATCH;
    if (str == "DELETE")  return HttpMethod::DELETE_;
    if (str == "OPTIONS") return HttpMethod::OPTIONS;
    if (str == "HEAD")    return HttpMethod::HEAD;
    return HttpMethod::GET;
}

/// HTTP response status codes with semantic names.
enum class HttpStatus {
    ok                    = 200,
    created               = 201,
    accepted              = 202,
    no_content            = 204,
    bad_request           = 400,
    unauthorized          = 401,
    forbidden             = 403,
    not_found             = 404,
    method_not_allowed    = 405,
    request_timeout       = 408,
    payload_too_large    = 413,
    internal_server_error = 500,
    bad_gateway           = 502,
    service_unavailable   = 503,
    gateway_timeout       = 504
};

/// Return the numeric status code for an HttpStatus value.
inline int to_status_code(HttpStatus status) {
    return static_cast<int>(status);
}

/// Return the standard reason phrase for an HttpStatus value.
inline std::string reason_phrase(HttpStatus status) {
    switch (status) {
        case HttpStatus::ok:                    return "OK";
        case HttpStatus::created:               return "Created";
        case HttpStatus::accepted:              return "Accepted";
        case HttpStatus::no_content:            return "No Content";
        case HttpStatus::bad_request:           return "Bad Request";
        case HttpStatus::unauthorized:          return "Unauthorized";
        case HttpStatus::forbidden:             return "Forbidden";
        case HttpStatus::not_found:             return "Not Found";
        case HttpStatus::method_not_allowed:    return "Method Not Allowed";
        case HttpStatus::request_timeout:       return "Request Timeout";
        case HttpStatus::payload_too_large:     return "Payload Too Large";
        case HttpStatus::internal_server_error: return "Internal Server Error";
        case HttpStatus::bad_gateway:           return "Bad Gateway";
        case HttpStatus::service_unavailable:   return "Service Unavailable";
        case HttpStatus::gateway_timeout:       return "Gateway Timeout";
    }
    return "Unknown";
}

} // namespace http_server

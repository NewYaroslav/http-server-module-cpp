#pragma once

#include <cstdint>
#include <string>

namespace http_server {

/// Emitted when the HTTP server has successfully started listening.
struct HttpServerStartedEvent {
    std::string address;
    uint16_t port = 0;
};

/// Emitted when the HTTP server has stopped.
struct HttpServerStoppedEvent {};

/// Emitted when a new HTTP request has been received.
struct HttpRequestReceivedEvent {
    std::string request_id;
    std::string method;
    std::string path;
};

/// Emitted when an HTTP request has been fully handled.
struct HttpRequestCompletedEvent {
    std::string request_id;
    int status_code = 0;
};

/// Emitted when handling an HTTP request resulted in an error.
struct HttpRequestFailedEvent {
    std::string request_id;
    int status_code = 500;
    std::string error;
};

/// Emitted when a stream session has been opened.
struct HttpStreamOpenedEvent {
    std::string request_id;
    std::string stream_id;
    std::string path;
};

/// Emitted when a stream session has been closed.
struct HttpStreamClosedEvent {
    std::string request_id;
    std::string stream_id;
    std::string reason;
};

} // namespace http_server

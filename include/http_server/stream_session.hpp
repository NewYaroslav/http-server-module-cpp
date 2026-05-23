#pragma once

#include <functional>
#include <memory>
#include <string>

namespace http_server {

/// Abstract interface for a streaming session (SSE or chunked).
/// Users receive a shared_ptr to a concrete subclass.
class HttpStreamSession : public std::enable_shared_from_this<HttpStreamSession> {
public:
    using CloseCallback = std::function<void(const std::string& stream_id)>;

    virtual ~HttpStreamSession() = default;

    virtual std::string id() const = 0;
    virtual bool is_closed() const = 0;

    virtual void send_chunk(std::string chunk) = 0;
    virtual void send_sse(std::string event_name, std::string data) = 0;
    virtual void send_sse_data(std::string data) = 0;
    virtual void send_sse_done() = 0;
    virtual void close() = 0;

    virtual void set_on_close(CloseCallback callback) = 0;
};

} // namespace http_server

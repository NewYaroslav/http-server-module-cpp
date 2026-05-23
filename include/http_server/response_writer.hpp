#pragma once

#include "enums.hpp"
#include "response.hpp"

#include <cassert>
#include <functional>

namespace http_server {

/// Wraps the send callback so a response can be sent exactly once.
class HttpResponseWriter {
public:
    using SendCallback = std::function<void(HttpResponseData)>;

    explicit HttpResponseWriter(SendCallback send_callback)
        : send_callback_(std::move(send_callback)) {}

    /// Send a response. Only the first call takes effect.
    void send(HttpResponseData response) {
        if (sent_) {
            assert(!"HttpResponseWriter::send called more than once");
            return;
        }
        sent_ = true;
        if (send_callback_) {
            send_callback_(std::move(response));
        }
    }

    /// Convenience: send a plain-text response.
    void send_text(HttpStatus status, std::string body) {
        send(HttpResponseData::text(status, std::move(body)));
    }

    /// Convenience: send a JSON response.
    void send_json(HttpStatus status, std::string body) {
        send(HttpResponseData::json(status, std::move(body)));
    }

    /// Convenience: send an empty response.
    void send_empty(HttpStatus status) {
        send(HttpResponseData::empty(status));
    }

    /// Whether a response has already been sent.
    bool is_sent() const noexcept {
        return sent_;
    }

private:
    SendCallback send_callback_;
    bool sent_ = false;
};

} // namespace http_server

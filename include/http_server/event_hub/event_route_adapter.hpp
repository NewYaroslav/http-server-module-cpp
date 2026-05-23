#pragma once

/// \file event_route_adapter.hpp
/// \brief Optional adapter for routing HTTP requests through an event-hub-cpp bus.
///
/// This header is only available when http-server-module-cpp is built with
/// event-hub-cpp support (HTTP_SERVER_MODULE_USE_EVENT_HUB).

#include "../route.hpp"
#include "../request.hpp"
#include "../response.hpp"
#include "../response_writer.hpp"

#include <event_hub.hpp>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

namespace http_server {

/// Dispatch mode for event routes.
enum class EventDispatchMode {
    emit_sync,  ///< Synchronously dispatch via event_hub::EventBus::emit().
    post_async  ///< Queue the command via event_hub::EventBus::post().
};

/// Adapts an HTTP direct route to an event-hub request/response cycle.
///
/// \tparam Command  Event type published to the bus (must derive from event_hub::Event).
/// \tparam Result   Event type awaited from the bus (must derive from event_hub::Event).
template <typename Command, typename Result>
struct EventRouteAdapter {
    /// Create a Command from the HTTP request context.
    std::function<Command(const HttpRequestContext&)> make_command;

    /// Check whether a received Result belongs to the issued Command.
    std::function<bool(const Result&, const Command&)> match_result;

    /// Convert a matching Result into an HTTP response.
    std::function<HttpResponseData(const Result&)> make_response;

    /// Maximum time to wait for the Result.
    std::chrono::milliseconds timeout{3000};
};

namespace detail {

/// Helper that generates a correlation id string.
inline std::string make_correlation_id() {
    static std::atomic<uint64_t> counter{0};
    return "evr-" + std::to_string(++counter);
}

} // namespace detail

} // namespace http_server

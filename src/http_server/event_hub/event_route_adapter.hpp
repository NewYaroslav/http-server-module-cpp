#pragma once

/// \file event_route_adapter.hpp
/// \brief Optional adapter for routing HTTP requests through an event-hub-cpp bus.
///
/// This header is only available when http-server-module-cpp is built with
/// event-hub-cpp support (HTTP_SERVER_MODULE_USE_EVENT_HUB).

#include "http_server/request.hpp"
#include "http_server/response.hpp"
#include "http_server/response_writer.hpp"
#include "http_server/route.hpp"

#include <event_hub.hpp>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <future>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

namespace http_server {

/// Adapts an HTTP direct route to an event-hub request/response cycle.
///
/// \tparam Command  Event type published to the bus.
/// \tparam Result   Event type awaited from the bus.
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

/// Blocking direct handler that routes through an event bus.
template <typename Command, typename Result>
DirectHandler make_event_route_handler(
    event_hub::EventBus* bus,
    EventRouteAdapter<Command, Result> adapter) {
    return [bus, adapter](const HttpRequestContext& ctx,
                          HttpResponseWriter& writer) {
        if (!bus) {
            writer.send_json(
                HttpStatus::internal_server_error,
                R"({"error":"event bus is null"})");
            return;
        }
        try {
            Command cmd = adapter.make_command(ctx);

            std::promise<Result> promise;
            std::future<Result> future = promise.get_future();

            auto awaiter = event_hub::EventAwaiter<Result>::create(
                *bus,
                [&cmd, &adapter](const Result& res) {
                    return adapter.match_result(res, cmd);
                },
                [&promise](const Result& res) {
                    promise.set_value(res);
                },
                event_hub::AwaitOptions::timeout_ms(
                    static_cast<std::int64_t>(adapter.timeout.count())),
                true);

            bus->emit(cmd);

            if (future.wait_for(adapter.timeout) ==
                std::future_status::ready) {
                Result res = future.get();
                writer.send(adapter.make_response(res));
            } else {
                awaiter->cancel();
                if (!writer.is_sent()) {
                    writer.send_json(
                        HttpStatus::gateway_timeout,
                        R"({"error":"gateway timeout"})");
                }
            }
        } catch (...) {
            if (!writer.is_sent()) {
                writer.send_json(
                    HttpStatus::internal_server_error,
                    R"({"error":"event route handler failed"})");
            }
        }
    };
}

} // namespace detail

// ---------------------------------------------------------------------------
// HttpServerModule::add_event_route inline definition
// ---------------------------------------------------------------------------

/// Register a direct route that routes requests through an event hub bus.
///
/// \tparam Command Event type to publish.
/// \tparam Result  Event type to await.
/// \param  config  Route configuration (method, path regex, name, ...).
/// \param  bus     Non-owning pointer to the event bus; must outlive the route.
/// \param  adapter Adapter with make_command, match_result, make_response.
template <typename Command, typename Result>
void HttpServerModule::add_event_route(
    HttpRouteConfig config,
    event_hub::EventBus* bus,
    EventRouteAdapter<Command, Result> adapter) {
    add_direct_route(std::move(config),
                     detail::make_event_route_handler(bus,
                                                       std::move(adapter)));
}

} // namespace http_server

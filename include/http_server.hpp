#pragma once

/// HTTP Server Module -- umbrella header.
/// Include this single header to get the full public API.

#include "http_server/version.hpp"
#include "http_server/config.hpp"
#include "http_server/enums.hpp"
#include "http_server/headers.hpp"
#include "http_server/request.hpp"
#include "http_server/response.hpp"
#include "http_server/route.hpp"
#include "http_server/route_registry.hpp"
#include "http_server/response_writer.hpp"
#include "http_server/stream_session.hpp"
#include "http_server/backend/i_http_server_backend.hpp"
#include "http_server/http_server_module.hpp"
#include "http_server/events/http_server_events.hpp"

#if __has_include("event_hub.hpp")
#include "http_server/event_hub/event_route_adapter.hpp"
#endif

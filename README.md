# http-server-module-cpp

Universal C++ HTTP server module with direct, event-driven, and streaming routes.

The library wraps an HTTP server backend and provides route registration,
request/response handling, streaming responses, and optional integration with
[event-hub-cpp](https://github.com/NewYaroslav/event-hub-cpp)-style modular architectures.

## Features

- **Direct routes** — register a callback that receives `HttpRequestContext` and writes an `HttpResponseWriter`.
- **Stream / SSE routes** — open a persistent session and push chunks or server-sent events.
- **Backend abstraction** — public API does not leak `Simple-Web-Server` (or any other backend) types.
- **Optional event-hub integration** — route HTTP requests through an `event_hub::EventBus` for modular, event-driven designs.
- **Header-friendly** — mostly header-only; only the backend implementation is compiled.
- **C++17** — no unstable language extensions.

## Requirements

- C++17 compiler
- [Asio](https://think-async.com/Asio) (standalone or Boost.Asio)
- [Simple-Web-Server](https://gitlab.com/eidheim/Simple-Web-Server)
- *(Optional)* [event-hub-cpp](https://github.com/NewYaroslav/event-hub-cpp)

All dependencies are expected as Git submodules under `external/`.

## Installation as a submodule

```bash
git submodule add https://github.com/NewYaroslav/http-server-module-cpp.git external/http-server-module-cpp
```

Then in your `CMakeLists.txt`:

```cmake
add_subdirectory(external/http-server-module-cpp)
target_link_libraries(your_target PRIVATE http_server_module::http_server_module)
```

## CMake options

| Option | Default | Description |
|--------|---------|-------------|
| `HTTP_SERVER_MODULE_BUILD_EXAMPLES` | `ON` when top-level | Build example executables |
| `HTTP_SERVER_MODULE_BUILD_TESTS` | `ON` when top-level | Build test executables |
| `HTTP_SERVER_MODULE_USE_EVENT_HUB` | `ON` | Enable event-hub-cpp integration |
| `HTTP_SERVER_MODULE_USE_STANDALONE_ASIO` | `ON` | Use standalone Asio instead of Boost.Asio |
| `HTTP_SERVER_MODULE_USE_EXTERNAL_DEPS` | `ON` | Use dependencies from `external/` subdirectory |

## Direct route example

```cpp
#include <http_server.hpp>
#include <iostream>

int main() {
    http_server::HttpServerConfig config;
    config.address = "0.0.0.0";
    config.port = 8080;

    http_server::HttpServerModule server(config);

    server.add_direct_route(
        {http_server::HttpMethod::GET, R"(^/health$)", "health"},
        [](const http_server::HttpRequestContext& ctx,
           http_server::HttpResponseWriter& res) {
            (void)ctx;
            res.send_json(
                http_server::HttpStatus::ok,
                R"({"status":"ok"})"
            );
        }
    );

    server.start();
    std::cin.get();
    server.stop();
}
```

## Stream / SSE example

```cpp
#include <http_server.hpp>
#include <chrono>
#include <thread>

int main() {
    http_server::HttpServerModule server;

    server.add_stream_route(
        {http_server::HttpMethod::GET, R"(^/events$)", "events", true, true},
        [](const http_server::HttpRequestContext& ctx,
           std::shared_ptr<http_server::HttpStreamSession> stream) {
            (void)ctx;
            stream->send_sse("connected", R"({"ok":true})");

            std::thread([stream] {
                for (int i = 0; i < 5; ++i) {
                    stream->send_sse("tick",
                        std::string("{\"value\":") + std::to_string(i) + "}");
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
                stream->send_sse_done();
                stream->close();
            }).detach();
        }
    );

    server.start();
    std::cin.get();
    server.stop();
}
```

## Event-hub route concept

When `HTTP_SERVER_MODULE_USE_EVENT_HUB` is enabled you can route an HTTP request
through the event bus:

1. HTTP request arrives.
2. An adapter creates a typed `Command` event from the request context.
3. The command is published to `event_hub::EventBus`.
4. The adapter awaits a matching `Result` event.
5. A `make_response` callback converts the result into `HttpResponseData`.
6. On timeout the adapter returns `504 Gateway Timeout`.

This keeps the HTTP layer decoupled from business logic.

**Note:** `EventRouteAdapter` and the event-hub headers are currently an
**experimental scaffold**. The full `add_event_route()` implementation with
`event_hub::EventBus::await_once` integration is planned for a follow-up PR.

## Backend notes

The default backend is **Simple-Web-Server**. The public API does not expose
`SimpleWeb::Server`, `SimpleWeb::Request`, or `SimpleWeb::Response`. In the
future a Boost.Beast, Crow, or Drogon backend can be added by implementing
`IHttpServerBackend`.

## Limitations

- No WebSocket support in the first version.
- No HTTP/2 or TLS configuration beyond what the backend provides.
- No built-in auth framework or rate limiter.
- No OpenAPI generator.
- Incoming request bodies are collected by the backend before the handler is invoked.

## License

MIT

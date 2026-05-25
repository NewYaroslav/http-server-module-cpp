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

## Named path parameters

Regex capture groups can be mapped to human-readable names via `path_param_names`:

```cpp
server.add_direct_route(
    {http_server::HttpMethod::GET,
     R"(^/users/([0-9]+)/posts/([0-9]+)$)",
     "user_post",
     true,
     {"user_id", "post_id"}},
    [](const http_server::HttpRequestContext& ctx,
       http_server::HttpResponseWriter& res) {
        auto u = ctx.path_params.find("user_id");
        auto p = ctx.path_params.find("post_id");
        res.send_json(
            http_server::HttpStatus::ok,
            R"({"user":")" + u->second + R"(","post":")" + p->second + R"("})"
        );
    }
);
```

When `path_param_names` is omitted or shorter than the number of capture groups,
the module falls back to numeric keys `"1"`, `"2"`, etc.

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
        {http_server::HttpMethod::GET, R"(^/events$)", "events", true, http_server::StreamMode::sse},
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

## Event-hub route example

When `HTTP_SERVER_MODULE_USE_EVENT_HUB` is enabled you can route HTTP requests
through an `event_hub::EventBus`:

```cpp
#include <http_server.hpp>
#include <event_hub.hpp>
#include <iostream>

struct EchoCommand { std::string id; std::string body; };
struct EchoResult  { std::string id; std::string body; };

int main() {
    http_server::HttpServerModule server;
    event_hub::EventBus bus;

    bus.subscribe<EchoCommand>(
        &bus,
        [&bus](const EchoCommand& cmd) {
            EchoResult res{cmd.id, cmd.body};
            bus.emit(res);
        });

    http_server::EventRouteAdapter<EchoCommand, EchoResult> adapter;
    adapter.make_command = [](const auto& ctx) -> EchoCommand {
        return {ctx.request_id, ctx.body};
    };
    adapter.match_result = [](const EchoResult& r, const EchoCommand& c) {
        return r.id == c.id;
    };
    adapter.make_response = [](const EchoResult& r) {
        return http_server::HttpResponseData::text(
            http_server::HttpStatus::ok, r.body);
    };
    adapter.timeout = std::chrono::milliseconds(1000);

    server.add_event_route(
        {http_server::HttpMethod::POST, R"(^/echo$)", "echo"},
        &bus,
        adapter);

    server.start();
    std::cin.get();
    server.stop();
}
```

Flow:
1. HTTP request arrives.
2. `make_command` creates a typed `Command` event from the request context.
3. The command is emitted synchronously to `event_hub::EventBus`.
4. `EventAwaiter` waits for a matching `Result` event.
5. `make_response` converts the result into `HttpResponseData`.
6. On timeout the adapter returns `504 Gateway Timeout`.

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
- `max_response_size` and `response_timeout` are currently backend-level
  Simple-Web-Server properties; they are not exposed as generic
  `HttpServerConfig` fields because they are not portable across future
  backends (e.g. Boost.Beast or Crow).

## License

MIT

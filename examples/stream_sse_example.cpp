#include <http_server.hpp>

#include <chrono>
#include <iostream>
#include <thread>

int main() {
    http_server::HttpServerConfig config;
    config.address = "0.0.0.0";
    config.port = 8080;

    http_server::HttpServerModule server(config);

    server.add_stream_route(
        {http_server::HttpMethod::GET, R"(^/events$)", "events", true, true},
        [](const http_server::HttpRequestContext& ctx,
           std::shared_ptr<http_server::HttpStreamSession> stream) {
            (void)ctx;
            stream->send_sse("connected", R"({"ok":true})");

            std::thread([stream] {
                for (int i = 0; i < 5; ++i) {
                    stream->send_sse(
                        "tick",
                        std::string("{\"value\":") + std::to_string(i) + "}"
                    );
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
                stream->send_done();
                stream->close();
            }).detach();
        }
    );

    std::cout << "Starting SSE server on " << config.address << ":" << config.port << "\n";
    server.start();

    std::cout << "Press Enter to stop...\n";
    std::cin.get();

    server.stop();
    return 0;
}

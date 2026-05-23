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

    server.add_direct_route(
        {http_server::HttpMethod::GET, R"(^/hello/([^/]+)$)", "hello"},
        [](const http_server::HttpRequestContext& ctx,
           http_server::HttpResponseWriter& res) {
            std::string name = "world";
            if (!ctx.query_params.empty()) {
                auto it = ctx.query_params.find("name");
                if (it != ctx.query_params.end()) {
                    name = it->second;
                }
            }
            res.send_json(
                http_server::HttpStatus::ok,
                std::string("{\"message\":\"Hello, ") + name + "!\"}"
            );
        }
    );

    std::cout << "Starting HTTP server on " << config.address << ":" << config.port << "\n";
    server.start();

    std::cout << "Press Enter to stop...\n";
    std::cin.get();

    server.stop();
    return 0;
}

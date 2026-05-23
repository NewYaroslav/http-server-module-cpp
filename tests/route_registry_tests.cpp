#include <http_server/route_registry.hpp>
#include <cassert>
#include <iostream>

using namespace http_server;

void test_add_and_find() {
    RouteRegistry reg;
    reg.add_route(HttpRoute{
        {HttpMethod::GET, R"(^/health$)", "health"},
        [](const HttpRequestContext&, HttpResponseWriter&) {}
    });

    auto found = reg.find_direct_route(HttpMethod::GET, "/health");
    assert(found.has_value());
    assert(found->config.name == "health");

    auto not_found = reg.find_direct_route(HttpMethod::GET, "/missing");
    assert(!not_found.has_value());

    std::cout << "test_add_and_find: OK\n";
}

void test_method_mismatch() {
    RouteRegistry reg;
    reg.add_route(HttpRoute{
        {HttpMethod::GET, R"(^/health$)", "health"},
        [](const HttpRequestContext&, HttpResponseWriter&) {}
    });

    auto found = reg.find_direct_route(HttpMethod::POST, "/health");
    assert(!found.has_value());

    std::cout << "test_method_mismatch: OK\n";
}

void test_stream_route() {
    RouteRegistry reg;
    reg.add_stream_route(HttpStreamRoute{
        {HttpMethod::GET, R"(^/events$)", "events", true, StreamMode::sse},
        [](const HttpRequestContext&, std::shared_ptr<HttpStreamSession>) {}
    });

    auto found = reg.find_stream_route(HttpMethod::GET, "/events");
    assert(found.has_value());
    assert(found->config.name == "events");

    auto not_found = reg.find_stream_route(HttpMethod::POST, "/events");
    assert(!not_found.has_value());

    std::cout << "test_stream_route: OK\n";
}

int main() {
    test_add_and_find();
    test_method_mismatch();
    test_stream_route();
    std::cout << "All route_registry_tests passed.\n";
    return 0;
}

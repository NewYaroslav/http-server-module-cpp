#include <http_server.hpp>
#include <http_server/enums.hpp>
#include <http_server/config.hpp>

#include <client_http.hpp>
#include <asio.hpp>

#include <atomic>
#include <cassert>
#include <chrono>
#include <future>
#include <iostream>
#include <string>
#include <thread>

using namespace http_server;

uint16_t find_free_port() {
    asio::io_context io;
    asio::ip::tcp::acceptor acceptor(io);
    acceptor.open(asio::ip::tcp::v4());
    acceptor.bind({asio::ip::make_address("127.0.0.1"), 0});
    uint16_t p = acceptor.local_endpoint().port();
    acceptor.close();
    return p;
}

void wait_for_server(const HttpServerModule& server,
                     std::chrono::milliseconds timeout = std::chrono::milliseconds(2000)) {
    auto start = std::chrono::steady_clock::now();
    while (!server.is_running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (std::chrono::steady_clock::now() - start > timeout) {
            assert(!"Server failed to start within timeout");
            return;
        }
    }
}

std::shared_ptr<SimpleWeb::Client<SimpleWeb::HTTP>::Response> do_request(
    const std::string& method,
    const std::string& path,
    uint16_t port,
    const std::string& body = "") {

    SimpleWeb::Client<SimpleWeb::HTTP> client("127.0.0.1:" + std::to_string(port));

    if (method == "GET") {
        return client.request("GET", path);
    }
    if (method == "POST") {
        if (body.empty()) {
            return client.request("POST", path);
        }
        return client.request("POST", path, body);
    }
    return client.request(method, path);
}

struct AsyncResponse {
    int status_code = 0;
    SimpleWeb::CaseInsensitiveMultimap headers;
    std::string body;
};

AsyncResponse do_sse_request(uint16_t port, const std::string& path) {
    SimpleWeb::Client<SimpleWeb::HTTP> client("127.0.0.1:" + std::to_string(port));
    client.io_service = std::make_shared<asio::io_context>();

    AsyncResponse result;
    std::promise<void> done;
    bool headers_captured = false;
    std::atomic<bool> finished{false};

    client.request("GET", path,
        [&result, &done, &finished, &headers_captured](
            std::shared_ptr<SimpleWeb::Client<SimpleWeb::HTTP>::Response> response,
            const SimpleWeb::error_code& ec) {
            if (!ec) {
                if (!headers_captured) {
                    headers_captured = true;
                    result.status_code = std::stoi(response->status_code);
                    result.headers = response->header;
                }
                result.body += response->content.string();
            } else {
                if (!headers_captured) {
                    headers_captured = true;
                    try {
                        result.status_code = std::stoi(response->status_code);
                    } catch (...) {
                        result.status_code = 0;
                    }
                    result.headers = response->header;
                }
                if (!finished.exchange(true)) {
                    done.set_value();
                }
            }
        });

    asio::steady_timer timer(*client.io_service);
    timer.expires_after(std::chrono::milliseconds(300));
    timer.async_wait([&client, &done, &finished](const SimpleWeb::error_code& ec) {
        if (!ec) {
            client.stop();
            if (!finished.exchange(true)) {
                done.set_value();
            }
        }
    });

    std::thread io_thread([io = client.io_service]() { io->run(); });

    done.get_future().get();
    io_thread.join();

    return result;
}

std::string do_raw_request(uint16_t port, const std::string& raw_request) {
    asio::io_context io;
    asio::ip::tcp::socket socket(io);
    asio::ip::tcp::resolver resolver(io);
    auto endpoints = resolver.resolve("127.0.0.1", std::to_string(port));
    asio::connect(socket, endpoints);

    asio::write(socket, asio::buffer(raw_request));

    std::string response;
    char buf[1024];
    std::error_code ec;
    while (true) {
        std::size_t len = socket.read_some(asio::buffer(buf), ec);
        if (ec == asio::error::eof) {
            break;
        }
        if (ec) {
            break;
        }
        response.append(buf, len);
    }
    socket.close();
    return response;
}

void test_direct_health() {
    uint16_t port = find_free_port();
    HttpServerConfig config;
    config.address = "127.0.0.1";
    config.port = port;
    HttpServerModule server(config);

    server.add_direct_route(
        {HttpMethod::GET, R"(^/health$)", "health"},
        [](const HttpRequestContext& ctx, HttpResponseWriter& res) {
            (void)ctx;
            res.send_json(HttpStatus::ok, R"({"status":"ok"})");
        });

    server.start();
    wait_for_server(server);

    auto resp = do_request("GET", "/health", port);
    assert(std::stoi(resp->status_code) == 200);
    assert(resp->content.string() == R"({"status":"ok"})");

    auto it = resp->header.find("Content-Type");
    assert(it != resp->header.end());
    assert(it->second.find("application/json") != std::string::npos);

    server.stop();
    std::cout << "test_direct_health: OK\n";
}

void test_direct_404() {
    uint16_t port = find_free_port();
    HttpServerConfig config;
    config.address = "127.0.0.1";
    config.port = port;
    HttpServerModule server(config);

    server.start();
    wait_for_server(server);

    auto resp = do_request("GET", "/missing", port);
    assert(std::stoi(resp->status_code) == 404);
    assert(resp->content.string().find("\"error\"") != std::string::npos);

    server.stop();
    std::cout << "test_direct_404: OK\n";
}

void test_direct_silent_handler_500() {
    uint16_t port = find_free_port();
    HttpServerConfig config;
    config.address = "127.0.0.1";
    config.port = port;
    HttpServerModule server(config);

    server.add_direct_route(
        {HttpMethod::GET, R"(^/silent$)", "silent"},
        [](const HttpRequestContext& ctx, HttpResponseWriter& res) {
            (void)ctx;
            (void)res;
        });

    server.start();
    wait_for_server(server);

    auto resp = do_request("GET", "/silent", port);
    assert(std::stoi(resp->status_code) == 500);
    assert(resp->content.string().find("handler did not send a response") != std::string::npos);

    server.stop();
    std::cout << "test_direct_silent_handler_500: OK\n";
}

void test_direct_post_echo() {
    uint16_t port = find_free_port();
    HttpServerConfig config;
    config.address = "127.0.0.1";
    config.port = port;
    HttpServerModule server(config);

    server.add_direct_route(
        {HttpMethod::POST, R"(^/echo$)", "echo"},
        [](const HttpRequestContext& ctx, HttpResponseWriter& res) {
            res.send_json(HttpStatus::ok, ctx.body);
        });

    server.start();
    wait_for_server(server);

    auto resp = do_request("POST", "/echo", port, "hello");
    assert(std::stoi(resp->status_code) == 200);
    assert(resp->content.string() == "hello");

    server.stop();
    std::cout << "test_direct_post_echo: OK\n";
}

void test_direct_path_params() {
    uint16_t port = find_free_port();
    HttpServerConfig config;
    config.address = "127.0.0.1";
    config.port = port;
    HttpServerModule server(config);

    server.add_direct_route(
        {HttpMethod::GET, R"(^/users/([0-9]+)$)", "user", true, {"user_id"}},
        [](const HttpRequestContext& ctx, HttpResponseWriter& res) {
            auto it = ctx.path_params.find("user_id");
            if (it != ctx.path_params.end()) {
                res.send_json(HttpStatus::ok, R"({"id":""" + it->second + R"("})");
            } else {
                res.send_json(HttpStatus::bad_request, R"({"error":"missing id"})");
            }
        });

    server.start();
    wait_for_server(server);

    auto resp = do_request("GET", "/users/42", port);
    assert(std::stoi(resp->status_code) == 200);
    assert(resp->content.string() == R"({"id":"42"})");

    server.stop();
    std::cout << "test_direct_path_params: OK\n";
}

void test_stream_sse_headers() {
    uint16_t port = find_free_port();
    HttpServerConfig config;
    config.address = "127.0.0.1";
    config.port = port;
    HttpServerModule server(config);

    server.add_stream_route(
        {HttpMethod::GET, R"(^/events$)", "events", true, StreamMode::sse},
        [](const HttpRequestContext& ctx, std::shared_ptr<HttpStreamSession> stream) {
            (void)ctx;
            stream->send_sse("connected", R"({"ok":true})");
            stream->send_sse_done();
            stream->close();
        });

    server.start();
    wait_for_server(server);

    auto resp = do_sse_request(port, "/events");
    assert(resp.status_code == 200);

    auto it = resp.headers.find("Content-Type");
    assert(it != resp.headers.end());
    assert(it->second.find("text/event-stream") != std::string::npos);

    it = resp.headers.find("Cache-Control");
    assert(it != resp.headers.end());
    assert(it->second == "no-cache");

    it = resp.headers.find("Connection");
    assert(it != resp.headers.end());
    assert(it->second == "keep-alive");

    server.stop();
    std::cout << "test_stream_sse_headers: OK\n";
}

void test_stream_sse_content() {
    uint16_t port = find_free_port();
    HttpServerConfig config;
    config.address = "127.0.0.1";
    config.port = port;
    HttpServerModule server(config);

    server.add_stream_route(
        {HttpMethod::GET, R"(^/events$)", "events", true, StreamMode::sse},
        [](const HttpRequestContext& ctx, std::shared_ptr<HttpStreamSession> stream) {
            (void)ctx;
            stream->send_sse("connected", R"({"ok":true})");
            stream->send_sse("tick", R"({"v":1})");
            stream->send_sse_done();
            stream->close();
        });

    server.start();
    wait_for_server(server);

    auto resp = do_sse_request(port, "/events");
    assert(resp.status_code == 200);

    assert(resp.body.find("event: connected") != std::string::npos);
    assert(resp.body.find("data: {\"ok\":true}") != std::string::npos);
    assert(resp.body.find("event: tick") != std::string::npos);
    assert(resp.body.find("data: [DONE]") != std::string::npos);

    server.stop();
    std::cout << "test_stream_sse_content: OK\n";
}

void test_stream_chunked_wire_format() {
    uint16_t port = find_free_port();
    HttpServerConfig config;
    config.address = "127.0.0.1";
    config.port = port;
    HttpServerModule server(config);

    server.add_stream_route(
        {HttpMethod::GET, R"(^/chunked$)", "chunked", true, StreamMode::chunked},
        [](const HttpRequestContext& ctx, std::shared_ptr<HttpStreamSession> stream) {
            (void)ctx;
            stream->send_chunk("hello");
            stream->send_chunk("world");
            stream->close();
        });

    server.start();
    wait_for_server(server);

    std::string response = do_raw_request(
        port,
        "GET /chunked HTTP/1.1\r\n"
        "Host: 127.0.0.1\r\n"
        "Connection: close\r\n"
        "\r\n");

    assert(response.find("Transfer-Encoding: chunked") != std::string::npos);
    assert(response.find("5\r\nhello\r\n") != std::string::npos);
    assert(response.find("5\r\nworld\r\n") != std::string::npos);
    assert(response.find("0\r\n\r\n") != std::string::npos);

    server.stop();
    std::cout << "test_stream_chunked_wire_format: OK\n";
}

void test_405_wrong_method() {
    uint16_t port = find_free_port();
    HttpServerConfig config;
    config.address = "127.0.0.1";
    config.port = port;
    HttpServerModule server(config);

    server.add_direct_route(
        {HttpMethod::GET, R"(^/health$)", "health"},
        [](const HttpRequestContext& ctx, HttpResponseWriter& res) {
            (void)ctx;
            res.send_json(HttpStatus::ok, R"({"status":"ok"})");
        });

    server.start();
    wait_for_server(server);

    auto resp = do_request("POST", "/health", port);
    assert(std::stoi(resp->status_code) == 405);

    auto it = resp->header.find("Allow");
    assert(it != resp->header.end());
    assert(it->second.find("GET") != std::string::npos);

    std::string body = resp->content.string();
    assert(body.find("\"error\"") != std::string::npos);

    server.stop();
    std::cout << "test_405_wrong_method: OK\n";
}

void test_server_restart() {
    uint16_t port = find_free_port();
    HttpServerConfig config;
    config.address = "127.0.0.1";
    config.port = port;
    HttpServerModule server(config);

    server.add_direct_route(
        {HttpMethod::GET, R"(^/health$)", "health"},
        [](const HttpRequestContext& ctx, HttpResponseWriter& res) {
            (void)ctx;
            res.send_json(HttpStatus::ok, R"({"status":"ok"})");
        });

    server.start();
    wait_for_server(server);

    auto resp1 = do_request("GET", "/health", port);
    assert(std::stoi(resp1->status_code) == 200);

    server.stop();

    server.start();
    wait_for_server(server);

    auto resp2 = do_request("GET", "/health", port);
    assert(std::stoi(resp2->status_code) == 200);

    server.stop();
    std::cout << "test_server_restart: OK\n";
}

int main() {
    test_direct_health();
    test_direct_404();
    test_direct_silent_handler_500();
    test_direct_post_echo();
    test_stream_sse_headers();
    test_stream_sse_content();
    test_stream_chunked_wire_format();
    test_405_wrong_method();
    test_server_restart();
    std::cout << "All integration tests passed.\n";
    return 0;
}

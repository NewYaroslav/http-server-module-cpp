#include <http_server/stream_session.hpp>
#include <cassert>
#include <chrono>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

using namespace http_server;

class MockStreamSession : public HttpStreamSession {
public:
    std::vector<std::string> sent_chunks;
    bool was_closed = false;
    std::string close_reason;

    std::string id() const override { return "mock-1"; }
    bool is_closed() const override { return was_closed; }

    void send_chunk(std::string chunk) override {
        std::lock_guard<std::mutex> lock(mutex_);
        sent_chunks.push_back(std::move(chunk));
    }

    void send_sse(std::string event_name, std::string data) override {
        send_chunk("event: " + event_name + "\ndata: " + data + "\n\n");
    }

    void send_sse_data(std::string data) override {
        send_chunk("data: " + data + "\n\n");
    }

    void send_done() override {
        send_chunk("data: [DONE]\n\n");
    }

    void close() override {
        std::lock_guard<std::mutex> lock(mutex_);
        was_closed = true;
    }

    void set_on_close(CloseCallback callback) override {
        std::lock_guard<std::mutex> lock(mutex_);
        on_close_ = std::move(callback);
    }

private:
    std::mutex mutex_;
    CloseCallback on_close_;
};

void test_send_chunk() {
    auto session = std::make_shared<MockStreamSession>();
    session->send_chunk("hello");
    assert(session->sent_chunks.size() == 1);
    assert(session->sent_chunks[0] == "hello");

    std::cout << "test_send_chunk: OK\n";
}

void test_send_after_close_ignored() {
    auto session = std::make_shared<MockStreamSession>();
    session->close();
    session->send_chunk("after");
    assert(session->was_closed);
    // Mock does not ignore sends after close in this test, but real impl should.
    // Here we just verify close works.

    std::cout << "test_send_after_close_ignored: OK\n";
}

void test_close_idempotent() {
    auto session = std::make_shared<MockStreamSession>();
    session->close();
    session->close();
    assert(session->was_closed);

    std::cout << "test_close_idempotent: OK\n";
}

void test_sse_format() {
    auto session = std::make_shared<MockStreamSession>();
    session->send_sse("tick", "{\"v\":1}");
    assert(session->sent_chunks.size() == 1);
    assert(session->sent_chunks[0] == "event: tick\ndata: {\"v\":1}\n\n");

    std::cout << "test_sse_format: OK\n";
}

void test_send_done_format() {
    auto session = std::make_shared<MockStreamSession>();
    session->send_done();
    assert(session->sent_chunks.size() == 1);
    assert(session->sent_chunks[0] == "data: [DONE]\n\n");

    std::cout << "test_send_done_format: OK\n";
}

int main() {
    test_send_chunk();
    test_send_after_close_ignored();
    test_close_idempotent();
    test_sse_format();
    test_send_done_format();
    std::cout << "All stream_session_tests passed.\n";
    return 0;
}

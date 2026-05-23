#include <http_server/response.hpp>
#include <cassert>
#include <iostream>

using namespace http_server;

void test_json_sets_content_type() {
    auto res = HttpResponseData::json(HttpStatus::ok, R"({"ok":true})");
    assert(res.status == HttpStatus::ok);
    assert(res.body == R"({"ok":true})");
    auto ct = res.headers.get("Content-Type");
    assert(ct.has_value());
    assert(ct->find("application/json") != std::string::npos);

    std::cout << "test_json_sets_content_type: OK\n";
}

void test_text_sets_content_type() {
    auto res = HttpResponseData::text(HttpStatus::ok, "hello");
    auto ct = res.headers.get("Content-Type");
    assert(ct.has_value());
    assert(ct->find("text/plain") != std::string::npos);

    std::cout << "test_text_sets_content_type: OK\n";
}

void test_empty_has_no_body() {
    auto res = HttpResponseData::empty(HttpStatus::no_content);
    assert(res.body.empty());
    assert(res.status == HttpStatus::no_content);

    std::cout << "test_empty_has_no_body: OK\n";
}

int main() {
    test_json_sets_content_type();
    test_text_sets_content_type();
    test_empty_has_no_body();
    std::cout << "All response_data_tests passed.\n";
    return 0;
}

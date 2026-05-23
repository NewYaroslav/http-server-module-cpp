#include <http_server/headers.hpp>
#include <cassert>
#include <iostream>

using namespace http_server;

void test_set_and_get() {
    HttpHeaders h;
    h.set("Content-Type", "application/json");
    assert(h.contains("Content-Type"));
    assert(h.get("Content-Type").value() == "application/json");

    std::cout << "test_set_and_get: OK\n";
}

void test_case_insensitive() {
    HttpHeaders h;
    h.set("Content-Type", "text/plain");
    assert(h.contains("content-type"));
    assert(h.get("CONTENT-TYPE").value() == "text/plain");

    std::cout << "test_case_insensitive: OK\n";
}

void test_add_multiple() {
    HttpHeaders h;
    h.add("X-Custom", "first");
    h.add("X-Custom", "second");
    assert(h.contains("X-Custom"));

    std::cout << "test_add_multiple: OK\n";
}

void test_missing() {
    HttpHeaders h;
    assert(!h.contains("Missing"));
    assert(!h.get("Missing").has_value());

    std::cout << "test_missing: OK\n";
}

int main() {
    test_set_and_get();
    test_case_insensitive();
    test_add_multiple();
    test_missing();
    std::cout << "All headers_tests passed.\n";
    return 0;
}

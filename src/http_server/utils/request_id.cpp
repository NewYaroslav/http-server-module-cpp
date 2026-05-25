#include "http_server/utils/request_id.hpp"

#include <atomic>
#include <string>

namespace http_server::utils {

namespace {
std::atomic<uint64_t> counter{0};
} // namespace

std::string generate_request_id() {
    return "req-" + std::to_string(++counter);
}

} // namespace http_server::utils

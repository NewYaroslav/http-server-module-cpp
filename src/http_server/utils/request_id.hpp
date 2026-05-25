#pragma once

#include <string>

namespace http_server::utils {

/// Generate a unique request ID (monotonic counter with prefix).
std::string generate_request_id();

} // namespace http_server::utils

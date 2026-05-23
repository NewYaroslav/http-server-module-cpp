#pragma once

#include <algorithm>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace http_server {

/// Case-insensitive HTTP header collection.
class HttpHeaders {
public:
    /// Set a header, replacing any existing entry with the same name.
    void set(std::string name, std::string value) {
        for (auto& hdr : headers_) {
            if (iequals(hdr.first, name)) {
                hdr.second = std::move(value);
                return;
            }
        }
        headers_.emplace_back(std::move(name), std::move(value));
    }

    /// Add a header without replacing existing entries (allows duplicates).
    void add(std::string name, std::string value) {
        headers_.emplace_back(std::move(name), std::move(value));
    }

    /// Check whether a header with the given name exists.
    bool contains(std::string_view name) const {
        for (const auto& hdr : headers_) {
            if (iequals(hdr.first, name)) return true;
        }
        return false;
    }

    /// Get the value of the first header matching the name, or std::nullopt.
    std::optional<std::string> get(std::string_view name) const {
        for (const auto& hdr : headers_) {
            if (iequals(hdr.first, name)) return hdr.second;
        }
        return std::nullopt;
    }

    /// Direct access to the underlying header entries.
    const std::vector<std::pair<std::string, std::string>>& items() const noexcept {
        return headers_;
    }

private:
    /// Case-insensitive string comparison.
    static bool iequals(std::string_view a, std::string_view b) {
        if (a.size() != b.size()) return false;
        return std::equal(a.begin(), a.end(), b.begin(),
            [](char ca, char cb) {
                return std::tolower(static_cast<unsigned char>(ca))
                     == std::tolower(static_cast<unsigned char>(cb));
            });
    }

    std::vector<std::pair<std::string, std::string>> headers_;
};

} // namespace http_server

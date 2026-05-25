cmake_minimum_required(VERSION 3.21)

project(http-server-module-cpp
    VERSION 0.1.0
    DESCRIPTION "C++17 HTTP server module with Simple-Web-Server backend"
    LANGUAGES CXX
)

include(GNUInstallDirs)

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(HTTP_SERVER_MODULE_TOP_LEVEL ON)
else()
    set(HTTP_SERVER_MODULE_TOP_LEVEL OFF)
endif()

option(HTTP_SERVER_MODULE_BUILD_EXAMPLES
    "Build http-server-module-cpp examples"
    ${HTTP_SERVER_MODULE_TOP_LEVEL}
)
option(HTTP_SERVER_MODULE_BUILD_TESTS
    "Build http-server-module-cpp tests"
    ${HTTP_SERVER_MODULE_TOP_LEVEL}
)
option(HTTP_SERVER_MODULE_USE_EVENT_HUB
    "Enable optional event-hub-cpp integration"
    ON
)
option(HTTP_SERVER_MODULE_USE_STANDALONE_ASIO
    "Use standalone Asio instead of Boost.Asio"
    ON
)
option(HTTP_SERVER_MODULE_USE_EXTERNAL_DEPS
    "Use dependencies from the external/ subdirectory"
    ON
)

# ---------------------------------------------------------------------------
# Core target
# ---------------------------------------------------------------------------
add_library(http_server_module STATIC)
add_library(http_server_module::http_server_module ALIAS http_server_module)

target_sources(http_server_module
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http_server/http_server_module.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http_server/backend/simple_web_server_backend.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http_server/utils/request_id.cpp
)

target_include_directories(http_server_module
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_features(http_server_module PUBLIC cxx_std_17)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
include(cmake/deps/asio.cmake)
include(cmake/deps/simple_web_server.cmake)
include(cmake/deps/event_hub.cmake)

use_or_fetch_asio(asio_target)
use_or_fetch_simple_web_server(sws_target)
if(HTTP_SERVER_MODULE_USE_EVENT_HUB)
    use_or_fetch_event_hub(event_hub_target)
    target_link_libraries(http_server_module PUBLIC ${event_hub_target})
endif()

target_link_libraries(http_server_module PRIVATE ${asio_target} ${sws_target})

# ---------------------------------------------------------------------------
# Subdirectories
# ---------------------------------------------------------------------------
if(HTTP_SERVER_MODULE_BUILD_EXAMPLES AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt")
    add_subdirectory(examples)
endif()

if(HTTP_SERVER_MODULE_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    add_subdirectory(tests)
endif()

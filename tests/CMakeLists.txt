cmake_minimum_required(VERSION 3.21)

add_executable(route_registry_tests route_registry_tests.cpp)
target_link_libraries(route_registry_tests PRIVATE http_server_module)

add_executable(headers_tests headers_tests.cpp)
target_link_libraries(headers_tests PRIVATE http_server_module)

add_executable(response_data_tests response_data_tests.cpp)
target_link_libraries(response_data_tests PRIVATE http_server_module)

add_executable(stream_session_tests stream_session_tests.cpp)
target_link_libraries(stream_session_tests PRIVATE http_server_module)

add_executable(integration_tests integration_tests.cpp)
target_link_libraries(integration_tests PRIVATE http_server_module)

# Simple test runner: just build them. Running is manual for now.

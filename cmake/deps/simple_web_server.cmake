include_guard(GLOBAL)

# ~~~
# use_or_fetch_simple_web_server(out_target)
#
# Resolves the Simple-Web-Server library and sets <out_target> to the
# CMake target name that provides its headers and link dependencies.
# ~~~
function(use_or_fetch_simple_web_server out_target)
    if(TARGET simple-web-server)
        set(${out_target} simple-web-server PARENT_SCOPE)
        return()
    endif()

    if(HTTP_SERVER_MODULE_USE_EXTERNAL_DEPS
       AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/Simple-Web-Server/CMakeLists.txt")
        set(USE_STANDALONE_ASIO ${HTTP_SERVER_MODULE_USE_STANDALONE_ASIO} CACHE BOOL
            "Use standalone Asio for Simple-Web-Server" FORCE
        )
        set(BUILD_TESTING OFF CACHE BOOL "Disable Simple-Web-Server tests" FORCE)
        set(BUILD_FUZZING OFF CACHE BOOL "Disable Simple-Web-Server fuzzing" FORCE)

        add_subdirectory(
            "${CMAKE_CURRENT_SOURCE_DIR}/external/Simple-Web-Server"
            "${CMAKE_CURRENT_BINARY_DIR}/external/Simple-Web-Server"
        )
        set(${out_target} simple-web-server PARENT_SCOPE)
        return()
    endif()

    message(FATAL_ERROR
        "Simple-Web-Server not found. "
        "Enable HTTP_SERVER_MODULE_USE_EXTERNAL_DEPS or provide Simple-Web-Server."
    )
endfunction()

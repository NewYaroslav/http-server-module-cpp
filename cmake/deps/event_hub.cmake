include_guard(GLOBAL)

# ~~~
# use_or_fetch_event_hub(out_target)
#
# Resolves event-hub-cpp and sets <out_target> to the CMake target name
# that provides its headers.  Disables optional time-shield integration
# unless the caller explicitly enables it.
# ~~~
function(use_or_fetch_event_hub out_target)
    if(TARGET event_hub::event_hub)
        set(${out_target} event_hub::event_hub PARENT_SCOPE)
        return()
    endif()

    if(HTTP_SERVER_MODULE_USE_EXTERNAL_DEPS
       AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/event-hub-cpp/CMakeLists.txt")
        set(EVENT_HUB_CPP_USE_TIME_SHIELD OFF CACHE BOOL
            "Disable time-shield for event-hub-cpp" FORCE
        )
        set(EVENT_HUB_CPP_BUILD_EXAMPLES OFF CACHE BOOL
            "Disable event-hub-cpp examples" FORCE
        )
        set(EVENT_HUB_CPP_BUILD_TESTS OFF CACHE BOOL
            "Disable event-hub-cpp tests" FORCE
        )

        add_subdirectory(
            "${CMAKE_CURRENT_SOURCE_DIR}/external/event-hub-cpp"
            "${CMAKE_CURRENT_BINARY_DIR}/external/event-hub-cpp"
        )
        set(${out_target} event_hub::event_hub PARENT_SCOPE)
        return()
    endif()

    message(FATAL_ERROR
        "event-hub-cpp not found. "
        "Enable HTTP_SERVER_MODULE_USE_EXTERNAL_DEPS or set "
        "HTTP_SERVER_MODULE_USE_EVENT_HUB=OFF."
    )
endfunction()

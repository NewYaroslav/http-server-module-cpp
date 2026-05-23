include_guard(GLOBAL)

# ~~~
# use_or_fetch_asio(out_target)
#
# Resolves standalone Asio and sets <out_target> to the CMake target name
# that provides asio headers and the ASIO_STANDALONE definition.
# ~~~
function(use_or_fetch_asio out_target)
    if(TARGET asio::asio)
        set(${out_target} asio::asio PARENT_SCOPE)
        return()
    endif()

    if(HTTP_SERVER_MODULE_USE_STANDALONE_ASIO)
        if(HTTP_SERVER_MODULE_USE_EXTERNAL_DEPS
           AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/asio/include/asio.hpp")
            # Asio is header-only with no CMake support -- create an
            # INTERFACE library that exposes the include directory and
            # the ASIO_STANDALONE define.
            add_library(asio_standalone INTERFACE)
            add_library(asio::asio ALIAS asio_standalone)
            target_include_directories(asio_standalone
                SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/external/asio/include"
            )
            target_compile_definitions(asio_standalone INTERFACE ASIO_STANDALONE)
            set(${out_target} asio::asio PARENT_SCOPE)
            return()
        endif()

        # Fallback: try to find asio via find_package (Conan / Vcpkg).
        find_package(asio QUIET)
        if(TARGET asio::asio)
            set(${out_target} asio::asio PARENT_SCOPE)
            return()
        endif()

        # Last resort: search for asio.hpp on the system.
        find_path(ASIO_INCLUDE_DIR asio.hpp)
        if(ASIO_INCLUDE_DIR)
            add_library(asio_standalone INTERFACE)
            add_library(asio::asio ALIAS asio_standalone)
            target_include_directories(asio_standalone
                SYSTEM INTERFACE "${ASIO_INCLUDE_DIR}"
            )
            target_compile_definitions(asio_standalone INTERFACE ASIO_STANDALONE)
            set(${out_target} asio::asio PARENT_SCOPE)
            return()
        endif()

        message(FATAL_ERROR
            "Standalone Asio not found. "
            "Set HTTP_SERVER_MODULE_USE_STANDALONE_ASIO=OFF or provide Asio."
        )
    else()
        # Boost.Asio path
        find_package(Boost 1.53.0 QUIET COMPONENTS system)
        if(NOT Boost_FOUND)
            find_package(Boost 1.53.0 REQUIRED)
        endif()
        set(${out_target} Boost::boost PARENT_SCOPE)
    endif()
endfunction()

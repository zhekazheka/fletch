cmake_minimum_required(VERSION 3.28)

project(fletch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(FLETCH_SANITIZE "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(FLETCH_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(FLETCH_CLANG_TIDY "Run clang-tidy on fletch targets" OFF)
option(FLETCH_BUILD_TESTS "Build the Catch2 test suite" ON)
option(FLETCH_BUILD_EXAMPLES "Build example programs" ON)

# Sanitizers are applied globally, before any FetchContent, so that third-party
# code (Catch2) is instrumented the same way as ours. Mixed instrumentation hides
# bugs at the boundary.
if(FLETCH_SANITIZE)
    if(MSVC)
        message(WARNING "FLETCH_SANITIZE is not configured for MSVC; ignoring")
    else()
        add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer
                            -fno-sanitize-recover=undefined)
        add_link_options(-fsanitize=address,undefined)
    endif()
endif()

# Shared compile settings for every fletch target. Link against this; do not
# repeat warning flags per target.
add_library(fletch_options INTERFACE)
add_library(fletch::options ALIAS fletch_options)

if(MSVC)
    target_compile_options(fletch_options INTERFACE /W4 /permissive-)
    if(FLETCH_WARNINGS_AS_ERRORS)
        target_compile_options(fletch_options INTERFACE /WX)
    endif()
else()
    target_compile_options(fletch_options INTERFACE
        -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion)
    if(FLETCH_WARNINGS_AS_ERRORS)
        target_compile_options(fletch_options INTERFACE -Werror)
    endif()
endif()

if(FLETCH_CLANG_TIDY)
    find_program(FLETCH_CLANG_TIDY_EXE NAMES clang-tidy REQUIRED)
    # Set on individual targets (see fletch_enable_tidy) rather than globally,
    # so Catch2 is not analysed.
    set(FLETCH_CLANG_TIDY_COMMAND "${FLETCH_CLANG_TIDY_EXE}" "--warnings-as-errors=*")
endif()

function(fletch_enable_tidy target)
    if(FLETCH_CLANG_TIDY)
        set_target_properties(${target} PROPERTIES CXX_CLANG_TIDY "${FLETCH_CLANG_TIDY_COMMAND}")
    endif()
endfunction()

add_subdirectory(src)

if(FLETCH_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(FLETCH_BUILD_TESTS)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif()
endif()

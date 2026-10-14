include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.8.1
    GIT_SHALLOW TRUE
    SYSTEM
)
FetchContent_MakeAvailable(Catch2)

# Catch2's TEST_CASE expands __COUNTER__, which Clang 22+ reports as a C2y
# extension under -Wpedantic. The suppression lives on an interface target
# linked after fletch::options so it is emitted after -Wpedantic; a PRIVATE
# compile option on the executable would come first and be re-enabled.
add_library(fletch_test_options INTERFACE)
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-Wno-c2y-extensions FLETCH_HAVE_WNO_C2Y_EXTENSIONS)
if(FLETCH_HAVE_WNO_C2Y_EXTENSIONS)
    target_compile_options(fletch_test_options INTERFACE -Wno-c2y-extensions)
endif()

add_executable(fletch_tests
    scaffold_test.cpp
)
target_link_libraries(fletch_tests PRIVATE
    fletch::fletch
    Catch2::Catch2WithMain
    fletch_test_options
)
fletch_enable_tidy(fletch_tests)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(fletch_tests)

add_library(fletch STATIC
    net/udp_socket.cpp
)
add_library(fletch::fletch ALIAS fletch)

target_include_directories(fletch PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(fletch PUBLIC fletch::options)

# The library itself is built without exceptions or RTTI: errors are values
# (std::expected), programmer errors are assertions. PRIVATE, so tests and
# examples keep the defaults; Catch2's REQUIRE needs exceptions.
if(MSVC)
    target_compile_options(fletch PRIVATE /GR- /EHs-c- /D_HAS_EXCEPTIONS=0)
else()
    target_compile_options(fletch PRIVATE -fno-exceptions -fno-rtti)
endif()

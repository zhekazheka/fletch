# The library is header-only until the first translation unit lands.
# When that happens (ticket 4 for src/protocol/, ticket 2 or 19 for src/net/):
#   1. change INTERFACE to STATIC,
#   2. list sources with target_sources(fletch PRIVATE protocol/packet.cpp ...),
#   3. change the PUBLIC/INTERFACE keywords below accordingly.
add_library(fletch INTERFACE)
add_library(fletch::fletch ALIAS fletch)

target_include_directories(fletch INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(fletch INTERFACE fletch::options)

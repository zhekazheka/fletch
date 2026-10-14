add_executable(fletch_hello hello/main.cpp)
target_link_libraries(fletch_hello PRIVATE fletch::fletch)
fletch_enable_tidy(fletch_hello)

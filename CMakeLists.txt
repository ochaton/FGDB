cmake_minimum_required (VERSION 3.1)
project(fgdb)

include_directories(src/lib/buddy)
include_directories(src/server)

add_subdirectory(src/lib)
add_subdirectory(src/server)

add_executable(server_test src/main.c)
target_link_libraries(server_test server)
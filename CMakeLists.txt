cmake_minimum_required (VERSION 3.1)
project(fgdb)

include_directories(src/include)
include_directories(src/lib/buddy)
include_directories(src/server)
include_directories(src/memory)

add_subdirectory(src/lib)
add_subdirectory(src/server)
add_subdirectory(src/memory)

add_executable(server_test src/main.c src/operations.c)
target_link_libraries(server_test server memory)
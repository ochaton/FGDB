cmake_minimum_required (VERSION 3.1)
project(fgdb)

include_directories(src/include)
include_directories(src/)

add_subdirectory(src/lib)
add_subdirectory(src/server)
add_subdirectory(src/memory)
add_subdirectory(src/transactions)

add_executable(server_test src/main.c src/operations.c)
add_executable(queue_test test/queue_test.c)
target_link_libraries(server_test server memory transaction_queue buddy)
target_link_libraries(queue_test transaction_queue request)

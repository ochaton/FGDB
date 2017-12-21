project(fgdb_test)
cmake_minimum_required(VERSION 3.1)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-g -O0")

include_directories(../src/)
include_directories(../src/include/)

add_library(unity STATIC unity/src/unity.c)
target_include_directories(unity PUBLIC unity/src)

add_executable(fgdb_test_exe fgdb.c)
target_link_libraries(fgdb_test_exe arena memory disk buddy vector lruq)
target_link_libraries(fgdb_test_exe unity HashMap transaction_queue util)
target_link_libraries(fgdb_test_exe recovery wal operations log)

add_executable(hashmap_test_exe hashmap/main.c)
target_link_libraries(hashmap_test_exe HashMap unity util)

add_executable(memory_test_exe memory/main.c)
target_link_libraries(memory_test_exe memory HashMap unity util)

include(CTest)

add_test(hashmap_test hashmap_test_exe)
add_test(memory_test memory_test_exe)
add_test(fgdb_test fgdb_test_exe)

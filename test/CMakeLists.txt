project(fgdb_test)
cmake_minimum_required(VERSION 3.1)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-g -O0")

include_directories(../src/)
include_directories(../src/include/)

add_subdirectory(../src/lib/buddy buddy)
add_subdirectory(../src/lib/vector vector)
add_subdirectory(../src/lru/ lruq)
add_subdirectory(../src/wal/ wal)
add_subdirectory(../src/lib/hashmap hashmap)
add_subdirectory(../src/transactions/ transactions)

add_library(unity STATIC unity/src/unity.c)
target_include_directories(unity PUBLIC unity/src)

set(arena_sources
	../src/arena/arena.c
	../src/arena/manager.c)

set(disk_sources
	../src/arena/disk.c)

set(memory_sources
	../src/memory/hashmap.c)

set(util_sources
	../src/util.c)

add_library(arena STATIC ${arena_sources})
add_library(util STATIC ${util_sources})
add_library(disk STATIC ${disk_sources})
add_library(memory STATIC ${memory_sources})

add_executable(fgdb_test_exe fgdb.c)
target_link_libraries(fgdb_test_exe arena memory disk buddy vector lruq wal unity HashMap transaction_queue util)

add_executable(hashmap_test_exe hashmap/main.c)
target_link_libraries(hashmap_test_exe HashMap unity util)

add_executable(memory_test_exe memory/main.c)
target_link_libraries(memory_test_exe memory HashMap unity util)

include(CTest)

add_test(hashmap_test hashmap_test_exe)
add_test(memory_test memory_test_exe)
add_test(fgdb_test fgdb_test_exe)

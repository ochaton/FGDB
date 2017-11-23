project(fgdb_test)
cmake_minimum_required(VERSION 3.1)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-g -O0")

include_directories(../src/)
include_directories(../src/include/)

add_subdirectory(../src/lib/buddy buddy)
add_subdirectory(../src/lib/vector vector)

set(arena_sources
	../src/arena/arena.c
	../src/arena/manager.c)

set(disk_sources
	../src/arena/disk.c)

add_library(arena STATIC ${arena_sources})
add_library(disk STATIC ${disk_sources})

add_executable(fgdb_test fgdb.c)
target_link_libraries(fgdb_test arena disk buddy vector)
# target_link_libraries(fgdb_test arena disk buddy)

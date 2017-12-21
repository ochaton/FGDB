cmake_minimum_required (VERSION 3.1)
project(fgdb)

find_library(PTHREAD_LIBRARY pthread)

if (PTHREAD_LIBRARY)
	message(STATUS "Found pthread library: ${PTHREAD_LIBRARY}")
else()
	message(FATAL_ERROR "Could not find pthread library")
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb -Werror -Wignored-qualifiers -Winit-self \
	-Wswitch-default -Wfloat-equal  \
	-Wtype-limits -Wempty-body -Wmissing-field-initializers -Wextra \
	-Wmissing-field-initializers -Wextra")

include_directories(src/include)
include_directories(src/)

add_subdirectory(src/lib/buddy buddy)
add_subdirectory(src/lib/vector vector)
add_subdirectory(src/lib/hashmap hashmap)

add_subdirectory(src/server)

add_subdirectory(src/lru/ lruq)
add_subdirectory(src/wal/ wal)

if (NOT TARGET transaction_queue)
	add_subdirectory(src/transactions)
endif()

if (NOT TARGET operations)
	add_subdirectory(src/operations operations)
endif()

set(arena_sources
	src/arena/arena.c
	src/arena/manager.c)

set(disk_sources
	src/arena/disk.c)

set(util_sources
	src/util.c)

add_library(arena STATIC ${arena_sources})
add_library(disk STATIC ${disk_sources})
add_library(util STATIC ${util_sources})

add_executable(fgdb src/main.c)
target_link_libraries(fgdb arena memory disk buddy vector lruq wal HashMap server operations transaction_queue pthread util)

# add_executable(queue_test test/queue_test.c)
# target_link_libraries(queue_test transaction_queue request pthread)

# enable_testing()
# add_test(NAME perl_test_server
# 	COMMAND make -C "${CMAKE_CURRENT_SOURCE_DIR}/client")

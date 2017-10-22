cmake_minimum_required (VERSION 3.1)
project(fgdb)

find_library(PTHREAD_LIBRARY pthread)

if (PTHREAD_LIBRARY)
	message(STATUS "Found pthread library: ${PTHREAD_LIBRARY}")
else()
	message(FATAL_ERROR "Could not find pthread library")
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -ggdb -Werror -Wignored-qualifiers -Winit-self\
	-Wswitch-default -Wfloat-equal -Wshadow -Wpointer-arith \
	-Wtype-limits -Wempty-body -Wmissing-field-initializers -Wextra \
	-Wmissing-field-initializers -Wextra")

include_directories(src/include)
include_directories(src/)

add_subdirectory(src/lib)
add_subdirectory(src/server)
add_subdirectory(src/memory)
add_subdirectory(src/transactions)

add_executable(server_test src/main.c src/operations.c)
add_executable(queue_test test/queue_test.c)
target_link_libraries(server_test server memory transaction_queue buddy pthread)
target_link_libraries(queue_test transaction_queue request pthread)

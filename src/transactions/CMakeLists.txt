cmake_minimum_required (VERSION 3.1)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -ggdb -Werror -Wignored-qualifiers -Winit-self\
	-Wswitch-default -Wfloat-equal -Wshadow -Wpointer-arith \
	-Wtype-limits -Wempty-body -Wmissing-field-initializers -Wextra \
	-Wmissing-field-initializers -Wextra")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set(queue_source
	queue.c
	)

if (NOT TARGET server)
	add_subdirectory(../server server)
endif()

if (NOT TARGET heap)
	add_subdirectory(../lib/heap heap)
endif()

add_library(transaction_queue STATIC ${queue_source})
target_link_libraries(transaction_queue heap pthread message)

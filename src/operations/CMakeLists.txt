cmake_minimum_required (VERSION 3.1)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-O0 -ggdb -Werror -Wignored-qualifiers -Winit-self\
	-Wswitch-default -Wfloat-equal -Wshadow \
	-Wno-int-to-pointer-cast -Wno-pointer-to-int-cast \
	-Wtype-limits -Wempty-body -Wmissing-field-initializers -Wextra \
	-Wmissing-field-initializers -Wextra")

add_library(operations STATIC operations.c)

if (NOT TARGET request)
	add_subdirectory(../server)
endif()

target_link_libraries(operations request)

if (NOT TARGET memory)
	add_subdirectory(../memory memory)
endif()

target_link_libraries(operations memory)

if (NOT TARGET transaction_queue)
	add_subdirectory(../transactions transaction_queue)
endif()

target_link_libraries(operations transaction_queue)

if (NOT TARGET wal)
	add_subdirectory(../wal wal)
endif()

target_link_libraries(operations wal)


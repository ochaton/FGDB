cmake_minimum_required (VERSION 3.1)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-O0 -ggdb -Werror -Wignored-qualifiers -Winit-self\
	-Wswitch-default -Wfloat-equal -Wshadow \
	-Wno-int-to-pointer-cast -Wno-pointer-to-int-cast \
	-Wtype-limits -Wempty-body -Wmissing-field-initializers -Wextra \
	-Wmissing-field-initializers -Wextra")

set(lruq_sources
	lruq.c
	)

add_library(lruq STATIC ${lruq_sources})
target_link_libraries(lruq)

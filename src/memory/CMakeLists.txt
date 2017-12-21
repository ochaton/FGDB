cmake_minimum_required (VERSION 3.1)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -ggdb -Werror -Wignored-qualifiers -Winit-self\
	-Wswitch-default -Wfloat-equal -Wshadow -Wpointer-arith \
	-Wtype-limits -Wempty-body -Wmissing-field-initializers -Wextra \
	-Wmissing-field-initializers -Wextra")

add_library(memory STATIC hashmap.c)

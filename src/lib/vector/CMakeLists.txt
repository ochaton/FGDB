project(fgdb_test)
cmake_minimum_required(VERSION 3.1)

set(CMAKE_C_STANDARD 99)

set(vector_sources
	vector.c)

add_library(vector STATIC ${vector_sources})

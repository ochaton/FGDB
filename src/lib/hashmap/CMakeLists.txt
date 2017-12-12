cmake_minimum_required(VERSION 3.1)

project(DB_struct)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-g -O0")

include_directories(.)

set(source_lib_hash_avl AVLNode.c Util.c HashMap.c Hash.c)
add_library(HashMap STATIC ${source_lib_hash_avl})

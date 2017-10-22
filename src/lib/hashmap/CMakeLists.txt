cmake_minimum_required(VERSION 3.1)

project(DB_struct) 
set(SOURCE_EXE mmain.c)

set(SOURCE_LIB_HASH_AVL AVLnode.c AVLnode_spesial.c HashMap.c hashf.c )


add_library(HashMap STATIC ${SOURCE_LIB_HASH_AVL})

add_executable(main ${SOURCE_EXE})

target_link_libraries(main HashMap)

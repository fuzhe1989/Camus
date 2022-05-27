cmake_minimum_required (VERSION 3.21)
project(DataStructures)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")

add_subdirectory (contrib EXCLUDE_FROM_ALL)

add_subdirectory(src)
add_subdirectory(tests)


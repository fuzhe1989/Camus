cmake_minimum_required (VERSION 3.21)
project(Hf)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fcoroutines-ts")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")

find_package(GTest)
find_package(Boost)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(examples)


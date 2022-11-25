cmake_minimum_required (VERSION 3.16)

project(Camus LANGUAGES C CXX ASM)

if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "None")
  set(CMAKE_BUILD_TYPE "RelWithDebInfo")
  message(STATUS "CMAKE_BUILD_TYPE is not set, set to default = ${CMAKE_BUILD_TYPE}")
endif()
message (STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_UC)

if(CMAKE_BUILD_TYPE_UC STREQUAL "DEBUG")
  option(ENABLE_ASSERTIONS "Enable assertions" ON)
else()
  option(ENABLE_ASSERTIONS "Enable assertions" OFF)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fcoroutines -Wno-deprecated")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
conan_basic_setup()

find_package(GTest)
find_package(Boost)
find_package(magic_enum)
find_package(robin_hood)
find_package(scn)
find_package(spdlog)
find_package(fmt)
find_package(folly)
find_package(asio-grpc)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(examples)


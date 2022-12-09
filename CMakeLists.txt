cmake_minimum_required (VERSION 3.16)

project(Camus LANGUAGES C CXX ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/BuildType.cmake)
include(cmake/EnableAssertions.cmake)
include(cmake/EnableTest.cmake)
include(cmake/CxxFlags.cmake)

include(cmake/SetupConan.cmake)
include(cmake/FindPackages.cmake)
include(cmake/Target.cmake)
include(cmake/PrintProperties.cmake)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(examples)


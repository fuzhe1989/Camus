include_directories (${CMAKE_CURRENT_BINARY_DIR})
include_directories (${DataStructures_SOURCE_DIR}/contrib/googletest/googletest/include)

add_subdirectory(common)
add_subdirectory(bench)


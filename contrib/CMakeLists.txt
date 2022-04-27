# Google Test from sources
add_subdirectory(${DataStructure_SOURCE_DIR}/contrib/googletest/googletest ${CMAKE_CURRENT_BINARY_DIR}/googletest)
# avoid problems with <regexp.h>
target_compile_definitions (gtest INTERFACE GTEST_HAS_POSIX_RE=0)
target_include_directories (gtest INTERFACE ${TiFlash_SOURCE_DIR}/contrib/googletest/googletest/include)

SET (BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable google-benchmark testing" FORCE)
SET (BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Disable google-benchmark testing" FORCE)
add_subdirectory(benchmark)

include_directories (${DataStructures_SOURCE_DIR}/contrib/googletest/googletest/include)

file(GLOB_RECURSE common_srcs "*.cpp")

add_executable(
    gtests_common
    ${common_srcs}
)
target_link_libraries(
    gtests_common
    gtest_main
)

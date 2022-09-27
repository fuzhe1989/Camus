file (GLOB_RECURSE common_srcs "*.cpp")

add_executable (gtests_common ${common_srcs})
target_link_libraries (gtests_common PRIVATE
    GTest::gtest
    GTest::gtest_main
)

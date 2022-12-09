file (GLOB_RECURSE common_srcs "*.cpp")
target_add_test(gtests_common ${common_srcs})
target_link_libraries(gtests_common test_main)

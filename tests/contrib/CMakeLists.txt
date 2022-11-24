file (GLOB_RECURSE contrib_srcs "*.cpp")

add_executable (gtests_contrib ${contrib_srcs})
target_link_libraries (gtests_contrib PRIVATE
    GTest::gtest
    GTest::gtest_main
    scn::scn
    ${Folly_folly_LINK_LIBS}
    glog
    gflags
)

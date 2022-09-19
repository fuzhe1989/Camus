file (GLOB_RECURSE contrib_srcs "*.cpp")

add_executable (gtests_contrib ${contrib_srcs})
target_link_libraries (gtests_contrib PRIVATE
    gtest::gtest
    scn::scn
)

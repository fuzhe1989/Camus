file (GLOB_RECURSE srcs "*.cpp")
target_add_test(gtests_example ${srcs})
target_link_libraries (gtests_example PRIVATE
  test_main
  common
  raft-v0
)

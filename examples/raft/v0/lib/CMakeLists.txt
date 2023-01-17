file (GLOB_RECURSE srcs "*.cpp")
target_add_lib(raft-v0 ${srcs})
target_link_libraries(raft-v0 common Folly::folly)

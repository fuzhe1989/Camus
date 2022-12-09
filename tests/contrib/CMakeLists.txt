file (GLOB_RECURSE contrib_srcs "*.cpp")
target_add_test(gtests_contrib ${contrib_srcs})
target_link_libraries (gtests_contrib PRIVATE
  test_main
  scn::scn
  spdlog::spdlog
  Folly::folly
  #${Folly_folly_LINK_LIBS}
)

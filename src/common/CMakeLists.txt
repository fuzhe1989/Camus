file (GLOB_RECURSE common_srcs "*.cpp")
target_add_lib(common ${common_srcs})
target_link_libraries(common Boost::boost magic_enum::magic_enum robin_hood::robin_hood scn::scn spdlog::spdlog fmt::fmt)


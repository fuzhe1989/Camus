file (GLOB_RECURSE common_srcs "*.cpp")
add_library (common ${common_srcs})
target_include_directories (common PRIVATE
  ${Hf_SOURCE_DIR}/src
  ${Hf_BUILD_DIR}/src
)
target_link_libraries(common Boost::boost magic_enum::magic_enum robin_hood::robin_hood scn::scn spdlog::spdlog fmt::fmt)


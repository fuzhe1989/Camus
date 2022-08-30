file (GLOB_RECURSE common_srcs "*.cpp")
add_library (common ${common_srcs})
target_include_directories (common PRIVATE
  ${Hf_SOURCE_DIR}/src
  ${Hf_BUILD_DIR}/src
)


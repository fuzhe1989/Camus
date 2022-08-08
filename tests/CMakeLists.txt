if (ENABLE_TESTS)
  include_directories (${Hf_SOURCE_DIR}/src)

  add_subdirectory (common)
  add_subdirectory (bench)
endif()

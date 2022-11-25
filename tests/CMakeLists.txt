if (ENABLE_TESTS)
  include_directories (${PROJECT_SOURCE_DIR}/src)

  add_subdirectory (common)
  add_subdirectory (contrib)
  add_subdirectory (bench)
endif()

if (ENABLE_TESTS)
  # must add main first
  add_subdirectory (main)

  add_subdirectory (common)
  add_subdirectory (contrib)
  add_subdirectory (bench)
endif()

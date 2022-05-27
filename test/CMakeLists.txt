if (ENABLE_TESTS)
    include_directories (${DataStructures_SOURCE_DIR}/contrib/googletest/googletest/include)
    include_directories (${DataStructures_SOURCE_DIR}/src)

    add_subdirectory (common)
    add_subdirectory (bench)
endif()

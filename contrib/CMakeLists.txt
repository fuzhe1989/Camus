set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")

function(add_contrib cmake_folder)
    set(base_folders ${cmake_folder})

    foreach (base_folder ${base_folders})
        # some typos in the code
        if (NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${base_folder}")
            message(FATAL_ERROR "No such base folder '${base_folder}' (for '${cmake_folder}' cmake folder). Typo in the base folder name?")
        endif()

        file(GLOB contrib_files "${base_folder}/*")
        if (NOT contrib_files)
            message(STATUS "submodule ${base_folder} is missing or empty. to fix try run:")
            message(STATUS "    git submodule update --init")
            return()
        endif()
    endforeach()

    message(STATUS "Adding contrib module ${base_folders} (configuring with ${cmake_folder})")
    add_subdirectory (${cmake_folder})
endfunction()

if (ENABLE_TESTS)
    add_contrib (googletest-cmake googletest)

    SET (BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable google-benchmark testing" FORCE)
    SET (BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Disable google-benchmark testing" FORCE)
    add_subdirectory(benchmark)
endif()

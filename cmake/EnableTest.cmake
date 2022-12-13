# from https://stackoverflow.com/a/65954915/19884603
if(CMAKE_BUILD_TYPE_UC STREQUAL "DEBUG")
  option(ENABLE_TESTS "Enable tests" ON)
else()
  option(ENABLE_TESTS "Enable tests" OFF)
endif()
message(STATUS "ENABLE_TESTS: ${ENABLE_TESTS}")

if (ENABLE_TESTS)
  enable_testing()
endif()


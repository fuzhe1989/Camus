# from https://stackoverflow.com/a/65954915/19884603
if(CMAKE_BUILD_TYPE_UC STREQUAL "DEBUG")
  option(ENABLE_ASSERTIONS "Enable assertions" ON)
else()
  option(ENABLE_ASSERTIONS "Enable assertions" OFF)
endif()
message(STATUS "ENABLE_ASSERTIONS: ${ENABLE_ASSERTIONS}")

if(ENABLE_ASSERTIONS)
  if(NOT MSVC)
    add_definitions(-D_DEBUG)
  endif()
   # On non-Debug builds cmake automatically defines NDEBUG, so we explicitly undefine it:
  if(NOT CMAKE_BUILD_TYPE_UC STREQUAL "DEBUG")
    # NOTE: use `add_compile_options` rather than `add_definitions` since
    # `add_definitions` does not support generator expressions.
    add_compile_options($<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:-UNDEBUG>)
  endif()
endif()


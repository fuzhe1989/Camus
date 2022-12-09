macro(target_add_lib NAME)
  #file(GLOB_RECURSE FILES CONFIGURE_DEPENDS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cc" "*.cpp")
  add_library(${NAME} STATIC ${ARGN})
  target_include_directories(${NAME}
    PUBLIC
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
      ${PROJECT_SOURCE_DIR}
      ${PROJECT_BINARY_DIR}/src
      ${PROJECT_BINARY_DIR}
  )
endmacro()

macro(target_add_bin NAME)
  #file(GLOB_RECURSE FILES CONFIGURE_DEPENDS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cc" "*.cpp")
  add_executable(${NAME} ${ARGN})
  target_include_directories(${NAME}
    PUBLIC
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
      ${PROJECT_SOURCE_DIR}
      ${PROJECT_BINARY_DIR}/src
      ${PROJECT_BINARY_DIR}
  )
  set_target_properties(${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endmacro()

macro(target_add_example NAME)
  #file(GLOB_RECURSE FILES CONFIGURE_DEPENDS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cc" "*.cpp")
  add_executable(${NAME} ${ARGN})
  target_include_directories(${NAME}
    PUBLIC
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
      ${PROJECT_SOURCE_DIR}
      ${PROJECT_BINARY_DIR}/src
      ${PROJECT_BINARY_DIR}
  )
  # TODO: output dir not work
  set_target_properties(${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples")
endmacro()

macro(target_add_test NAME)
  add_executable(${NAME} ${ARGN})
  add_test(NAME ${NAME} COMMAND ${NAME})
  target_include_directories(${NAME}
    PUBLIC
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
      ${PROJECT_SOURCE_DIR}
      ${PROJECT_BINARY_DIR}/src
      ${PROJECT_BINARY_DIR}
  )
  # TODO: output dir not work
  set_target_properties(${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests")
endmacro()


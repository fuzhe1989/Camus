add_executable (spdlog_example example.cpp)
target_link_libraries (spdlog_example PRIVATE
  spdlog::spdlog
)


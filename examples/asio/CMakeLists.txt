add_executable (example_asio_echo_server echo_server.cpp)
target_link_libraries (example_asio_echo_server PRIVATE
    boost::boost
)



add_subdirectory(proto)

# examples
function(asio_grpc_add_example _asio_grpc_name)
    add_executable(asio-grpc-example-${_asio_grpc_name})

    target_sources(asio-grpc-example-${_asio_grpc_name} PRIVATE ${_asio_grpc_name}.cpp)

    target_link_libraries(
        asio-grpc-example-${_asio_grpc_name}
        PRIVATE asio-grpc-example-protos)
endfunction()

include_directories (${PROJECT_SOURCE_DIR}/src)
include_directories (${PROJECT_SOURCE_DIR})
include_directories (${PROJECT_BINARY_DIR}/src)
include_directories (${PROJECT_BINARY_DIR})

asio_grpc_add_example(hello-world-client)
target_link_libraries(asio-grpc-example-hello-world-client PRIVATE asio-grpc::asio-grpc)

asio_grpc_add_example(hello-world-server)
target_link_libraries(asio-grpc-example-hello-world-server PRIVATE asio-grpc::asio-grpc)

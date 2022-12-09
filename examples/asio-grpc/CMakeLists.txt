add_subdirectory(proto)

# examples
function(asio_grpc_add_example _asio_grpc_name)
  target_add_example(asio-grpc-example-${_asio_grpc_name} ${_asio_grpc_name}.cpp)
  target_link_libraries(asio-grpc-example-${_asio_grpc_name} asio-grpc-example-protos asio-grpc::asio-grpc)
endfunction()

asio_grpc_add_example(hello-world-client)
asio_grpc_add_example(hello-world-server)

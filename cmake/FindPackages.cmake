find_package(GTest)
find_package(Boost)
find_package(magic_enum)
find_package(robin_hood)
find_package(scn)
find_package(spdlog)
find_package(fmt)
find_package(folly)
find_package(asio-grpc)
find_package(mimalloc)

target_compile_definitions(spdlog::spdlog INTERFACE -DSPDLOG_ACTIVE_LEVEL=1)


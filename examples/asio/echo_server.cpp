#define BOOST_ASIO_HAS_CO_AWAIT
#define BOOST_ASIO_HAS_STD_COROUTINE

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>
#include <cstdio>

using boost::asio::awaitable;
using boost::asio::buffer;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;
using boost::asio::ip::tcp;
using boost::asio::signal_set;
using boost::asio::io_context;
namespace this_coro = boost::asio::this_coro;

awaitable<void> echo(tcp::socket socket) {
    try {
        char data[1024];
        for (;;) {
            std::size_t n = co_await socket.async_read_some(buffer(data), use_awaitable);
            co_await async_write(socket, buffer(data, n), use_awaitable);
        }
    } catch (std::exception & e) { std::printf("echo Exception: %s\n", e.what()); }
}

awaitable<void> listener() {
    auto executor = co_await this_coro::executor;
    tcp::acceptor acceptor(executor, {tcp::v4(), 55555});
    for (;;) {
        tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
        co_spawn(executor, echo(std::move(socket)), detached);
    }
}

int main() {
    try {
        io_context context(1);
        signal_set signals(context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) { context.stop(); });

        co_spawn(context, listener(), detached);
        context.run();
    } catch (std::exception & e) { std::printf("Exception: %s\n", e.what()); }
}

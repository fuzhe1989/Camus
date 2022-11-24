#include <folly/Random.h>
#include <folly/experimental/coro/BlockingWait.h>
#include <folly/experimental/coro/BoundedQueue.h>
#include <folly/experimental/coro/Collect.h>
#include <folly/experimental/coro/GtestHelpers.h>
#include <folly/experimental/coro/Task.h>

namespace hf::tests {
namespace {
TEST(FollyCoro, testProducerConsumer) {
    folly::coro::BoundedQueue<int, true, true> q(1);
    auto producer = [&]() -> folly::coro::Task<int64_t> {
        int64_t sum = 0;
        for (int i = 0; i < 1000; ++i) {
            auto v = folly::Random::rand32(100);
            sum += v;
            co_await q.enqueue(v);
        }
        co_return sum;
    };

    auto consumer = [&]() -> folly::coro::Task<int64_t> {
        int64_t sum = 0;
        for (int i = 0; i < 1000; ++i) {
            sum += co_await q.dequeue();
        }
        co_return sum;
    };

    folly::coro::blockingWait([&]() -> folly::coro::Task<void> {
        auto [pcount, ccount] = co_await folly::coro::collectAll(producer(), consumer());
        CO_ASSERT_EQ(pcount, ccount);
    }());
}
} // namespace
} // namespace hf::tests

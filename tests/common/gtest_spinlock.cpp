#include <common/spinlock/nginx_spinlock.h>
#include <common/spinlock/ticket_spinlock.h>
#include <common/spinlock/trivial_exchange_spinlock.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <thread>
#include <vector>

namespace camus::tests {
namespace {

template <typename T>
class SpinLockTest : public ::testing::Test {
};

using SpinLockTypes = ::testing::Types<
    NginxSpinLock,
    TrivialSpinLock,
    TicketSpinLock>;
TYPED_TEST_SUITE(SpinLockTest, SpinLockTypes);

TYPED_TEST(SpinLockTest, testLock) {
    TypeParam lk;
    static const int64_t thread_count = std::thread::hardware_concurrency();
    int64_t value = 0;

    auto f = [&] {
        for (int i = 0; i < 1000000; ++i) {
            lk.lock();
            value += 1;
            lk.unlock();
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < thread_count; ++i)
        threads.emplace_back(f);

    for (auto & t : threads)
        t.join();

    ASSERT_EQ(value, thread_count * 1000000);
}

} // namespace
} // namespace camus::tests

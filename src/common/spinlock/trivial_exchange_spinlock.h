#pragma once

#include <atomic>
#include <cstdint>
#include <thread>

namespace camus {
class TrivialSpinLock {
public:
    TrivialSpinLock() = default;

    void lock() {
        while (true) {
            int64_t expected = 0;
            if (m_lock.compare_exchange_strong(expected, 1, std::memory_order_acquire))
                return;
        }
    }

    void unlock() {
        m_lock.store(0, std::memory_order_release);
    }

private:
    alignas(64) std::atomic<int64_t> m_lock{0};
};
} // namespace camus

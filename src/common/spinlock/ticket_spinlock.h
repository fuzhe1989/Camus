#pragma once

#include <emmintrin.h>
#include <atomic>
#include <cstdint>
#include <thread>

namespace hf {
class TicketSpinLock {
public:
    TicketSpinLock() = default;

    void lock() {
        auto ticket = m_ticket.fetch_add(1, std::memory_order_acquire);
        while (true) {
            if (m_lock.load(std::memory_order_relaxed) == ticket) {
                return;
            }
        }
    }

    void unlock() {
        m_lock.fetch_add(1, std::memory_order_release);
    }
private:

    alignas(64) std::atomic<int64_t> m_ticket{0};
    alignas(64) std::atomic<int64_t> m_lock{0};
};
} // namespace hf


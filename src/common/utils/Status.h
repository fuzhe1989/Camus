#pragma once

#include <fmt/format.h>

#include <any>
#include <cstdint>
#include <string>
#include <string_view>

namespace camus {
using StatusCode = uint16_t;

class Status {
public:
    Status() = default;
    explicit Status(StatusCode code, std::string_view msg = {})
        : ptr_(code) {
        if (!msg.empty()) {
            setMessage(msg);
        }
    }

    Status(const Status & other) {
        ptr_ = other.code();
        auto * impl = other.getImpl();
        if (impl) {
            auto * selfImpl = tryInitImpl();
            *selfImpl = *impl;
        }
    }

    Status(Status && other)
        : ptr_(std::exchange(other.ptr_, 0)) {}

    Status & operator=(const Status & other) {
        if (this != std::addressof(other)) {
            auto tmp = other;
            *this = std::move(tmp);
        }
        return *this;
    }

    Status & operator=(Status && other) {
        resetImpl();
        ptr_ = std::exchange(other.ptr_, 0);
        return *this;
    }

    StatusCode code() const {
        return static_cast<StatusCode>(ptr_ & kCodeMask);
    }

    bool isOk() const {
        return code() == StatusCode(0);
    }

    std::string_view message() const {
        auto * impl = getImpl();
        if (impl)
            return impl->message;
        return {};
    }

    bool hasPayload() const {
        auto * impl = getImpl();
        if (impl)
            return impl->payload.has_value();
        return false;
    }

    template <typename T>
    const std::any * payload() const {
        auto * impl = getImpl();
        if (impl) {
            return std::any_cast<T>(impl->payload);
        }
        return nullptr;
    }

    std::string toString() const {
        auto msg = message();
        if (msg.empty())
            return fmt::format("(code={})", code());
        return fmt::format("{}(code={})", msg, code());
    }

    void setMessage(std::string_view msg) {
        auto * impl = tryInitImpl();
        impl->message = msg;
    }

    template <typename T>
    void setPayload(T && arg) {
        auto * impl = tryInitImpl();
        impl->payload = std::forward<T>(arg);
    }

    template <typename T, typename... Args>
    void emplacePayload(Args &&... args) {
        auto * impl = tryInitImpl();
        impl->payload.emplace<T>(std::forward<Args>(args)...);
    }

    void clearMessage() {
        auto * impl = getImpl();
        if (impl) {
            impl->message = {};
            if (!impl->payload.has_value())
                resetImpl();
        }
    }

    void clearPayload() {
        auto * impl = getImpl();
        if (impl) {
            impl->payload.reset();
            if (impl->message.empty())
                resetImpl();
        }
    }

private:
    struct Impl {
        std::string message;
        std::any payload;
    };

    Impl * getImpl() const {
        auto p = ptr_ >> kImplShift;
        if (p)
            return reinterpret_cast<Impl *>(p);
        return nullptr;
    }

    Impl * tryInitImpl() {
        auto * p = getImpl();
        if (p)
            return p;
        auto * impl = new Impl;
        ptr_ |= (reinterpret_cast<uintptr_t>(impl) << kImplShift);
        return impl;
    }

    void resetImpl() {
        auto * impl = getImpl();
        delete impl;
        ptr_ = code();
    }

    static_assert(sizeof(StatusCode) == 2);
    static_assert(sizeof(uintptr_t) == 8);
    static constexpr uint64_t kImplShift = sizeof(StatusCode);
    static constexpr uint64_t kCodeMask = (static_cast<uint64_t>(1) << kImplShift) - 1;
    // low 16b: status code
    // high 48b: impl
    uintptr_t ptr_{0};
};

} // namespace camus

// source: https://github.com/ClickHouse/ClickHouse/blob/69fc75a717da98d92e5278fbb6fe792d9440c333/base/base/strong_typedef.h

#pragma once

#include <fmt/format.h>

#include <functional>
#include <type_traits>
#include <utility>

namespace camus {
template <typename T, typename Tag>
struct StrongTypedef {
private:
    using Self = StrongTypedef;
    T t;

public:
    using UnderlyingType = T;
    constexpr explicit StrongTypedef(const T & t_) requires(std::is_copy_constructible_v<T>)
        : t(t_) {}
    constexpr explicit StrongTypedef(T && t_) requires(std::is_move_constructible_v<T>)
        : t(std::move(t_)) {}

    constexpr StrongTypedef() requires(std::is_default_constructible_v<T>)
        : t() {}

    constexpr StrongTypedef(const Self &) = default;
    constexpr StrongTypedef(Self &&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;

    Self & operator=(const Self &) = default;
    Self & operator=(Self &&) noexcept(std::is_nothrow_move_assignable_v<T>) = default;

    // disable assignment between two StrongTypedef of T with different Tags.
    // use `lhs.toUnderType() = rhs` instead.
    Self & operator=(T) = delete;
    Self & operator=(const T &) = delete;

    Self & operator=(T && rhs) requires(std::is_move_assignable_v<T>) {
        t = std::move(rhs);
        return *this;
    }

    // NOLINTBEGIN(google-explicit-constructor)
    constexpr operator const T &() const { return t; }
    constexpr operator T &() { return t; }
    // NOLINTEND(google-explicit-constructor)

    auto operator<=>(const Self &) const = default;

    constexpr T & toUnderType() { return t; }
    constexpr const T & toUnderType() const { return t; }
};

} // namespace camus

template <typename T, typename Tag>
struct std::hash<camus::StrongTypedef<T, Tag>> {
    size_t operator()(const camus::StrongTypedef<T, Tag> & x) const {
        return std::hash<T>()(x.toUnderType());
    }
};

template <typename T, typename Tag>
struct fmt::formatter<camus::StrongTypedef<T, Tag>> : formatter<T> {
    template <typename FmtContext>
    auto format(const camus::StrongTypedef<T, Tag> & x, FmtContext & ctx) const {
        return formatter<T>::format(x.toUnderType(), ctx);
    }
};

// NOLINTBEGIN(bugprone-macro-parentheses)

#define STRONG_TYPEDEF(T, D) \
  struct D##Tag {};          \
  using D = ::camus::StrongTypedef<T, D##Tag>

// NOLINTEND(bugprone-macro-parentheses)

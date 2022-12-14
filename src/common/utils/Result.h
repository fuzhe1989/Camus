#pragma once

#include <folly/Expected.h>

#include "Status.h"

namespace camus {
template <typename T>
using Result = folly::Expected<T, Status>;

using Void = folly::Unit;

template <typename... Args>
[[nodiscard]] inline folly::Unexpected<Status> makeError(Args &&... args) {
    return folly::makeUnexpected(Status(std::forward<Args>(args)...));
}
} // namespace camus

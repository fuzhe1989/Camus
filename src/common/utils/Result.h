#pragma once

#include <folly/Expected.h>

#include "Status.h"

namespace camus {
template <typename T>
using Result = folly::Expected<T, Status>;

using Void = folly::Unit;
} // namespace camus

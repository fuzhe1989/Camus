#pragma once

#include <cstdint>
#include <string>

#include "common/utils/Status.h"
#include "common/utils/StrongTypedef.h"

namespace camus::raft::v0 {
STRONG_TYPEDEF(int64_t, Term);
STRONG_TYPEDEF(std::string, NodeId);
STRONG_TYPEDEF(int64_t, LogIndex);
STRONG_TYPEDEF(uint64_t, Timestamp);
STRONG_TYPEDEF(uint64_t, Interval);

inline StatusCode kNotLeader = 1;
} // namespace camus::raft::v0

#pragma once

#include <cstdint>
#include <string>

#include "common/utils/Status.h"
#include "common/utils/StrongTypedef.h"

namespace camus::raft::v0 {
STRONG_TYPEDEF(size_t, Term);
STRONG_TYPEDEF(std::string, NodeId);
STRONG_TYPEDEF(size_t, LogIndex);
STRONG_TYPEDEF(size_t, Timestamp);
STRONG_TYPEDEF(size_t, Interval);

inline constexpr StatusCode kNotLeader = 1;
inline constexpr StatusCode kStaleTerm = 2;
inline constexpr StatusCode kPrevLogMismatch = 3;
inline constexpr StatusCode kVoteGranted = 4;
inline constexpr StatusCode kStaleLogIndex = 5;
} // namespace camus::raft::v0

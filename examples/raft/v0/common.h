#pragma once

#include <cstdint>
#include <string>

#include "common/utils/StrongTypedef.h"

namespace camus::raft::v0 {
STRONG_TYPEDEF(int64_t, Term);
STRONG_TYPEDEF(std::string, NodeId);
STRONG_TYPEDEF(int64_t, LogIndex);
} // namespace camus::raft::v0

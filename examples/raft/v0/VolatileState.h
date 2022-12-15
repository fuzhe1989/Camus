#pragma once

#include "common.h"

namespace camus::raft::v0 {
struct CommonVolatileState {
    // index of highest log entry known to be committed (initialized to 0, increases monotonically)
    LogIndex commitIndex{0};
    // index of highest log entry applied to state machine (initialized to 0, increases monotonically)
    LogIndex lastApplied{0};
};

struct LeaderVolatileState {
    // for each server, index of the next log entry to send to that server (initialized to leader last log index + 1)
    std::vector<LogIndex> nextIndice;
    // for each server, index of highest log entry known to be replicated on server (initialized to 0, increases monotonically)
    std::vector<LogIndex> matchIndice;
};
} // namespace camus::raft::v0

#pragma once

#include <map>

#include "Message.h"
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
    std::map<NodeId, LogIndex> nextIndice;
    // for each server, index of highest log entry known to be replicated on server (initialized to 0, increases monotonically)
    std::map<NodeId, LogIndex> matchIndice;

    Timestamp leaseStart{0};
    Timestamp leaseEnd{0};

    std::map<LogIndex, Message> pendingWriteRequests;
};

struct FollowerVolatileState {
    Timestamp lastHeartbeatTime{0};
};

struct CandidateVolatileState {
};
} // namespace camus::raft::v0

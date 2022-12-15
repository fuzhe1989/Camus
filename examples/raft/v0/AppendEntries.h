#pragma once

#include "Command.h"
#include "common.h"

namespace camus::raft::v0 {
struct AppendEntriesRequest {
    // leader’s term
    Term term{0};
    // so follower can redirect clients
    NodeId leaderId;
    // index of log entry immediately preceding new ones
    LogIndex prevLogIndex{0};
    // term of prevLogIndex entry
    Term prevLogTerm{0};
    // log entries to store (empty for heartbeat; may send more than one for efficiency)
    std::vector<Command> entries;
    // leader’s commitIndex
    LogIndex leaderCommit{0};
};

struct AppendEntriesResponse {
    // currentTerm, for leader to update itself
    Term term{0};
    // true if follower contained entry matching prevLogIndex and prevLogTerm
    bool success = false;
};
} // namespace camus::raft::v0

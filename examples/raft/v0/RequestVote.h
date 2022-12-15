#pragma once

#include "common.h"

namespace camus::raft::v0 {
struct RequestVoteRequest {
    // candidate’s term
    Term term{0};
    // candidate requesting vote
    NodeId candidateId;
    // index of candidate’s last log entry
    LogIndex lastLogIndex{0};
    // term of candidate’s last log entry
    Term lastLogTerm{0};
};

struct RequestVoteResponse {
    // currentTerm, for candidate to update itself
    Term term{0};
    // true means candidate received vote
    bool voteGranted = false;
};
} // namespace camus::raft::v0

#pragma once

#include "Payload.h"
#include "common.h"

namespace camus::raft::v0 {
struct RequestVoteRequest : Payload {
    // candidate’s term
    Term term{0};
    // candidate requesting vote
    NodeId candidateId;
    // index of candidate’s last log entry
    LogIndex lastLogIndex{0};
    // term of candidate’s last log entry
    Term lastLogTerm{0};

    std::string toString() const override;
};

struct RequestVoteResponse : Payload {
    // currentTerm, for candidate to update itself
    Term term{0};
    // true means candidate received vote
    bool voteGranted = false;

    std::string toString() const override;
};
} // namespace camus::raft::v0

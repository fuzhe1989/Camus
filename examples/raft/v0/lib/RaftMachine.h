#pragma once

#include <optional>

#include "MachineBase.h"
#include "PersistentState.h"
#include "VolatileState.h"
#include "common.h"

namespace camus::raft::v0 {
enum class Role {
    LEADER,
    CANDIDATE,
    FOLLOWER,
};

struct RaftMachine : MachineBase {
    void startImpl(Timestamp now) override;
    void shutdownImpl(bool critical) override;
    void handleImpl(Timestamp now) override;
    void handleMessage(Timestamp now, Message msg) override;
    void handleRequestTimeout(Timestamp now, Message msg) override;

    void handleAppendEntriesRequest(Timestamp now, Message msg);
    void handleAppendEntriesResponse(Timestamp now, Message msg);
    void handleRequestVoteRequest(Timestamp now, Message msg);
    void handleRequestVoteResponse(Timestamp now, Message msg);
    void handleWriteRequest(Timestamp now, Message msg);
    void handleReadRequest(Timestamp now, Message msg);

    Role role() const;

    PersistentState persistentState;
    CommonVolatileState volatileState;
    std::variant<LeaderVolatileState, FollowerVolatileState, CandidateVolatileState> roleState;
    std::map<std::string, std::string> data;
};
} // namespace camus::raft::v0

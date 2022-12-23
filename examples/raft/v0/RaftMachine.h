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
    void handleMessageImpl(Timestamp now, const Message & msg) override;

    void handleAppendEntriesRequest(Timestamp now, const Message & msg);
    void handleAppendEntriesResponse(Timestamp now, const Message & msg);
    void handleRequestVoteRequest(Timestamp now, const Message & msg);
    void handleRequestVoteResponse(Timestamp now, const Message & msg);

    Role role{Role::FOLLOWER};
    PersistentState persistentState;
    CommonVolatileState volatileState;
    std::optional<LeaderVolatileState> leaderState;
    std::map<std::string, std::string> data;
};
} // namespace camus::raft::v0

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
    void handleRequest(Timestamp now, Message msg) override;
    void handleResponse(Timestamp now, Message request, Message response) override;
    void handleRequestTimeout(Timestamp now, Message msg) override;

    void handleAppendEntriesRequest(Timestamp now, Message msg);
    void handleAppendEntriesResponse(Timestamp now, Message request, Message response);
    void handleRequestVoteRequest(Timestamp now, Message msg);
    void handleRequestVoteResponse(Timestamp now, Message request, Message response);
    void handleWriteRequest(Timestamp now, Message msg);
    void handleReadRequest(Timestamp now, Message msg);

    void sendAppendEntriesRequests(Timestamp now);

    Role role() const;

    PersistentState persistentState;
    CommonVolatileState volatileState;
    std::variant<LeaderVolatileState, FollowerVolatileState, CandidateVolatileState> roleState;
    std::map<std::string, std::string> data;
    std::vector<NodeId> nodes;
};
} // namespace camus::raft::v0
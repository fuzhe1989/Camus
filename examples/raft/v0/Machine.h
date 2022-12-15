#pragma once

#include <deque>
#include <map>
#include <optional>

#include "Message.h"
#include "PersistentState.h"
#include "VolatileState.h"
#include "common.h"

namespace camus::raft::v0 {
enum class MachineState {
    NORMAL,
    SHUTDOWN,
    NET_IN_BROKEN,
    NET_OUT_BROKEN,
    HANG,
};

enum class MachineEvent {
    SHUTDOWN,
    CORRUPTION,
    NET_IN_BORKEN,
    NET_OUT_BROKEN,
    HANG,
};

enum class Role {
    LEADER,
    CANDIDATE,
    FOLLOWER,
};

struct Machine {
    void start(Timestamp now);
    void shutdown(bool critical);
    void handle(Timestamp now);
    void handleMessage(Timestamp now, const Message & msg);
    void handleAppendEntriesRequest(Timestamp now, const Message & msg);
    void handleAppendEntriesResponse(Timestamp now, const Message & msg);
    void handleRequestVoteRequest(Timestamp now, const Message & msg);
    void handleRequestVoteResponse(Timestamp now, const Message & msg);
    void insertMessage(Message msg);
    void receive(Timestamp now, Message msg);
    void triggerEvent(Timestamp now, MachineEvent e);
    void checkRecover(Timestamp now);
    void send(Timestamp now, Machine * m, Message msg);
    void discardTimeoutRequests(Timestamp now);

    NodeId id;
    std::map<NodeId, Machine *> * machines = nullptr;

    std::deque<Message> mailbox;
    std::map<int64_t, Message> ongoingRequests;

    MachineState state{MachineState::SHUTDOWN};
    std::optional<Timestamp> recoverTime;
    std::map<std::string, std::string> data;

    Role role{Role::FOLLOWER};
    PersistentState persistentState;
    CommonVolatileState volatileState;
    std::optional<LeaderVolatileState> leaderState;
};
} // namespace camus::raft::v0

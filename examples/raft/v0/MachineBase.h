#pragma once

#include <deque>
#include <map>
#include <optional>

#include "Message.h"
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

struct MachineBase {
    void start(Timestamp now);
    void shutdown(bool critical);
    void handle(Timestamp now);
    void handleMessage(Timestamp now, const Message & msg);
    void insertMessage(Message msg);
    void receive(Timestamp now, Message msg);
    void triggerEvent(Timestamp now, MachineEvent e);
    void checkRecover(Timestamp now);
    void send(Timestamp now, MachineBase * m, Message msg);
    void discardTimeoutRequests(Timestamp now);

    virtual void startImpl(Timestamp now) = 0;
    virtual void shutdownImpl(bool critical) = 0;
    virtual void handleImpl(Timestamp now) = 0;
    virtual void handleMessageImpl(Timestamp now, const Message & msg) = 0;

    NodeId id;
    std::map<NodeId, MachineBase *> * machines = nullptr;

    std::deque<Message> mailbox;
    std::map<int64_t, Message> ongoingRequests;

    MachineState state{MachineState::SHUTDOWN};
    std::optional<Timestamp> recoverTime;
};
} // namespace camus::raft::v0

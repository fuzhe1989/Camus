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
    void handleMessages(Timestamp now);
    void insertMessage(Message msg);
    void receive(Timestamp now, Message msg);
    void triggerEvent(Timestamp now, MachineEvent e);
    void checkRecover(Timestamp now);
    void send(Timestamp now, MachineBase * m, Message msg);
    void discardTimeoutRequests(Timestamp now);

    virtual void startImpl(Timestamp now) = 0;
    virtual void shutdownImpl(bool critical) = 0;
    virtual void handleImpl(Timestamp now) = 0;
    virtual void handleRequest(Timestamp now, Message msg) = 0;
    virtual void handleResponse(Timestamp now, Message request, Message response) = 0;
    virtual void handleRequestTimeout(Timestamp now, Message msg) = 0;

    NodeId id;
    const std::map<NodeId, MachineBase *> * machines = nullptr;

    std::deque<Message> mailbox;
    std::map<int64_t, Message> ongoingRequests;

    MachineState state{MachineState::SHUTDOWN};
    std::optional<Timestamp> recoverTime;
};
} // namespace camus::raft::v0

#define MLOG(fmt, ...)

#define MASSERT(condition, fmt, ...)

#include "AppendEntries.h"
#include "MessageFmt.h"
#include "Parameters.h"
#include "RaftMachine.h"
#include "ReadRequest.h"
#include "RequestVote.h"
#include "WriteRequest.h"

namespace camus::raft::v0 {
Role RaftMachine::role() const {
    switch (roleState.index()) {
    case 0:
        return Role::LEADER;
    case 1:
        return Role::FOLLOWER;
    case 2:
        return Role::CANDIDATE;
    default:
        return Role::FOLLOWER;
    }
}

void RaftMachine::startImpl(Timestamp) {
    volatileState = CommonVolatileState{};
    roleState = FollowerVolatileState{};
}

void RaftMachine::shutdownImpl(bool critical) {
    if (critical) {
        persistentState = PersistentState{};
        data.clear();
    }
}

void RaftMachine::handleImpl(Timestamp now) {
    // TODO
    (void)now;
}

void RaftMachine::handleMessage(Timestamp now, Message msg) {
    if (dynamic_cast<AppendEntriesRequest *>(msg.payload.get())) {
        handleAppendEntriesRequest(now, std::move(msg));
    } else if (dynamic_cast<AppendEntriesResponse *>(msg.payload.get())) {
        handleAppendEntriesResponse(now, std::move(msg));
    } else if (dynamic_cast<RequestVoteRequest *>(msg.payload.get())) {
        handleRequestVoteRequest(now, std::move(msg));
    } else if (dynamic_cast<RequestVoteResponse *>(msg.payload.get())) {
        handleRequestVoteResponse(now, std::move(msg));
    } else if (dynamic_cast<WriteRequest *>(msg.payload.get())) {
        handleWriteRequest(now, std::move(msg));
    } else if (dynamic_cast<ReadRequest *>(msg.payload.get())) {
        handleReadRequest(now, std::move(msg));
    } else {
        MASSERT(false, "invalid message type:{}", msg);
    }
}

void RaftMachine::handleAppendEntriesRequest(Timestamp now, Message msg) {
    // TODO
    (void)now;
    (void)msg;
}

void RaftMachine::handleAppendEntriesResponse(Timestamp now, Message msg) {
    // TODO
    (void)now;
    (void)msg;
}

void RaftMachine::handleRequestVoteRequest(Timestamp now, Message msg) {
    // TODO
    (void)now;
    (void)msg;
}

void RaftMachine::handleRequestVoteResponse(Timestamp now, Message msg) {
    // TODO
    (void)now;
    (void)msg;
}

void RaftMachine::handleWriteRequest(Timestamp now, Message msg) {
    auto payload = dynamic_pointer_cast<WriteRequest>(msg.payload);
    auto rsp = std::make_shared<WriteResponse>();

    [&] {
        if (role() != Role::LEADER) {
            rsp->success = false;
            rsp->leader = persistentState.voteFor;
            return;
        }
        auto & leaderState = std::get<LeaderVolatileState>(roleState);
        if (leaderState.leaseEnd <= now + parameters::heartbeatInterval) {
            rsp->success = false;
            return;
        }
        persistentState.logs.push_back(payload->command);
        leaderState.pendingRequests[LogIndex(persistentState.logs.size() - 1)] = msg;
    }();

    // TODO
}

void RaftMachine::handleReadRequest(Timestamp now, Message msg) {
    auto payload = dynamic_pointer_cast<ReadRequest>(msg.payload);
    auto rsp = std::make_shared<ReadResponse>();

    if (role() != Role::LEADER) {
        rsp->success = false;
        rsp->leader = persistentState.voteFor;
    } else {
        auto & leaderState = std::get<LeaderVolatileState>(roleState);
        if (leaderState.leaseEnd <= now + parameters::heartbeatInterval) {
            rsp->success = false;
        } else if (data.contains(payload->key)) {
            rsp->success = true;
            rsp->value = data[payload->key];
        } else {
            rsp->success = true;
        }
    }
    send(now, msg.from, Message::makeRsp(now, this, msg.requestId, std::move(rsp)));
}
} // namespace camus::raft::v0

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

void RaftMachine::handleRequest(Timestamp now, Message msg) {
    if (dynamic_cast<AppendEntriesRequest *>(msg.payload.get())) {
        handleAppendEntriesRequest(now, std::move(msg));
    } else if (dynamic_cast<RequestVoteRequest *>(msg.payload.get())) {
        handleRequestVoteRequest(now, std::move(msg));
    } else if (dynamic_cast<WriteRequest *>(msg.payload.get())) {
        handleWriteRequest(now, std::move(msg));
    } else if (dynamic_cast<ReadRequest *>(msg.payload.get())) {
        handleReadRequest(now, std::move(msg));
    } else {
        MASSERT(false, "invalid message type:{}", msg);
    }
}

void RaftMachine::handleResponse(Timestamp now, Message request, Message response) {
    if (dynamic_cast<AppendEntriesResponse *>(response.payload.get())) {
        handleAppendEntriesResponse(now, std::move(request), std::move(response));
    } else if (dynamic_cast<RequestVoteResponse *>(response.payload.get())) {
        handleRequestVoteResponse(now, std::move(request), std::move(response));
    } else {
        MASSERT(false, "invalid message type:{}", response);
    }
}

void RaftMachine::handleAppendEntriesRequest(Timestamp now, Message msg) {
    // TODO
    (void)now;
    (void)msg;
}

void RaftMachine::handleAppendEntriesResponse(Timestamp now, Message request, Message response) {
    // TODO
    (void)now;
    (void)request;
    (void)response;
}

void RaftMachine::handleRequestVoteRequest(Timestamp now, Message msg) {
    // TODO
    (void)now;
    (void)msg;
}

void RaftMachine::handleRequestVoteResponse(Timestamp now, Message request, Message response) {
    // TODO
    (void)now;
    (void)request;
    (void)response;
}

void RaftMachine::handleWriteRequest(Timestamp now, Message msg) {
    auto payload = dynamic_pointer_cast<WriteRequest>(msg.payload);
    auto rsp = std::make_shared<WriteResponse>();

    auto sendBack = [&] {
        send(now, msg.from, Message::makeRsp(now, this, msg.requestId, std::move(rsp)));
    };

    if (role() != Role::LEADER) {
        rsp->status = Status(kNotLeader);
        rsp->leader = persistentState.voteFor;
        sendBack();
        return;
    }
    auto & leaderState = std::get<LeaderVolatileState>(roleState);
    if (leaderState.leaseEnd <= now + parameters::heartbeatInterval) {
        rsp->success = false;
        sendBack();
        return;
    }
    persistentState.logs.push_back({persistentState.currentTerm, payload->command});
    if (nodes.size() == 1) {
        // TODO: directly commit
    } else {
        leaderState.pendingWriteRequests[LogIndex(persistentState.logs.size() - 1)] = msg;
        sendAppendEntriesRequests(now);
    }
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

void RaftMachine::sendAppendEntriesRequests(Timestamp now) {
    auto lastIndex = LogIndex(persistentState.logs.size());
    auto & leaderState = std::get<LeaderVolatileState>(roleState);
    for (const auto & nodeId : nodes) {
        if (nodeId == id)
            continue;
        auto nextIndex = leaderState.nextIndice[nodeId];
        if (nextIndex < lastIndex) {
            auto payload = std::make_shared<AppendEntriesRequest>();
            payload->term = persistentState.currentTerm;
            payload->leaderId = id;
            payload->prevLogIndex = LogIndex(nextIndex - 1);
            payload->prevLogTerm = persistentState.logs[nextIndex - 1].term;
            for (auto i = nextIndex; i < lastIndex; ++i)
                payload->entries.push_back(persistentState.logs[nextIndex]);
            payload->leaderCommit = volatileState.commitIndex;

            send(
                now,
                machines->at(nodeId),
                Message::makeReq(
                    now,
                    Interval(parameters::appendEntriesRequestTimeout),
                    this,
                    std::move(payload)));
        }
    }
}
} // namespace camus::raft::v0

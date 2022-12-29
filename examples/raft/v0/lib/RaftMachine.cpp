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

void RaftMachine::startImpl(Timestamp now) {
    volatileState = CommonVolatileState{};
    roleState = FollowerVolatileState{};
    asFollower().lastHeartbeatReceivedTime = now;
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
    (void)now;
    (void)msg;

    auto payload = dynamic_pointer_cast<AppendEntriesRequest>(msg.payload);
    auto rsp = std::make_shared<AppendEntriesResponse>();

    // TODO: what if payload->term == persistentState.currentTerm while payload->leaderId != persistentState.leaderId?
    // should we reject this request or overwrite persistentState?

    if (payload->term < persistentState.currentTerm) {
        rsp->status = Status(kStaleTerm);
    } else if (persistentState.logs.size() < payload->prevLogIndex) {
        rsp->status = Status(kPrevLogMismatch);
    } else if (persistentState.logs[payload->prevLogIndex - 1].term != payload->prevLogTerm) {
        rsp->status = Status(kPrevLogMismatch);
    } else {
        if (role() != Role::FOLLOWER) {
            roleState = FollowerVolatileState();
            asFollower().lastHeartbeatReceivedTime = now;
            persistentState.currentTerm = payload->term;
            persistentState.leader = payload->leaderId;
        }

        for (size_t i = 0; i < payload->entries.size(); ++i) {
            auto j = payload->prevLogIndex + i - 1;
            if (j < persistentState.logs.size()) {
                if (persistentState.logs[j].term != payload->entries[i].term) {
                    persistentState.logs.resize(j);
                    persistentState.logs.push_back(payload->entries[i]);
                }
            } else {
                persistentState.logs.push_back(payload->entries[i]);
            }
        }

        if (payload->leaderCommit > volatileState.commitIndex) {
            volatileState.commitIndex = std::min(
                payload->leaderCommit,
                LogIndex(persistentState.logs.size()));
        }
    }

    rsp->term = persistentState.currentTerm;
    sendBack(now, msg, std::move(rsp));
}

void RaftMachine::handleAppendEntriesResponse(Timestamp now, Message request, Message response) {
    (void)now;
    (void)request;
    (void)response;

    auto reqPayload = dynamic_pointer_cast<AppendEntriesRequest>(request.payload);
    auto rspPayload = dynamic_pointer_cast<AppendEntriesResponse>(response.payload);

    if (role() != Role::LEADER)
        return;

    auto peerId = response.from->id;
    if (reqPayload->prevLogIndex != asLeader().nextIndice[peerId] - 1)
        return;

    if (reqPayload->term != persistentState.currentTerm)
        return;

    if (rspPayload->status.isOk()) {
        auto newNextIndex = LogIndex(reqPayload->prevLogIndex + reqPayload->entries.size() + 1);
        asLeader().nextIndice[peerId] = newNextIndex;
        asLeader().matchIndice[peerId] = LogIndex(newNextIndex - 1);

        tryPromoteCommitIndex();
    } else {
        switch (rspPayload->status.code()) {
        case kPrevLogMismatch:
            --asLeader().nextIndice[peerId];
            sendAppendEntriesRequests(now, peerId);
            break;
        default:
            break;
        }
    }
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

    if (role() != Role::LEADER) {
        rsp->status = Status(kNotLeader);
        rsp->leader = persistentState.voteFor;
        sendBack(now, msg, std::move(rsp));
        return;
    }
    if (asLeader().leaseEnd <= now + parameters::heartbeatInterval) {
        rsp->status = Status(kNotLeader);
        sendBack(now, msg, std::move(rsp));
        return;
    }
    persistentState.logs.push_back({persistentState.currentTerm, payload->command});
    if (nodes.size() == 1) {
        // TODO: directly commit
    } else {
        asLeader().pendingWriteRequests[LogIndex(persistentState.logs.size())] = msg;
        sendAppendEntriesRequests(now);
    }
}

void RaftMachine::handleReadRequest(Timestamp now, Message msg) {
    auto payload = dynamic_pointer_cast<ReadRequest>(msg.payload);
    auto rsp = std::make_shared<ReadResponse>();

    if (role() != Role::LEADER) {
        rsp->status = Status(kNotLeader);
        rsp->leader = persistentState.voteFor;
    } else {
        if (asLeader().leaseEnd <= now + parameters::heartbeatInterval) {
            rsp->status = Status(kNotLeader);
        } else if (data.contains(payload->key)) {
            rsp->value = data[payload->key];
        }
    }
    send(now, msg.from, Message::makeRsp(now, this, msg.requestId, std::move(rsp)));
}

void RaftMachine::sendAppendEntriesRequests(Timestamp now) {
    for (const auto & nodeId : nodes) {
        if (nodeId == id)
            continue;
        sendAppendEntriesRequests(now, nodeId);
    }
}

void RaftMachine::sendAppendEntriesRequests(Timestamp now, const NodeId & nodeId) {
    auto lastIndex = LogIndex(persistentState.logs.size());
    auto nextIndex = asLeader().nextIndice[nodeId];
    auto payload = std::make_shared<AppendEntriesRequest>();
    payload->term = persistentState.currentTerm;
    payload->leaderId = id;
    payload->prevLogIndex = LogIndex(nextIndex - 1);
    payload->prevLogTerm = nextIndex == 1 ? Term(0) : persistentState.logs[nextIndex - 2].term;
    for (auto i = nextIndex; i <= lastIndex; ++i)
        payload->entries.push_back(persistentState.logs[i - 1]);
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

LeaderVolatileState & RaftMachine::asLeader() {
    return std::get<LeaderVolatileState>(roleState);
}

FollowerVolatileState & RaftMachine::asFollower() {
    return std::get<FollowerVolatileState>(roleState);
}

CandidateVolatileState & RaftMachine::asCandidate() {
    return std::get<CandidateVolatileState>(roleState);
}

void RaftMachine::sendBack(Timestamp now, const Message & request, std::shared_ptr<Payload> payload) {
    send(now, request.from, Message::makeRsp(now, this, request.requestId, std::move(payload)));
}

void RaftMachine::tryPromoteCommitIndex() {
    auto minIndex = LogIndex(std::numeric_limits<size_t>::max());
    for ([[maybe_unused]] const auto & [_, v] : asLeader().matchIndice)
        minIndex = std::min(minIndex, v);
    minIndex = std::max(minIndex, volatileState.commitIndex);
    for (size_t i = minIndex + 1; i <= persistentState.logs.size(); ++i) {
        size_t cnt = 1;
        for ([[maybe_unused]] const auto & [_, v] : asLeader().matchIndice)
            cnt += v >= i;
        if (cnt * 2 < nodes.size()) {
            volatileState.commitIndex = LogIndex(i - 1);
            break;
        }
    }
}
} // namespace camus::raft::v0

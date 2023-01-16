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
    if (role() == Role::FOLLOWER) {
        auto & volatileState = asFollower();
        if (volatileState.lastHeartbeatReceivedTime + parameters::heartbeatTimeout < now) {
            convertToCandidate(now);
        }
    } else if (role() == Role::CANDIDATE) {
        auto & volatileState = asCandidate();
        if (volatileState.electionStartTime + parameters::electionTimeout < now) {
            // TODO: should we retry current round or start next round of vote?
        }
    } else {
        auto & volatileState = asLeader();
        if (volatileState.lastHeartbeatSentTime + parameters::heartbeatInterval < now) {
            sendAppendEntriesRequests(now);
        }
    }
}

void RaftMachine::handleRequestTimeout(Timestamp now, Message msg) {
    (void)now;
    (void)msg;
    // TODO
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
    auto payload = dynamic_pointer_cast<AppendEntriesRequest>(msg.payload);
    auto rsp = std::make_shared<AppendEntriesResponse>();

    if (payload->term < persistentState.currentTerm) {
        rsp->status = Status(kStaleTerm);
    } else if (persistentState.logs.size() < payload->prevLogIndex) {
        rsp->status = Status(kPrevLogMismatch);
    } else if (persistentState.logs[payload->prevLogIndex - 1].term != payload->prevLogTerm) {
        rsp->status = Status(kPrevLogMismatch);
    } else {
        if (payload->term == persistentState.currentTerm) {
            assert(!persistentState.leader || payload->leaderId == *persistentState.leader);
        }

        convertToFollower(now, payload->term, payload->leaderId);

        persistentState.voteFor.reset();
        persistentState.logs.resize(payload->prevLogIndex);
        persistentState.logs.insert(
            persistentState.logs.end(),
            payload->entries.begin(),
            payload->entries.end());

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
    auto reqPayload = dynamic_pointer_cast<AppendEntriesRequest>(request.payload);
    auto rspPayload = dynamic_pointer_cast<AppendEntriesResponse>(response.payload);

    if (role() != Role::LEADER)
        return;

    auto peerId = response.from->id;
    if (reqPayload->prevLogIndex != asLeader().nextIndice[peerId] - 1)
        return;

    if (reqPayload->term != persistentState.currentTerm)
        return;

    if (rspPayload->term > persistentState.currentTerm) {
        convertToFollower(now, rspPayload->term, std::nullopt);
        return;
    }

    if (rspPayload->status.isOk()) {
        auto newNextIndex = LogIndex(reqPayload->prevLogIndex + reqPayload->entries.size() + 1);
        asLeader().nextIndice[peerId] = newNextIndex;
        asLeader().matchIndice[peerId] = LogIndex(newNextIndex - 1);

        tryPromoteCommitIndex(now);
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
    auto payload = dynamic_pointer_cast<RequestVoteRequest>(msg.payload);
    auto rsp = std::make_shared<RequestVoteResponse>();

    if (payload->term < persistentState.currentTerm) {
        rsp->status = Status(kStaleTerm);
    }

    [&] {
        if (persistentState.voteFor && payload->candidateId != *persistentState.voteFor) {
            rsp->status = Status(kVoteGranted);
            return;
        }
        if (payload->lastLogIndex < persistentState.logs.size()) {
            rsp->status = Status(kStaleLogIndex);
            return;
        }
        convertToFollower(now, payload->term, std::nullopt);
        persistentState.voteFor = payload->candidateId;
    }();

    rsp->term = persistentState.currentTerm;
    sendBack(now, msg, std::move(rsp));
}

void RaftMachine::handleRequestVoteResponse(Timestamp now, Message request, Message response) {
    auto reqPayload = dynamic_pointer_cast<RequestVoteRequest>(request.payload);
    auto rspPayload = dynamic_pointer_cast<RequestVoteResponse>(response.payload);

    if (role() != Role::CANDIDATE)
        return;

    auto peerId = response.from->id;
    if (reqPayload->lastLogIndex != persistentState.logs.size())
        return;

    if (reqPayload->term != persistentState.currentTerm)
        return;

    if (rspPayload->term > persistentState.currentTerm) {
        convertToFollower(now, rspPayload->term, std::nullopt);
        return;
    }

    if (rspPayload->status.isOk()) {
        asCandidate().votes.insert(peerId);
        if (asCandidate().votes.size() * 2 > nodes.size())
            convertToLeader(now);
    }
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
        tryPromoteCommitIndex(now);
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
    payload->prevLogTerm = nextIndex == LogIndex(1) ? Term(0) : persistentState.logs[nextIndex - 2].term;
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

void RaftMachine::tryPromoteCommitIndex(Timestamp now) {
    auto oldCommitIndex = volatileState.commitIndex;
    if (nodes.size() == 1) {
        volatileState.commitIndex = LogIndex(persistentState.logs.size());
    } else {
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
    if (oldCommitIndex != volatileState.commitIndex) {
        std::map<LogIndex, Message> tmp;
        for (auto & [li, msg] : asLeader().pendingWriteRequests) {
            if (li <= volatileState.commitIndex) {
                auto rsp = std::make_shared<WriteResponse>();
                sendBack(now, msg, std::move(rsp));
            } else {
                tmp[li] = std::move(msg);
            }
        }
        asLeader().pendingWriteRequests = std::move(tmp);
    }
}

void RaftMachine::convertToFollower(Timestamp now, Term term, std::optional<NodeId> leaderId) {
    if (role() != Role::FOLLOWER) {
        roleState = FollowerVolatileState();
        asFollower().lastHeartbeatReceivedTime = now;
    }
    persistentState.currentTerm = term;
    persistentState.voteFor.reset();
    persistentState.leader = std::move(leaderId);
}

void RaftMachine::convertToCandidate(Timestamp now) {
    if (role() != Role::CANDIDATE) {
        CandidateVolatileState state;
        state.votes.insert(id);
        state.electionStartTime = now;

        roleState = state;
    }

    persistentState.currentTerm = Term(persistentState.currentTerm + 1);
    persistentState.voteFor = id;
    persistentState.leader.reset();
}

void RaftMachine::convertToLeader(Timestamp now) {
    if (role() != Role::LEADER) {
        LeaderVolatileState state;
        for (const auto & nodeId : nodes) {
            if (nodeId == id)
                continue;
            state.nextIndice[nodeId] = LogIndex(persistentState.logs.size() + 1);
            state.matchIndice[nodeId] = LogIndex(0);
        }

        state.leaseStart = now;
        state.leaseEnd = Timestamp(now + parameters::leaseLength);

        roleState = state;
    }

    persistentState.leader = id;
    persistentState.voteFor.reset();
}
} // namespace camus::raft::v0

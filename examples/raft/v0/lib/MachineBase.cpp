#include <folly/Random.h>

#include <magic_enum.hpp>

#include "MachineBase.h"
#include "Parameters.h"

namespace camus::raft::v0 {
void MachineBase::start(Timestamp now) {
    mailbox.clear();
    ongoingRequests.clear();
    state = MachineState::NORMAL;
    recoverTime.reset();

    startImpl(now);

    MLOG("start");
}

void MachineBase::shutdown(bool critical) {
    shutdownImpl(critical);
}

void MachineBase::handle(Timestamp now) {
    checkRecover(now);

    if (state != MachineState::SHUTDOWN && state != MachineState::HANG) {
        discardTimeoutRequests(now);
        handleMessages(now);
        handleImpl(now);
    } else {
        MLOG("skip handle: {}", magic_enum::enum_name(state));
    }
}

void MachineBase::handleMessages(Timestamp now) {
    while (!mailbox.empty() && mailbox.front().arriveTime <= now) {
        auto msg = std::move(mailbox.front());
        mailbox.pop_front();

        MLOG("handle message: {}", msg);

        if (msg.isRequest) {
            handleRequest(now, std::move(msg));
        } else if (ongoingRequests.contains(msg.requestId)) {
            auto request = std::move(ongoingRequests[msg.requestId]);
            ongoingRequests.erase(msg.requestId);
            handleResponse(now, std::move(request), std::move(msg));
        } else {
            MLOG("discard response: corresponding request not found");
        }
    }
}

void MachineBase::insertMessage(Message msg) {
    mailbox.push_back(std::move(msg));
    for (auto i = mailbox.size(); i > 1; --i) {
        if (mailbox[i - 1].arriveTime < mailbox[i - 2].arriveTime) {
            std::swap(mailbox[i - 1], mailbox[i - 2]);
        } else {
            break;
        }
    }
}

void MachineBase::receive(Timestamp now, Message msg) {
    if (state != MachineState::SHUTDOWN && state != MachineState::NET_IN_BROKEN) {
        MLOG("receive message: {}", msg);
        insertMessage(std::move(msg));
    } else {
        MLOG("discard message: {}", msg);
    }
}

void MachineBase::triggerEvent(Timestamp now, MachineEvent e) {
    switch (e) {
    case MachineEvent::SHUTDOWN:
        shutdown(/*critical=*/false);
        break;
    case MachineEvent::CORRUPTION:
        shutdown(/*critical=*/true);
        break;
    case MachineEvent::NET_IN_BORKEN:
        state = MachineState::NET_IN_BROKEN;
        mailbox.clear();
        break;
    case MachineEvent::NET_OUT_BROKEN:
        state = MachineState::NET_OUT_BROKEN;
        break;
    case MachineEvent::HANG:
        state = MachineState::HANG;
        break;
    }
    recoverTime = Timestamp(now + folly::Random::rand32(parameters::maxRecoverTime) + 1);
    MLOG("triggerEvent e:{} recover:{}", magic_enum::enum_name(e), *recoverTime);
}

void MachineBase::checkRecover(Timestamp now) {
    if (recoverTime && *recoverTime <= now) {
        MASSERT(state != MachineState::NORMAL, "");
        MLOG("recover from {}", magic_enum::enum_name(state));
        if (state == MachineState::SHUTDOWN)
            start(now);
        state = MachineState::NORMAL;
        recoverTime.reset();
    }
}

void MachineBase::send(Timestamp now, MachineBase * m, Message msg) {
    if (msg.isRequest)
        ongoingRequests[msg.requestId] = msg;
    if (folly::Random::rand32(100) < parameters::networkPacketLossPercent) {
        MLOG("send message lost to:{} msg:{}", m->id, msg);
    } else if (state == MachineState::NET_OUT_BROKEN) {
        MLOG("send message failed to:{} msg:{} reason:NET_OUT_BROKEN", m->id, msg);
    } else {
        MLOG("send message to:{} msg:{}", m->id, msg);
        m->receive(now, std::move(msg));
    }
}

void MachineBase::discardTimeoutRequests(Timestamp now) {
    std::vector<Message> timeoutedRequests;
    for ([[maybe_unused]] const auto & [rid, msg] : ongoingRequests) {
        MASSERT(msg.isRequest, "");
        if (msg.sentTime + msg.timeout <= now) {
            timeoutedRequests.push_back(msg);
        }
    }
    for (const auto & req : timeoutedRequests) {
        MLOG("discard timeouted request {}", req);
        ongoingRequests.erase(req.requestId);
    }
    for (auto && req : timeoutedRequests) {
        handleRequestTimeout(now, std::move(req));
    }
}
} // namespace camus::raft::v0

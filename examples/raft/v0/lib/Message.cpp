#include <folly/Random.h>

#include "Message.h"
#include "Parameters.h"


namespace camus::raft::v0 {
int64_t Message::nextRequestId = 0;

Message Message::makeReq(Timestamp now, Interval timeout, MachineBase * from, std::shared_ptr<Payload> payload) {
    Message msg;
    msg.sentTime = now;
    msg.arriveTime = Timestamp(now + folly::Random::rand32(parameters::maxNetworkTime) + 1);
    msg.timeout = timeout;
    msg.requestId = nextRequestId++;
    msg.from = from;
    msg.payload = std::move(payload);
    return msg;
}

Message Message::makeRsp(Timestamp now, MachineBase * from, int64_t requestId, std::shared_ptr<Payload> payload) {
    Message msg;
    msg.sentTime = now;
    msg.arriveTime = Timestamp(now + folly::Random::rand32(parameters::maxNetworkTime) + 1);
    msg.requestId = requestId;
    msg.isRequest = false;
    msg.from = from;
    msg.payload = std::move(payload);
    return msg;
}
} // namespace camus::raft::v0

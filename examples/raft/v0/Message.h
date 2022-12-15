#pragma once

#include <folly/Random.h>

#include <any>
#include <cstdint>

#include "Parameters.h"
#include "common/utils/StrongTypedef.h"

namespace camus::raft::v0 {
STRONG_TYPEDEF(int64_t, Timestamp);

struct Machine;
struct Message {
    Timestamp sentTime{0};
    Timestamp arriveTime{0};
    int64_t requestId{0};
    bool isRequest = true;
    Machine * from = nullptr;
    std::any payload;

    static int64_t nextRequestId;

    static Message makeReq(Timestamp now, Machine * from, auto && payload) {
        Message msg;
        msg.sentTime = now;
        msg.from = from;
        msg.arriveTime = Timestamp(now + folly::Random::rand32(parameters::maxNetworkTime) + 1);
        msg.payload = std::forward<decltype(payload)>(payload);
        msg.requestId = nextRequestId++;
        return msg;
    }

    static Message makeRsp(Timestamp now, Machine * from, int64_t requestId, auto && payload) {
        Message msg;
        msg.sentTime = now;
        msg.from = from;
        msg.arriveTime = Timestamp(now + folly::Random::rand32(parameters::maxNetworkTime) + 1);
        msg.payload = std::forward<decltype(payload)>(payload);
        msg.isRequest = false;
        msg.requestId = requestId;
        return msg;
    }
};
} // namespace camus::raft::v0

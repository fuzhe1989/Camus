#pragma once

#include <cstdint>
#include <memory>

#include "Payload.h"
#include "common.h"

namespace camus::raft::v0 {
struct MachineBase;
struct Message {
    Timestamp sentTime{0};
    Timestamp arriveTime{0};
    Interval timeout{0};
    int64_t requestId{0};
    bool isRequest = true;
    MachineBase * from = nullptr;
    std::shared_ptr<Payload> payload;

    static int64_t nextRequestId;

    static Message makeReq(Timestamp now, Interval timeout, MachineBase * from, std::shared_ptr<Payload> payload);
    static Message makeRsp(Timestamp now, MachineBase * from, int64_t requestId, std::shared_ptr<Payload> payload);
};
} // namespace camus::raft::v0

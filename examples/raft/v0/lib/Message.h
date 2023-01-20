#pragma once

#include <fmt/format.h>

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

    std::string toString() const;

    static int64_t nextRequestId;

    static Message makeReq(Timestamp now, Interval timeout, MachineBase * from, std::shared_ptr<Payload> payload);
    static Message makeRsp(Timestamp now, MachineBase * from, int64_t requestId, std::shared_ptr<Payload> payload);
};
} // namespace camus::raft::v0

template <>
struct fmt::formatter<camus::raft::v0::Message> : formatter<std::string_view> {
    template <typename FmtContext>
    auto format(const camus::raft::v0::Message & x, FmtContext & ctx) const {
        return formatter<std::string_view>::format(x.toString(), ctx);
    }
};

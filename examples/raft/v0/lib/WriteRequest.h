#pragma once

#include "Command.h"
#include "Payload.h"
#include "common.h"

namespace camus::raft::v0 {
struct WriteRequest : Payload {
    Command command;

    std::string toString() const override;
};

struct WriteResponse : Payload {
    bool success = false;
    std::optional<NodeId> leader;

    std::string toString() const override;
};
} // namespace camus::raft::v0

#pragma once

#include "Command.h"
#include "Payload.h"
#include "common.h"

namespace camus::raft::v0 {
struct ReadRequest : Payload {
    std::string key;

    std::string toString() const override;
};

struct ReadResponse : Payload {
    bool success = false;
    std::optional<NodeId> leader;
    std::optional<std::string> value;

    std::string toString() const override;
};
} // namespace camus::raft::v0

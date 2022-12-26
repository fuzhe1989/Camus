#pragma once

#include <optional>
#include <vector>

#include "Command.h"
#include "common.h"

namespace camus::raft::v0 {
struct PersistentState {
    // latest term server has seen (initialized to 0 on first boot, increases monotonically)
    Term currentTerm{0};
    // candidateId that received vote in current term (or null if none)
    std::optional<NodeId> voteFor;
    // known leader of current term
    std::optional<NodeId> leader;
    // log entries; each entry contains command for state machine, and term when entry was received by leader (first index is 1)
    std::vector<Command> logs;
};
} // namespace camus::raft::v0

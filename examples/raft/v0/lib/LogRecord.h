#pragma once

#include "Command.h"
#include "common.h"

namespace camus::raft::v0 {
struct LogRecord {
    Term term{0};
    Command command;
};
} // namespace camus::raft::v0

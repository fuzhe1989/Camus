#include <cassert>

#include "Command.h"

namespace camus::raft::v0 {
Command Command::put(std::string key, std::string value) {
    assert(!key.empty());
    assert(!value.empty());
    Command cmd;
    cmd.type_ = CommandType::Put;
    cmd.key_ = std::move(key);
    cmd.value_ = std::move(value);
    return cmd;
}

Command Command::del(std::string key) {
    assert(!key.empty());
    Command cmd;
    cmd.type_ = CommandType::Delete;
    cmd.key_ = std::move(key);
    return cmd;
}
} // namespace camus::raft::v0

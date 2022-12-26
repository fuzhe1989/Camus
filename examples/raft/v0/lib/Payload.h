#pragma once

#include <string>

namespace camus::raft::v0 {
struct Payload {
    virtual ~Payload() = default;
    virtual std::string toString() const = 0;
};
} // namespace camus::raft::v0

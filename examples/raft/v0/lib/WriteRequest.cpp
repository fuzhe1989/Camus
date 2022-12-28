#include <fmt/format.h>

#include "WriteRequest.h"

namespace camus::raft::v0 {
std::string WriteRequest::toString() const {
    return fmt::format("type:WriteRequest command:{{{}}}", command.toString());
}

std::string WriteResponse::toString() const {
    return fmt::format("type:WriteResponse status:{} leader:{}", status.toString(), leader.value_or(NodeId{}));
}
} // namespace camus::raft::v0

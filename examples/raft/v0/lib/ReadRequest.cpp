#include <fmt/format.h>

#include "ReadRequest.h"

namespace camus::raft::v0 {
std::string ReadRequest::toString() const {
    return fmt::format("type:ReadRequest key:{}", key);
}

std::string ReadResponse::toString() const {
    return fmt::format("type:ReadResponse success:{} leader:{} value:{}", success, leader.value_or(""), value.value_or(""));
}
} // namespace camus::raft::v0

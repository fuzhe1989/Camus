#include <fmt/format.h>

#include "RequestVote.h"

namespace camus::raft::v0 {
std::string RequestVoteRequest::toString() const {
    return fmt::format(
        "type:RequestVoteRequest term:{} candidate:{} lastLogIndex:{} lastLogTerm:{}",
        term,
        candidateId,
        lastLogIndex,
        lastLogTerm);
}

std::string RequestVoteResponse::toString() const {
    return fmt::format(
        "type:RequestVoteResponse term:{} status:{}",
        term,
        status.toString());
}
} // namespace camus::raft::v0

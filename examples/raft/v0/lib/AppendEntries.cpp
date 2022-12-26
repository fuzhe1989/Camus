#include <fmt/format.h>

#include "AppendEntries.h"

namespace camus::raft::v0 {
std::string AppendEntriesRequest::toString() const {
    std::vector<std::string> entries;
    for (const auto & c : this->entries)
        entries.push_back(c.toString());
    return fmt::format(
        "type:AppendEntriesRequest term:{} leaderId:{} prevLogIndex:{} prevLogTerm:{} leaderCommit:{} entries:{}",
        term,
        leaderId,
        prevLogIndex,
        prevLogTerm,
        leaderCommit,
        fmt::join(entries, ","));
}

std::string AppendEntriesResponse::toString() const {
    return fmt::format(
        "type:AppendEntriesResponse term:{} success:{}",
        term,
        success);
}
} // namespace camus::raft::v0

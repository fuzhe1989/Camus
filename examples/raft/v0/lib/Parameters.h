#pragma once

#include <cstdint>

namespace camus::raft::v0::parameters {
inline uint32_t maxNetworkTime = 8;
inline uint32_t maxRecoverTime = 50;
inline uint32_t networkPacketLossPercent = 10;
inline uint32_t readRequestTimeout = 20;
inline uint32_t appendEntriesRequestTimeout = 20;
inline uint32_t leaseLength = 300;
inline uint32_t heartbeatInterval = 50;
inline uint32_t heartbeatTimeout = 200;
inline uint32_t electionTimeout = 200;
}; // namespace camus::raft::v0::parameters

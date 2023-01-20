#include <gtest/gtest.h>

#include "examples/raft/v0/lib/Parameters.h"
#include "examples/raft/v0/lib/RaftMachine.h"

namespace camus::tests {
namespace {
using namespace camus::raft::v0;

class RaftV0Test : public ::testing::Test {};

TEST_F(RaftV0Test, testStart) {
    RaftMachine m0;
    m0.id = NodeId("m0");

    std::map<NodeId, MachineBase *> machines = {{m0.id, &m0}};
    m0.machines = &machines;

    ASSERT_EQ(m0.id, "m0");
    ASSERT_EQ(m0.state, MachineState::SHUTDOWN);

    m0.start(Timestamp(1));
    ASSERT_TRUE(m0.mailbox.empty());
    ASSERT_TRUE(m0.ongoingRequests.empty());
    ASSERT_TRUE(!m0.recoverTime.has_value());
    ASSERT_EQ(m0.role(), Role::FOLLOWER);
    ASSERT_EQ(m0.asFollower().lastHeartbeatReceivedTime, 1u);
}

TEST_F(RaftV0Test, testElection_Single) {
    RaftMachine m0;
    m0.id = NodeId("m0");

    std::map<NodeId, MachineBase *> machines = {{m0.id, &m0}};
    m0.machines = &machines;

    m0.start(Timestamp(0));
    m0.handle(Timestamp(1 + parameters::heartbeatTimeout));

    ASSERT_EQ(m0.role(), Role::CANDIDATE);
}
} // namespace
} // namespace camus::tests

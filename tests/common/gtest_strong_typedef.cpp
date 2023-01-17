#include <gtest/gtest.h>

#include <map>

#include "common/utils/StrongTypedef.h"

namespace camus::tests {
namespace {

TEST(StrongTypedefTest, testTrivial) {
    STRONG_TYPEDEF(int, UserId);
    static_assert(std::is_same_v<int, typename UserId::UnderlyingType>);
    static_assert(!std::is_assignable_v<UserId, int>);

    UserId id0;
    ASSERT_EQ(id0, 0);
    ASSERT_EQ(id0.toUnderType(), 0);

    UserId id1(5);
    ASSERT_EQ(id1, 5);
    ASSERT_EQ(id1.toUnderType(), 5);

    UserId id2(id1);
    ASSERT_EQ(id2, 5);

    UserId id3(std::move(id2));
    ASSERT_EQ(id3, 5);

    id0 = UserId(10);
    ASSERT_EQ(id0, 10);
    ASSERT_LT(id3, id0);
    ASSERT_GT(id0, id3);
}

TEST(StrongTypedefTest, testSpetialization) {
    STRONG_TYPEDEF(int, UserId);

    std::unordered_map<UserId, int> m = {
        {UserId(1), 1},
        {UserId(2), 2},
        {UserId(3), 3},
    };

    for (const auto & [k, v] : m) {
        auto ks = fmt::format("{}", k);
        auto vs = fmt::format("{}", v);
        ASSERT_EQ(ks, vs);
    }
}

TEST(StrongTypedefTest, testComparison) {
    STRONG_TYPEDEF(size_t, UserId);

    UserId id0(5);
    ASSERT_EQ(id0, 5);
    ASSERT_EQ(id0, 5u);
    ASSERT_TRUE(id0 < 10);
    ASSERT_TRUE(10 > id0);
    ASSERT_TRUE(id0 > 0);
    ASSERT_TRUE(0u < id0);

    STRONG_TYPEDEF(std::string, NodeId);

    NodeId n("abc");
    ASSERT_TRUE(n > "");
    ASSERT_EQ(n, "abc");
    ASSERT_EQ(n, std::string("abc"));
}

} // namespace
} // namespace camus::tests

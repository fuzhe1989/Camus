#include <gtest/gtest.h>
#include <scn/scn.h>
#include <scn/tuple_return.h>

#include <cstdint>
#include <string_view>
#include <thread>
#include <vector>

namespace hf::tests {
namespace {

TEST(Scnlib, testReadString) {
    std::string word;
    auto result = scn::scan("Hello world", "{}", word);
    ASSERT_EQ(static_cast<bool>(result), true);
    ASSERT_EQ(word, "Hello");
    ASSERT_EQ(result.range_as_string(), " world");
}

TEST(Scnlib, testReadMultipleValues) {
    int i, j;
    auto result = scn::scan("123 456 foo", "{} {}", i, j);
    ASSERT_EQ(static_cast<bool>(result), true);
    ASSERT_EQ(i, 123);
    ASSERT_EQ(j, 456);

    std::string str;
    auto ret = scn::scan(result.range(), "{}", str);
    ASSERT_EQ(static_cast<bool>(ret), true);
    ASSERT_EQ(str, "foo");
}

TEST(Scnlib, testReturnTuple) {
    auto [r, i, s] = scn::scan_tuple<int, std::string>("42 foo", "{}{}");
    ASSERT_EQ(static_cast<bool>(r), true);
    ASSERT_EQ(i, 42);
    ASSERT_EQ(s, "foo");
}

} // namespace
} // namespace hf::tests

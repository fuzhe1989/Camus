#include <gtest/gtest.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/spdlog.h>

#include <cstdint>
#include <string_view>
#include <vector>

namespace camus::tests {
namespace {

TEST(Spdlog, testAsync) {
    spdlog::drop("");
    auto asyncLogger = spdlog::create_async<spdlog::sinks::rotating_file_sink_mt>("", "test.log", 8192, 10);
    spdlog::set_default_logger(asyncLogger);
    ASSERT_EQ(asyncLogger->sinks().size(), 1);
    auto sinkPtr = dynamic_pointer_cast<spdlog::sinks::rotating_file_sink_mt>(asyncLogger->sinks().front());
    ASSERT_TRUE(sinkPtr);

    asyncLogger->set_level(spdlog::level::debug);

    for (int i = 0; i < 65536; ++i) {
        SPDLOG_DEBUG("Hello {} {}", "World", i);
    }
}

} // namespace
} // namespace camus::tests

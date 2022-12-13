#pragma once

#include <string>
#include <string_view>

namespace camus::raft::v0 {
enum class CommandType {
    Invalid = 0,
    Put = 1,
    Delete = 2,
};

class Command {
public:
    Command() = default;
    static Command put(std::string key, std::string value);
    static Command del(std::string key);

    CommandType type() const {
        return type_;
    }

    std::string_view key() const {
        return key_;
    }

    std::string_view value() const {
        return value_;
    }

private:
    CommandType type_{CommandType::Invalid};
    std::string key_;
    std::string value_;
};
} // namespace camus::raft::v0

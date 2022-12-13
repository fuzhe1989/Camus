#pragma once

#include <cstdint>
#include <memory>

namespace camus::raft::v0 {
enum class LogRecordType : uint16_t {
    Invalid = 0,
    Put = 1,
    Delete = 2,
};

class ILogPayload {
public:
    virtual ~ILogPayload() = default;

    virtual LogRecordType type() const = 0;
};

class LogRecord {
public:
private:
    LogRecordType type_;
};
} // namespace camus::raft::v0

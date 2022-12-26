#pragma once

#include <fmt/format.h>

#include "MachineBase.h"
#include "Message.h"

template <>
struct fmt::formatter<camus::raft::v0::Message> : fmt::formatter<std::string_view> {
    template <typename FormatContext>
    auto format(const camus::raft::v0::Message & msg, FormatContext & ctx) const {
        format_to(ctx.out(), "sent:{} arrive:{} timeout:{} from:{}", msg.sentTime, msg.arriveTime, msg.timeout, msg.from->id);
        if (!msg.isRequest) {
            format_to(ctx.out(), " requestId:{}", msg.requestId);
        }
        return format_to(ctx.out(), " payload:{{{}}}", msg.payload->toString());
    }
};

#pragma once

#include <google/protobuf/service.h> // google::protobuf::Service

#include <map>

namespace hf::rpc {
class Server {
public:
    enum class Status {
        UNINITIALIZED = 0,
        READY = 1,
        RUNNING = 2,
        STOPPING = 3,
    };

    enum class ServiceOwnership {
        SERVER_OWNS_SERVICE,
        SERVER_DOESNT_OWN_SERVICE
    };

    struct ServiceProperty {
        bool is_builtin_service = false;
        ServiceOwnership ownership = ServiceOwnership::SERVER_DOESNT_OWN_SERVICE;
        // `service' and `restful_map' are mutual exclusive, they can't be
        // both non-NULL. If `restful_map' is not NULL, the URL should be
        // further matched by it.
        google::protobuf::Service * service = nullptr;

        bool isUserService() const { return !is_builtin_service; }

        const std::string & serviceName() const;
    };
    using ServiceMap = std::map<std::string, ServiceProperty>;
};
} // namespace hf::rpc

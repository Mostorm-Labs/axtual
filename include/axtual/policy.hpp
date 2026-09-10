#pragma once

#include "axtual/endpoint.hpp"

#include <cstdint>
#include <string>

namespace axtual {

class PolicyProjection {
public:
    PolicyProjection(PolicyDisposition disposition, std::string reason, std::uint64_t revision)
        : _disposition(disposition), _reason(std::move(reason)), _revision(revision) {}

    void apply(Endpoint& endpoint) const {
        endpoint.policy = _disposition;
        endpoint.policyReason = _reason;
        endpoint.policyRevision = _revision;
    }

private:
    PolicyDisposition _disposition;
    std::string _reason;
    std::uint64_t _revision;
};

}  // namespace axtual

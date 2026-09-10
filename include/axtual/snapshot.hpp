#pragma once

#include "axtual/endpoint.hpp"

#include <string>

namespace axtual {

struct EndpointSnapshot {
    EndpointSnapshot(const Endpoint& endpoint)
        : id(endpoint.id), enumerated(endpoint.enumerated), openable(endpoint.openable),
          policy(endpoint.policy), selected(endpoint.selected), effective(endpoint.effective),
          enhancement(endpoint.enhancement) {}

    std::string id;
    bool enumerated;
    TriState openable;
    PolicyDisposition policy;
    bool selected;
    bool effective;
    EnhancementState enhancement;
};

}  // namespace axtual

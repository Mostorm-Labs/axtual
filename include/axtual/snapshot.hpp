#pragma once

#include "axtual/binding.hpp"
#include "axtual/endpoint.hpp"
#include "axtual/enhancement.hpp"

#include <optional>
#include <string>
#include <utility>

namespace axtual {

struct ReconciliationObservation {
    std::string endpointId;
    bool effective = false;
};

struct EndpointSnapshot {
    const std::string id;
    const bool enumerated;
    const TriState openable;
    const PolicyDisposition policy;
    const bool selected;
    const bool applicable;
    const std::optional<bool> effective;
    const EnhancementState enhancement;
};

class StateAggregator {
public:
    static EndpointSnapshot project(
        const Endpoint& endpoint, const BindingStore& bindings, const Enhancement& enhancement,
        std::optional<ReconciliationObservation> observation = std::nullopt) {
        const auto bindingKind = kindFor(endpoint.kind);
        const bool selected = bindingMatches(endpoint, bindings, bindingKind);
        const bool applicable = selected && bindings.applicable(bindingKind);
        std::optional<bool> effective;
        if (observation.has_value() && observation->endpointId == endpoint.id) {
            effective = observation->effective;
        }
        return {endpoint.id, endpoint.enumerated, endpoint.openable, endpoint.policy, selected,
                applicable, effective, enhancement.state()};
    }

private:
    static BindingKind kindFor(EndpointKind kind) {
        if (kind == EndpointKind::kVideoInput) return BindingKind::kVideoInput;
        if (kind == EndpointKind::kAudioOutput) return BindingKind::kHealthCheckAudioOutput;
        return BindingKind::kAudioInput;
    }

    static bool bindingMatches(const Endpoint& endpoint, const BindingStore& bindings,
                               BindingKind kind) {
        if (kind == BindingKind::kVideoInput) return bindings.videoInput() == endpoint.id;
        if (kind == BindingKind::kHealthCheckAudioOutput) {
            return bindings.healthCheckAudioOutput() == endpoint.id;
        }
        return bindings.audioInput() == endpoint.id;
    }
};

}  // namespace axtual

#pragma once

#include "axtual/endpoint.hpp"

#include <string>

namespace axtual {

class Enhancement {
public:
    void nominateCandidate() { _state = EnhancementState::kCandidate; }
    void associate(std::string id) {
        _id = std::move(id);
        _state = EnhancementState::kAssociated;
    }
    void markAmbiguous() { _state = EnhancementState::kAmbiguous; }
    void lose() { _state = EnhancementState::kLost; }
    [[nodiscard]] EnhancementState state() const { return _state; }

private:
    std::string _id;
    EnhancementState _state = EnhancementState::kNone;
};

class OpenProbe {
public:
    bool invoked = false;
    void invoke() { invoked = true; }
};

class SelfLoopGuard {
public:
    static bool allow(const Endpoint& endpoint, EndpointKind requestedKind, OpenProbe& probe) {
        if (endpoint.origin == EndpointOrigin::kNearityVirtual && endpoint.kind == requestedKind) {
            return false;
        }
        probe.invoke();
        return true;
    }
};

class SemanticOperations {
public:
    void inventory() {}
    void projectPolicy() {}
    void associateEnhancement() {}
    [[nodiscard]] bool persistentDemand() const { return _persistentDemand; }

private:
    bool _persistentDemand = false;
};

}  // namespace axtual

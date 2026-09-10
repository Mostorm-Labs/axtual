#pragma once

#include "axtual/endpoint.hpp"

#include <cstdint>
#include <optional>
#include <string>

namespace axtual {

enum class BindingKind { kAudioInput, kVideoInput, kHealthCheckAudioOutput };
enum class CallerIdentity { kUi, kApi, kCli };

struct SelectionResult {
    bool accepted = false;
    std::string reason;
};

class BindingStore {
public:
    void commit(BindingKind kind, std::string endpointId) {
        binding(kind) = std::move(endpointId);
        ++_revision;
    }

    void observeMetadataChange(const std::string&, const std::string&) {}

    void markGone(const std::string& endpointId) {
        if (audioInput() == endpointId) {
            _audioEffective = false;
        }
        if (videoInput() == endpointId) {
            _videoEffective = false;
        }
    }

    void blockByPolicy(const std::string& endpointId) {
        if (audioInput() == endpointId) {
            _audioEffective = false;
        }
        if (videoInput() == endpointId) {
            _videoEffective = false;
        }
    }

    void setPresentationHidden(const std::string&) {}

    [[nodiscard]] const std::optional<std::string>& audioInput() const { return _audioInput; }
    [[nodiscard]] const std::optional<std::string>& videoInput() const { return _videoInput; }
    [[nodiscard]] std::uint64_t revision() const { return _revision; }
    [[nodiscard]] bool effective(BindingKind kind) const {
        return kind == BindingKind::kVideoInput ? _videoEffective : _audioEffective;
    }

private:
    std::optional<std::string>& binding(BindingKind kind) {
        if (kind == BindingKind::kVideoInput) return _videoInput;
        return _audioInput;
    }

    std::optional<std::string> _audioInput;
    std::optional<std::string> _videoInput;
    std::optional<std::string> _healthCheckOutput;
    std::uint64_t _revision = 0;
    bool _audioEffective = true;
    bool _videoEffective = true;
};

class SelectionValidator {
public:
    SelectionResult select(const EndpointRegistry& registry, BindingStore& bindings,
                           const std::string& endpointId, BindingKind bindingKind,
                           CallerIdentity, PolicyDisposition explicitPolicy = PolicyDisposition::kEligible) const {
        const Endpoint* endpoint = registry.find(endpointId);
        if (endpoint == nullptr) return {false, "missing_endpoint"};
        if (endpoint->policy == PolicyDisposition::kBlockedByPolicy ||
            explicitPolicy == PolicyDisposition::kBlockedByPolicy) {
            return {false, "blocked_by_policy"};
        }
        if (!endpoint->available || endpoint->capability != Capability::kSupported) {
            return {false, "generic_capability_unavailable"};
        }
        bindings.commit(bindingKind, endpointId);
        return {true, "accepted"};
    }
};

}  // namespace axtual

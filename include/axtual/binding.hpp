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
        applicability(kind) = true;
        ++_revision;
        revision(kind) = _revision;
    }

    void observeMetadataChange(const std::string&, const std::string&) {}

    void markGone(const std::string& endpointId) {
        if (audioInput() == endpointId) {
            _audioApplicable = false;
        }
        if (videoInput() == endpointId) {
            _videoApplicable = false;
        }
        if (healthCheckAudioOutput() == endpointId) {
            _healthApplicable = false;
        }
    }

    void blockByPolicy(const std::string& endpointId) {
        if (audioInput() == endpointId) {
            _audioApplicable = false;
        }
        if (videoInput() == endpointId) {
            _videoApplicable = false;
        }
        if (healthCheckAudioOutput() == endpointId) {
            _healthApplicable = false;
        }
    }

    void setPresentationHidden(const std::string&) {}

    [[nodiscard]] const std::optional<std::string>& audioInput() const { return _audioInput; }
    [[nodiscard]] const std::optional<std::string>& videoInput() const { return _videoInput; }
    [[nodiscard]] const std::optional<std::string>& healthCheckAudioOutput() const {
        return _healthCheckOutput;
    }
    [[nodiscard]] std::uint64_t revision() const { return _revision; }
    [[nodiscard]] std::uint64_t audioInputRevision() const { return _audioRevision; }
    [[nodiscard]] std::uint64_t healthCheckOutputRevision() const { return _healthRevision; }
    [[nodiscard]] bool applicable(BindingKind kind) const {
        if (kind == BindingKind::kVideoInput) return _videoApplicable;
        if (kind == BindingKind::kHealthCheckAudioOutput) return _healthApplicable;
        return _audioApplicable;
    }

private:
    std::optional<std::string>& binding(BindingKind kind) {
        if (kind == BindingKind::kVideoInput) return _videoInput;
        if (kind == BindingKind::kHealthCheckAudioOutput) return _healthCheckOutput;
        return _audioInput;
    }

    bool& applicability(BindingKind kind) {
        if (kind == BindingKind::kVideoInput) return _videoApplicable;
        if (kind == BindingKind::kHealthCheckAudioOutput) return _healthApplicable;
        return _audioApplicable;
    }

    std::uint64_t& revision(BindingKind kind) {
        if (kind == BindingKind::kHealthCheckAudioOutput) return _healthRevision;
        if (kind == BindingKind::kVideoInput) return _videoRevision;
        return _audioRevision;
    }

    std::optional<std::string> _audioInput;
    std::optional<std::string> _videoInput;
    std::optional<std::string> _healthCheckOutput;
    std::uint64_t _revision = 0;
    bool _audioApplicable = false;
    bool _videoApplicable = false;
    bool _healthApplicable = false;
    std::uint64_t _audioRevision = 0;
    std::uint64_t _videoRevision = 0;
    std::uint64_t _healthRevision = 0;
};

class SelectionValidator {
public:
    SelectionResult select(const EndpointRegistry& registry, BindingStore& bindings,
                           const std::string& endpointId, BindingKind bindingKind,
                           CallerIdentity,
                           PolicyDisposition explicitPolicy = PolicyDisposition::kEligible) const {
        const Endpoint* endpoint = registry.find(endpointId);
        if (endpoint == nullptr) return {false, "missing_endpoint"};
        const bool kindMatches =
            (bindingKind == BindingKind::kAudioInput && endpoint->kind == EndpointKind::kAudioInput) ||
            (bindingKind == BindingKind::kVideoInput && endpoint->kind == EndpointKind::kVideoInput) ||
            (bindingKind == BindingKind::kHealthCheckAudioOutput &&
             endpoint->kind == EndpointKind::kAudioOutput);
        if (!kindMatches) return {false, "binding_kind_mismatch"};
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

#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <utility>

namespace axtual {

enum class EndpointKind { kAudioInput, kAudioOutput, kVideoInput };
enum class TriState { kUnknown, kSupported, kUnsupported };
enum class Capability { kUnknown, kSupported, kUnsupported };
enum class PolicyDisposition { kEligible, kRecommended, kHiddenByDefault, kBlockedByPolicy };
enum class EndpointOrigin { kPhysical, kThirdPartyVirtual, kNearityVirtual };
enum class EnhancementState { kNone, kCandidate, kAssociated, kAmbiguous, kLost };
enum class IdentityQuality { kUnknown, kStable, kAmbiguous };

struct Endpoint {
    Endpoint(std::string endpointId, EndpointKind endpointKind, std::string display,
             std::string platform = {})
        : id(std::move(endpointId)), kind(endpointKind), displayIdentity(std::move(display)),
          platformToken(std::move(platform)) {}

    std::string id;
    EndpointKind kind;
    std::string displayIdentity;
    std::string platformToken;
    std::uint64_t connectionGeneration = 1;
    bool available = true;
    bool enumerated = true;
    TriState openable = TriState::kUnknown;
    Capability capability = Capability::kUnknown;
    PolicyDisposition policy = PolicyDisposition::kEligible;
    std::string policyReason;
    std::uint64_t policyRevision = 0;
    EndpointOrigin origin = EndpointOrigin::kPhysical;
    IdentityQuality identityQuality = IdentityQuality::kUnknown;
};

class EndpointRegistry {
public:
    Endpoint& add(std::string id, EndpointKind kind, std::string displayIdentity) {
        const std::string key = id;
        auto [iterator, inserted] = _endpoints.emplace(
            key, Endpoint{std::move(id), kind, std::move(displayIdentity)});
        if (!inserted) {
            iterator->second.kind = kind;
        }
        return iterator->second;
    }

    Endpoint* find(const std::string& id) {
        auto iterator = _endpoints.find(id);
        return iterator == _endpoints.end() ? nullptr : &iterator->second;
    }

    const Endpoint* find(const std::string& id) const {
        auto iterator = _endpoints.find(id);
        return iterator == _endpoints.end() ? nullptr : &iterator->second;
    }

private:
    std::map<std::string, Endpoint> _endpoints;
};

enum class ReconnectOutcome { kSame, kReplaced, kAmbiguous, kGone };

class Reconciler {
public:
    static ReconnectOutcome reconcile(const Endpoint& previous, const Endpoint& current) {
        if (current.identityQuality == IdentityQuality::kAmbiguous) return ReconnectOutcome::kAmbiguous;
        if (current.id.empty()) return ReconnectOutcome::kGone;
        if (previous.id == current.id && current.identityQuality == IdentityQuality::kStable) return ReconnectOutcome::kSame;
        return ReconnectOutcome::kReplaced;
    }
    static ReconnectOutcome reconcile(const std::string& previousId, const std::string& currentId,
                                      const std::string& displayIdentity) {
        if (currentId.empty()) {
            return ReconnectOutcome::kGone;
        }
        if (previousId == currentId) {
            return ReconnectOutcome::kSame;
        }
        (void)displayIdentity;
        return ReconnectOutcome::kReplaced;
    }

    static ReconnectOutcome ambiguous(const std::string&, const std::string&, const std::string&) {
        return ReconnectOutcome::kAmbiguous;
    }
};

}  // namespace axtual

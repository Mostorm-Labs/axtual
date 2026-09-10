#include "i30_02_test.hpp"

namespace i30_02 {

void appendEnhancementCases(std::vector<TestCase>& tests) {
    tests.push_back({"enhancement_gain_does_not_change_binding", [] {
        axtual::BindingStore bindings;
        bindings.commit(axtual::BindingKind::kAudioInput, "mic-1");
        axtual::Enhancement enhancement;
        enhancement.associate("near-1");
        require(bindings.audioInput() == "mic-1" &&
                    enhancement.state() == axtual::EnhancementState::kAssociated,
                "gain independent");
    }});

    tests.push_back({"enhancement_loss_preserves_generic_applicability", [] {
        axtual::Endpoint endpoint{"mic-1", axtual::EndpointKind::kAudioInput, "Mic"};
        makeSupported(endpoint);
        endpoint.policy = axtual::PolicyDisposition::kEligible;
        axtual::Enhancement enhancement;
        enhancement.associate("near-1");
        enhancement.lose();
        require(enhancement.state() == axtual::EnhancementState::kLost &&
                    endpoint.policy != axtual::PolicyDisposition::kBlockedByPolicy,
                "generic stays applicable");
    }});

    tests.push_back({"enhancement_required_loss_policy_blocks_without_fallback", [] {
        axtual::Endpoint endpoint{"mic-1", axtual::EndpointKind::kAudioInput, "Mic"};
        axtual::Enhancement enhancement;
        enhancement.lose();
        axtual::PolicyProjection policy(axtual::PolicyDisposition::kBlockedByPolicy,
                                        "enhancement_required", 3);
        policy.apply(endpoint);
        require(endpoint.policy == axtual::PolicyDisposition::kBlockedByPolicy && endpoint.id == "mic-1",
                "policy blocks explicitly");
    }});

    tests.push_back({"new_policy_block_stops_applicability_without_alternate", [] {
        axtual::BindingStore bindings;
        bindings.commit(axtual::BindingKind::kAudioInput, "mic-1");
        bindings.blockByPolicy("mic-1");
        require(bindings.audioInput() == "mic-1" &&
                    !bindings.applicable(axtual::BindingKind::kAudioInput),
                "policy block retains intent and stops applicability");
    }});

    tests.push_back({"hidden_presentation_change_keeps_active_selection", [] {
        axtual::BindingStore bindings;
        bindings.commit(axtual::BindingKind::kAudioInput, "mic-1");
        bindings.setPresentationHidden("mic-1");
        require(bindings.audioInput() == "mic-1" &&
                    bindings.applicable(axtual::BindingKind::kAudioInput),
                "hidden does not invalidate active applicability");
    }});

    tests.push_back({"nearity_virtual_mic_self_loop_rejected_before_open", [] {
        axtual::Endpoint endpoint{"vmic", axtual::EndpointKind::kAudioInput, "Virtual Mic"};
        endpoint.origin = axtual::EndpointOrigin::kNearityVirtual;
        axtual::OpenProbe probe;
        require(!axtual::SelfLoopGuard::allow(endpoint, axtual::EndpointKind::kAudioInput, probe) &&
                    !probe.invoked,
                "self loop rejected before open");
    }});

    tests.push_back({"nearity_virtual_camera_self_loop_rejected_before_open", [] {
        axtual::Endpoint endpoint{"vcam", axtual::EndpointKind::kVideoInput, "Virtual Camera"};
        endpoint.origin = axtual::EndpointOrigin::kNearityVirtual;
        axtual::OpenProbe probe;
        require(!axtual::SelfLoopGuard::allow(endpoint, axtual::EndpointKind::kVideoInput, probe) &&
                    !probe.invoked,
                "self loop rejected before open");
    }});

    tests.push_back({"third_party_virtual_endpoint_not_rejected_solely_for_virtual", [] {
        axtual::Endpoint endpoint{"third", axtual::EndpointKind::kAudioInput, "Virtual Mic"};
        endpoint.origin = axtual::EndpointOrigin::kThirdPartyVirtual;
        axtual::OpenProbe probe;
        require(axtual::SelfLoopGuard::allow(endpoint, axtual::EndpointKind::kAudioInput, probe) &&
                    probe.invoked,
                "third party virtual allowed");
    }});

    tests.push_back({"inventory_policy_enhancement_do_not_acquire_demand", [] {
        axtual::SemanticOperations operations;
        operations.inventory();
        operations.projectPolicy();
        operations.associateEnhancement();
        require(!operations.persistentDemand(), "semantic operations are side effect free");
    }});

    tests.push_back({"caller_paths_share_common_validator", [] {
        axtual::EndpointRegistry registry;
        auto& endpoint = registry.add("mic-1", axtual::EndpointKind::kAudioInput, "Mic");
        makeSupported(endpoint);
        endpoint.policy = axtual::PolicyDisposition::kBlockedByPolicy;
        axtual::BindingStore bindings;
        axtual::SelectionValidator validator;
        auto ui = validator.select(registry, bindings, "mic-1", axtual::BindingKind::kAudioInput,
                                   axtual::CallerIdentity::kUi);
        auto api = validator.select(registry, bindings, "mic-1", axtual::BindingKind::kAudioInput,
                                    axtual::CallerIdentity::kApi);
        require(!ui.accepted && !api.accepted && ui.reason == api.reason,
                "caller paths share validator");
    }});
}

}  // namespace i30_02

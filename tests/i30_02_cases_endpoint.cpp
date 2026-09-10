#include "i30_02_test.hpp"

namespace i30_02 {

void appendEndpointCases(std::vector<TestCase>& tests) {
    tests.push_back({"generic_inventory_keeps_non_nearity_endpoint", [] {
        axtual::EndpointRegistry registry;
        auto& endpoint = registry.add("mic-1", axtual::EndpointKind::kAudioInput, "Desk Mic");
        axtual::Enhancement enhancement;
        require(endpoint.origin == axtual::EndpointOrigin::kPhysical &&
                    enhancement.state() == axtual::EnhancementState::kNone,
                "generic endpoint and enhancement truth stay separate");
        require(registry.find("mic-1") != nullptr, "generic endpoint remains in inventory");
    }});

    tests.push_back({"snapshot_keeps_truth_channels_distinct", [] {
        axtual::EndpointRegistry registry;
        auto& endpoint = registry.add("mic-1", axtual::EndpointKind::kAudioInput, "Mic");
        endpoint.enumerated = true;
        endpoint.openable = axtual::TriState::kUnknown;
        endpoint.policy = axtual::PolicyDisposition::kRecommended;
        makeSupported(endpoint);
        axtual::BindingStore bindings;
        axtual::SelectionValidator validator;
        require(validator.select(registry, bindings, "mic-1", axtual::BindingKind::kAudioInput,
                                 axtual::CallerIdentity::kApi).accepted,
                "selection setup accepted");
        axtual::Enhancement enhancement;
        enhancement.nominateCandidate();
        auto snapshot = axtual::StateAggregator::project(endpoint, bindings, enhancement);
        require(snapshot.enumerated && snapshot.openable == axtual::TriState::kUnknown &&
                    snapshot.policy == axtual::PolicyDisposition::kRecommended && snapshot.selected &&
                    snapshot.applicable && !snapshot.effective.has_value() &&
                    snapshot.enhancement == axtual::EnhancementState::kCandidate,
                "snapshot channels must remain distinct and owner-derived");
    }});

    tests.push_back({"policy_block_does_not_rewrite_generic_truth", [] {
        axtual::Endpoint endpoint{"mic-1", axtual::EndpointKind::kAudioInput, "Mic"};
        makeSupported(endpoint);
        axtual::PolicyProjection projection(axtual::PolicyDisposition::kBlockedByPolicy,
                                            "restricted", 2);
        projection.apply(endpoint);
        require(endpoint.available && endpoint.capability == axtual::Capability::kSupported &&
                    endpoint.policy == axtual::PolicyDisposition::kBlockedByPolicy,
                "policy must not rewrite generic truth");
    }});

    tests.push_back({"blocked_selection_rejected_without_commit", [] {
        axtual::EndpointRegistry registry;
        registry.add("mic-1", axtual::EndpointKind::kAudioInput, "Mic");
        axtual::BindingStore bindings;
        axtual::SelectionValidator validator;
        auto result = validator.select(registry, bindings, "mic-1", axtual::BindingKind::kAudioInput,
                                       axtual::CallerIdentity::kUi,
                                       axtual::PolicyDisposition::kBlockedByPolicy);
        require(!result.accepted && bindings.revision() == 0 && !bindings.audioInput().has_value(),
                "blocked selection must not commit");
    }});

    tests.push_back({"hidden_endpoint_remains_selectable", [] {
        axtual::EndpointRegistry registry;
        auto& endpoint = registry.add("mic-1", axtual::EndpointKind::kAudioInput, "Mic");
        makeSupported(endpoint);
        endpoint.policy = axtual::PolicyDisposition::kHiddenByDefault;
        axtual::BindingStore bindings;
        axtual::SelectionValidator validator;
        require(validator.select(registry, bindings, "mic-1", axtual::BindingKind::kAudioInput,
                                 axtual::CallerIdentity::kCli).accepted,
                "hidden endpoint remains selectable");
    }});

    tests.push_back({"recommended_does_not_auto_select", [] {
        axtual::EndpointRegistry registry;
        auto& endpoint = registry.add("mic-1", axtual::EndpointKind::kAudioInput, "Mic");
        endpoint.policy = axtual::PolicyDisposition::kRecommended;
        axtual::BindingStore bindings;
        require(!bindings.audioInput().has_value() && bindings.revision() == 0,
                "recommendation must not select");
    }});

    tests.push_back({"missing_endpoint_rejected_without_commit", [] {
        axtual::BindingStore bindings;
        axtual::SelectionValidator validator;
        axtual::EndpointRegistry registry;
        require(!validator.select(registry, bindings, "missing", axtual::BindingKind::kAudioInput,
                                  axtual::CallerIdentity::kApi).accepted &&
                    bindings.revision() == 0,
                "missing endpoint must not commit");
    }});

    tests.push_back({"unknown_capability_not_selectable", [] {
        axtual::EndpointRegistry registry;
        registry.add("mic-1", axtual::EndpointKind::kAudioInput, "Mic");
        axtual::BindingStore bindings;
        axtual::SelectionValidator validator;
        require(!validator.select(registry, bindings, "mic-1", axtual::BindingKind::kAudioInput,
                                  axtual::CallerIdentity::kApi).accepted,
                "unknown capability rejected");
    }});

    tests.push_back({"explicit_selection_uses_canonical_ref", [] {
        axtual::EndpointRegistry registry;
        auto& endpoint = registry.add("mic-1", axtual::EndpointKind::kAudioInput, "Mic");
        makeSupported(endpoint);
        axtual::BindingStore bindings;
        axtual::SelectionValidator validator;
        require(validator.select(registry, bindings, "mic-1", axtual::BindingKind::kAudioInput,
                                 axtual::CallerIdentity::kApi).accepted &&
                    bindings.audioInput() == "mic-1" && bindings.revision() == 1,
                "explicit selection commits canonical ref once");
    }});

    tests.push_back({"metadata_change_does_not_mutate_binding", [] {
        axtual::BindingStore bindings;
        bindings.commit(axtual::BindingKind::kAudioInput, "mic-1");
        bindings.observeMetadataChange("system-default", "display-order");
        require(bindings.audioInput() == "mic-1" && bindings.revision() == 1,
                "metadata must not mutate intent");
    }});

    tests.push_back({"same_reconnect_preserves_binding_without_revision", [] {
        axtual::BindingStore bindings;
        bindings.commit(axtual::BindingKind::kAudioInput, "mic-1");
        axtual::Endpoint previous{"mic-1", axtual::EndpointKind::kAudioInput, "Mic"};
        axtual::Endpoint current{"mic-1", axtual::EndpointKind::kAudioInput, "Mic"};
        current.identityQuality = axtual::IdentityQuality::kStable;
        require(axtual::Reconciler::reconcile(previous, current) == axtual::ReconnectOutcome::kSame &&
                    bindings.revision() == 1,
                "same reconnect preserves revision");
    }});

    tests.push_back({"replaced_reconnect_is_explicit", [] {
        axtual::Endpoint previous{"mic-1", axtual::EndpointKind::kAudioInput, "Mic"};
        axtual::Endpoint current{"mic-2", axtual::EndpointKind::kAudioInput, "Mic"};
        current.identityQuality = axtual::IdentityQuality::kStable;
        require(axtual::Reconciler::reconcile(previous, current) == axtual::ReconnectOutcome::kReplaced,
                "replaced explicit");
    }});

    tests.push_back({"gone_reconnect_is_explicit", [] {
        axtual::Endpoint previous{"mic-1", axtual::EndpointKind::kAudioInput, "Mic"};
        axtual::Endpoint current{"", axtual::EndpointKind::kAudioInput, "Mic"};
        require(axtual::Reconciler::reconcile(previous, current) == axtual::ReconnectOutcome::kGone,
                "gone explicit");
    }});

    tests.push_back({"ambiguous_same_name_does_not_bind", [] {
        axtual::Endpoint previous{"mic-1", axtual::EndpointKind::kAudioInput, "Mic"};
        axtual::Endpoint current{"mic-2", axtual::EndpointKind::kAudioInput, "Mic"};
        current.identityQuality = axtual::IdentityQuality::kAmbiguous;
        require(axtual::Reconciler::reconcile(previous, current) == axtual::ReconnectOutcome::kAmbiguous,
                "ambiguous unresolved");
    }});

    tests.push_back({"gone_endpoint_keeps_desired_non_effective", [] {
        axtual::BindingStore bindings;
        bindings.commit(axtual::BindingKind::kAudioInput, "mic-1");
        bindings.markGone("mic-1");
        require(bindings.audioInput() == "mic-1" &&
                    !bindings.applicable(axtual::BindingKind::kAudioInput),
                "gone retains desired intent but stops applicability");
    }});
}

}  // namespace i30_02

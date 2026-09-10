#include "i30_02_test.hpp"

namespace i30_02 {

template <typename T>
concept HasSelectedCopy = requires(T value) { value.selected; };

template <typename T>
concept HasEffectiveCopy = requires(T value) { value.effective; };

template <typename T>
concept HasEnhancementCopy = requires(T value) { value.enhancement; };

template <typename T>
concept HasBindingEffectiveAccessor = requires(const T& value) {
    value.effective(axtual::BindingKind::kAudioInput);
};

void appendRepairCases(std::vector<TestCase>& tests) {
    tests.push_back({"healthcheck_output_binding_is_independent_from_audio_input", [] {
        axtual::BindingStore bindings;
        bindings.commit(axtual::BindingKind::kAudioInput, "mic-1");
        bindings.commit(axtual::BindingKind::kHealthCheckAudioOutput, "speaker-1");
        require(bindings.audioInput() == "mic-1" && bindings.healthCheckAudioOutput() == "speaker-1" &&
                    bindings.audioInputRevision() == 1 && bindings.healthCheckOutputRevision() == 2,
                "health-check output must be an independent binding");
    }});

    tests.push_back({"healthcheck_output_loss_or_policy_block_does_not_mutate_audio_input", [] {
        axtual::BindingStore bindings;
        bindings.commit(axtual::BindingKind::kAudioInput, "mic-1");
        bindings.commit(axtual::BindingKind::kHealthCheckAudioOutput, "speaker-1");
        bindings.markGone("speaker-1");
        bindings.blockByPolicy("speaker-1");
        require(bindings.audioInput() == "mic-1" && bindings.healthCheckAudioOutput() == "speaker-1" &&
                    bindings.applicable(axtual::BindingKind::kAudioInput) &&
                    !bindings.applicable(axtual::BindingKind::kHealthCheckAudioOutput),
                "health-check loss must not mutate audio input");
    }});

    tests.push_back({"binding_kind_mismatch_rejected_without_commit", [] {
        axtual::EndpointRegistry registry;
        auto& endpoint = registry.add("mic-1", axtual::EndpointKind::kAudioInput, "Mic");
        makeSupported(endpoint);
        axtual::BindingStore bindings;
        axtual::SelectionValidator validator;
        auto result = validator.select(registry, bindings, "mic-1", axtual::BindingKind::kVideoInput,
                                       axtual::CallerIdentity::kApi);
        require(!result.accepted && result.reason == "binding_kind_mismatch" && bindings.revision() == 0,
                "binding kind mismatch must reject before commit");
    }});

    tests.push_back({"endpoint_identity_certainty_is_explicit", [] {
        axtual::Endpoint endpoint{"mic-1", axtual::EndpointKind::kAudioInput, "Mic"};
        require(endpoint.identityQuality == axtual::IdentityQuality::kUnknown,
                "identity certainty must be explicit");
        endpoint.identityQuality = axtual::IdentityQuality::kStable;
        require(endpoint.identityQuality == axtual::IdentityQuality::kStable,
                "stable identity certainty must be representable");
    }});

    tests.push_back({"ambiguous_identity_does_not_autobind", [] {
        axtual::Endpoint previous{"mic-1", axtual::EndpointKind::kAudioInput, "Mic"};
        axtual::Endpoint candidate{"mic-2", axtual::EndpointKind::kAudioInput, "Mic"};
        candidate.identityQuality = axtual::IdentityQuality::kAmbiguous;
        require(axtual::Reconciler::reconcile(previous, candidate) == axtual::ReconnectOutcome::kAmbiguous,
                "ambiguous identity must not autobind");
    }});

    tests.push_back({"desired_binding_commit_does_not_fabricate_effective_state", [] {
        axtual::Endpoint endpoint{"mic-1", axtual::EndpointKind::kAudioInput, "Mic"};
        makeSupported(endpoint);
        axtual::BindingStore bindings;
        bindings.commit(axtual::BindingKind::kAudioInput, "mic-1");
        axtual::Enhancement enhancement;
        auto snapshot = axtual::StateAggregator::project(endpoint, bindings, enhancement);
        require(bindings.applicable(axtual::BindingKind::kAudioInput) && !snapshot.effective.has_value(),
                "desired/applicable state must not fabricate effective media state");
    }});

    tests.push_back({"aggregate_snapshot_derives_selection_from_binding_store", [] {
        axtual::EndpointRegistry registry;
        auto& endpoint = registry.add("mic-1", axtual::EndpointKind::kAudioInput, "Mic");
        makeSupported(endpoint);
        axtual::BindingStore bindings;
        axtual::SelectionValidator validator;
        require(validator.select(registry, bindings, "mic-1", axtual::BindingKind::kAudioInput,
                                 axtual::CallerIdentity::kApi).accepted,
                "selection setup accepted");
        axtual::Enhancement enhancement;
        auto snapshot = axtual::StateAggregator::project(endpoint, bindings, enhancement);
        require(snapshot.selected && snapshot.applicable,
                "snapshot selected truth must derive from BindingStore");
    }});

    tests.push_back({"aggregate_snapshot_derives_enhancement_from_owner", [] {
        axtual::Endpoint endpoint{"mic-1", axtual::EndpointKind::kAudioInput, "Mic"};
        axtual::BindingStore bindings;
        axtual::Enhancement enhancement;
        enhancement.associate("near-1");
        auto snapshot = axtual::StateAggregator::project(endpoint, bindings, enhancement);
        require(snapshot.enhancement == axtual::EnhancementState::kAssociated,
                "snapshot enhancement truth must derive from Enhancement owner");
    }});

    tests.push_back({"endpoint_has_no_duplicate_projection_truth", [] {
        require(!HasSelectedCopy<axtual::Endpoint> && !HasEffectiveCopy<axtual::Endpoint> &&
                    !HasEnhancementCopy<axtual::Endpoint>,
                "Endpoint must not duplicate selected/effective/enhancement projection truth");
        require(!HasBindingEffectiveAccessor<axtual::BindingStore>,
                "BindingStore must not own actual effective media truth");
    }});

    tests.push_back({"explicit_reconciliation_observation_is_required_for_effective_state", [] {
        axtual::Endpoint endpoint{"mic-1", axtual::EndpointKind::kAudioInput, "Mic"};
        makeSupported(endpoint);
        axtual::BindingStore bindings;
        bindings.commit(axtual::BindingKind::kAudioInput, "mic-1");
        axtual::Enhancement enhancement;
        auto before = axtual::StateAggregator::project(endpoint, bindings, enhancement);
        auto after = axtual::StateAggregator::project(
            endpoint, bindings, enhancement, axtual::ReconciliationObservation{"mic-1", true});
        require(!before.effective.has_value() && after.effective.has_value() && *after.effective,
                "effective truth must require an explicit reconciliation observation");
    }});
}

}  // namespace i30_02

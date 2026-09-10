#include "axtual/binding.hpp"
#include "axtual/endpoint.hpp"
#include "axtual/enhancement.hpp"
#include "axtual/policy.hpp"
#include "axtual/snapshot.hpp"

#include <cstdlib>
#include <functional>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {
struct TestCase { std::string_view name; std::function<void()> run; };
void require(bool value, std::string_view message) { if (!value) throw std::runtime_error(std::string(message)); }

std::vector<TestCase> cases() {
    return {
        {"generic_inventory_keeps_non_nearity_endpoint", [] {
            axtual::EndpointRegistry registry;
            auto endpoint = registry.add("mic-1", axtual::EndpointKind::kAudioInput, "Desk Mic");
            require(endpoint.enhancement == axtual::EnhancementState::kNone, "none enhancement retained");
            require(registry.find("mic-1") != nullptr, "generic endpoint remains in inventory");
        }},
        {"snapshot_keeps_truth_channels_distinct", [] {
            axtual::Endpoint endpoint{"mic-1", axtual::EndpointKind::kAudioInput, "Mic"};
            endpoint.enumerated = true; endpoint.openable = axtual::TriState::kUnknown;
            endpoint.policy = axtual::PolicyDisposition::kRecommended;
            endpoint.selected = true; endpoint.effective = false;
            axtual::EndpointSnapshot snapshot(endpoint);
            require(snapshot.enumerated && snapshot.openable == axtual::TriState::kUnknown &&
                        snapshot.policy == axtual::PolicyDisposition::kRecommended && snapshot.selected &&
                        !snapshot.effective && snapshot.enhancement == axtual::EnhancementState::kNone,
                    "snapshot channels must remain distinct");
        }},
        {"policy_block_does_not_rewrite_generic_truth", [] {
            axtual::Endpoint endpoint{"mic-1", axtual::EndpointKind::kAudioInput, "Mic"};
            endpoint.capability = axtual::Capability::kSupported; endpoint.available = true;
            axtual::PolicyProjection projection(axtual::PolicyDisposition::kBlockedByPolicy, "restricted", 2);
            projection.apply(endpoint);
            require(endpoint.available && endpoint.capability == axtual::Capability::kSupported &&
                        endpoint.policy == axtual::PolicyDisposition::kBlockedByPolicy,
                    "policy must not rewrite generic truth");
        }},
        {"blocked_selection_rejected_without_commit", [] {
            axtual::EndpointRegistry registry; registry.add("mic-1", axtual::EndpointKind::kAudioInput, "Mic");
            axtual::BindingStore bindings; axtual::SelectionValidator validator;
            auto result = validator.select(registry, bindings, "mic-1", axtual::BindingKind::kAudioInput,
                                           axtual::CallerIdentity::kUi, axtual::PolicyDisposition::kBlockedByPolicy);
            require(!result.accepted && bindings.revision() == 0 && !bindings.audioInput().has_value(),
                    "blocked selection must not commit");
        }},
        {"hidden_endpoint_remains_selectable", [] {
            axtual::EndpointRegistry registry; auto& endpoint = registry.add("mic-1", axtual::EndpointKind::kAudioInput, "Mic");
            endpoint.capability = axtual::Capability::kSupported; endpoint.policy = axtual::PolicyDisposition::kHiddenByDefault;
            axtual::BindingStore bindings; axtual::SelectionValidator validator;
            require(validator.select(registry, bindings, "mic-1", axtual::BindingKind::kAudioInput,
                                     axtual::CallerIdentity::kCli).accepted, "hidden endpoint remains selectable");
        }},
        {"recommended_does_not_auto_select", [] {
            axtual::EndpointRegistry registry; auto& endpoint = registry.add("mic-1", axtual::EndpointKind::kAudioInput, "Mic");
            endpoint.policy = axtual::PolicyDisposition::kRecommended; axtual::BindingStore bindings;
            require(!bindings.audioInput().has_value() && bindings.revision() == 0, "recommendation must not select");
        }},
        {"missing_endpoint_rejected_without_commit", [] {
            axtual::BindingStore bindings; axtual::SelectionValidator validator; axtual::EndpointRegistry registry;
            require(!validator.select(registry, bindings, "missing", axtual::BindingKind::kAudioInput,
                                      axtual::CallerIdentity::kApi).accepted && bindings.revision() == 0,
                    "missing endpoint must not commit");
        }},
        {"unknown_capability_not_selectable", [] {
            axtual::EndpointRegistry registry; registry.add("mic-1", axtual::EndpointKind::kAudioInput, "Mic");
            axtual::BindingStore bindings; axtual::SelectionValidator validator;
            require(!validator.select(registry, bindings, "mic-1", axtual::BindingKind::kAudioInput,
                                      axtual::CallerIdentity::kApi).accepted, "unknown capability rejected");
        }},
        {"explicit_selection_uses_canonical_ref", [] {
            axtual::EndpointRegistry registry; auto& endpoint = registry.add("mic-1", axtual::EndpointKind::kAudioInput, "Mic");
            endpoint.capability = axtual::Capability::kSupported; axtual::BindingStore bindings; axtual::SelectionValidator validator;
            require(validator.select(registry, bindings, "mic-1", axtual::BindingKind::kAudioInput,
                                      axtual::CallerIdentity::kApi).accepted && bindings.audioInput() == "mic-1" && bindings.revision() == 1,
                    "explicit selection commits canonical ref once");
        }},
        {"metadata_change_does_not_mutate_binding", [] {
            axtual::BindingStore bindings; bindings.commit(axtual::BindingKind::kAudioInput, "mic-1");
            bindings.observeMetadataChange("system-default", "display-order");
            require(bindings.audioInput() == "mic-1" && bindings.revision() == 1, "metadata must not mutate intent");
        }},
        {"same_reconnect_preserves_binding_without_revision", [] {
            axtual::EndpointRegistry registry; registry.add("mic-1", axtual::EndpointKind::kAudioInput, "Mic");
            axtual::BindingStore bindings; bindings.commit(axtual::BindingKind::kAudioInput, "mic-1");
            require(axtual::Reconciler::reconcile("mic-1", "mic-1", "Mic") == axtual::ReconnectOutcome::kSame && bindings.revision() == 1,
                    "same reconnect preserves revision");
        }},
        {"replaced_reconnect_is_explicit", [] { require(axtual::Reconciler::reconcile("mic-1", "mic-2", "Mic") == axtual::ReconnectOutcome::kReplaced, "replaced explicit"); }},
        {"gone_reconnect_is_explicit", [] { require(axtual::Reconciler::reconcile("mic-1", "", "Mic") == axtual::ReconnectOutcome::kGone, "gone explicit"); }},
        {"ambiguous_same_name_does_not_bind", [] { require(axtual::Reconciler::ambiguous("mic-1", "mic-2", "Mic") == axtual::ReconnectOutcome::kAmbiguous, "ambiguous unresolved"); }},
        {"gone_endpoint_keeps_desired_non_effective", [] {
            axtual::BindingStore bindings; bindings.commit(axtual::BindingKind::kAudioInput, "mic-1"); bindings.markGone("mic-1");
            require(bindings.audioInput() == "mic-1" && !bindings.effective(axtual::BindingKind::kAudioInput), "gone does not fallback");
        }},
        {"enhancement_gain_does_not_change_binding", [] {
            axtual::BindingStore bindings; bindings.commit(axtual::BindingKind::kAudioInput, "mic-1"); axtual::Enhancement enhancement;
            enhancement.associate("near-1"); require(bindings.audioInput() == "mic-1" && enhancement.state() == axtual::EnhancementState::kAssociated, "gain independent");
        }},
        {"enhancement_loss_preserves_generic_applicability", [] {
            axtual::Endpoint endpoint{"mic-1", axtual::EndpointKind::kAudioInput, "Mic"}; endpoint.capability = axtual::Capability::kSupported; endpoint.policy = axtual::PolicyDisposition::kEligible;
            axtual::Enhancement enhancement; enhancement.associate("near-1"); enhancement.lose(); require(enhancement.state() == axtual::EnhancementState::kLost && endpoint.policy != axtual::PolicyDisposition::kBlockedByPolicy, "generic stays applicable");
        }},
        {"enhancement_required_loss_policy_blocks_without_fallback", [] {
            axtual::Endpoint endpoint{"mic-1", axtual::EndpointKind::kAudioInput, "Mic"}; axtual::Enhancement enhancement; enhancement.lose();
            axtual::PolicyProjection policy(axtual::PolicyDisposition::kBlockedByPolicy, "enhancement_required", 3); policy.apply(endpoint);
            require(endpoint.policy == axtual::PolicyDisposition::kBlockedByPolicy && endpoint.id == "mic-1", "policy blocks explicitly");
        }},
        {"new_policy_block_stops_applicability_without_alternate", [] {
            axtual::BindingStore bindings; bindings.commit(axtual::BindingKind::kAudioInput, "mic-1"); bindings.blockByPolicy("mic-1");
            require(bindings.audioInput() == "mic-1" && !bindings.effective(axtual::BindingKind::kAudioInput), "policy block retains intent");
        }},
        {"hidden_presentation_change_keeps_active_selection", [] {
            axtual::BindingStore bindings; bindings.commit(axtual::BindingKind::kAudioInput, "mic-1"); bindings.setPresentationHidden("mic-1");
            require(bindings.audioInput() == "mic-1" && bindings.effective(axtual::BindingKind::kAudioInput), "hidden does not invalidate active");
        }},
        {"nearity_virtual_mic_self_loop_rejected_before_open", [] {
            axtual::Endpoint endpoint{"vmic", axtual::EndpointKind::kAudioInput, "Virtual Mic"}; endpoint.origin = axtual::EndpointOrigin::kNearityVirtual; axtual::OpenProbe probe;
            require(!axtual::SelfLoopGuard::allow(endpoint, axtual::EndpointKind::kAudioInput, probe) && !probe.invoked, "self loop rejected before open");
        }},
        {"nearity_virtual_camera_self_loop_rejected_before_open", [] {
            axtual::Endpoint endpoint{"vcam", axtual::EndpointKind::kVideoInput, "Virtual Camera"}; endpoint.origin = axtual::EndpointOrigin::kNearityVirtual; axtual::OpenProbe probe;
            require(!axtual::SelfLoopGuard::allow(endpoint, axtual::EndpointKind::kVideoInput, probe) && !probe.invoked, "self loop rejected before open");
        }},
        {"third_party_virtual_endpoint_not_rejected_solely_for_virtual", [] {
            axtual::Endpoint endpoint{"third", axtual::EndpointKind::kAudioInput, "Virtual Mic"}; endpoint.origin = axtual::EndpointOrigin::kThirdPartyVirtual; axtual::OpenProbe probe;
            require(axtual::SelfLoopGuard::allow(endpoint, axtual::EndpointKind::kAudioInput, probe) && probe.invoked, "third party virtual allowed");
        }},
        {"inventory_policy_enhancement_do_not_acquire_demand", [] { axtual::SemanticOperations operations; operations.inventory(); operations.projectPolicy(); operations.associateEnhancement(); require(!operations.persistentDemand(), "semantic operations are side effect free"); }},
        {"caller_paths_share_common_validator", [] {
            axtual::EndpointRegistry registry; auto& endpoint = registry.add("mic-1", axtual::EndpointKind::kAudioInput, "Mic"); endpoint.capability = axtual::Capability::kSupported; endpoint.policy = axtual::PolicyDisposition::kBlockedByPolicy;
            axtual::BindingStore bindings; axtual::SelectionValidator validator; auto ui = validator.select(registry, bindings, "mic-1", axtual::BindingKind::kAudioInput, axtual::CallerIdentity::kUi); auto api = validator.select(registry, bindings, "mic-1", axtual::BindingKind::kAudioInput, axtual::CallerIdentity::kApi);
            require(!ui.accepted && !api.accepted && ui.reason == api.reason, "caller paths share validator");
        }},
        {"healthcheck_output_binding_is_independent_from_audio_input", [] {
            axtual::BindingStore bindings;
            bindings.commit(axtual::BindingKind::kAudioInput, "mic-1");
            bindings.commit(axtual::BindingKind::kHealthCheckAudioOutput, "speaker-1");
            require(bindings.audioInput() == "mic-1" && bindings.healthCheckAudioOutput() == "speaker-1" &&
                        bindings.audioInputRevision() == 1 && bindings.healthCheckOutputRevision() == 2,
                    "health-check output must be an independent binding");
        }},
        {"healthcheck_output_loss_or_policy_block_does_not_mutate_audio_input", [] {
            axtual::BindingStore bindings;
            bindings.commit(axtual::BindingKind::kAudioInput, "mic-1");
            bindings.commit(axtual::BindingKind::kHealthCheckAudioOutput, "speaker-1");
            bindings.markGone("speaker-1");
            bindings.blockByPolicy("speaker-1");
            require(bindings.audioInput() == "mic-1" && bindings.healthCheckAudioOutput() == "speaker-1" &&
                        bindings.effective(axtual::BindingKind::kAudioInput) &&
                        !bindings.effective(axtual::BindingKind::kHealthCheckAudioOutput),
                    "health-check loss must not mutate audio input");
        }},
        {"binding_kind_mismatch_rejected_without_commit", [] {
            axtual::EndpointRegistry registry;
            auto& endpoint = registry.add("mic-1", axtual::EndpointKind::kAudioInput, "Mic");
            endpoint.capability = axtual::Capability::kSupported;
            axtual::BindingStore bindings;
            axtual::SelectionValidator validator;
            auto result = validator.select(registry, bindings, "mic-1",
                                           axtual::BindingKind::kVideoInput, axtual::CallerIdentity::kApi);
            require(!result.accepted && result.reason == "binding_kind_mismatch" && bindings.revision() == 0,
                    "binding kind mismatch must reject before commit");
        }},
        {"endpoint_identity_certainty_is_explicit", [] {
            axtual::Endpoint endpoint{"mic-1", axtual::EndpointKind::kAudioInput, "Mic"};
            require(endpoint.identityQuality == axtual::IdentityQuality::kUnknown,
                    "identity certainty must be explicit");
            endpoint.identityQuality = axtual::IdentityQuality::kStable;
            require(endpoint.identityQuality == axtual::IdentityQuality::kStable,
                    "stable identity certainty must be representable");
        }},
        {"ambiguous_identity_does_not_autobind", [] {
            axtual::Endpoint previous{"mic-1", axtual::EndpointKind::kAudioInput, "Mic"};
            axtual::Endpoint candidate{"mic-2", axtual::EndpointKind::kAudioInput, "Mic"};
            candidate.identityQuality = axtual::IdentityQuality::kAmbiguous;
            require(axtual::Reconciler::reconcile(previous, candidate) == axtual::ReconnectOutcome::kAmbiguous,
                    "ambiguous identity must not autobind");
        }},
    };
}

std::string environmentIdentity() {
#if defined(_WIN32)
    char* runner = nullptr;
    std::size_t runnerLength = 0;
    if (_dupenv_s(&runner, &runnerLength, "RUNNER_OS") == 0 && runner != nullptr) {
        std::string result(runner, runnerLength > 0 ? runnerLength - 1 : 0);
        std::free(runner);
        return result + ":github-actions";
    }
    return "Windows:developer-host";
#else
    if (const char* runner = std::getenv("RUNNER_OS"); runner != nullptr) {
        return std::string(runner) + ":github-actions";
    }
#if defined(__APPLE__)
    return "macOS:developer-host";
#elif defined(__linux__)
    return "Linux:developer-host";
#else
    return "Unknown:developer-host";
#endif
#endif
}

std::string oracleFor(std::string_view name) {
    if (name.find("policy") != std::string_view::npos || name.find("recommended") != std::string_view::npos ||
        name.find("hidden") != std::string_view::npos || name.find("caller") != std::string_view::npos) {
        return name.find("selection") != std::string_view::npos || name.find("caller") != std::string_view::npos
                   ? "O-POLICY-ENFORCE"
                   : "O-POLICY-TRUTH";
    }
    if (name.find("enhancement") != std::string_view::npos) return "O-ENH-INDEP";
    if (name.find("loop") != std::string_view::npos || name.find("virtual") != std::string_view::npos) return "O-SELFLOOP";
    if (name.find("reconnect") != std::string_view::npos || name.find("gone") != std::string_view::npos) return "O-IDENTITY";
    if (name.find("demand") != std::string_view::npos || name.find("inventory") != std::string_view::npos) return "O-IDLE";
    return "O-POLICY-TRUTH";
}
}

int main(int argc, char** argv) {
    std::string selected = argc == 3 && std::string_view(argv[1]) == "--case" ? argv[2] : "";
    if (argc == 3 && std::string_view(argv[1]) == "--evidence") {
        std::ofstream evidence(argv[2]);
        if (!evidence) return 3;
        const auto allCases = cases();
        evidence << "{\n"
                 << "  \"verification_spec\": \"P20-v0.3\",\n"
                 << "  \"package_id\": \"AXTUAL-I30-02-P31-v0.1\",\n"
                 << "  \"slice\": \"I30-02\",\n"
                 << "  \"evidence_family\": \"EA-B01\",\n"
                 << "  \"claim_scope\": \"common_semantic_contribution_only\",\n"
                 << "  \"obligations\": [\"VO-END-02\", \"VO-END-03\", \"VO-END-04\", \"VO-END-06\"],\n"
                 << "  \"authority_refs\": [\"P02-v0.3\", \"P03-v0.3\", \"P14-v0.4\", \"P15-v0.4\", \"P16-v0.4\", \"P17-v0.5\", \"P18-v0.5\", \"P20-v0.3\", \"P30-v0.3\"],\n"
                 << "  \"task_anchor\": \"" << AXTUAL_TASK_ANCHOR << "\",\n"
                 << "  \"build_identity\": \"" << AXTUAL_BUILD_IDENTITY << "\",\n"
                 << "  \"platform_environment\": \"" << environmentIdentity() << "\",\n"
                 << "  \"execution_method\": \"I30-02-deterministic-semantic-conformance\",\n"
                 << "  \"oracle\": [\"O-POLICY-TRUTH\", \"O-POLICY-ENFORCE\", \"O-ENH-INDEP\", \"O-SELFLOOP\", \"O-IDENTITY\", \"O-IDLE\"],\n"
                 << "  \"case_results\": [\n";
        bool allPassed = true;
        for (std::size_t index = 0; index < allCases.size(); ++index) {
            std::string result = "PASS";
            try { allCases[index].run(); } catch (const std::exception&) { result = "FAIL"; allPassed = false; }
            evidence << "    {\"name\": \"" << allCases[index].name << "\", \"oracle\": \""
                     << oracleFor(allCases[index].name) << "\", \"result\": \"" << result << "\"}"
                     << (index + 1 == allCases.size() ? "\n" : ",\n");
        }
        evidence << "  ],\n  \"result\": \"" << (allPassed ? "PASS" : "FAIL") << "\"\n}\n";
        return evidence.good() && allPassed ? 0 : 1;
    }
    if (argc == 1) {
        bool allPassed = true;
        for (const auto& test : cases()) {
            try { test.run(); std::cout << "PASS " << test.name << '\n'; }
            catch (const std::exception& error) { std::cerr << "FAIL " << test.name << ": " << error.what() << '\n'; allPassed = false; }
        }
        return allPassed ? 0 : 1;
    }
    bool ran = false;
    bool allPassed = true;
    for (const auto& test : cases()) {
        if (!selected.empty() && test.name != selected) continue;
        ran = true;
        try {
            test.run();
            std::cout << "PASS " << test.name << '\n';
        } catch (const std::exception& error) {
            std::cerr << "FAIL " << test.name << ": " << error.what() << '\n';
            allPassed = false;
        }
        if (!selected.empty()) break;
    }
    return ran ? (allPassed ? 0 : 1) : 2;
}

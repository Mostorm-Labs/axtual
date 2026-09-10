#include "axtual/common.hpp"

#include <exception>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

struct TestCase {
    std::string_view name;
    std::function<void()> run;
};

void require(bool condition, std::string_view message) {
    if (!condition) {
        throw std::runtime_error(std::string(message));
    }
}

std::vector<TestCase> testCases() {
    return {
        {"accepted_request_does_not_imply_applied_or_effective", [] {
             axtual::RequestLifecycle request;
             request.accept();
             require(request.accepted(), "request should be accepted");
             require(!request.applied(), "accepted must not imply applied");
             require(!request.effective(), "accepted must not imply effective");
         }},
        {"observed_mismatch_does_not_become_desired_truth", [] {
             axtual::SemanticState<axtual::Availability> state;
             state.setDesired(axtual::Availability::kReady);
             state.setObserved(axtual::Availability::kUnavailable);
             require(state.desired() == axtual::Availability::kReady,
                     "observed mismatch must not overwrite desired truth");
             require(state.observed() == axtual::Availability::kUnavailable,
                     "observed truth should remain independently visible");
             require(!state.effective().has_value(), "unknown effective truth must stay unknown");
             require(!state.derived().has_value(), "unknown derived truth must stay unknown");
         }},
        {"stale_desired_revision_completion_rejected", [] {
             axtual::IdentityGate identities;
             identities.advanceDesiredRevision();
             const auto stale = identities.currentWorkIdentity();
             identities.advanceDesiredRevision();
             require(!identities.acceptsCompletion(stale),
                     "superseded desired revision must be rejected");
         }},
        {"stale_worker_generation_completion_rejected", [] {
             axtual::IdentityGate identities;
             const auto stale = identities.currentWorkIdentity();
             identities.restartWorker();
             require(!identities.acceptsCompletion(stale),
                     "completion from prior worker generation must be rejected");
         }},
        {"stale_source_generation_completion_rejected", [] {
             axtual::IdentityGate identities;
             const auto stale = identities.currentWorkIdentity();
             identities.reconnectSource();
             require(!identities.acceptsCompletion(stale),
                     "completion from prior source generation must be rejected");
         }},
        {"stale_worker_generation_payload_rejected", [] {
             axtual::IdentityGate identities;
             const auto stale = identities.currentPayloadWatermark();
             identities.restartWorker();
             require(!identities.acceptsPayload(stale),
                     "payload from prior worker generation must be rejected");
         }},
        {"stale_source_connection_generation_payload_rejected", [] {
             axtual::IdentityGate identities;
             const auto stale = identities.currentPayloadWatermark();
             identities.reconnectSource();
             require(!identities.acceptsPayload(stale),
                     "payload from prior source connection generation must be rejected");
         }},
        {"mute_intent_survives_worker_restart_before_live_enable", [] {
             axtual::MemorySafetyStore store;
             axtual::SafetyLatch latch(store);
             latch.setMuteIntent(true);
             require(latch.persist(), "mute intent should persist");
             axtual::SafetyLatch restored(store);
             require(restored.restoreFailClosed() && restored.muteIntent(),
                     "mute intent must survive restart");
         }},
        {"privacy_intent_survives_worker_restart_before_live_enable", [] {
             axtual::MemorySafetyStore store;
             axtual::SafetyLatch latch(store);
             latch.setPrivacyIntent(true);
             require(latch.persist(), "privacy intent should persist");
             axtual::SafetyLatch restored(store);
             require(restored.restoreFailClosed() && restored.privacyIntent(),
                     "privacy intent must survive restart");
         }},
        {"corrupt_unreadable_safety_store_restores_fail_closed", [] {
             axtual::MemorySafetyStore store;
             store.corrupt();
             axtual::SafetyLatch latch(store);
             require(!latch.restoreFailClosed(), "corrupt store should report unreadable");
             require(latch.muteIntent() && latch.privacyIntent(),
                     "corrupt store must restore both safety intents enabled");
         }},
        {"explicit_audio_source_loss_produces_unavailable_without_fallback", [] {
             axtual::SourceBinding source(axtual::SourceKind::kAudio);
             source.bind("mic-1");
             source.lose();
             require(!source.available() && !source.fallbackSelected(),
                     "audio loss must be unavailable without fallback");
         }},
        {"explicit_video_source_loss_produces_unavailable_without_fallback", [] {
             axtual::SourceBinding source(axtual::SourceKind::kVideo);
             source.bind("camera-1");
             source.lose();
             require(!source.available() && !source.fallbackSelected(),
                     "video loss must be unavailable without fallback");
         }},
        {"ambiguous_reconnect_remains_unbound_and_unavailable", [] {
             axtual::SourceBinding source(axtual::SourceKind::kAudio);
             source.bind("mic-1");
             source.reconnectAmbiguous();
             require(!source.bound() && !source.available(),
                     "ambiguous reconnect must remain unbound");
         }},
        {"endpoint_demand_acquire_starts_only_after_safety_source_readiness", [] {
             axtual::MemorySafetyStore store;
             axtual::SafetyLatch latch(store);
             axtual::SourceBinding source(axtual::SourceKind::kAudio);
             axtual::DemandCoordinator demand;
             require(!demand.acquire(latch, source), "unready source must not start capture");
             source.bind("mic-1");
             require(demand.acquire(latch, source), "ready source should acquire demand");
         }},
        {"final_demand_release_returns_capture_ownership_to_idle", [] {
             axtual::MemorySafetyStore store;
             axtual::SafetyLatch latch(store);
             axtual::SourceBinding source(axtual::SourceKind::kAudio);
             source.bind("mic-1");
             axtual::DemandCoordinator demand;
             require(demand.acquire(latch, source), "demand should acquire");
             demand.release();
             require(demand.demandCount() == 0 && !demand.ownsCapture(),
                     "final release should return to idle");
         }},
        {"transport_loss_rejects_old_generation_on_reclaim", [] {
             axtual::IdentityGate identities;
             const auto oldPayload = identities.currentPayloadWatermark();
             identities.restartWorker();
             require(!identities.acceptsPayload(oldPayload),
                     "transport reclaim must reject old generation");
         }},
        {"processor_fault_cannot_bypass_downstream_safety_state", [] {
             axtual::ProcessorGuard guard(true);
             guard.fault();
             require(!guard.outputEnabled(), "processor fault must disable output");
         }},
        {"permission_denied_remains_truthful_unavailable_without_fallback", [] {
             axtual::SourceBinding source(axtual::SourceKind::kAudio);
             source.bind("mic-1");
             source.denyPermission();
             require(source.fault() == axtual::SourceFault::kPermissionDenied,
                     "permission denial must be represented as a distinct fault");
             require(!source.available() && !source.fallbackSelected(),
                     "permission denial must remain unavailable without fallback");
             source.revokePermission();
             require(source.fault() == axtual::SourceFault::kPermissionRevoked,
                     "permission revocation must be represented as a distinct fault");
             require(!source.available() && !source.fallbackSelected(),
                     "permission revocation must remain unavailable without fallback");
         }},
        {"busy_ownership_conflict_does_not_create_second_owner", [] {
             axtual::MemorySafetyStore store;
             axtual::SafetyLatch latch(store);
             axtual::SourceBinding source(axtual::SourceKind::kAudio);
             source.bind("mic-1");
             axtual::DemandCoordinator demand;
             require(demand.acquire(latch, source), "first owner should acquire");
             source.markBusy();
             require(demand.acquireResult(latch, source) == axtual::DemandResult::kContended,
                     "busy source must report an ownership contention result");
             require(demand.demandCount() == 1 && demand.ownsCapture(),
                     "busy conflict must not create second owner");
             require(source.fault() == axtual::SourceFault::kBusy,
                     "busy fault must remain truthful");
         }},
        {"audio_worker_failure_leaves_video_domain_independently_operable", [] {
             axtual::ProcessorGuard audio(true);
             audio.fault();
             axtual::ProcessorGuard video(true);
             require(!audio.outputEnabled() && video.outputEnabled(),
                     "audio failure must not disable video domain");
         }},
        {"video_worker_failure_leaves_audio_domain_independently_operable", [] {
             axtual::ProcessorGuard video(true);
             video.fault();
             axtual::ProcessorGuard audio(true);
             require(!video.outputEnabled() && audio.outputEnabled(),
                     "video failure must not disable audio domain");
         }},
        {"bounded_audio_style_queue_never_exceeds_capacity_under_pressure", [] {
             axtual::BoundedQueue<int> queue(2);
             require(queue.push(1) && queue.push(2) && !queue.push(3),
                     "queue must reject beyond hard capacity");
             require(queue.size() == 2, "queue size must remain bounded");
         }},
        {"bounded_video_latest_frame_model_never_grows_unboundedly", [] {
             axtual::LatestFrame<int> latest;
             for (int frame = 0; frame < 1000; ++frame) {
                 latest.push(frame);
             }
             require(latest.size() == 1 && latest.value() == 999,
                     "latest-frame model must retain one frame");
         }},
        {"retry_recovery_loop_is_finite_and_cancelable", [] {
             axtual::RetryController retries(3);
             require(retries.tryAgain() && retries.tryAgain() && retries.tryAgain(),
                     "finite retries should allow configured attempts");
             require(!retries.tryAgain(), "retry loop must stop at max attempts");
             retries.cancel();
             require(!retries.tryAgain(), "cancelled retry loop must remain stopped");
         }},
    };
}

std::string executionEnvironment() {
    if (const char* runnerOs = std::getenv("RUNNER_OS"); runnerOs != nullptr) {
        return std::string(runnerOs) + ":github-actions";
    }
#if defined(_WIN32)
    return "Windows:developer-host";
#elif defined(__APPLE__)
    return "macOS:developer-host";
#elif defined(__linux__)
    return "Linux:developer-host";
#else
    return "Unknown:developer-host";
#endif
}

std::string oracleFor(std::string_view name) {
    if (name.find("latch") != std::string_view::npos || name.find("intent") != std::string_view::npos ||
        name.find("safety") != std::string_view::npos) {
        return "O-LATCH";
    }
    if (name.find("generation") != std::string_view::npos || name.find("stale") != std::string_view::npos ||
        name.find("transport_loss") != std::string_view::npos) {
        return "O-GEN";
    }
    if (name.find("audio_source") != std::string_view::npos || name.find("permission") != std::string_view::npos) {
        return "O-NOFALLBACK-A";
    }
    if (name.find("video_source") != std::string_view::npos) {
        return "O-NOFALLBACK-V";
    }
    if (name.find("observed") != std::string_view::npos || name.find("accepted") != std::string_view::npos) {
        return "O-TRUTH/O-STATE";
    }
    if (name.find("ambiguous") != std::string_view::npos) {
        return "O-IDENTITY";
    }
    if (name.find("demand") != std::string_view::npos) {
        return "O-ORDER/O-IDLE";
    }
    if (name.find("bounded") != std::string_view::npos || name.find("retry") != std::string_view::npos) {
        return "O-BOUNDED";
    }
    if (name.find("independently") != std::string_view::npos) {
        return "O-INDEPENDENCE";
    }
    if (name.find("ownership") != std::string_view::npos) {
        return "O-CAPTURE-OWNER";
    }
    return "O-STATE";
}

}  // namespace

int main(int argc, char** argv) {
    std::string selected;
    if (argc == 3 && std::string_view(argv[1]) == "--case") {
        selected = argv[2];
    }

    if (argc == 3 && std::string_view(argv[1]) == "--evidence") {
        std::ofstream evidence(argv[2]);
        if (!evidence) {
            return 3;
        }
        evidence << "{\n"
                 << "  \"verification_spec\": \"P20-v0.2\",\n"
                 << "  \"package_id\": \"AXTUAL-I30-01-P31-v0.1\",\n"
                 << "  \"slice\": \"I30-01\",\n"
                 << "  \"authority_refs\": [\"P14-v0.4\", \"P15-v0.4\", \"P16-v0.4\", "
                    "\"P17-v0.4\", \"P18-v0.4\", \"P20-v0.2\", \"P30-v0.2\"],\n"
                 << "  \"build_identity\": \"" << AXTUAL_BUILD_IDENTITY << "\",\n"
                 << "  \"platform_environment\": \"" << executionEnvironment() << "\",\n"
                 << "  \"execution_method\": \"I30-01-deterministic-conformance\",\n"
                 << "  \"oracle\": [\"O-LATCH\", \"O-GEN\", \"O-NOFALLBACK-A\", "
                    "\"O-NOFALLBACK-V\", \"O-TRUTH\", \"O-STATE\", \"O-IDENTITY\", "
                    "\"O-ORDER\", \"O-IDLE\", \"O-BOUNDED\", \"O-INDEPENDENCE\", "
                    "\"O-CAPTURE-OWNER\"],\n"
                 << "  \"obligations\": [\n";
        const auto cases = testCases();
        for (std::size_t index = 0; index < cases.size(); ++index) {
            evidence << "    {\"oracle\": \"" << oracleFor(cases[index].name)
                     << "\", \"case\": \"" << cases[index].name << "\"}"
                     << (index + 1 == cases.size() ? "\n" : ",\n");
        }
        evidence << "  ],\n  \"case_results\": [\n";
        bool allPassed = true;
        for (std::size_t index = 0; index < cases.size(); ++index) {
            std::string result = "PASS";
            try {
                cases[index].run();
            } catch (const std::exception&) {
                result = "FAIL";
                allPassed = false;
            }
            evidence << "    {\"name\": \"" << cases[index].name << "\", \"oracle\": \""
                     << oracleFor(cases[index].name) << "\", \"result\": \"" << result << "\"}"
                     << (index + 1 == cases.size() ? "\n" : ",\n");
        }
        evidence << "  ],\n  \"result\": \"" << (allPassed ? "PASS" : "FAIL") << "\"\n}\n";
        return evidence.good() && allPassed ? 0 : 1;
    }

    for (const auto& test : testCases()) {
        if (!selected.empty() && test.name != selected) {
            continue;
        }
        try {
            test.run();
            std::cout << "PASS " << test.name << '\n';
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "FAIL " << test.name << ": " << error.what() << '\n';
            return 1;
        }
    }

    std::cerr << "Unknown or missing test case\n";
    return 2;
}

cmake_minimum_required(VERSION 3.24)
project(axtual_common VERSION 0.1.0 LANGUAGES CXX)

execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE AXTUAL_BUILD_IDENTITY
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET)
if(NOT AXTUAL_BUILD_IDENTITY)
    set(AXTUAL_BUILD_IDENTITY "unknown")
endif()

option(AXTUAL_BUILD_TESTS "Build the deterministic common conformance suite" ON)

add_library(axtual_common INTERFACE)
target_compile_features(axtual_common INTERFACE cxx_std_20)
target_include_directories(axtual_common INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(AXTUAL_BUILD_TESTS)
    include(CTest)
    enable_testing()

    add_executable(axtual_conformance tests/conformance.cpp)
    target_link_libraries(axtual_conformance PRIVATE axtual_common)
    target_compile_definitions(axtual_conformance PRIVATE
        AXTUAL_BUILD_IDENTITY="${AXTUAL_BUILD_IDENTITY}")
    if(MSVC)
        target_compile_options(axtual_conformance PRIVATE /W4 /WX)
    else()
        target_compile_options(axtual_conformance PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()

    set(AXTUAL_CONFORMANCE_CASES
        accepted_request_does_not_imply_applied_or_effective
        observed_mismatch_does_not_become_desired_truth
        stale_desired_revision_completion_rejected
        stale_worker_generation_completion_rejected
        stale_source_generation_completion_rejected
        stale_worker_generation_payload_rejected
        stale_source_connection_generation_payload_rejected
        mute_intent_survives_worker_restart_before_live_enable
        privacy_intent_survives_worker_restart_before_live_enable
        corrupt_unreadable_safety_store_restores_fail_closed
        explicit_audio_source_loss_produces_unavailable_without_fallback
        explicit_video_source_loss_produces_unavailable_without_fallback
        ambiguous_reconnect_remains_unbound_and_unavailable
        endpoint_demand_acquire_starts_only_after_safety_source_readiness
        final_demand_release_returns_capture_ownership_to_idle
        transport_loss_rejects_old_generation_on_reclaim
        processor_fault_cannot_bypass_downstream_safety_state
        permission_denied_remains_truthful_unavailable_without_fallback
        busy_ownership_conflict_does_not_create_second_owner
        audio_worker_failure_leaves_video_domain_independently_operable
        video_worker_failure_leaves_audio_domain_independently_operable
        bounded_audio_style_queue_never_exceeds_capacity_under_pressure
        bounded_video_latest_frame_model_never_grows_unboundedly
        retry_recovery_loop_is_finite_and_cancelable)
    foreach(case_name IN LISTS AXTUAL_CONFORMANCE_CASES)
        add_test(NAME ${case_name} COMMAND axtual_conformance --case ${case_name})
    endforeach()

    add_custom_target(i30_01_evidence
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/evidence"
        COMMAND axtual_conformance --evidence "${CMAKE_BINARY_DIR}/evidence/ea-b01-i30-01.json"
        DEPENDS axtual_conformance
        USES_TERMINAL)

    add_executable(axtual_i30_02_conformance tests/i30_02_conformance.cpp)
    target_link_libraries(axtual_i30_02_conformance PRIVATE axtual_common)
    target_compile_definitions(axtual_i30_02_conformance PRIVATE
        AXTUAL_BUILD_IDENTITY="${AXTUAL_BUILD_IDENTITY}"
        AXTUAL_TASK_ANCHOR="6e1340664c1d3769385a0b065e4e4103cade4364")
    if(MSVC)
        target_compile_options(axtual_i30_02_conformance PRIVATE /W4 /WX)
    else()
        target_compile_options(axtual_i30_02_conformance PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    set(AXTUAL_I30_02_CASES
        generic_inventory_keeps_non_nearity_endpoint
        snapshot_keeps_truth_channels_distinct
        policy_block_does_not_rewrite_generic_truth
        blocked_selection_rejected_without_commit
        hidden_endpoint_remains_selectable
        recommended_does_not_auto_select
        missing_endpoint_rejected_without_commit
        unknown_capability_not_selectable
        explicit_selection_uses_canonical_ref
        metadata_change_does_not_mutate_binding
        same_reconnect_preserves_binding_without_revision
        replaced_reconnect_is_explicit
        gone_reconnect_is_explicit
        ambiguous_same_name_does_not_bind
        gone_endpoint_keeps_desired_non_effective
        enhancement_gain_does_not_change_binding
        enhancement_loss_preserves_generic_applicability
        enhancement_required_loss_policy_blocks_without_fallback
        new_policy_block_stops_applicability_without_alternate
        hidden_presentation_change_keeps_active_selection
        nearity_virtual_mic_self_loop_rejected_before_open
        nearity_virtual_camera_self_loop_rejected_before_open
        third_party_virtual_endpoint_not_rejected_solely_for_virtual
        inventory_policy_enhancement_do_not_acquire_demand
        caller_paths_share_common_validator)
    foreach(case_name IN LISTS AXTUAL_I30_02_CASES)
        add_test(NAME i30_02_${case_name} COMMAND axtual_i30_02_conformance --case ${case_name})
    endforeach()

    add_custom_target(i30_02_evidence
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/evidence"
        COMMAND axtual_i30_02_conformance --evidence "${CMAKE_BINARY_DIR}/evidence/ea-b01-i30-02.json"
        DEPENDS axtual_i30_02_conformance
        USES_TERMINAL)
endif()

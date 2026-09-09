# AXTUAL P31 Task Package — I30-01 Common Safety/State + Deterministic Harness v0.1

Status: APPROVED_FOR_P32
Stage owner: aegis-implementation
Stage: P31 Task Packaging
Package ID: AXTUAL-I30-01-P31-v0.1

## 1. Purpose

Bootstrap the empty `Mostorm-Labs/axtual` repository and implement the first P30 v0.2 vertical slice: the cross-platform Common Safety/State Runtime plus deterministic conformance harness. This package establishes the reusable semantic kernel required before Generic Endpoint, Product Policy, Nearity Enhancement, public API, or real Windows/macOS media endpoints are implemented.

Repository bootstrap in this package is limited to build/test/evidence scaffolding required to execute I30-01. It is not authorization for later slices.

## 2. Trusted Basis

- P02 V1 Product Requirements v0.3 — PASS / FROZEN
  - https://app.notion.com/p/3d64c57a590c8129af38e0c46d9bd5a9?pvs=204
- P03 V1 Capability Traceability v0.3 — PASS / FROZEN
  - https://app.notion.com/p/3d64c57a590c8192bbecf5649897f030?pvs=204
- P14 System Architecture v0.4 — PASS / FROZEN / RECONCILED
  - https://app.notion.com/p/3d64c57a590c81679e3cf339c057834b?pvs=204
- P15 Module Design v0.4 — PASS / FROZEN / RECONCILED
  - https://app.notion.com/p/3d64c57a590c8104afc3dce3965766a6?pvs=204
- P16 Runtime Data Flow v0.4 — PASS / FROZEN / RECONCILED
  - https://app.notion.com/p/3d64c57a590c81f6be52ee00d38ebef0?pvs=204
- P17 Platform Contract v0.4 — PASS / FROZEN / RECONCILED
  - https://app.notion.com/p/3d64c57a590c81ad882ac8fb6ccf449c?pvs=204
- P18 Engineering & Optimization v0.4 — PASS / FROZEN / RECONCILED
  - https://app.notion.com/p/3d64c57a590c813088cac5dd0d427161?pvs=204
- P20 Verification Design v0.2 — PASS / FROZEN / RECONCILED
  - https://app.notion.com/p/3d64c57a590c8106aaa3d6f88b80415a?pvs=204
  - incorporates the retained P20 v0.1 base safety/runtime oracles for EA-B01
- P30 Implementation Plan v0.2 — PASS / FROZEN / RECONCILED / READY_FOR_P31
  - https://app.notion.com/p/3d64c57a590c810381cdf030fd26f46d?pvs=204

VerificationSpec: `P20-v0.2`
Evidence family: `EA-B01 Runtime Safety Conformance` — I30-01 common/base contribution only.

## 3. Repository Binding

```yaml
repository:
  provider: github
  full_name: Mostorm-Labs/axtual
  repository_id: "1359894171"
  default_branch: main
initial_repository_state: empty
```

The immutable commit containing this package is the package materialization ref and task anchor for the first P32 execution. Exact SHA is bound by the P31 surface handoff after this file is materialized.

## 4. Authorized Technical Baseline

For the empty-repository bootstrap, freeze the following implementation choices for I30-01 only:

- Language: C++20.
- Build system: CMake 3.24+ with CTest.
- Common runtime code must use the C++ standard library only unless a dependency is demonstrably necessary to close this package. Do not introduce a general dependency framework in I30-01.
- Common runtime must compile on Windows and macOS; Linux may be used as the canonical deterministic conformance host.
- No UI framework, WebSocket library, OS media SDK abstraction, installer framework, driver SDK, DSP library, or package manager integration is authorized by this package.

These choices are implementation details for the first common-core slice and do not redefine upstream architecture.

## 5. Scope Contract

### Required changes

1. Bootstrap a minimal CMake/C++20 repository layout suitable for later common Core slices.
2. Implement semantic state primitives representing `desired`, `observed`, `effective`, and `derived` truth without conflating them.
3. Implement request lifecycle semantics preserving `Accepted != Applied != Effective`.
4. Implement monotonic identities for at least:
   - desired revision;
   - worker generation;
   - source connection generation.
5. Implement stale completion/event rejection so superseded revision/generation work cannot overwrite current state or become current payload truth.
6. Implement fail-closed safety latch semantics for software mute and camera privacy intent, including a persistence abstraction and deterministic corrupt/unreadable-store behavior.
7. Implement explicit source binding state with no implicit system-default fallback.
8. Implement bounded queue primitives sufficient for deterministic audio/video/control pressure tests; no unbounded media backlog is permitted.
9. Implement finite/cancelable recovery/retry primitives sufficient to prove no runaway retry loop.
10. Implement deterministic fake adapters and fault injection seams for:
    - source loss/reconnect;
    - ambiguous reconnect identity outcome;
    - worker restart;
    - Core restart/reconstruction;
    - virtual transport loss;
    - permission denied;
    - device/camera busy;
    - processor fault;
    - endpoint/observation-style demand acquire/release;
    - stalled/slow consumer pressure.
11. Implement payload watermark support in the harness so stale generation leakage is externally observable.
12. Implement a deterministic conformance executable/test harness that exercises the frozen I30-01 oracle set and can emit a machine-readable evidence input JSON.
13. Add GitHub Actions workflows that:
    - run canonical deterministic conformance on Linux and upload the evidence JSON;
    - build and run the common test suite on Windows;
    - build and run the common test suite on macOS.
14. Add concise developer documentation for configuring, building, testing, and generating I30-01 evidence.

### Forbidden changes

- No Generic Endpoint Registry, Product Policy, or Nearity Enhancement semantics from I30-02.
- No public WebSocket/bootstrap/Observation implementation from I30-03.
- No physical Nearity DSP/HID/firmware implementation from I30-04.
- No WASAPI/MMDevice/Media Foundation/Core Audio/AVFoundation platform endpoint implementation.
- No Virtual Mic/Virtual Camera driver or extension code.
- No UI application.
- No installer/update/repair/release-set implementation.
- No diagnostics/support-bundle implementation.
- No PC-side NR/AGC/Limiter/EQ processing.
- No automatic fallback to OS default source.
- No product-level rollback.
- No new blocking evidence or new product requirement beyond P20/P30 Authority.

## 6. Frozen Acceptance Oracles

I30-01 binds the common/base portion of EA-B01 to these retained P20 oracles:

- `O-LATCH`: safety latch survives restart/recovery and corrupt persistence fails safe.
- `O-GEN`: stale revision/generation events and payloads never become current effective truth/output.
- `O-NOFALLBACK-A`: loss of explicitly bound audio source never selects another/default audio source.
- `O-NOFALLBACK-V`: loss of explicitly bound video source never selects another/default video source.
- `O-TRUTH` / `O-STATE`: accepted/applied/effective and desired/observed truth remain distinct; unknown stays unknown.
- `O-IDENTITY`: ambiguous reconnect does not auto-bind.
- `O-ORDER`: safety is established before any current-generation live enable.
- `O-IDLE`: no legitimate demand means capture/open ownership is released in the harness model.
- `O-BOUNDED`: queues/retries remain finite and bounded under pressure/faults.
- `O-INDEPENDENCE`: modeled audio and video failure domains do not destroy each other.
- `O-CAPTURE-OWNER`: modeled busy/ownership conflicts never create a second capture owner.

I30-01 does not claim the real OS endpoint portions of these oracles; those remain for later platform slices.

## 7. Required Test Cases

At minimum, the deterministic suite must contain independently named cases for:

1. accepted request does not imply applied/effective;
2. observed mismatch does not silently become desired truth;
3. stale desired revision completion rejected;
4. stale worker generation payload rejected;
5. stale source connection generation payload rejected;
6. mute intent survives worker restart before live enable;
7. privacy intent survives worker restart before live enable;
8. corrupt/unreadable safety store restores fail-closed;
9. explicit audio source loss produces unavailable/safe state and no fallback marker;
10. explicit video source loss produces unavailable/safe state and no fallback marker;
11. ambiguous reconnect remains unbound/unavailable;
12. endpoint/demand acquire starts only after safety/source readiness;
13. final demand release returns modeled capture ownership to idle;
14. transport loss rejects old generation on reclaim;
15. processor fault cannot bypass downstream safety state;
16. permission denied remains truthful unavailable and does not fallback;
17. busy ownership conflict does not create second owner;
18. audio worker failure leaves modeled video domain independently operable;
19. video worker failure leaves modeled audio domain independently operable;
20. bounded audio-style queue never exceeds configured hard capacity under stalled consumer;
21. bounded video latest-frame model never grows unboundedly;
22. retry/recovery loop is finite/cancelable under repeated recoverable faults.

Test names may differ, but one-to-one traceability from the evidence output to these cases is required.

## 8. Evidence Compilation Contract

The canonical conformance run must emit `ea-b01-i30-01.json` with at least:

```yaml
verification_spec: P20-v0.2
package_id: AXTUAL-I30-01-P31-v0.1
slice: I30-01
obligations:
  - common/base EA-B01 oracle bindings named above
authority_refs:
  - P14-v0.4
  - P15-v0.4
  - P16-v0.4
  - P17-v0.4
  - P18-v0.4
  - P20-v0.2
  - P30-v0.2
build_identity: <exact git revision supplied at execution time>
platform_environment: <host identity>
execution_method: I30-01-deterministic-conformance
oracle: <mapped O-* set>
case_results: <machine-readable named cases>
result: PASS | FAIL | BLOCKED_ENVIRONMENT
```

The harness may populate exact revision/environment at runtime or CI time. It must not hard-code a future result revision into source.

This JSON is an EvidenceInput for later evidence compilation/review. The executor return must navigate to the durable provider artifact/ref and must not duplicate its machine facts as prose proof.

## 9. EXECUTION_CLOSURE_CONTRACT

```yaml
EXECUTION_CLOSURE_CONTRACT:
  implementation:
    required_changes:
      - bootstrap C++20/CMake common-core repository scaffold
      - implement desired/observed/effective/derived state model
      - implement Accepted/Applied/Effective request lifecycle separation
      - implement revision/generation identities and stale rejection
      - implement fail-closed mute/privacy latch persistence abstraction
      - implement explicit binding/no-fallback common semantics
      - implement bounded queue and finite/cancelable recovery primitives
      - implement deterministic fake adapters/fault injection/payload watermarks
      - implement conformance harness and machine-readable evidence output
      - add Linux canonical conformance CI plus Windows/macOS common build-test CI
    forbidden_changes:
      - I30-02 or later semantic/platform/product slices
      - OS media APIs or virtual device drivers/extensions
      - UI/public WebSocket implementation
      - installer/lifecycle/diagnostic implementation
      - new product semantics or blocking evidence

  tests:
    required:
      - id: configure
        command_or_oracle: cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DAXTUAL_BUILD_TESTS=ON
        expected_result: configure succeeds without unauthorized dependencies
        blocking_reason: repository must have a reproducible common build
      - id: build
        command_or_oracle: cmake --build build --config Debug
        expected_result: common runtime, unit tests, and conformance harness build successfully
        blocking_reason: implementation must be executable before evidence generation
      - id: deterministic-suite
        command_or_oracle: ctest --test-dir build -C Debug --output-on-failure
        expected_result: all package-required deterministic cases pass
        blocking_reason: closes the I30-01 common/base EA-B01 semantic proof gap
      - id: evidence-generation
        command_or_oracle: cmake --build build --config Debug --target i30_01_evidence
        expected_result: build/evidence/ea-b01-i30-01.json is generated and reports PASS for the exact tested revision
        blocking_reason: reviewer-accessible machine-readable EvidenceInput is frozen by P20/P31

  hosted_verification:
    required:
      - linux canonical deterministic conformance job at exact result revision with uploaded ea-b01-i30-01.json
      - Windows common build + deterministic test job at exact result revision
      - macOS common build + deterministic test job at exact result revision
    optional:
      - Linux ASAN/UBSAN conformance job

  evidence:
    blocking:
      - EA-B01 I30-01 common/base contribution via exact hosted evidence artifact/ref
    corroborative:
      - optional sanitizer observations

  terminal_success:
    all_of:
      - repository changes remain within I30-01 authorized scope
      - all required local tests/oracles pass
      - required Linux/Windows/macOS hosted jobs pass on the exact result revision
      - canonical machine-readable evidence artifact is reviewer-accessible
      - exact result revision is pushed to a reviewer-accessible remote branch or PR

  terminal_blockers:
    explicit_classes:
      - AUTHORITY_CONFLICT
      - MISSING_REQUIRED_INPUT
      - ENVIRONMENT_BLOCKER
      - FROZEN_VERIFICATION_FAILURE
      - NEW_HIGH_IMPACT_FAILURE_MODE
      - BLOCKED_REPOSITORY_IDENTITY
      - BLOCKED_EVIDENCE
      - TASK_PACKAGE_DEFECT
      - VERIFICATION_DESIGN_DEFECT

  return_policy:
    continue_until_terminal_state: true
```

## 10. P32 Materialization / Review Boundary

P32 must work on a branch descending from the exact P31 task anchor. Recommended execution branch: `codex/i30-01-common-safety-state`.

Before mutation P32 must perform:

1. repository identity preflight;
2. exact package/materialization resolution;
3. task-anchor ancestry check;
4. PackageBindingPreflight;
5. EvidenceContractPreflight.

Before returning to CONTROL_REVIEW, P32 must provide exact navigation identities only:

- `result_revision`;
- reviewer-accessible `materialized_ref` (remote branch or PR exact head/ref);
- exact `evidence_input_refs` for the uploaded `ea-b01-i30-01.json`;
- exact provider run/attempt/job/artifact refs required by this package.

P32 must not claim P34 PASS.

## 11. Terminal Boundary

P31 authorization ends at I30-01. Completion of this package does not authorize I30-02. Any missing requirement discovered during execution must be classified to its owning layer rather than silently expanding this package.

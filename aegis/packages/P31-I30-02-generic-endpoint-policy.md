# AXTUAL P31 Task Package — I30-02 Generic Endpoint + Product Policy + Nearity Enhancement Semantics v0.1

Status: APPROVED_FOR_P32
Stage owner: aegis-implementation
Stage: P31 Task Packaging
Package ID: AXTUAL-I30-02-P31-v0.1
Process profile: Standard

## 1. Purpose

Implement the second P30 vertical slice on top of the Gate-closed and canonically integrated I30-01 common runtime: the common, deterministic semantics for Generic Endpoint truth, Product Policy projection/validation, canonical endpoint binding, identity/reconnect reconciliation, and optional Nearity Enhancement association.

This package establishes the three-truth media model before any real Windows/macOS endpoint adapter, Public WebSocket boundary, Nearity device-control transport, or virtual endpoint depends on it.

The governing product invariant remains:

> Generic media core. Nearity enhanced. Hardware first. Virtual devices stay optional until they prove user value.

I30-02 closes only the deterministic/common semantic contribution assigned to this slice. It does not claim real-OS Generic Endpoint qualification or production client-boundary proof.

## 2. Trusted Basis

Current Authority:

- P02 V1 Product Requirements v0.3 — PASS / FROZEN / Current
  - https://app.notion.com/p/3d64c57a590c8129af38e0c46d9bd5a9?pvs=204
- P03 V1 Capability Traceability v0.3 — PASS / FROZEN / Current / DOWNSTREAM_RECONCILED
  - https://app.notion.com/p/3d64c57a590c8192bbecf5649897f030?pvs=204
- Device Qualification Contract v0.2 — Current for Nearity Enhanced per-model facts only
  - https://app.notion.com/p/3d64c57a590c81889536d4fc3ef37340?pvs=204
- P14 System Architecture v0.4 — PASS / FROZEN / RECONCILED
  - https://app.notion.com/p/3d64c57a590c81679e3cf339c057834b?pvs=204
  - incorporates Generic Endpoint / Product Policy / Nearity Enhancement architecture from P14 v0.3 unchanged
- P15 Module Design v0.4 — PASS / FROZEN / RECONCILED
  - https://app.notion.com/p/3d64c57a590c8104afc3dce3965766a6?pvs=204
  - incorporates END-01..04, POL-01..03, NEA-01..06 and CORE-14 ownership from P15 v0.3 unchanged
- P16 Runtime Data Flow v0.4 — PASS / FROZEN / RECONCILED
  - https://app.notion.com/p/3d64c57a590c81f6be52ee00d38ebef0?pvs=204
  - incorporates Generic Endpoint / policy / enhancement flows from P16 v0.3 unchanged
- P17 Platform Contract v0.5 — PASS / FROZEN / RECONCILED / Current
  - https://app.notion.com/p/3d74c57a590c812ca1b8cfad24553785?pvs=204
- P18 Engineering & Optimization v0.5 — PASS / FROZEN / RECONCILED / Current
  - https://app.notion.com/p/3d74c57a590c8154915af268b531e174?pvs=204
- P20 Verification Design v0.3 — PASS / FROZEN / RECONCILED / Current
  - https://app.notion.com/p/3d74c57a590c819f850bdfa6a6d469bb?pvs=204
  - incorporates P20 v0.2 common semantic obligations unchanged except native realization refinements that do not affect I30-02
- P30 Implementation Plan v0.3 — PASS / FROZEN / PLATFORM-REALIZATION RECONCILED / Current
  - https://app.notion.com/p/3d74c57a590c81da8a06e8ac0c4b2443?pvs=204
  - incorporates the I30-02 slice definition and dependency ordering from P30 v0.2 unchanged
- I30-01 P24 Canonical Integration Closure v0.1 — PASS / CANONICAL_INTEGRATION_CLOSED
  - https://app.notion.com/p/3d74c57a590c81e68260fa34434ecb0c?pvs=204

VerificationSpec: `P20-v0.3`
Primary evidence family for this package: `EA-B01 Runtime Safety / Semantic Conformance` — I30-02 semantic contribution only.

Frozen P20 obligations contributed by I30-02 through EA-B01:

- `VO-END-02` — Capability / Product Policy / Nearity Enhancement remain separate truths.
- `VO-END-03` — `blocked_by_policy` is enforced in Core for every caller path that reaches the common validator.
- `VO-END-04` — Nearity enhancement failure does not tear down healthy generic media unless policy explicitly requires it.
- `VO-END-06` — inventory/probing has no persistent-capture side effect and Nearity virtual endpoints cannot recursively select themselves as upstream sources.

This package also preserves inherited EA-B01 `O-IDENTITY`, `O-TRUTH/O-STATE`, `O-NOFALLBACK-*`, `O-IDLE`, `O-GEN`, and boundedness regressions from I30-01 where they intersect the new semantics.

Not claimed by this package:

- `EA-B11` Public Control & Observation Boundary — belongs to I30-03.
- `EA-B12` Generic Endpoint Platform Qualification — belongs to I30-05W/M and real OS adapters.
- full real-platform closure of `VO-END-01/04/05/06` where P20 requires EA-B12.

## 3. Repository Binding and Task Anchor

```yaml
repository:
  provider: github
  full_name: Mostorm-Labs/axtual
  repository_id: "1359894171"
  default_branch: main
routing_baseline:
  ref: refs/heads/main
  revision: 6e1340664c1d3769385a0b065e4e4103cade4364
  state: I30-01_P24_CANONICAL_INTEGRATION_CLOSED
task_anchor:
  repository: Mostorm-Labs/axtual
  revision: 6e1340664c1d3769385a0b065e4e4103cade4364
  relation: ancestor
  meaning: trusted canonical product-code baseline before I30-02 package materialization
package_materialization_branch: refs/heads/aegis/p31-i30-02-generic-endpoint-policy
```

The immutable commit containing this file is the reviewer-accessible package materialization ref. Its exact SHA is bound by the P31 surface handoff after materialization.

P32 must resolve the same repository and exact package materialization ref before checking task-anchor ancestry. An accepted execution starting revision must descend from the task anchor and include this package materialization commit.

## 4. Current Repository Reality

At the frozen routing baseline, repository implementation reality is intentionally small:

- `include/axtual/common.hpp` contains I30-01 common state/safety/generation/boundedness primitives.
- `tests/conformance.cpp` contains the I30-01 deterministic conformance harness and EvidenceInput generator.
- `CMakeLists.txt` provides C++20/CTest and the I30-01 evidence target.
- `.github/workflows/conformance.yml` runs Linux canonical conformance plus Windows/macOS common build/test.

I30-01 is trusted downstream input, not a license to reinterpret its helper types as future production architecture. In particular, the existing `DemandCoordinator` is a minimal I30-01 harness/common primitive and is **not** production Authority for multi-client lease admission, Observation ownership, or final media-demand semantics.

## 5. Authorized Technical Baseline

- Language remains C++20.
- Build remains CMake 3.24+ with CTest.
- Common semantic code remains standard-library-only unless a dependency is strictly necessary to satisfy this frozen package. Do not add a general dependency framework.
- Keep deterministic common logic free of hidden worker threads and suitable for explicit/manual polling or direct calls.
- Public common headers must not include Windows/macOS media SDKs, WebSocket libraries, installer frameworks, HID/vendor transports, or driver/extension APIs.
- Preserve all passing I30-01 common behavior and conformance cases.
- Prefer independent common-domain headers instead of turning `common.hpp` into the owner of every later semantic domain.

Expected file/module scope:

```plain text
include/axtual/common.hpp                       # only narrowly shared primitive changes if required
include/axtual/endpoint.hpp                     # Generic Endpoint records/registry/reconnect semantics
include/axtual/policy.hpp                       # Product Policy projection + common selection validation
include/axtual/enhancement.hpp                  # optional Nearity association truth only
include/axtual/binding.hpp                      # canonical generic desired binding semantics
include/axtual/snapshot.hpp                     # optional if needed to preserve separated snapshot truth
tests/i30_02_conformance.cpp                    # deterministic I30-02 cases + EvidenceInput generation
CMakeLists.txt                                  # add I30-02 tests/evidence without removing I30-01 regression
.github/workflows/conformance.yml               # run/upload I30-02 evidence and common cross-platform tests
README.md                                       # concise build/evidence update if needed
```

The executor may make a smaller file split if repository style makes one header unnecessary, but ownership boundaries and package scope must remain equivalent. A file-layout preference never authorizes additional product behavior.

## 6. Scope Contract — Required Changes

### 6.1 Generic Endpoint observed truth

1. Implement a vendor-neutral endpoint kind domain for exactly the currently frozen generic kinds:
   - `audio_input`;
   - `audio_output`;
   - `video_input`.
2. Implement canonical endpoint records/registry semantics that can represent at least:
   - canonical endpoint ref;
   - kind;
   - display identity;
   - opaque platform identity token or deterministic fake equivalent;
   - connection generation;
   - availability;
   - identity quality/certainty;
   - generic capability/openability state;
   - optional origin/ownership metadata needed for self-loop detection.
3. Endpoint existence/availability/capability must remain observed generic truth. Lack of Nearity association must never delete the endpoint or mark generic capability unsupported.
4. Preserve the distinction `enumerated != openable`. Unknown openability remains unknown rather than being fabricated as supported/unsupported.

### 6.2 Identity and reconnect reconciliation

5. Implement deterministic common reconciliation outcomes:
   - `same`;
   - `replaced`;
   - `ambiguous`;
   - `gone`.
6. A stable identity match may restore the same canonical binding without creating a new user desired revision.
7. Ambiguous identity must remain unresolved and must not auto-bind by display name, system-default status, list order, or first matching device.
8. A `gone`/loss result changes observed/effective applicability but does not silently rewrite the desired binding to another endpoint.
9. Nearity-qualified identity hints may improve certainty in a deterministic fixture, but generic correctness cannot require those hints.

### 6.3 Product Policy truth and enforcement

10. Implement versioned/common Product Policy projection semantics per endpoint using the frozen dispositions:
    - `eligible`;
    - `recommended`;
    - `hidden_by_default`;
    - `blocked_by_policy`;
    with an optional reason code and a policy revision/generation sufficient for deterministic recomputation.
11. Policy projection is derived truth and must not mutate endpoint availability or generic capability fields.
12. `recommended` must not create desired selection by itself.
13. `hidden_by_default` changes presentation eligibility only; it remains selectable when generic capability requirements are otherwise satisfied.
14. `blocked_by_policy` must be rejected by the common selection validator before desired binding commit.
15. Rejected selection must not change the selected endpoint ref or increment desired binding revision.
16. Missing endpoint or insufficient/negative generic capability must fail selection truthfully before desired commit.
17. All deterministic caller identities used by the harness must reach the same common policy validator. I30-02 must not implement UI-only policy enforcement.

### 6.4 Canonical generic binding semantics

18. Implement desired bindings for:
    - selected audio input;
    - selected video input;
    - selected health-check audio output.
19. Binding is by canonical generic endpoint ref, never by Nearity-only device reference.
20. Successful explicit selection advances desired revision monotonically and records only explicit user/client intent.
21. System default changes, policy recommendation, endpoint arrival, or Nearity enhancement gain must not silently create a desired binding revision.
22. Open/busy/permission/format/effective failure after an already-valid desired commit must not silently select another endpoint. The I30-02 common model may represent effective/applicability failure without implementing real media open.
23. When a policy revision newly blocks an already-selected endpoint, the observable result must be:
    - no alternate endpoint selection;
    - no fabricated generic capability/availability failure;
    - active applicability becomes blocked/non-effective with explicit policy reason;
    - desired intent is not silently rewritten to a different endpoint.
   Internal representation of retained blocked intent may remain an implementation detail as allowed by P16; do not invent a new wire/storage schema.

### 6.5 Nearity Enhancement association truth

24. Implement only the common association semantics needed by I30-02:
    - `none`;
    - `candidate`;
    - `associated`;
    - `ambiguous`;
    - `lost`;
    plus an enhancement generation/revision when needed to reject stale association state.
25. Enhancement association is optional state attached to a generic endpoint; it is not generic endpoint identity or eligibility Authority.
26. Enhancement gain must not change generic desired binding or create a media source switch.
27. Enhancement loss must invalidate/clear enhanced actual fields or mark them unavailable/unknown; stale enhanced actual values must not remain authoritative.
28. When policy allows generic use, enhancement loss must leave generic endpoint/binding applicability intact.
29. If a deterministic SKU/product policy explicitly requires enhancement, association loss may block/stop applicability, but the result must be attributable to policy rather than fabricated as generic media failure, and it must not trigger fallback.
30. Do not implement real Nearity DSP, HID, firmware, preset, qualification-profile transport, or device-control operations in I30-02.

### 6.6 Self-loop and no-demand side effects

31. Represent enough endpoint-origin metadata to reject a Nearity-owned Virtual Mic as its own `audio_input` upstream and a Nearity-owned Virtual Camera as its own `video_input` upstream.
32. Reject self-loop deterministically before any capture/open seam is invoked.
33. Do not reject an unrelated third-party virtual endpoint solely because it is virtual; ordinary capability and Product Policy rules still apply.
34. Registry update, capability projection, policy projection, identity reconciliation, and enhancement association by themselves must not acquire persistent capture/media demand.
35. Do not extend the I30-01 `DemandCoordinator` into production multi-client/Observation lease semantics in this package. If it is used as test instrumentation, its role must remain explicitly test/common-scoped and no new production Authority may be inferred from it.

### 6.7 Deterministic conformance and evidence

36. Add an independently named I30-02 deterministic conformance suite exercising the frozen cases below.
37. Preserve and rerun every existing I30-01 CTest regression on every required hosted OS.
38. Add a machine-readable I30-02 EvidenceInput generator at `build/evidence/ea-b01-i30-02.json`.
39. Linux hosted CI must generate and upload that I30-02 EvidenceInput for the exact result revision.
40. Windows and macOS hosted CI must compile and run I30-01 + I30-02 common tests at the exact same result revision.
41. Do not create a second proof artifact merely to prove the EvidenceInput generator. Existing test execution + hosted job identity + artifact binding are the independent mechanisms for this Standard-profile slice.

## 7. Forbidden Changes / Non-goals

- No I30-03 Public WebSocket/bootstrap/session/auth/subscription/Observation implementation.
- No claim that deterministic fake callers close `EA-B11` production client-boundary verification.
- No I30-04 Nearity DSP/HID/firmware/preset/device-control transport or per-model implementation.
- No I30-05W/M real Generic Endpoint realization: no MMDevice, WASAPI, Media Foundation, Core Audio, AVFoundation/Core Media endpoint enumeration/open/render/capture.
- No `EA-B12` real-platform Generic Endpoint qualification claim.
- No Virtual Mic/Virtual Camera driver, Audio Server Driver Plug-in, Camera Extension, `IMFVirtualCamera`, Frame Server source, WDM/PortCls/WaveRT code.
- No UI application or UI-specific canonical inventory/policy state.
- No installer/update/repair/release-set/config-migration/diagnostic/support-bundle implementation.
- No PC-side NR/AGC/Limiter/EQ processing.
- No automatic system-default or alternate-endpoint fallback.
- No Virtual Speaker implementation.
- No new Product Policy dispositions or product requirements beyond Current Authority.
- No new blocking evidence family or proof-of-proof requirement.
- No rewrite of P20/P30 Authority to fit implementation convenience.
- No use of I30-01 `DemandCoordinator` as the final production admission/lease contract for future I30-03 or media slices.

## 8. Frozen Acceptance Oracles and Evidence Ownership

### 8.1 `O-POLICY-TRUTH`

Independently vary generic capability, Product Policy and Nearity association for one endpoint. Snapshot/common state must preserve the truth channels independently. Policy block/recommendation/hiding must not rewrite OS/generic capability or availability. Enhancement `none/lost` must not be fabricated as endpoint absence.

### 8.2 `O-POLICY-ENFORCE`

A `blocked_by_policy` selection is rejected through every deterministic caller path that reaches the common Core-style validator, with desired binding/ref/revision unchanged. `hidden_by_default` remains selectable when generic capability permits. I30-02 proves the common semantic enforcement point only; later I30-03/EA-B11 proves real authorized client paths reach it.

### 8.3 `O-ENH-INDEP`

Start from a generic endpoint/binding state that is healthy/applicable under policy, then remove/fail enhancement association. Generic truth and desired binding remain valid when policy allows; enhanced actual state becomes unavailable/unknown rather than stale. If policy requires enhancement, the blocked/non-effective result is explicitly policy-caused and no fallback binding is created.

### 8.4 `O-SELFLOOP`

Selecting a Nearity-owned Virtual Mic/Camera as its corresponding own upstream source is rejected before capture/open. An unrelated third-party virtual endpoint is not rejected solely for being virtual.

### 8.5 inherited `O-IDENTITY`

Ambiguous reconnect never auto-binds. I30-02 extends the deterministic endpoint-domain fixture to all four common outcomes `same/replaced/ambiguous/gone` without same-name/default/order heuristics.

### 8.6 inherited `O-IDLE` / no-demand semantic portion

Inventory, policy projection and enhancement association alone create no persistent capture ownership/demand. Real platform probe/capture behavior remains for I30-05W/M and EA-B12.

### 8.7 inherited I30-01 regression set

All I30-01 Gate-closed common cases continue to pass. No I30-02 implementation may weaken generation, safety latch, no-fallback, truth separation, boundedness or failure-domain behavior.

## 9. Required Deterministic Test Cases

At minimum the suite must contain independently named cases traceable to the package oracles:

1. non-Nearity endpoint remains in generic inventory with enhancement `none`;
2. snapshot keeps `enumerated/openable/policy/selected/effective/enhanced` as distinct fields/states;
3. policy block does not rewrite generic availability/capability truth;
4. `blocked_by_policy` selection is rejected without desired-ref or desired-revision change;
5. `hidden_by_default` endpoint remains explicitly selectable when generic capability permits;
6. `recommended` projection does not auto-select or advance desired revision;
7. missing endpoint selection is rejected without desired commit;
8. unsupported or unknown required generic capability is not fabricated as selectable;
9. successful explicit selection stores canonical generic endpoint ref and advances desired revision once;
10. system-default/display-order metadata change does not mutate desired binding;
11. `same` reconnect can reconcile current canonical binding without new desired revision;
12. `replaced` outcome is represented explicitly and does not masquerade as `same`;
13. ambiguous reconnect with the same display name does not auto-bind;
14. `gone` endpoint leaves desired intent unresolved/non-effective and does not fallback;
15. enhancement gain does not change generic binding or desired revision;
16. enhancement loss under generic-allowed policy clears/invalidates enhanced actual truth while preserving generic applicability;
17. enhancement loss under enhancement-required policy becomes explicitly policy-blocked/non-effective without fabricating generic failure or fallback;
18. policy revision that newly blocks the selected endpoint stops applicability without selecting an alternative endpoint;
19. presentation-only change to `hidden_by_default` does not invalidate an otherwise allowed active selection;
20. Nearity Virtual Mic self-loop selection is rejected before open/capture;
21. Nearity Virtual Camera self-loop selection is rejected before open/capture;
22. unrelated third-party virtual endpoint is not rejected solely for being virtual;
23. endpoint inventory/policy/enhancement operations alone do not acquire capture/media demand;
24. two deterministic caller identities reach the same common policy validator and both reject the same blocked selection.

The executor may add narrowly necessary cases to cover an already-frozen invariant, but must not add new blocking requirements or later-slice behavior. Test names may differ only if one-to-one traceability remains obvious in the EvidenceInput.

## 10. Evidence Compilation Contract

The canonical I30-02 conformance run must emit:

`build/evidence/ea-b01-i30-02.json`

with at least:

```yaml
verification_spec: P20-v0.3
package_id: AXTUAL-I30-02-P31-v0.1
slice: I30-02
evidence_family: EA-B01
claim_scope: common_semantic_contribution_only
obligations:
  - VO-END-02
  - VO-END-03
  - VO-END-04
  - VO-END-06
authority_refs:
  - P02-v0.3
  - P03-v0.3
  - P14-v0.4
  - P15-v0.4
  - P16-v0.4
  - P17-v0.5
  - P18-v0.5
  - P20-v0.3
  - P30-v0.3
task_anchor: 6e1340664c1d3769385a0b065e4e4103cade4364
build_identity: <exact git revision supplied at execution time>
platform_environment: <host identity>
execution_method: I30-02-deterministic-semantic-conformance
oracle:
  - O-POLICY-TRUTH
  - O-POLICY-ENFORCE
  - O-ENH-INDEP
  - O-SELFLOOP
  - O-IDENTITY
  - O-IDLE
case_results: <machine-readable named cases>
result: PASS | FAIL | BLOCKED_ENVIRONMENT
```

The generator must execute the cases it reports and return nonzero on any failed required case. It must not hard-code a future result revision or copy a predeclared PASS value without execution.

The evidence JSON is an EvidenceInput for later P34 review. The executor return should provide navigation identities to the exact hosted artifact/run/ref rather than retyping machine-owned totals as prose proof.

## 11. EXECUTION_CLOSURE_CONTRACT

```yaml
EXECUTION_CLOSURE_CONTRACT:
  implementation:
    required_changes:
      - implement vendor-neutral Generic Endpoint record/registry semantics for audio_input, audio_output, video_input
      - implement explicit generic capability/openability truth separate from inventory/availability
      - implement same/replaced/ambiguous/gone identity-reconnect reconciliation without name/default/order heuristics
      - implement Product Policy projection eligible/recommended/hidden_by_default/blocked_by_policy plus reason/revision
      - implement common selection validation before desired commit
      - implement canonical generic desired bindings for audio input, video input, health-check audio output
      - implement optional Nearity enhancement association truth and gain/loss independence semantics
      - implement policy-required-enhancement blocking attribution without generic-truth corruption or fallback
      - implement Nearity-owned virtual endpoint self-loop rejection before any open/capture seam
      - implement separated aggregate snapshot/common state truth
      - add deterministic I30-02 conformance cases and ea-b01-i30-02.json generation
      - preserve and rerun all I30-01 common regressions
      - update hosted Linux/Windows/macOS common verification for the exact result revision
    forbidden_changes:
      - I30-03 Public WebSocket or Observation implementation
      - I30-04 Nearity physical device-control/DSP/HID/firmware/preset implementation
      - I30-05W/M real OS generic endpoint adapters or EA-B12 claims
      - native Virtual Mic/Virtual Camera implementation
      - UI, installer, lifecycle, diagnostic, config-migration or product DSP implementation
      - automatic system-default/alternate-endpoint fallback
      - production lease/admission semantics based on the I30-01 DemandCoordinator
      - new product requirements, policy dispositions, blocking evidence families or proof-recursion artifacts

  tests:
    required:
      - id: configure
        command_or_oracle: cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DAXTUAL_BUILD_TESTS=ON
        expected_result: configure succeeds without unauthorized dependencies
        blocking_reason: common semantic slice must remain reproducible and cross-platform
      - id: build
        command_or_oracle: cmake --build build --config Debug
        expected_result: I30-01 regression harness and I30-02 semantic harness build successfully
        blocking_reason: all frozen deterministic evidence must be executable
      - id: deterministic-suite
        command_or_oracle: ctest --test-dir build -C Debug --output-on-failure
        expected_result: all existing I30-01 regressions plus all package-required I30-02 cases pass
        blocking_reason: closes the EA-B01 I30-02 semantic contribution without weakening I30-01
      - id: i30-02-evidence-generation
        command_or_oracle: cmake --build build --config Debug --target i30_02_evidence
        expected_result: build/evidence/ea-b01-i30-02.json is generated by executing required cases and reports PASS for the exact tested revision
        blocking_reason: P20/P31 freeze reviewer-accessible machine-readable EA-B01 semantic EvidenceInput

  hosted_verification:
    required:
      - Linux common conformance job at exact result revision running I30-01 + I30-02 tests and uploading ea-b01-i30-02.json
      - Windows common build + I30-01/I30-02 deterministic tests at exact result revision
      - macOS common build + I30-01/I30-02 deterministic tests at exact result revision
    optional:
      - sanitizer run if already easy to execute without changing frozen scope

  evidence:
    blocking:
      - EA-B01 I30-02 common semantic contribution for VO-END-02/03/04/06 via exact hosted ea-b01-i30-02.json artifact/ref
    corroborative:
      - exact CTest logs from hosted Windows/macOS runs
      - optional sanitizer observations
    explicitly_deferred:
      - EA-B11 production client-boundary proof to I30-03
      - EA-B12 real Windows/macOS Generic Endpoint qualification to I30-05W/M

  terminal_success:
    all_of:
      - repository changes remain within I30-02 authorized common-semantic scope
      - PackageBindingPreflight and EvidenceContractPreflight resolve exact Current Authority/package identities
      - all required local configure/build/CTest/evidence-generation steps pass
      - all I30-01 Gate-closed regression cases remain passing
      - required Linux/Windows/macOS hosted jobs pass on one exact result revision
      - exact Linux ea-b01-i30-02.json artifact is reviewer-accessible and bound to that result revision
      - exact result revision is pushed to a reviewer-accessible remote branch or PR
      - no later-slice implementation or unsupported evidence claim is included

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
      - BLOCKED_EXECUTION_DIVERGENCE

  return_policy:
    continue_until_terminal_state: true
```

## 12. P32 Materialization / Review Boundary

Recommended execution branch:

`codex/i30-02-generic-endpoint-policy`

P32 must begin with:

1. resolve `repository.provider/full_name` and repository ID;
2. resolve the same-repository exact package materialization ref;
3. verify accepted starting revision descends from `task_anchor = 6e1340664c1d3769385a0b065e4e4103cade4364` and contains this package commit;
4. run `PackageBindingPreflight` against Current P20 v0.3 / P30 v0.3 and the exact package;
5. run `EvidenceContractPreflight` for EA-B01 I30-02 semantic contribution;
6. only then mutate code.

Before returning to CONTROL_REVIEW, P32 must provide exact navigation identities required by this package:

- `result_revision`;
- reviewer-accessible `materialized_ref` at that exact revision;
- exact Linux `ea-b01-i30-02.json` evidence artifact/ref;
- exact hosted workflow run/attempt/job refs for Linux, Windows and macOS.

P32 must not emit or imply official P34 PASS.

## 13. Terminal Boundary

P31 authorization ends at I30-02 common semantics.

Completion of I30-02 may unlock successor routing for I30-03 Public Control, I30-04 Nearity Enhanced Companion, and I30-05W/M real Generic Endpoint platform trains according to P30. It does not itself authorize those Primary/substantive stages.

Any discovered missing Authority, Verification obligation, or package requirement must be classified to its owning layer instead of silently expanding this frozen package. A genuine new high-impact uncovered failure mode returns for classification; additional confidence alone is not a reason to move the finish line.

---
title: '1.5 Verify Canonical Field Behavior'
type: 'feature'
created: '2026-08-27'
status: 'done'
baseline_revision: 'a3d3c2215f4394215eb1b82caaf4251e66b01267'
review_loop_iteration: 0
followup_review_recommended: false
context:
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/1-5-verify-canonical-field-behavior/_bmad-output/project-context.md'
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/1-5-verify-canonical-field-behavior/_bmad-output/implementation-artifacts/epic-1-context.md'
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/1-5-verify-canonical-field-behavior/_bmad-output/game-architecture.md'
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/1-5-verify-canonical-field-behavior/_bmad-output/planning-artifacts/story-specs-life-game-2026-08-25.md'
warnings: []
deferred: []
---

<intent-contract>

## Intent

**Problem:** The Field MVP already has focused Conway, boundary, scheduler, and input tests, but it lacks one deterministic application trace proving that the same controlled clock and input sequence produces the same byte state on every run.

**Approach:** Preserve the existing finite Conway implementation and add focused, window-free verification around its canonical fixtures and `RaylibApplication::processIteration` seam. Reuse explicit elapsed durations and logical pointer samples to compare post-generation and post-input buffers byte-for-byte.

## Boundaries & Constraints

**Always:** Use standard finite Conway rules, row-major dense byte buffers, the existing 250 ms scheduler, the existing simulation-before-input phase order, and deterministic fixtures. Keep all verification window-free and independent of wall-clock timing.

**Block If:** The acceptance evidence requires changing Life rules, field ownership, scheduler semantics, or an unapproved test/runtime architecture.

**Never:** Add alternate rules, wrapping, field expansion, persistence, randomness, real-time sleeps, worker/event infrastructure, a new input queue, or any change to `sprint-status.yaml`.

## I/O & Edge-Case Matrix

| Scenario | Input / State | Expected Output / Behavior | Error Handling |
|----------|---------------|---------------------------|----------------|
| CANONICAL_FIXTURES | 3×3 lone cell at (1,1), 4×4 2×2 block, 5×5 vertical blinker, and 3×3 edge fixtures | Exact expected byte buffers; edge dimensions and storage remain unchanged | No error expected |
| CONTROLLED_TRACE | Two equal 7×7 fields, durations 249 ms, 1 ms, 499 ms, 1 ms, and explicit press/hold/release samples | Each expected generation and post-input buffer matches independently, and both application runs match byte-for-byte | No error expected |

</intent-contract>

## Code Map

- `src/domain/simulation/conway-simulation.cpp:12-74` -- bounded neighbor counting and complete next-buffer publication; treat this as the unchanged behavior under test.
- `tests/unit/domain/conway-simulation-test.cpp:22-216` -- existing exact lone-cell, block, blinker, edge, and buffer-isolation fixtures, including the added side-edge regression.
- `src/application/simulation/simulation-scheduler.cpp:9-47` -- deterministic 250 ms accumulation and capped sequential generations; accepts supplied durations rather than sampling time.
- `src/presentation/application/raylib-application.cpp:38-67` -- `processIteration` runs the batch before applying supplied `FrameInput`; this is the outer window-free trace surface.
- `tests/unit/presentation/raylib-application-test.cpp:57-159` -- existing pre-input ordering and outside-input evidence plus the identical-trace oracle using `FieldRenderer::calculateRenderPlan` for logical cell points.
- `tests/CMakeLists.txt:49-85` -- existing application and presentation Catch2 targets; keep coverage in the registered window-free suite.

## Tasks & Acceptance

**Execution:**
- `tests/unit/domain/conway-simulation-test.cpp` -- strengthen finite-edge evidence with exact left/right side-edge buffers and dimension/storage assertions while preserving the existing exact lone-cell, block, and blinker fixtures -- keep the mathematical evidence byte-for-byte and finite.
- `tests/unit/presentation/raylib-application-test.cpp` -- add two equal applications running the same controlled elapsed-duration and pointer-input trace, comparing buffers after every iteration -- verify deterministic generation and post-input state without opening a window.

**Acceptance Criteria:**
- Given lone-cell, stable-block, blinker, and finite-edge fixtures, when the deterministic tests run, then every expected outcome passes with exact field bytes and no unintended dimension or storage change.
- Given the same initial field and controlled clock/input trace, when two application runs process the trace, then their generation and post-input field states match byte-for-byte after each iteration.

## Spec Change Log

## Review Triage Log

### 2026-08-27 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 5: (high 0, medium 1, low 4)
- defer: 0
- reject: 14: (high 0, medium 0, low 14)
- addressed_findings:
  - `[medium]` `[patch]` Added independent expected buffers after every controlled application-trace step so two identically-wrong runs cannot satisfy the replay test.
  - `[low]` `[patch]` Added dimension, height, and storage-size invariants to the new left/right side-edge regression.
  - `[low]` `[patch]` Updated the Code Map line anchors for the expanded domain and presentation tests.
  - `[low]` `[patch]` Clarified that the domain task strengthens the new side-edge evidence while retaining the existing exact canonical fixture tests.
  - `[low]` `[patch]` Made the I/O matrix concrete with fixture dimensions, coordinates, durations, and pointer phases.

### 2026-08-27 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 5: (high 0, medium 1, low 4)
- defer: 0
- reject: 18: (high 0, medium 0, low 18)
- addressed_findings:
  - `[medium]` `[patch]` Added an application-level new press after the release sample so stale gesture capture cannot pass the controlled-trace verification.
  - `[low]` `[patch]` Added a bounds assertion to the logical-pointer fixture helper.
  - `[low]` `[patch]` Added bounds assertions to the expected-state fixture helper.
  - `[low]` `[patch]` Updated the presentation Code Map anchor after extending the trace.
  - `[low]` `[patch]` Recorded the complete Debug and Release verification outcomes in Auto Run Result.

### 2026-08-27 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 1: (high 0, medium 0, low 1)
- defer: 0
- reject: 21: (high 0, medium 0, low 21)
- addressed_findings:
  - `[low]` `[patch]` Restored the missing `## Auto Run Result` section with this pass's review accounting and verification evidence.

## Design Notes

The production seams already make the required evidence deterministic: the scheduler receives elapsed duration as an argument, and `processIteration` accepts a complete `FrameInput` before any render. The trace should use the existing logical coordinate conversion rather than reaching into private state or testing raylib polling.

## Verification

**Commands:**
- `cmake --preset dev-debug` -- expected: configure succeeds.
- `cmake --build --preset dev-debug` -- expected: all tests compile.
- `ctest --preset dev-debug --output-on-failure` -- expected: the complete window-free suite passes.
- `cmake --preset dev-release` -- expected: Release configure succeeds.
- `cmake --build --preset dev-release` -- expected: Release tests compile.
- `ctest --preset dev-release --output-on-failure` -- expected: deterministic Release suite passes.
- `git diff --check` -- expected: no whitespace errors.

## Auto Run Result

Status: done

Summary: Hardened canonical finite-edge Conway evidence and verified the deterministic, window-free two-application trace with controlled durations and press/hold/release input. This fresh review pass found no implementation defect and restored the required run-result record.

Files changed:
- `tests/unit/domain/conway-simulation-test.cpp` -- verifies exact left- and right-edge generation buffers while preserving dimensions and storage size.
- `tests/unit/presentation/raylib-application-test.cpp` -- verifies the controlled trace byte-for-byte, guards fixture coordinates, and verifies post-release input starts a fresh gesture.
- `_bmad-output/implementation-artifacts/spec-1-5-verify-canonical-field-behavior.md` -- records review triage, verification evidence, and final status.
- `_bmad-output/implementation-artifacts/sprint-status.yaml` -- contains an orchestrator-owned bookkeeping update observed in the review diff; this run did not modify or revert it.

Review findings breakdown: 1 patch applied (0 high, 0 medium, 1 low), 0 items deferred, 21 items rejected.

Follow-up review recommendation: false. Patched finding counts: high 0, medium 0, low 1; score = 3 × 0 + 1 × 1 = 1.

Verification performed:
- `cmake --preset dev-debug` -- passed; configure completed with the existing raylib OpenGL deprecation developer warning.
- `cmake --build --preset dev-debug` -- passed; no work was required.
- `ctest --preset dev-debug --output-on-failure` -- passed, 64/64 tests.
- `cmake --preset dev-release` -- passed; configure completed with the existing raylib OpenGL deprecation developer warning.
- `cmake --build --preset dev-release` -- passed; no work was required.
- `ctest --preset dev-release --output-on-failure` -- passed, 64/64 tests.
- `git diff --check` -- passed with no whitespace errors.

Residual risks: The controlled trace derives logical cell-center samples through the existing render-plan conversion; independent coordinate-boundary coverage remains in the existing presentation suite. No implementation or verification failures remain.

---
title: '1.4 Respect Permanent-Dead Boundaries'
type: 'feature'
created: '2026-08-27'
status: 'done'
baseline_revision: '1a963afa6c57225509eeffbb9ff172a6c5c6b36b'
review_loop_iteration: 0
followup_review_recommended: true
context:
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/1-4-respect-permanent-dead-boundaries/_bmad-output/project-context.md'
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/1-4-respect-permanent-dead-boundaries/_bmad-output/implementation-artifacts/epic-1-context.md'
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/1-4-respect-permanent-dead-boundaries/_bmad-output/game-architecture.md'
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/1-4-respect-permanent-dead-boundaries/_bmad-output/planning-artifacts/story-specs-life-game-2026-08-25.md'
warnings: []
deferred: []
---

## Intent

**Problem:** The Field MVP must make its finite boundary observable and provable. Without explicit edge, presentation, and input evidence, a future change could accidentally introduce wrapping, hidden cells, or edits in gray space.

**Approach:** Preserve the existing bounds-safe field, finite Conway neighbor evaluation, gray out-of-field rendering, and rejected coordinate conversion. Add focused regression coverage that distinguishes finite behavior from toroidal behavior and proves outside input leaves field bytes unchanged.

## Boundaries & Constraints

**Always:** Treat every coordinate outside the fixed field as permanently dead, exclude it from neighbor counts, never expand or resize the field during evolution, render visible non-field space as `#808080`, and reject outside-field input before command execution. Preserve row-major byte storage, half-open bounds, synchronous generations, and the approved layer direction.

**Block If:** Satisfying the boundary contract requires a new simulation rule, a change to field ownership, or a dependency on a not-yet-approved camera or interaction architecture decision.

**Never:** Add toroidal wrapping, field expansion, a third persisted cell state, hidden-cell mutation, worker/event infrastructure, Die or selection implementation, camera navigation, persistence, or edits to `sprint-status.yaml`.

## I/O & Edge-Case Matrix

| Scenario | Input / State | Expected Output / Behavior | Error Handling |
|----------|---------------|-----------------------------|----------------|
| FINITE_EDGE | 3×3 field with the complete top row live | One generation leaves only `(1,0)` and births `(1,1)`; no bottom-row wrap birth occurs; dimensions and byte-buffer size stay unchanged | No error expected |
| GRAY_VIEW | 50×50 field rendered in a 1280×720 logical viewport | The finite field rectangle is surrounded by `#808080` presentation space; no outside coordinate maps to a cell | No error expected |
| GRAY_INPUT | Live press or held/released sample in gray space just outside the field | No command is emitted and every field byte remains byte-for-byte unchanged | No error expected |

## Code Map

- `src/domain/field/field.hpp:20-34` and `field.cpp:40-68` -- validating finite dimensions, half-open `contains`, and no-op out-of-bounds reads/writes; preserve these guards and their row-major storage.
- `src/domain/simulation/conway-simulation.cpp:12-53` -- bounded neighbor counting and complete next-buffer publication; add a wrap-distinguishing edge fixture without changing the synchronous algorithm.
- `src/presentation/rendering/field-renderer.cpp:10-91` -- exposes the approved dead/live/gray palette, fills the viewport gray, and draws only in-field cells; retain gray as presentation-only space.
- `src/presentation/camera/coordinate-converter.cpp:12-42` -- rejects non-finite, viewport-outside, and half-open field-rectangle-outside points; this is the single mapping gate for input.
- `src/presentation/input/input-router.cpp:18-77` and `src/application/field-command-executor.cpp:3-8` -- convert accepted samples into Live commands and delegate mutation to bounds-safe `Field::setLive`; outside samples must remain no-ops.
- `tests/unit/domain/conway-simulation-test.cpp`, `tests/unit/presentation/field-renderer-test.cpp`, `coordinate-converter-test.cpp`, `input-router-test.cpp`, and `raylib-application-test.cpp` -- extend window-free regression evidence for finite edges, gray layout/mapping, and application-level no-mutation behavior.

## Tasks & Acceptance

**Execution:**
- `tests/unit/domain/conway-simulation-test.cpp` -- add a top-edge fixture that would differ under wrapping and assert fixed dimensions/storage after evolution -- prevent toroidal or expanding-boundary regressions.
- `tests/unit/presentation/field-renderer-test.cpp` and `tests/unit/presentation/coordinate-converter-test.cpp` -- cover gray space around the finite render rectangle and points just outside its right/bottom half-open edges -- prove gray is not a cell state.
- `tests/unit/presentation/input-router-test.cpp` and `tests/unit/presentation/raylib-application-test.cpp` -- exercise gray presses and held/released input, asserting no emitted Live commands and byte-for-byte unchanged fields -- protect the outer input surface.

**Acceptance Criteria:**
- Given a finite field with live cells at an edge, when one Conway generation is evaluated, then only in-field neighbors contribute, no opposite edge participates, and the field dimensions and storage size remain unchanged.
- Given the field is rendered with visible space beyond its finite rectangle, when the Field surface draws, then that space is gray `#808080`, is not represented in the field buffer, and cannot be mapped to a logical cell.
- Given Live input lands outside the finite field, when the input router or application iteration processes the press, hold, or release, then no command edits the field and the field bytes remain unchanged.

## Design Notes

The current MVP has no camera offset and no implemented Die or selection command. The renderer's full-viewport gray fill plus the centralized coordinate converter already provide the intended boundary surface, so this story hardens those seams with tests rather than widening Epic 1 into later navigation or editing work. A captured drag retains its last valid in-field anchor while an outside sample is observed and resumes from that anchor on re-entry; outside samples themselves still emit no mutation.

## Verification

**Commands:**
- `cmake --preset dev-debug` -- expected: configure succeeds.
- `cmake --build --preset dev-debug` -- expected: all domain, application, presentation, and regression tests compile.
- `ctest --preset dev-debug --output-on-failure` -- expected: all tests pass without opening a window or reading runtime storage.
- `cmake --preset dev-release && cmake --build --preset dev-release` -- expected: Release compilation succeeds.
- `ctest --preset dev-release --output-on-failure` -- expected: the deterministic suite passes in Release.
- `git diff --check` -- expected: no whitespace errors.

## Review Triage Log

### 2026-08-27 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 4: (high 0, medium 0, low 4)
- defer: 0
- reject: 14: (high 0, medium 0, low 14)
- addressed_findings:
  - `[low]` `[patch]` Added a bottom-edge Conway fixture to complement the top-edge wrap guard.
  - `[low]` `[patch]` Added gray-space coverage above the render rectangle for coordinate conversion.
  - `[low]` `[patch]` Added gray-space coverage above the render rectangle for input routing.
  - `[low]` `[patch]` Defined captured-drag re-entry to resume from the last valid in-field anchor.

### 2026-08-27 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 1: (high 1, medium 0, low 0)
- defer: 0
- reject: 18: (high 0, medium 0, low 18)
- addressed_findings:
  - `[high]` `[patch]` Added a window-free draw-path test and isolated renderer test target to verify the opaque gray viewport fill and finite draw bounds.

### 2026-08-27 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 4: (high 2, medium 0, low 2)
- defer: 0
- reject: 18: (high 0, medium 0, low 18)
- addressed_findings:
  - `[high]` `[patch]` Strengthened the renderer draw-path fixture with a live cell and assertions for all four cell rectangles and their dead/live colors, preventing a cell-render omission from passing.
  - `[high]` `[patch]` Verified the live-cell palette through the captured renderer calls instead of checking only the gray background.
  - `[low]` `[patch]` Covered gray-space press, hold, and release samples through `RaylibApplication::processIteration`.
  - `[low]` `[patch]` Used an all-dead application fixture so an erroneous outside mapping cannot be masked by re-setting an already-live cell.

## Auto Run Result

Summary: Completed the permanent-dead-boundary follow-up review and strengthened the window-free renderer and application input regression evidence. Finite Conway edges, gray presentation space, half-open coordinate rejection, and outside-input no-mutation behavior remain covered without changing runtime boundary rules.

Files changed:
- `tests/unit/presentation/field-renderer-test.cpp` — asserts the renderer emits every finite cell with the correct dead/live palette in addition to the gray viewport fill.
- `tests/unit/presentation/raylib-application-test.cpp` — covers gray-space press, hold, and release samples with byte-for-byte no-mutation checks.
- `tests/unit/domain/conway-simulation-test.cpp` — covers top and bottom edge evolution without wrapping or field resizing.
- `tests/unit/presentation/coordinate-converter-test.cpp` — covers gray-space and half-open outside-field rejection.
- `tests/unit/presentation/input-router-test.cpp` — covers gray-space press, hold, and release no-ops.
- `tests/CMakeLists.txt` — keeps the deterministic renderer draw-path test target wired into CTest.
- `_bmad-output/implementation-artifacts/spec-1-4-respect-permanent-dead-boundaries.md` — records this review and verification result.

Review findings breakdown: 4 patches applied (high 2, medium 0, low 2); 0 items deferred; 18 items rejected. The orchestrator-owned `sprint-status.yaml` bookkeeping change was neither modified nor treated as a defect.

Follow-up review recommendation: true. Patched findings by severity: high 2, medium 0, low 2. Score: 0; the high-severity patches make the recommendation true.

Verification performed:
- `cmake --preset dev-debug` — passed.
- `cmake --build --preset dev-debug` — passed.
- `ctest --preset dev-debug --output-on-failure` — passed, 62/62 tests.
- `cmake --preset dev-release` — passed.
- `cmake --build --preset dev-release` — passed.
- `ctest --preset dev-release --output-on-failure` — passed, 62/62 tests.
- `git diff --check` — passed.
- All four mandated review layers returned results after synchronous waits; no bad-spec or intent-gap loopback was required.

Residual risks: The real OS/window rendering and input polling path remains outside the window-free suite; no window was opened during verification. The worktree retains the orchestrator-owned `sprint-status.yaml` modification by design.

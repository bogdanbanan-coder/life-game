---
title: '2.1 Paint Live by Drag'
type: 'feature'
created: '2026-08-28'
status: 'done'
baseline_revision: 'ba73c3ac5ed56eea388dc8bf5edbee270af55dd1'
review_loop_iteration: 0
followup_review_recommended: false
context:
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260828-234508-b520/worktrees/2-1-paint-live-by-drag/_bmad-output/project-context.md'
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260828-234508-b520/worktrees/2-1-paint-live-by-drag/_bmad-output/implementation-artifacts/epic-2-context.md'
warnings: []
deferred:
  - summary: >-
      Windowed macOS and Linux high-DPI visual equivalence remains unverified in this headless run.
    evidence: |-
      The production path uses logical client coordinates and the existing window-free tests pass, but no platform-window visual release check was available for this story.
    location: >-
      src/presentation/application/raylib-application.cpp:13-35
    severity: low
---

<intent-contract>

## Intent

**Problem:** Live painting must remain useful while a finite Conway field is running or paused: a sparse pointer drag must make every crossed cell alive without gaps, while UI-owned and out-of-field input stays inert.

**Approach:** Reuse the existing typed Live command, logical cell conversion, captured drag rasterizer, and Field Screen phase seams. Verify or complete the path so accepted edits mutate the current field after the scheduled batch and appear in the same final render.

## Boundaries & Constraints

**Always:** Apply Live to every touched in-bounds cell on press or drag, including already-live cells, and leave unrelated bytes unchanged. Interpolate between cell samples with the shared inclusive integer path; capture through release, stop mutating outside the field, and resume from a valid re-entry while held. Modal and toolbar ownership wins over the field and must clear the interpolation anchor so no edit crosses an owned surface. Use logical client coordinates and half-open field bounds. Keep simulation-before-input ordering, synchronous main-thread execution, the 250 ms scheduler, and macOS/Linux semantics.

**Block If:** The behavior would require changing the approved simulation-before-input order, retaining missed input in a project-owned queue, changing pointer ownership or coordinate semantics, or requiring an external action outside the repository.

**Never:** Add a second DPI transform, callbacks/events, replay or undo infrastructure, per-cell UI objects, persistence, sessions, Bank, Settings, alternate Life rules, or changes to `sprint-status.yaml`.

## I/O & Edge-Case Matrix

| Scenario | Input / State | Expected Output / Behavior | Error Handling |
|----------|--------------|-----------------------------|----------------|
| HAPPY_PATH | Live selected; press an in-bounds cell | Exactly that cell is alive and unrelated bytes are unchanged | No error expected |
| SPARSE_DRAG | Live selected; held samples jump between in-bounds cells | Every inclusive rasterized cell becomes alive; repeated samples are harmless and release ends capture | No error expected |
| PAUSED_EDIT | Paused field; Live selected; held field gesture | Live edits are accepted without advancing generations or changing unrelated bytes | No error expected |
| OUTSIDE_OR_OWNER | Held pointer enters gray space, toolbar, or modal ownership | No field byte changes; re-entry starts at its own valid segment and cannot paint through the owner | Consume or ignore without logging |
| SAME_ITERATION_BATCH | Running field with a due generation and Live input | The scheduled batch completes first, then Live mutates the resulting current field before the final render | No error expected |

</intent-contract>

## Code Map

- `src/domain/field/field.hpp:16-43` and `src/domain/field/field.cpp:40-69` -- reuse bounds-safe `contains` and `setLive(coordinate, true)`; dense row-major bytes are the authoritative current field.
- `src/application/commands/field-command.hpp:9-22` and `src/application/field-command-executor.cpp:7-23` -- keep Live mutation as a typed application command, outside presentation, with invalid coordinates as no-ops.
- `src/presentation/camera/coordinate-converter.cpp:9-56` -- authoritative logical viewport, field rectangle, and half-open cell conversion; do not map pointer coordinates elsewhere.
- `src/presentation/input/drag-rasterizer.cpp:17-78` and `src/presentation/input/input-router.cpp:66-160` -- bounded inclusive Bresenham segments, modal/toolbar/field ownership, captured anchors, and Live command emission.
- `src/presentation/application/raylib-application.cpp:39-85` -- `processIteration` advances the scheduler before sampling input, executes commands, and returns the state rendered by `FieldScreen`.
- `tests/unit/presentation/input-router-test.cpp:421-497, 570-622` and `tests/unit/presentation/drag-rasterizer-test.cpp:10-78` -- existing Live press, sparse drag, release, ownership, re-entry, and path fixtures to extend rather than duplicate.
- `tests/unit/presentation/raylib-application-test.cpp` and `tests/CMakeLists.txt:66-105` -- add application-level phase/state evidence to the existing window-free Catch2 target.

## Tasks & Acceptance

**Execution:**
- `src/presentation/input/input-router.*`, `src/presentation/input/drag-rasterizer.*`, and `src/presentation/camera/coordinate-converter.*` -- preserve or complete gap-free Live drag routing -- retain logical conversion, ownership precedence, release cleanup, and outside-field filtering.
- `src/application/commands/field-command.*` and `src/application/field-command-executor.*` -- preserve typed Live execution -- ensure only valid in-bounds cells mutate and no presentation code writes the Field.
- `src/presentation/application/raylib-application.*` -- preserve the simulation/input/render contract -- allow Live edits in Running and Paused states and keep same-iteration edits after the scheduled batch.
- `tests/unit/presentation/input-router-test.cpp` and `tests/unit/presentation/raylib-application-test.cpp` -- add paused-state Live coverage and a sparse-jump application trace -- prove exact cells, ownership rejection, release behavior, byte preservation, and phase ordering without a window or wall-clock timing.

**Acceptance Criteria:**
- Given a Running or Paused field with Live selected, when the player presses or drags across in-bounds cells, then every touched cell is alive in the current field and unrelated bytes are unchanged.
- Given a held Live gesture with separated pointer samples, when each sample is processed, then every cell on the inclusive rasterized segments is made alive without skipped gaps, and capture ends on release.
- Given the pointer enters gray out-of-field space or a toolbar/modal owner while held, when the sample is processed, then no field byte changes and a later valid re-entry cannot paint through the excluded region.
- Given a generation is due in the same Running iteration as accepted Live input, when the iteration completes, then the generation consumes the pre-input state and the Live edit is visible in the resulting final render state.

## Spec Change Log

## Review Triage Log

### 2026-08-29 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 5: (high 0, medium 2, low 3)
- defer: 1: (high 0, medium 0, low 1)
- reject: 18: (high 0, medium 0, low 18)
- addressed_findings:
  - `[medium]` `[patch]` Replaced the period-two paused fixture with a lone cell so accidental scheduler advancement cannot return the same bytes.
  - `[medium]` `[patch]` Seeded an already-live cell on the Live path and compared the complete buffer to verify Live painting is idempotent.
  - `[low]` `[patch]` Asserted typed Live command coordinates and ordering for the paused press and rasterized drag.
  - `[low]` `[patch]` Added a new press after release to verify captured-gesture cleanup.
  - `[low]` `[patch]` Asserted the application-level first Live press before the same-iteration sparse drag.

### 2026-08-29 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 3: (high 0, medium 2, low 1)
- defer: 1: (high 0, medium 0, low 1)
- reject: 16: (high 0, medium 0, low 16)
- addressed_findings:
  - `[medium]` `[patch]` Added a toolbar release edge before the paused Live gesture so the application trace models separate pointer interactions.
  - `[medium]` `[patch]` Added a release edge and a follow-up press to the running sparse-drag trace to verify captured-gesture cleanup.
  - `[low]` `[patch]` Released the paused drag at a new in-bounds cell to cover final-cell rasterization on release.

### 2026-08-29 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 3: (high 0, medium 0, low 3)
- defer: 0
- reject: 24: (high 0, medium 0, low 24)
- addressed_findings:
  - `[low]` `[patch]` Executed and asserted the paused post-release Live press so the second gesture's mutation and byte preservation are covered.
  - `[low]` `[patch]` Added a held sample after release to prove a completed gesture cannot resume without a new press.
  - `[low]` `[patch]` Recorded the actual Debug, Release, and whitespace-check outcomes in the auto-run result.

## Design Notes

The shared router emits one typed command per rasterized cell, leaving mutation and simulation in their owning layers. A captured gesture retains no project-owned replay queue: outside or owned samples produce no commands, and clearing the last-cell anchor on ownership changes makes the next valid sample a new segment.

## Verification

**Commands:**
- `cmake --preset dev-debug` -- expected: configuration succeeds with committed dependencies and presets.
- `cmake --build --preset dev-debug` -- expected: application and window-free presentation tests compile under the warning policy.
- `ctest --preset dev-debug --output-on-failure` -- expected: all discovered tests pass, including Live press, drag, paused editing, ownership, and phase-order cases.
- `cmake --preset dev-release && cmake --build --preset dev-release` -- expected: Release configuration and build succeed.
- `ctest --preset dev-release --output-on-failure` -- expected: the Release suite passes.
- `git diff --check` -- expected: no whitespace errors.

## Auto Run Result

Summary: Completed the fresh review pass for Paint Live by Drag. The existing typed Live command path and captured drag behavior are covered for running and paused fields, sparse rasterized input, ownership boundaries, release cleanup, and simulation-before-input ordering.

Files changed:
- `tests/unit/presentation/input-router-test.cpp` -- verifies post-release held input is inert and executes the follow-up paused Live press.
- `tests/unit/presentation/raylib-application-test.cpp` -- covers sparse Live input after a scheduled generation and paused Live editing.
- `_bmad-output/implementation-artifacts/spec-2-1-paint-live-by-drag.md` -- records this review, verification outcomes, and residual risk.

Review findings breakdown: 3 low-severity patches applied, 0 items newly deferred, and 24 low-severity findings rejected as duplicates, noise, or orchestrator-owned bookkeeping. The existing high-DPI deferred item was not reopened or modified.

Follow-up review recommendation: false (patched findings: high 0, medium 0, low 3; score 3).

Verification performed:
- `cmake --preset dev-debug` -- passed.
- `cmake --build --preset dev-debug` -- passed.
- `ctest --preset dev-debug --output-on-failure` -- passed: 90/90 tests.
- `cmake --preset dev-release` -- passed.
- `cmake --build --preset dev-release` -- passed.
- `ctest --preset dev-release --output-on-failure` -- passed: 90/90 tests.
- `git diff --check` -- passed.

Residual risks: Windowed macOS and Linux high-DPI visual equivalence remains unverified in this headless run; the existing deferred-work ledger entry remains unchanged for orchestrator follow-up.

---
title: '2.2 Paint Dead by Drag'
type: 'feature'
created: '2026-08-27'
status: 'done'
baseline_revision: 'ad4420e6ef41e7e8613ac88054eb640e2e84608e'
review_loop_iteration: 0
followup_review_recommended: false
context:
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/2-2-paint-dead-by-drag/_bmad-output/project-context.md'
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/2-2-paint-dead-by-drag/_bmad-output/implementation-artifacts/epic-2-context.md'
warnings: []
deferred: []
---

<intent-contract>

## Intent

**Problem:** The Field MVP can draw live cells but cannot correct a live construction by drawing cells dead. Die must work on both individual presses and sparse captured drags while preserving the finite field and deterministic frame contract.

**Approach:** Add a typed Die paint command and bounds-safe application execution, then generalize the existing selected-paint-mode input path so Die reuses logical conversion, ownership checks, and gap-free drag rasterization. Wire the existing toolbar Die control to the persistent paint selection without changing the scheduler or adding an input queue.

## Boundaries & Constraints

**Always:** Set every touched in-bounds cell directly dead, including cells that were live; leave unrelated bytes unchanged. Keep Live behavior and default selection intact. Use the centralized logical-coordinate converter, zero-based half-open cell bounds, and the existing captured rasterizer. Toolbar/modal ownership and gray or other out-of-field positions must never edit cells. Finish the scheduled simulation batch before sampling and executing Die commands; accepted edits are visible in that frame and affect only future generations. Keep raylib types in presentation and preserve macOS/Linux logical-pixel semantics.

**Block If:** The behavior requires changing the approved simulation-before-input phase order, adding project-owned input retention/replay, or choosing a new ownership or coordinate model.

**Never:** Add pause/resume, camera navigation, persistence, sessions, Bank, Settings, alternate Life rules, callbacks/events, per-cell UI objects, a second DPI transform, or changes to `sprint-status.yaml`.

## I/O & Edge-Case Matrix

| Scenario | Input / State | Expected Output / Behavior | Error Handling |
|----------|---------------|-----------------------------|----------------|
| HAPPY_PATH | Die selected; press on an in-bounds cell, including a live cell | The selected cell becomes dead in the current Field and unrelated bytes are unchanged | No error expected |
| SPARSE_DRAG | Captured Die gesture with separated in-bounds samples | Every inclusive rasterized cell becomes dead, repeated samples are harmless, and capture ends on release | No error expected |
| OUTSIDE_OR_OWNER | Die sample in gray space, outside the finite Field, toolbar, or modal ownership | No paint command reaches the Field; field bytes remain byte-for-byte unchanged | Consume or ignore without logging |
| SAME_ITERATION_BATCH | Running Field with a due generation and accepted Die input | The scheduled generation completes first; Die then mutates the resulting current Field before the final render | No error expected |

</intent-contract>

## Code Map

- `src/domain/field/field.hpp:28-31` and `src/domain/field/field.cpp:44-62` -- reuse `contains`, `isLive`, and `setLive(coordinate, false)` for exact dead-cell mutation and bounds-safe no-ops; no new cell state is needed.
- `src/application/commands/field-command.hpp` and `src/application/field-command-executor.*` -- add a distinct typed Die command and execute it in the application layer without moving mutation into input presentation.
- `src/presentation/camera/coordinate-converter.*` and `src/presentation/input/drag-rasterizer.*` -- retain the authoritative logical rectangle, half-open filtering, and deterministic inclusive segment path for both paint modes.
- `src/presentation/input/input-router.*` -- carry the selected Live/Die paint mode, consume the existing toolbar/modal ownership boundary, capture until release, and emit the matching typed command for each accepted cell.
- `src/presentation/ui/toolbar.*`, `src/presentation/screens/field-screen.*`, and `src/presentation/application/raylib-application.*` -- wire the existing Live/Die controls and selected mode through the Field screen while preserving the simulation, input, and render phase order.
- `tests/unit/presentation/input-router-test.cpp`, `tests/unit/presentation/raylib-application-test.cpp`, and `tests/CMakeLists.txt` -- extend window-free command, drag, ownership, and same-iteration tests; keep the existing test target and deterministic fixtures.

## Tasks & Acceptance

**Execution:**
- `src/application/commands/field-command.hpp` and `src/application/field-command-executor.*` -- define and execute typed Die edits -- make dead-cell mutation explicit, synchronous, and bounds-safe.
- `src/presentation/input/input-router.*` -- generalize the existing captured Live path for the selected Die mode -- preserve rasterization, release handling, outside-field exclusion, and toolbar/modal click-through prevention.
- `src/presentation/application/raylib-application.*`, `src/presentation/screens/field-screen.*`, and `src/presentation/ui/toolbar.*` -- connect Die selection to the existing Field Screen controls -- keep mode state persistent and preserve scheduler-before-input ordering.
- `tests/unit/presentation/input-router-test.cpp` and `tests/unit/presentation/raylib-application-test.cpp` -- cover dead presses, sparse drags, live-to-dead replacement, rejected ownership/outside samples, release, and a due-generation trace -- prevent regressions without opening a window or using wall-clock timing.

**Acceptance Criteria:**
- Given Die is selected, when the player presses or drags across in-bounds cells, then every touched cell is dead in the current Field, including previously live cells, and unrelated bytes are unchanged.
- Given a captured Die gesture moves between separated cell samples, when each sample is processed, then the same inclusive rasterized cells that the shared drag path crosses become dead without skipped gaps, and capture ends only on release.
- Given Die input is outside the finite Field or a toolbar/modal owns the pointer, when the sample is processed, then no field byte changes and a later accepted gesture starts at its own first cell.
- Given a Running Field has a scheduled generation in the same iteration as an accepted Die edit, when the iteration completes, then the generation consumed the pre-input state, the edit is applied afterward, and the edit is visible in the final rendered state.

## Spec Change Log

## Review Triage Log

### 2026-08-27 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 9: (high 0, medium 1, low 8)
- defer: 0
- reject: 11: (high 0, medium 0, low 11)
- addressed_findings:
  - `[low]` `[patch]` Added direct typed Die executor coverage for valid, already-dead, unrelated, and invalid coordinates.
  - `[low]` `[patch]` Added exact horizontal and diagonal Die command-sequence assertions with an unrelated live-cell sentinel.
  - `[low]` `[patch]` Covered a release sample at a different in-bounds endpoint and verified capture cleanup.
  - `[low]` `[patch]` Covered Die stroke re-entry after gray out-of-field space.
  - `[low]` `[patch]` Covered toolbar ownership during a captured Die gesture and fresh-segment re-entry.
  - `[low]` `[patch]` Covered repeated Die samples as harmless idempotent input.
  - `[medium]` `[patch]` Covered switching back from Die to Live and verified the following Live edit.
  - `[low]` `[patch]` Verified the application starts with Live selected and preserves idle selection state.
  - `[low]` `[patch]` Prevented toolbar mode selection from accepting points outside the logical client viewport.

### 2026-08-27 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 1: (high 0, medium 1, low 0)
- defer: 0
- reject: 18: (high 0, medium 0, low 18)
- addressed_findings:
  - `[medium]` `[patch]` Prevented sparse captured Die drags from rasterizing through toolbar-covered cells by using the centralized cell-center conversion and added a regression test for the ownership boundary.

## Design Notes

Keep `PaintLiveCommand` and `PaintDeadCommand` distinct at the application boundary so a Die edit cannot accidentally become a Live edit. The input router may share one mode-aware gesture implementation, but it emits one typed command per rasterized coordinate. The selected paint mode belongs to the Field interaction state and must not be inferred from cell contents; selecting Die consumes the toolbar press and never begins a field gesture.

## Verification

**Commands:**
- `cmake --preset dev-debug` -- expected: configuration succeeds with the committed preset.
- `cmake --build --preset dev-debug` -- expected: project and test targets compile with the warning policy.
- `ctest --preset dev-debug --output-on-failure` -- expected: all discovered tests pass, including the Die command, drag, ownership, and phase-order cases.
- `git diff --check` -- expected: no whitespace errors.

## Auto Run Result

Status: done

Summary: Implemented typed Die painting, persistent toolbar selection, bounds-safe dead-cell execution, captured sparse-drag coverage, and toolbar ownership filtering for interpolated drag cells.

Files changed:
- `_bmad-output/implementation-artifacts/epic-2-context.md` — generated Epic 2 planning context.
- `_bmad-output/implementation-artifacts/spec-2-2-paint-dead-by-drag.md` — story specification, review triage, and run evidence.
- `_bmad-output/implementation-artifacts/sprint-status.yaml` — pre-existing orchestrator bookkeeping; not touched in this pass.
- `src/application/commands/field-command.hpp` — typed Live and Die paint commands.
- `src/application/field-command-executor.cpp` — synchronous bounds-safe execution for both paint commands.
- `src/application/field-command-executor.hpp` — executor overloads for typed commands.
- `src/presentation/application/raylib-application.cpp` — persistent mode propagation and input execution.
- `src/presentation/application/raylib-application.hpp` — application paint-mode state and accessor.
- `src/presentation/camera/coordinate-converter.cpp` — centralized cell-center conversion for ownership checks.
- `src/presentation/camera/coordinate-converter.hpp` — cell-center conversion declaration.
- `src/presentation/input/input-router.cpp` — mode-aware commands and toolbar-safe drag rasterization.
- `src/presentation/input/input-router.hpp` — mode-aware input command surface.
- `src/presentation/screens/field-screen.cpp` — forwards the selected paint mode to the toolbar.
- `src/presentation/screens/field-screen.hpp` — carries paint mode through field rendering.
- `src/presentation/ui/toolbar.cpp` — renders the selected Live or Die control.
- `src/presentation/ui/toolbar.hpp` — paint-mode-aware toolbar API.
- `tests/unit/presentation/input-router-test.cpp` — Die press/drag, ownership, executor, and toolbar-crossing regressions.
- `tests/unit/presentation/raylib-application-test.cpp` — application mode, phase-order, and Die integration coverage.

Review findings breakdown: 1 patch applied (medium), 0 items deferred, 18 items rejected.

Follow-up review recommendation: false (patched findings: high 0, medium 1, low 0; score 3).

Verification performed:
- `cmake --preset dev-debug` — passed.
- `cmake --build --preset dev-debug` — passed after the ownership patch.
- `ctest --preset dev-debug --output-on-failure` — passed, 75/75 tests.
- `git diff --check` — passed.
- `.clang-format` inspection — the installed Homebrew clang-format 22.1.1 cannot parse the repository's `Standard: c++23`; no formatting changes were made, and the patch was checked manually against the repository style.

Residual risks: visual toolbar rendering and platform/DPI equivalence remain covered by implementation inspection and existing presentation tests rather than a windowed cross-platform test in this run.

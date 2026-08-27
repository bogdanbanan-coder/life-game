---
title: '2.3 Pause Without Losing State'
type: 'feature'
created: '2026-08-27'
status: 'done'
baseline_revision: '808a12f11d145be1f1d7d56ff1491bc60201145e'
review_loop_iteration: 0
followup_review_recommended: false
context:
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/2-3-pause-without-losing-state/_bmad-output/project-context.md'
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/2-3-pause-without-losing-state/_bmad-output/implementation-artifacts/epic-2-context.md'
warnings: []
deferred: []
---

<intent-contract>

## Intent

**Problem:** The Field currently advances whenever positive elapsed time is supplied and has no run-state transition, so the player cannot pause evolution while retaining the exact current field. The toolbar also lacks an explicit paused status.

**Approach:** Add an application-owned `RunState` and a typed pause request routed from the toolbar. Gate the fixed-step scheduler while paused, clear its partial accumulator on the Running-to-Paused transition, and expose the state through the Field Screen and Toolbar without changing the approved simulation-before-input order.

## Boundaries & Constraints

**Always:** Start a new Field in `Running`; keep `RunState` independent from persistent `PaintMode`; process the clock snapshot and any due generations before sampling the pause control; apply the pause transition through a named method; clear all accumulated simulation time when pausing; skip scheduler advancement for every paused iteration; preserve field bytes during paused time; consume the toolbar control without emitting a paint command; show explicit `Running` or `Paused` text. Preserve the 250 ms interval, four-generation catch-up policy, logical-coordinate ownership, and synchronous main-thread execution.

**Block If:** Satisfying the story requires changing simulation-before-input ordering, retaining missed input in a project-owned queue, coupling run state to paint mode, or introducing a worker thread or callback/event mechanism.

**Never:** Add persistence, sessions, Bank, camera navigation, alternate Life rules, a replay/history system, or Resume behavior beyond the state-aware presentation seam needed for the follow-up story. Never modify `sprint-status.yaml`.

## I/O & Edge-Case Matrix

| Scenario | Input / State | Expected Output / Behavior | Error Handling |
|----------|--------------|---------------------------|----------------|
| HAPPY_PATH | Running Field; pause control pressed | A typed pause request is accepted, the Field becomes Paused, no paint command is emitted, and the current bytes remain intact | No error expected |
| SAME_ITERATION_BATCH | Running Field with a due generation and pause pressed in the same iteration | The due batch completes before the pause request; the resulting Field is then Paused and no later paused iteration advances it | No error expected |
| PAUSED_ELAPSED | Paused Field with any elapsed duration, including multiple intervals | No generation runs, field bytes remain byte-for-byte unchanged, and no elapsed time is retained for a later run | No error expected |
| TOOLBAR_OWNERSHIP | Pointer press on the pause control over the Field | The toolbar owns and consumes the press; no underlying cell is edited and paint mode is unchanged | No error expected |

</intent-contract>

## Code Map

- `src/application/simulation/simulation-scheduler.hpp:10-24` and `src/application/simulation/simulation-scheduler.cpp:9-47` -- retain the fixed-step accumulator and catch-up implementation; add a named, no-throw operation that clears only accumulated timing debt without touching the Field.
- `src/application/commands/run-command.hpp` -- define the typed pause request and `RunState { Running, Paused }` at the application boundary, separate from paint commands and presentation types.
- `src/presentation/input/input-router.hpp:19-46` and `src/presentation/input/input-router.cpp:109-143` -- extend `InputCommands` with the typed pause request and recognize `Toolbar::calculateLayout(...).controls[2]` before the generic toolbar-owner rejection; preserve modal/UI ownership and emit no paint command.
- `src/presentation/application/raylib-application.hpp:19-39` and `src/presentation/application/raylib-application.cpp:39-74` -- own the run state, gate `SimulationScheduler::advance`, clear the scheduler on the named pause transition, and keep input after the scheduled batch.
- `src/presentation/screens/field-screen.hpp:10-18` and `src/presentation/screens/field-screen.cpp:5-9` -- pass `RunState` through the Field render surface so state shown by the toolbar matches application state.
- `src/presentation/ui/toolbar.hpp:10-25` and `src/presentation/ui/toolbar.cpp:67-118` -- keep the existing layout and active paint-mode styling, render state-aware Pause/Resume control text and explicit Running/Paused status, and expose a small pure label seam for window-free tests.
- `tests/unit/application/simulation-scheduler-test.cpp:17-137` -- verify accumulator clearing and that elapsed time supplied while the application is paused cannot produce generations or an early post-resume step.
- `tests/unit/presentation/input-router-test.cpp:168-214`, `tests/unit/presentation/raylib-application-test.cpp:59-221`, and `tests/unit/presentation/toolbar-test.cpp:8-27` -- cover pause-control ownership, typed command output, state transitions, same-iteration ordering, byte preservation, and state-to-label/status mapping without opening a window.

## Tasks & Acceptance

**Execution:**
- `src/application/commands/run-command.hpp` -- add typed pause/run-state definitions -- keep control intent distinct from cell painting and ready for the follow-up resume story.
- `src/application/simulation/simulation-scheduler.hpp` and `src/application/simulation/simulation-scheduler.cpp` -- add an explicit accumulator-clear operation -- discard partial and whole timing debt at the pause transition while preserving current catch-up behavior.
- `src/presentation/input/input-router.hpp` and `src/presentation/input/input-router.cpp` -- route the toolbar pause control to the typed request -- preserve modal/toolbar precedence and prevent click-through into the Field.
- `src/presentation/application/raylib-application.hpp` and `src/presentation/application/raylib-application.cpp` -- add named Running-to-Paused state handling -- skip advancement while paused, clear timing debt exactly on pause, and preserve phase ordering.
- `src/presentation/screens/field-screen.hpp`, `src/presentation/screens/field-screen.cpp`, `src/presentation/ui/toolbar.hpp`, and `src/presentation/ui/toolbar.cpp` -- render the current run state -- show explicit status and state-appropriate control copy while retaining existing paint-mode selection.
- `tests/unit/application/simulation-scheduler-test.cpp`, `tests/unit/presentation/input-router-test.cpp`, `tests/unit/presentation/raylib-application-test.cpp`, and `tests/unit/presentation/toolbar-test.cpp` -- add deterministic pause, ownership, accumulator, state-display, byte-preservation, and same-iteration regression coverage.

**Acceptance Criteria:**
- Given a newly opened Field is Running, when the player presses the toolbar pause control, then RunState becomes Paused, the control emits no paint command, and the current Field bytes are unchanged by the pause transition.
- Given a Running Field has a scheduled generation in the same iteration as a pause press, when the iteration completes, then the scheduled batch has run before input is applied, the resulting Field is Paused, and subsequent paused iterations execute no generations.
- Given a Paused Field, when one or more intervals of elapsed time are processed, then the Field bytes remain byte-for-byte identical and the scheduler retains no accumulated timing debt.
- Given either run state, when the Field Screen renders the toolbar, then it presents the matching textual `Running` or `Paused` status and does not change the persistent paint mode as a side effect.

## Spec Change Log

### 2026-08-27 — Review repair

- Triggering findings: the prior review pass removed implicit Running defaults from the Field Screen and Toolbar, and removed the redundant pause-request alias.
- Amendment: the render APIs now require explicit run state, and `PauseCommand` is the single public typed pause request.
- Known-bad state avoided: callers cannot silently render Running by omitting state, and the pause command has no competing public alias.
- KEEP: preserve the application-owned run-state boundary, explicit state-aware presentation, and the existing simulation-before-input ordering.

## Review Triage Log

### 2026-08-27 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 2: (high 0, medium 1, low 1)
- defer: 0
- reject: 22: (high 0, medium 4, low 18)
- addressed_findings:
  - `[medium]` `[patch]` Removed default run-state arguments from FieldScreen and Toolbar so a caller cannot silently render Running after omitting the actual state.
  - `[low]` `[patch]` Removed the redundant PauseRequest alias so the typed pause command has one public name.

### 2026-08-27 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 2: (high 0, medium 0, low 2)
- defer: 0
- reject: 13: (high 0, medium 3, low 10)
- addressed_findings:
  - `[low]` `[patch]` Added application-level regression coverage proving pausing preserves a selected `PaintMode::Die`.
  - `[low]` `[patch]` Recorded the prior review repairs in the Spec Change Log.

## Design Notes

Run state is an application transition axis separate from `PaintMode`: pausing controls scheduler advancement but does not alter the selected editing tool or field contents. The pause request is sampled after the scheduled batch, so a generation already due at iteration start is intentionally completed before the player’s pause command takes effect; clearing the accumulator afterward prevents that batch’s timing remainder from leaking across the pause boundary.

## Verification

**Commands:**
- `cmake --preset dev-debug` -- expected: configuration succeeds with the committed preset.
- `cmake --build --preset dev-debug` -- expected: project and test targets compile with the warning policy.
- `ctest --preset dev-debug --output-on-failure` -- expected: all discovered tests pass, including pause state, accumulator, ownership, and phase-order cases.
- `git diff --check` -- expected: no whitespace errors.

## Auto Run Result

Summary: Reviewed the completed pause/run-state implementation and strengthened its application-level regression coverage so pausing is verified to preserve a non-default paint mode. The review confirmed that scheduler gating, accumulator clearing, input ownership, simulation-before-input ordering, and state-aware labels align with the intent contract.

Files changed:
- `src/application/commands/run-command.hpp` -- defines application-owned run state and typed pause command.
- `src/application/simulation/simulation-scheduler.hpp` and `src/application/simulation/simulation-scheduler.cpp` -- expose accumulator clearing without mutating the Field.
- `src/presentation/application/raylib-application.hpp` and `src/presentation/application/raylib-application.cpp` -- own run state, gate advancement, and apply the named pause transition.
- `src/presentation/input/input-router.hpp` and `src/presentation/input/input-router.cpp` -- route the toolbar pause control without paint click-through.
- `src/presentation/screens/field-screen.hpp` and `src/presentation/screens/field-screen.cpp` -- forward run state to the render surface.
- `src/presentation/ui/toolbar.hpp` and `src/presentation/ui/toolbar.cpp` -- render state-aware control and status labels.
- `tests/unit/application/simulation-scheduler-test.cpp` -- covers accumulator clearing and fresh scheduler timing.
- `tests/unit/presentation/input-router-test.cpp` -- covers pause ownership and typed command output.
- `tests/unit/presentation/raylib-application-test.cpp` -- covers state transitions, ordering, byte preservation, and paint-mode preservation.
- `tests/unit/presentation/toolbar-test.cpp` -- covers state-to-label mapping.
- `_bmad-output/implementation-artifacts/spec-2-3-pause-without-losing-state.md` -- records review, verification, and run results.
- `_bmad-output/implementation-artifacts/sprint-status.yaml` -- orchestrator-owned bookkeeping retained unchanged by this run.

Review findings breakdown: 2 low-severity patches applied, 0 items deferred, and 13 items rejected as out of scope, already covered by the existing seams, or inconsistent with the explicit no-resume and orchestrator-ownership constraints.

Follow-up review recommendation: false. Patched findings: high 0, medium 0, low 2; score `3 × 0 + 1 × 2 = 2`.

Verification performed:
- `cmake --preset dev-debug` -- passed; the existing macOS OpenGL deprecation warning remains non-fatal.
- `cmake --build --preset dev-debug` -- passed.
- `ctest --preset dev-debug --output-on-failure` -- passed: 81/81 tests.
- `git diff --check` -- passed with no whitespace errors.

Residual risks: the review remains window-free, so the raygui drawing output itself is not rendered in automated tests; the pure label seams and direct Field Screen forwarding are covered by inspection. Resume behavior remains intentionally deferred to the follow-up story.

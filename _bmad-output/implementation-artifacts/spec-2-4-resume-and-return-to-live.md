---
title: '2.4 Resume and Return to Live'
type: 'feature'
created: '2026-08-27'
status: 'done'
baseline_revision: 'a59e3fef65a524aeafea4d74a5d48d590b9b9286'
review_loop_iteration: 0
followup_review_recommended: false
context:
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/2-4-resume-and-return-to-live/_bmad-output/project-context.md'
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/2-4-resume-and-return-to-live/_bmad-output/implementation-artifacts/epic-2-context.md'
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/2-4-resume-and-return-to-live/_bmad-output/implementation-artifacts/spec-2-3-pause-without-losing-state.md'
warnings: [oversized]
deferred: []
---

<intent-contract>

## Intent

**Problem:** The paused Field already displays a `Resume` control, but input is always translated to the pause request, so a paused experiment cannot restart. Resuming must also return the editing tool to Live and avoid immediately repaying time that elapsed while paused.

**Approach:** Add a distinct typed resume request to the existing application command boundary and make the toolbar route its state-aware control to pause or resume. Give the application a named Paused-to-Running transition that selects Live, clears scheduler debt, and leaves the established simulation-before-input phase order intact.

## Boundaries & Constraints

**Always:** A newly opened Field remains Running with Live selected. A pause press continues to be consumed by the toolbar and emits no paint command. A resume press is recognized only while Paused, changes the run state to Running, selects Live, and preserves every Field byte on the command iteration. Clear all accumulated timing debt at the resume transition so the next generation requires a fresh full 250 ms interval. A resume iteration must not run a generation before input; scheduled work is still processed before input whenever the iteration begins Running. Preserve modal and active-gesture ownership, logical-pixel coordinates, toolbar click-through prevention, the four-generation cap, and synchronous main-thread execution.

**Block If:** Satisfying the story requires changing simulation-before-input ordering, retaining missed input in a project-owned queue, coupling run state to paint mode outside the explicit resume transition, or introducing callbacks, worker threads, or asynchronous execution.

**Never:** Add persistence, sessions, Bank, camera navigation, alternate Life rules, replay/history, undo/redo, or a second source of run-state truth. Never modify `sprint-status.yaml`.

## I/O & Edge-Case Matrix

| Scenario | Input / State | Expected Output / Behavior | Error Handling |
|----------|--------------|---------------------------|----------------|
| HAPPY_PATH | Paused Field; pointer press on the toolbar `Resume` control | A typed resume request is accepted, the Field becomes Running, Live is selected, and no cell is edited | No error expected |
| FRESH_INTERVAL | Paused Field after any elapsed duration; resume, then elapsed time below 250 ms | No generation runs before a complete new interval; the preserved Field bytes remain unchanged until scheduled work is due | No error expected |
| DIE_TO_LIVE | Paused Field with Die selected; resume, then paint a cell | Resume selects Live before the later paint, so the accepted cell is set alive | No error expected |
| SAME_ITERATION | Paused Field; resume press supplied with elapsed time that would otherwise be due | The paused scheduler is skipped before input; resume changes state only, with no same-iteration catch-up generation | No error expected |
| OWNERSHIP | Modal-owned input or a captured field gesture over the toolbar | Resume is not emitted and no underlying cell is changed | No error expected |

</intent-contract>

## Code Map

- `src/application/commands/run-command.hpp:5-14` -- owns `RunState` and the existing typed `PauseCommand`; add one distinct typed resume command without mixing run control into `PaintCommand`.
- `src/application/simulation/simulation-scheduler.hpp:18-24` and `src/application/simulation/simulation-scheduler.cpp:50` -- reuse `clearAccumulator()` as the no-Field-mutation primitive for both pause and resume fresh-interval boundaries; leave `advance()` catch-up semantics unchanged.
- `src/presentation/input/input-router.hpp:19-46` and `src/presentation/input/input-router.cpp:59-143` -- extend `InputCommands` and the state-aware sampling path so toolbar control `controls[2]` emits `PauseCommand` while Running or the resume command while Paused; preserve modal, toolbar, and captured-gesture precedence and the legacy Live-only overload.
- `src/presentation/application/raylib-application.hpp:24-43` and `src/presentation/application/raylib-application.cpp:39-94` -- retain scheduler gating and simulation-before-input order, consume the typed resume request, and add a guarded named transition that clears timing debt, selects Live, and enters Running.
- `src/presentation/ui/toolbar.cpp:99-123` and `src/presentation/screens/field-screen.cpp:5-9` -- existing state-aware `Resume`/`Paused` presentation and explicit run-state forwarding are read-only reuse points; keep the visible status aligned with application state without adding another state source.
- `tests/unit/presentation/input-router-test.cpp:139-245` and `tests/unit/presentation/raylib-application-test.cpp:91-231` -- extend window-free coverage for state-aware command routing, resume ownership, Die-to-Live reset, byte preservation, same-iteration ordering, and the fresh-interval threshold.

## Tasks & Acceptance

**Execution:**
- `src/application/commands/run-command.hpp` -- define a typed resume request beside `PauseCommand` -- keep pause and resume intent explicit at the application boundary.
- `src/presentation/input/input-router.hpp` and `src/presentation/input/input-router.cpp` -- accept the current `RunState` for toolbar sampling and emit the matching run-control command -- make the existing Pause/Resume label behavior functional without click-through.
- `src/presentation/application/raylib-application.hpp` and `src/presentation/application/raylib-application.cpp` -- handle resume through a named guarded transition -- select Live, clear the scheduler, and enter Running only after the paused scheduler phase.
- `tests/unit/presentation/input-router-test.cpp` and `tests/unit/presentation/raylib-application-test.cpp` -- add deterministic resume and edge-case regression tests -- verify all matrix scenarios while preserving prior pause behavior.

**Acceptance Criteria:**
- Given a Paused Field, when the player presses the toolbar control labeled `Resume`, then the Field becomes Running, the persistent paint mode becomes Live, and the current Field bytes are unchanged by that iteration.
- Given a Paused Field and any elapsed time accumulated while it was paused, when the player resumes and then processes less than one full 250 ms interval, then no generation runs and the next scheduled generation begins from a fresh interval rather than paused-time debt.
- Given a Paused Field with Die selected, when the player resumes and paints a cell afterward, then the active mode is Live and the accepted paint sets that cell alive.
- Given input is modal-owned or belongs to a captured field gesture, when the pointer is over the toolbar resume control, then no resume request is emitted and no underlying Field cell changes.
- Given either run state, when the Field Screen renders the toolbar, then its status and Pause/Resume label match the application run state and resume does not create a second state or alter the Field.

## Review Triage Log

### 2026-08-27 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 5: (high 0, medium 0, low 5)
- defer: 0
- reject: 10: (high 0, medium 0, low 10)
- addressed_findings:
  - `[low]` `[patch]` Documented that the compatibility input-router overload uses the prior Running-state behavior, preventing accidental use for paused resume handling.
  - `[low]` `[patch]` Added realistic press/release edges to resume application tests and the state-aware router test.
  - `[low]` `[patch]` Made the captured-gesture test execute the initial paint and target the actual Resume control while asserting both run-control commands are suppressed.
  - `[low]` `[patch]` Replaced the four-cycle blinker fixture with a lone cell so a due-time resume regression cannot be masked by periodic state restoration.
  - `[low]` `[patch]` Corrected the run-command Code Map line anchor after adding `ResumeCommand`.

### 2026-08-27 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 5: (high 0, medium 0, low 5)
- defer: 0
- reject: 18: (high 0, medium 0, low 18)
- addressed_findings:
  - `[low]` `[patch]` Added a nonzero partial-running interval to the application pause/resume trace so the fresh-interval boundary is exercised with timing debt present before the pause.
  - `[low]` `[patch]` Added a release edge between the state-aware pause and resume router samples.
  - `[low]` `[patch]` Corrected the captured-gesture toolbar sample to represent a held drag rather than a second press edge.
  - `[low]` `[patch]` Asserted that releasing a captured gesture over the Resume control emits no run-control or paint command and preserves the Field.
  - `[low]` `[patch]` Recorded this pass's configure, build, test, and diff-check outcomes under Auto Run Result.

## Design Notes

Resume is applied after the iteration-start scheduler decision, so a resume press received while Paused cannot cause a generation in that same iteration. Clearing the accumulator at the transition makes the first subsequent Running iteration measure only time after resume; this is the same timing boundary used by pause and keeps controlled traces deterministic. The toolbar remains the sole owner of the run-control press, while `FieldCommandExecutor` stays limited to cell-paint commands.

## Verification

**Commands:**
- `cmake --preset dev-debug` -- expected: configuration succeeds with committed settings.
- `cmake --build --preset dev-debug` -- expected: project and test targets compile without new warnings.
- `ctest --preset dev-debug --output-on-failure` -- expected: all discovered tests pass, including pause/resume ordering and fresh-interval coverage.
- `git diff --check` -- expected: no whitespace errors.

### 2026-08-27 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 2: (high 0, medium 0, low 2)
- defer: 0
- reject: 18: (high 0, medium 0, low 18)
- addressed_findings:
  - `[low]` `[patch]` Added the required `## Auto Run Result` section with implementation, review, residual-risk, and verification details.
  - `[low]` `[patch]` Recorded concrete outcomes for every prescribed verification command.

## Auto Run Result

Summary: Reviewed the typed resume command, state-aware toolbar routing, guarded Paused-to-Running transition, Live-mode reset, scheduler reset, and deterministic regression coverage. No story-specific implementation defect was identified.

Files changed:
- `src/application/commands/run-command.hpp` -- adds the distinct `ResumeCommand` type.
- `src/presentation/input/input-router.hpp` and `src/presentation/input/input-router.cpp` -- routes the toolbar control to pause or resume based on `RunState` while preserving ownership precedence.
- `src/presentation/application/raylib-application.hpp` and `src/presentation/application/raylib-application.cpp` -- handles the guarded resume transition after the scheduler phase.
- `tests/unit/presentation/input-router-test.cpp` and `tests/unit/presentation/raylib-application-test.cpp` -- covers state-aware routing, ownership, Live reset, byte preservation, and fresh-interval behavior.
- `_bmad-output/implementation-artifacts/spec-2-4-resume-and-return-to-live.md` -- records this review and its verification result.

Review findings breakdown: 2 low-severity patches applied; 0 items deferred; 18 low-severity observations rejected as orchestrator-owned bookkeeping, redundant coverage requests, or non-defects under the intent contract. `sprint-status.yaml` was preserved and not modified by this pass.

Follow-up review recommendation: false. Patched findings: high 0, medium 0, low 2; score 2 (`3 × 0 + 1 × 2`).

Verification performed:
- `cmake --preset dev-debug` -- passed; configuration and generation completed.
- `cmake --build --preset dev-debug` -- passed; Ninja reported no work to do.
- `ctest --preset dev-debug --output-on-failure` -- passed; 86/86 tests passed.
- `git diff --check` -- passed with no output.

Residual risks: No new story-specific risk was found. This pass used deterministic window-free application/input tests; OS DPI and live rendered-window evidence remain existing release-gate concerns.

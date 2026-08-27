---
title: '2.5 Show the Active Tool'
type: 'feature'
created: '2026-08-27'
status: 'in-progress'
baseline_revision: '532a0fb5bf8851df464e13c9e24fcac32b315f81'
review_loop_iteration: 0
followup_review_recommended: false
context:
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/2-5-show-the-active-tool/_bmad-output/project-context.md'
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/2-5-show-the-active-tool/_bmad-output/implementation-artifacts/epic-2-context.md'
warnings: []
deferred: []
---

<intent-contract>

## Intent

**Problem:** A player must be able to identify the persistent editing tool and simulation run state at a glance while editing a Field. The toolbar already carries these values, but their presentation contract needs an explicit, regression-tested mapping so a selected tool cannot be confused with an ordinary button or with the transient Pause/Resume control.

**Approach:** Keep `PaintMode` and `RunState` as the only state sources, make the toolbar's selected-mode mapping and labels explicit pure presentation seams, and render the existing active fill/white outline plus textual `Live`/`Die` and `Running`/`Paused` cues from the values forwarded by `FieldScreen`.

## Boundaries & Constraints

**Always:** Show exactly one active persistent mode (`Live` or `Die`), with the active button using the existing active fill and 2 logical px white outline as well as its literal text label; do not rely on gray shade alone. Show explicit `Running` or `Paused` status independently of the state-aware `Pause` or `Resume` button label. Preserve the upper-right toolbar layout, minimum 32 logical px pointer targets, 14 logical px compact labels, and logical-client-pixel semantics. Render the current application values without mutating them; a mode selection or resume transition must be reflected by the next toolbar render. Keep the future static `Move` button outside this story's state model, preserve toolbar/modal ownership and simulation-before-input ordering, and keep presentation code as the only owner of raylib/raygui types.

**Block If:** Meeting the cues requires a second source of `PaintMode` or `RunState`, changes to the approved toolbar/accessibility contract, a new event or callback mechanism, or a window-rendering test dependency that cannot be satisfied by the existing pure presentation seams.

**Never:** Add persistence, camera controls, Bank behavior, additional editing modes, generation counters, alternate Life rules, or project-owned input retention. Never modify `sprint-status.yaml`.

## I/O & Edge-Case Matrix

| Scenario | Input / State | Expected Output / Behavior | Error Handling |
|----------|--------------|---------------------------|----------------|
| LIVE_RUNNING | `PaintMode::Live`, `RunState::Running` | `Live` is the sole active tool cue, status is `Running`, and the run control says `Pause` | No error expected |
| DIE_RUNNING | `PaintMode::Die`, `RunState::Running` | `Die` is the sole active tool cue, `Live` is visibly inactive, status is `Running`, and the run control says `Pause` | No error expected |
| DIE_PAUSED | `PaintMode::Die`, `RunState::Paused` | `Die` remains active, status is `Paused`, and the run control says `Resume`; rendering does not reset the tool | No error expected |
| LIVE_PAUSED | `PaintMode::Live`, `RunState::Paused` | `Live` remains active, status is `Paused`, and the run control says `Resume` | No error expected |
| MODE_TRANSITION | Application changes the selected mode, then Field Screen renders | The next render uses the new mode's active cue and leaves all other controls inactive | No error expected |

</intent-contract>

## Code Map

- `src/presentation/ui/toolbar.hpp:10-29` and `src/presentation/ui/toolbar.cpp:18-128` -- own the fixed toolbar layout, literal labels, active-button fill/2 px white outline, and `Running`/`Paused` text; add small pure mode-label/active-index seams and make `render` consume those mappings without adding state.
- `src/presentation/screens/field-screen.hpp:10-18` and `src/presentation/screens/field-screen.cpp:5-9` -- are the render boundary that forwards both application-owned `PaintMode` and `RunState` every frame; keep this forwarding explicit and read-only.
- `src/presentation/application/raylib-application.hpp:22-43` and `src/presentation/application/raylib-application.cpp:39-94` -- own the authoritative mode/run values and named transitions; verify selection and resume paths leave the toolbar inputs consistent, without moving state into `Toolbar`.
- `src/presentation/input/input-router.cpp:109-145` -- already consumes toolbar controls before field editing and emits mode/run commands; preserve its ownership behavior while exercising mode changes through the application surface.
- `tests/unit/presentation/toolbar-test.cpp:8-32` -- existing window-free layout and run-label tests; extend them for the active Live/Die mapping and both independent state axes.
- `tests/unit/presentation/raylib-application-test.cpp:91-267` -- existing deterministic application integration coverage for mode selection, pause, and resume-to-Live; add/retain assertions that each transition is visible through the values passed to the Field Screen without changing Field bytes.

## Tasks & Acceptance

**Execution:**
- `src/presentation/ui/toolbar.hpp` and `src/presentation/ui/toolbar.cpp` -- expose pure mappings for the selected paint label and active control, and use them in `render` with the existing active styling and state labels -- make the visual contract deterministic and prevent a second state source.
- `src/presentation/screens/field-screen.hpp` and `src/presentation/screens/field-screen.cpp` -- preserve explicit forwarding of `PaintMode` and `RunState` to the toolbar -- keep presentation synchronized with application state and side-effect free.
- `tests/unit/presentation/toolbar-test.cpp` -- cover Live/Die active-index and label mapping, independent Running/Paused labels, and the unchanged pointer-sized layout -- verify cues without opening a window.
- `tests/unit/presentation/raylib-application-test.cpp` -- cover mode selection, pause preservation, and resume-to-Live as render-facing state transitions -- prove the active tool remains correct while the Field and scheduler semantics stay unchanged.

**Acceptance Criteria:**
- Given a Field in either run state, when the Field Screen renders with `Live` selected, then the toolbar visibly marks only `Live` as active with its textual label and active cue, while `Die` is inactive.
- Given a Field in either run state, when the Field Screen renders with `Die` selected, then the toolbar visibly marks only `Die` as active with its textual label and active cue, while `Live` is inactive.
- Given either `Running` or `Paused`, when the toolbar renders, then it shows the matching explicit status text and the matching `Pause` or `Resume` label independently of the selected paint mode.
- Given the application changes paint mode, pauses, or resumes, when the next Field frame is rendered, then the toolbar reflects the current application values without changing Field bytes, scheduler behavior, or the persistent mode as a rendering side effect.
- Given toolbar, modal, or captured-gesture ownership is active, when input is sampled over the controls, then the active-tool display remains read-only and no underlying Field edit occurs.

## Spec Change Log

## Review Triage Log

## Design Notes

The active-index and label helpers are presentation mappings, not additional state. `Toolbar::render` should derive all button/status copy from its arguments on every call; the application remains the sole owner of transitions. Window-free tests can therefore verify the observable mapping and integration state while the existing raygui draw path remains responsible for the actual fill and outline.

## Verification

**Commands:**
- `cmake --preset dev-debug` -- expected: configuration succeeds with committed settings.
- `cmake --build --preset dev-debug` -- expected: project and presentation tests compile with the warning policy.
- `ctest --preset dev-debug --output-on-failure` -- expected: all discovered tests pass, including active-tool and run-state regressions.
- `git diff --check` -- expected: no whitespace errors.

---
title: '2.5 Show the Active Tool'
type: 'feature'
created: '2026-08-29'
status: 'done'
baseline_revision: '5ee2008a9f4dfdaacba460c89242f1456f8a66ec'
review_loop_iteration: 0
followup_review_recommended: false
context:
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260828-234508-b520/worktrees/2-5-show-the-active-tool/_bmad-output/project-context.md'
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260828-234508-b520/worktrees/2-5-show-the-active-tool/_bmad-output/implementation-artifacts/epic-2-context.md'
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260828-234508-b520/worktrees/2-5-show-the-active-tool/_bmad-output/implementation-artifacts/spec-2-4-resume-and-return-to-live.md'
warnings: [oversized]
deferred: []
---

<intent-contract>

## Intent

**Problem:** The Field supports Live and Die selection, but the active-tool contract is only implicit in rendering code and has no complete window-free evidence that exactly one persistent mode is visibly selected at all times.

**Approach:** Make the Toolbar's active-mode mapping and textual cue explicit and testable while preserving the application-owned `PaintMode` as the sole source of truth. Keep the selected control visibly active, retain the existing run-state status, and leave input, simulation timing, and field ownership unchanged.

## Boundaries & Constraints

**Always:** Start with Live selected. Show exactly one active persistent mode continuously; the selected Live or Die control uses the active fill, a 2 logical px white inset outline, and its literal text label. Keep the selected mode until another persistent mode is chosen, while preserving Resume's established reset to Live. Show explicit `Running` or `Paused` text alongside the state-aware `Pause` or `Resume` control. Keep toolbar presses out of the Field, preserve logical-pixel coordinates, and retain simulation-before-input ordering.

**Block If:** The contract would require a second selection state, a new persistent mode outside the current Live/Die scope, a changed input ownership or simulation phase, or native-window-only verification that cannot be represented by deterministic presentation seams.

**Never:** Add Move/Highlight behavior, persistence, sessions, Bank, camera navigation, alternate Life rules, callbacks, input queues, per-cell UI objects, or changes to `sprint-status.yaml`.

## I/O & Edge-Case Matrix

| Scenario | Input / State | Expected Output / Behavior | Error Handling |
|----------|--------------|-----------------------------|----------------|
| DEFAULT_MODE | Newly constructed Field Screen/Application | Live is the sole active tool; its label remains visible with active styling | No error expected |
| MODE_SWITCH | Toolbar selects Live or Die | The requested mode becomes the sole active mode and stays selected until a later mode selection; no cell is edited by the toolbar press | No error expected |
| RUN_STATE_DISPLAY | Running or Paused Field with either tool selected | The toolbar displays the selected tool cue, explicit `Running`/`Paused` text, and matching `Pause`/`Resume` label | No error expected |
| OWNED_INPUT | Modal-owned or toolbar-owned pointer sample | No paint command or underlying Field mutation is produced | Ignore the sample without logging |

</intent-contract>

## Code Map

- `src/presentation/ui/toolbar.hpp:10-64` and `src/presentation/ui/toolbar.cpp:13-204` -- own the upper-right layout, literal control labels, selected-control styling, named control indices, active-tool/run-state status, and the pure mapping seams used by window-free assertions.
- `src/presentation/application/raylib-application.hpp:24-46` and `src/presentation/application/raylib-application.cpp:39-106` -- own the authoritative `paintMode_` (Live by default), apply toolbar selection, and forward the same mode/run state to the Field Screen; do not create another state source.
- `src/presentation/input/input-router.hpp:19-55` and `src/presentation/input/input-router.cpp:85-158` -- emit typed Live/Die selection before field editing and enforce modal/toolbar ownership; preserve this boundary while extending only the evidence needed for the active-tool contract.
- `src/presentation/screens/field-screen.hpp:10-23` and `src/presentation/screens/field-screen.cpp:5-12` -- pass the authoritative paint mode and run state to the Toolbar without deriving either from Field bytes.
- `tests/unit/presentation/toolbar-test.cpp`, `tests/unit/presentation/input-router-test.cpp`, and `tests/unit/presentation/raylib-application-test.cpp` -- add deterministic assertions for active-mode mapping/text, default Live, persistent switching, run-state display seams, toolbar click-through prevention, and unchanged Field bytes.

## Tasks & Acceptance

**Execution:**
- `src/presentation/ui/toolbar.hpp` and `src/presentation/ui/toolbar.cpp` -- expose small pure active-mode label/selection seams and use one authoritative mapping when rendering -- make exactly-one selection, active styling, and textual cues reviewable without opening a window.
- `src/presentation/application/raylib-application.hpp`, `src/presentation/application/raylib-application.cpp`, `src/presentation/screens/field-screen.hpp`, and `src/presentation/screens/field-screen.cpp` -- preserve the existing single `PaintMode` flow and explicit run-state forwarding -- ensure rendering reflects application state without changing simulation, input, or resume semantics.
- `tests/unit/presentation/toolbar-test.cpp`, `tests/unit/presentation/input-router-test.cpp`, and `tests/unit/presentation/raylib-application-test.cpp` -- cover default mode, both selections, persistence, status labels, ownership, and field-byte preservation -- verify every matrix case through existing window-free seams.

**Acceptance Criteria:**
- Given a newly opened Field, when the Toolbar renders, then Live is the only active persistent mode and its literal `Live` cue is continuously visible with the active fill and 2 logical px white outline.
- Given Live or Die is selected, when the other persistent mode is not selected, then the current mode remains the sole active control; selecting the other mode changes only the application selection and does not edit the Field.
- Given either Running or Paused state, when the Toolbar renders, then it shows the current active-tool cue plus explicit `Running` or `Paused` text and the matching `Pause` or `Resume` label.
- Given a modal-owned or toolbar-owned pointer press, when input is sampled, then no paint command is emitted and no Field byte changes while the active mode remains valid.
- Given the existing resume transition, when Resume is accepted from Paused, then the application still selects Live and the Toolbar reflects that selection without introducing a second state source.

## Spec Change Log

## Review Triage Log

### 2026-08-29 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 7: (high 0, medium 0, low 7)
- defer: 0
- reject: 15: (high 0, medium 0, low 15)
- addressed_findings:
  - `[low]` `[patch]` Added an explicit `Active: <tool> | <run state>` Toolbar status so the active tool has a user-visible textual cue alongside its existing active styling.
  - `[low]` `[patch]` Replaced separate paint-mode ternaries with one descriptor-backed mapping for the control index and literal label.
  - `[low]` `[patch]` Made an invalid paint-mode value terminate explicitly instead of silently presenting Live as valid.
  - `[low]` `[patch]` Shared named Live, Die, and run-control indices between Toolbar rendering and input hit-testing.
  - `[low]` `[patch]` Shared the Toolbar control-count constant between its layout and label array.
  - `[low]` `[patch]` Added a symmetric Live-selection router test proving toolbar ownership and byte-for-byte Field preservation.
  - `[low]` `[patch]` Strengthened application mode-switch coverage to prove both toolbar selections leave the Field unchanged.

### 2026-08-29 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 7: (high 0, medium 0, low 7)
- defer: 0
- reject: 11: (high 0, medium 0, low 11)
- addressed_findings:
  - `[low]` `[patch]` Added a pure `ToolbarRenderPlan` consumed by the real render path, with tests proving exactly one active control and the correct Live/Die labels across both run states.
  - `[low]` `[patch]` Anchored the active outline width at the tested 2 logical px `Toolbar::ACTIVE_OUTLINE_WIDTH` constant used by active rendering.
  - `[low]` `[patch]` Routed rendered status and Pause/Resume labels through the same deterministic render plan and verified all four mode/run-state combinations.
  - `[low]` `[patch]` Extended the paint-mode descriptor with its run-state status strings so active-tool text has one authoritative mapping.
  - `[low]` `[patch]` Made invalid run-state handling explicit and consistent across Toolbar label helpers.
  - `[low]` `[patch]` Replaced raw Toolbar control indices in application tests with the shared named constants.
  - `[low]` `[patch]` Recorded the actual verification outcomes in this spec's auto-run result.

## Design Notes

The active-tool presentation must be derived from the same `PaintMode` that the input router and application already use. A pure mapping seam is preferable to inspecting raygui state: it lets tests prove that one and only one of the current Live/Die controls is active while native rendering remains a presentation concern. Run state remains a separate state axis even though the visible status combines both cues; Pause/Resume is transient and must not alter the persistent tool except for the already-approved Resume-to-Live transition.

## Verification

**Commands:**
- `cmake --preset dev-debug` -- expected: configure succeeds.
- `cmake --build --preset dev-debug` -- expected: project and window-free tests compile with the warning policy.
- `ctest --preset dev-debug --output-on-failure` -- expected: all discovered tests pass, including active-tool, state-display, ownership, and field-preservation coverage.
- `git diff --check` -- expected: no whitespace errors.

### 2026-08-29 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 3: (high 0, medium 0, low 3)
- defer: 0
- reject: 13: (high 0, medium 0, low 13)
- addressed_findings:
  - `[low]` `[patch]` Updated the Toolbar Code Map ranges to include the render-plan API and render path.
  - `[low]` `[patch]` Replaced remaining raw Toolbar indices in input-router tests with shared named constants.
  - `[low]` `[patch]` Added render-plan assertions for both Live and Die labels at their named control positions.

## Auto Run Result

Status: done

Summary: Completed a fresh review pass for Story 2.5. The remaining test literals now use the shared Toolbar control indices, the render-plan tests assert both persistent tool labels, and the Toolbar Code Map covers the current implementation. No runtime production code changed during this pass.

Files changed:
- `src/presentation/ui/toolbar.hpp` and `src/presentation/ui/toolbar.cpp` -- provide the authoritative active-tool render plan, labels, status text, active styling, and 2 logical px outline.
- `src/presentation/input/input-router.cpp` -- routes named Toolbar controls without allowing toolbar input to paint the Field.
- `tests/unit/presentation/input-router-test.cpp` -- uses shared named Toolbar indices for ownership and run-control coverage.
- `tests/unit/presentation/toolbar-test.cpp` -- verifies one active mode, complete Live/Die labels, status text, and run-state labels.
- `tests/unit/presentation/raylib-application-test.cpp` -- verifies persistent mode selection, Resume-to-Live, and Field-byte preservation.
- `_bmad-output/implementation-artifacts/spec-2-5-show-the-active-tool.md` -- records this review pass and verification evidence.
- `_bmad-output/implementation-artifacts/sprint-status.yaml` -- orchestrator-owned bookkeeping present in the reviewed baseline diff; not edited by this run.

Review findings: 3 low-severity patches applied, 0 items deferred, and 13 low-severity findings rejected. The rejected render-boundary suggestion would add native/raygui capture beyond the explicit pure, window-free render-plan seam in the intent and design notes; sprint-status findings remain orchestrator-owned.

Follow-up review recommendation: false (patched findings: high 0, medium 0, low 3; score 3).

Verification performed:
- `cmake --preset dev-debug` -- passed; emitted only the existing raylib macOS OpenGL deprecation developer warning.
- `cmake --build --preset dev-debug` -- passed.
- `ctest --preset dev-debug --output-on-failure` -- passed: 94/94 tests.
- `git diff --check` -- passed.

Residual risks: Native-window pixel output is not captured by unit tests; the deterministic Toolbar render plan remains the approved window-free seam consumed by the real renderer. Orchestrator-owned sprint bookkeeping was intentionally left untouched.

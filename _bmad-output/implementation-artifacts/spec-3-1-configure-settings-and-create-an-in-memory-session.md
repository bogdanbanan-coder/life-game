---
title: '3.1 Configure Settings and Create an In-Memory Session'
type: 'feature'
created: '2026-08-29'
status: 'blocked'
baseline_revision: '1bcf51fe350a66462ba9b86248c0ae73b135238'
review_loop_iteration: 0
followup_review_recommended: false
context:
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260828-234508-b520/worktrees/3-1-configure-settings-and-create-an-in-memory-session/_bmad-output/project-context.md'
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260828-234508-b520/worktrees/3-1-configure-settings-and-create-an-in-memory-session/_bmad-output/implementation-artifacts/epic-3-context.md'
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260828-234508-b520/worktrees/3-1-configure-settings-and-create-an-in-memory-session/_bmad-output/game-architecture.md'
warnings: [oversized]
deferred: []
---

<intent-contract>

## Intent

**Problem:** The application currently starts directly in one fixed 50×50 Field, with no Start Screen, global Settings, named sessions, or configurable generation interval.

**Approach:** Add a typed in-memory Settings/session flow owned by the Start Screen. Validate and stage Settings and session names before committing them, then open each newly created session in the existing Field flow with its fixed dimensions, saved interval, and Live selected.

## Boundaries & Constraints

**Always:** Use one global Settings value with defaults 50×50 cells and 250 ms; validate dimensions through `FieldDimensions`; accept the generation interval as a positive base-10 number of seconds using `.` as the decimal separator, rejecting zero, negative, nonnumeric, NaN, and infinity without silent clamping or rounding. Trim and NFC-normalize names, require 1–64 Unicode code points, preserve display case/internal whitespace, and compare names case-insensitively. Settings Save is all-or-nothing. Invalid input remains visible and disables Save; failed Save preserves the active configuration and staged edits. Create and Settings Cancel are non-mutating. A session captures its dimensions at creation and later Settings edits never resize it. Preserve simulation-before-input ordering, typed commands, logical coordinates, direct synchronous calls, and the existing Field/Toolbar behavior.

**Block If:** Completing the story would require SQLite/filesystem persistence, an external service, a new product decision about interval/name semantics, or native-window-only behavior that cannot be represented by deterministic seams.

**Never:** Add persistence, per-session Settings, automatic resizing, alternate Life rules, callbacks/events/input queues, a second `RunState`/paint-mode source, or changes to `sprint-status.yaml`.

## I/O & Edge-Case Matrix

| Scenario | Input / State | Expected Output / Behavior | Error Handling |
|----------|--------------|-----------------------------|----------------|
| DEFAULT_SETTINGS | Fresh Start Screen | Settings show 50×50 and 0.25 seconds; no session exists | No error expected |
| INVALID_SETTINGS | Non-numeric, zero, negative, NaN/infinite interval, or out-of-range dimension edit | Text is retained, field-local validation is visible, Save is disabled, active settings remain unchanged | No logging for validation |
| SETTINGS_SAVE | Three valid edited values | All three replace the global in-memory value together | No partial commit |
| SETTINGS_SAVE_FAILURE | Valid staged values and a failed Save result | Prior settings remain active and edits remain visible; Error dialog offers Retry and Cancel | Propagate `ErrorCode`; log only at origin |
| CREATE_SESSION | Valid unique name and valid Settings | One named fixed-dimension session opens Field with Live and a fresh configured interval | No error expected |
| REJECT_CREATE | Blank/invalid or case-insensitive duplicate name, or Cancel | No session/card/settings mutation; entered text remains available for correction | Validation is non-modal and unlogged |
| FIXED_DIMENSIONS | Existing session after global width/height edit | Existing Field dimensions and cells are unchanged; only later-created sessions use new defaults | No error expected |

</intent-contract>

## Code Map

- `src/domain/field/field-dimensions.hpp:10-37` and `src/domain/field/field.hpp:16-43` -- authoritative dimension limits, overflow-safe validation, and empty Field construction to reuse for session creation.
- `src/foundation/result.hpp` and `src/foundation/error-code.hpp` -- existing typed success/failure transport; callers must propagate codes without message reconstruction.
- `src/application/simulation/simulation-scheduler.hpp:9-28` and `.cpp:10-48` -- currently hardcode 250 ms; add a validated interval while preserving accumulator, four-step cap, and default behavior.
- `src/presentation/application/raylib-application.hpp:24-46` and `.cpp:10-106` -- current Field-only loop and authoritative paint/run state; refactor composition/navigation here without duplicating state or changing phase order.
- `src/presentation/screens/field-screen.*`, `src/presentation/ui/toolbar.*`, and `src/presentation/input/input-router.*` -- preserve existing Field rendering, toolbar ownership, and modal-input boundary while routing Start Screen transitions.
- `src/bootstrap/main.cpp:7-18`, `src/domain/CMakeLists.txt`, `src/application/CMakeLists.txt`, `src/presentation/CMakeLists.txt`, and `tests/CMakeLists.txt` -- composition and explicit target/test registration points; no adapter or SQLite code belongs in this story.
- New `src/domain/session/session-name.*`, `src/domain/session/session.*`, `src/application/configuration/simulation-settings.*`, `src/application/configuration/settings-service.*`, `src/application/session/session-service.*`, `src/presentation/screens/start-screen/start-screen.*`, and `src/presentation/ui/{settings-panel,name-dialog,numeric-field,text-field,status-message}.*` files -- own validated session/settings values, in-memory orchestration, dialogs, fields, feedback, and Start Screen presentation; keep raylib types in presentation.

## Tasks & Acceptance

**Execution:**
- `src/domain/session/session-name.*` and `src/domain/session/session.*` plus `src/application/configuration/simulation-settings.*` and `settings-service.*` -- add validating typed names, global Settings, and fixed-dimension session values with 50×50/250 ms defaults -- keep invariants outside UI and persistence.
- `src/application/session/session-service.*` and `src/application/simulation/simulation-scheduler.*` -- implement in-memory create/settings orchestration and configurable scheduling -- parse positive decimal seconds into a typed duration, commit staged values atomically, reject duplicate names, and preserve fresh-interval creation semantics.
- `src/presentation/screens/start-screen/start-screen.*`, `src/presentation/ui/settings-panel.*`, `name-dialog.*`, `numeric-field.*`, `text-field.*`, `status-message.*`, and `src/presentation/application/*` -- render the Start Screen, two-column Settings table, numeric/text validation, modal actions, and navigation into Field -- prevent modal click-through and retain existing Field behavior.
- `src/bootstrap/main.cpp` and owning `CMakeLists.txt` files -- compose the Start Screen flow and register all new sources without introducing adapters -- keep bootstrap free of gameplay rules.
- `tests/unit/domain/session-name-test.cpp`, `tests/unit/domain/session-test.cpp`, `tests/unit/application/settings-service-test.cpp`, `tests/unit/application/session-service-test.cpp`, `tests/unit/application/simulation-scheduler-test.cpp`, `tests/unit/presentation/start-screen-test.cpp`, `tests/unit/presentation/settings-panel-test.cpp`, and `tests/unit/presentation/raylib-application-test.cpp` -- add deterministic tests for the matrix and acceptance behavior -- verify rollback, uniqueness, fixed dimensions, custom interval timing, cancellation, and modal ownership without opening a window.

**Acceptance Criteria:**
- Given the Start Screen is open, when Settings is opened, then it renders one two-column table for global Field width, Field height, and generation interval.
- Given any invalid numeric Settings input, when it is edited, then the input remains visible with field-local validation, Save is disabled, and the active configuration is unchanged.
- Given three valid Settings values, when Save succeeds, then width, height, and interval become one committed global configuration with no partial update.
- Given a Settings Save failure, when the Error dialog is shown, then the prior configuration remains active, staged edits remain visible, and Retry repeats only Save while Cancel leaves the valid configuration in place.
- Given valid Settings and a valid unique name, when Create is committed, then a fixed-dimension in-memory session opens the Field with Live selected and the saved interval beginning fresh.
- Given an invalid or case-insensitive duplicate name, when Create is attempted, then no session or global value changes and the entered value remains available for correction.
- Given an existing session, when global width or height is changed and another session is created, then the existing session dimensions and cells remain unchanged while the new session uses the new defaults.
- Given Settings or Create is canceled, when the owning surface returns, then no staged global value or session is committed and no obscured Field input is triggered.

## Spec Change Log

## Review Triage Log

## Design Notes

Stage Settings and names as presentation/application drafts and replace the committed value only after the complete candidate validates. Keep the session's Field and creation-time dimensions together so later global-default edits cannot resize it. The scheduler should accept a typed interval with the existing 250 ms default, letting current tests retain their behavior while new tests prove a custom interval. Use pure layout/validation/render-plan seams for window-free evidence; raygui remains only the presentation implementation.

## Verification

**Commands:**
- `cmake --preset dev-debug` -- expected: configuration succeeds.
- `cmake --build --preset dev-debug` -- expected: all new domain/application/presentation code and tests compile under the warning policy.
- `ctest --preset dev-debug --output-on-failure` -- expected: all discovered tests pass, including settings validation/rollback, name uniqueness, session creation, fixed dimensions, custom timing, and modal ownership.
- `cmake --preset dev-release && cmake --build --preset dev-release` -- expected: Release configuration and build succeed.
- `ctest --preset dev-release --output-on-failure` -- expected: the Release suite passes.
- `git diff --check` -- expected: no whitespace errors.

## Auto Run Result

Status: blocked
Blocking condition: dirty working tree before implementation; untracked generated story artifacts remain after the required `git add --refresh -- .` check.

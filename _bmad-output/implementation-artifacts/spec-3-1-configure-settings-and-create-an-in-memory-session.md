---
title: '3.1 Configure Settings and Create an In-Memory Session'
type: 'feature'
created: '2026-08-29'
status: 'done'
baseline_revision: '190ec784f60453da4544dab806db24fe40ce5085'
review_loop_iteration: 0
followup_review_recommended: true
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
- New `src/domain/session/session-name.*`, `src/domain/session/session.*`, `src/application/configuration/simulation-settings.*`, `src/application/configuration/settings-service.*`, `src/application/session/session-service.*`, `src/presentation/screens/start-screen.*`, and `src/presentation/ui/{settings-panel,name-dialog,numeric-field,text-field,status-message,error-dialog}.*` files -- own validated session/settings values, in-memory orchestration, dialogs, fields, feedback, and Start Screen presentation; keep raylib types in presentation. Pinned utf8proc supplies complete Unicode NFC normalization and case folding for the domain name value.

## Tasks & Acceptance

**Execution:**
- `src/domain/session/session-name.*` and `src/domain/session/session.*` plus `src/application/configuration/simulation-settings.*` and `settings-service.*` -- add validating typed names, global Settings, and fixed-dimension session values with 50×50/250 ms defaults -- keep invariants outside UI and persistence.
- `src/application/session/session-service.*` and `src/application/simulation/simulation-scheduler.*` -- implement in-memory create/settings orchestration and configurable scheduling -- parse positive decimal seconds into a typed duration, commit staged values atomically, reject duplicate names, and preserve fresh-interval creation semantics.
- `src/presentation/screens/start-screen.*`, `src/presentation/ui/settings-panel.*`, `name-dialog.*`, `numeric-field.*`, `text-field.*`, `status-message.*`, `error-dialog.*`, and `src/presentation/application/*` -- render the Start Screen, two-column Settings table, numeric/text validation, modal actions, and navigation into Field -- prevent modal click-through and retain existing Field behavior.
- `src/bootstrap/main.cpp` and owning `CMakeLists.txt` files -- compose the Start Screen flow and register all new sources without introducing adapters -- keep bootstrap free of gameplay rules.
- `tests/unit/domain/session-name-test.cpp`, `tests/unit/domain/session-test.cpp`, `tests/unit/application/settings-service-test.cpp`, `tests/unit/application/session-service-test.cpp`, `tests/unit/application/simulation-scheduler-test.cpp`, `tests/unit/presentation/start-screen-test.cpp`, `tests/unit/presentation/settings-panel-test.cpp`, `tests/unit/presentation/raylib-application-test.cpp`, and `tests/unit/presentation/raylib-application-story-test.cpp` -- add deterministic tests for the matrix and acceptance behavior -- verify rollback, uniqueness, fixed dimensions, custom interval timing, cancellation, session opening, and modal ownership without opening a window.

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

### 2026-08-29 — Implementation and verification recovery

- Recovered the preserved Story 3.1 implementation after the automation session timed out, completed the in-memory Settings/session flow, and hardened Unicode name handling with pinned utf8proc 2.11.2.
- Added regression evidence for complete Unicode normalization/case folding, configured scheduler timing, opening existing sessions with the current interval, and atomic modal flows.

## Review Triage Log

### 2026-08-29 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 3: (high 0, medium 1, low 2)
- defer: 0
- reject: 0
- addressed_findings:
  - `[medium]` `[patch]` Replaced the incomplete hand-written Unicode normalization subset with pinned utf8proc NFC normalization and full case folding, with regression coverage for composed names and sharp-S case folding.
  - `[low]` `[patch]` Revalidated Settings and name fields before action handling so text changes made by the immediate-mode text boxes cannot leave Save/Create state stale.
  - `[low]` `[patch]` Appended new session error aliases after the existing ErrorCode values to preserve the numeric values of previously defined persistence errors.
- delegated_review: The synchronous read-only reviewer did not return before its bounded wait; the implementation was completed through the local adversarial review recorded here.

### 2026-08-29 — Automated hardening pass

- intent_gap: 0
- bad_spec: 0
- patch: 7: (high 0, medium 3, low 4)
- defer: 0
- reject: 8: (high 0, medium 2, low 6)
- addressed_findings:
  - `[medium]` `[patch]` Made text editing safe at the UTF-8 byte boundary, gave raygui the full editable buffer, and prevented application-level input from being applied twice.
  - `[medium]` `[patch]` Added dimension-bound validation and refreshed field validation before Save/Create actions so immediate-mode edits cannot leave stale state.
  - `[medium]` `[patch]` Reserved in-memory session capacity so returned session handles remain stable while the supported session limit is populated.
  - `[low]` `[patch]` Rejected unrepresentable decimal scales, kept interval display parser-compatible, and removed ambiguous scheduler duration overloads and invalid-interval constructor fallbacks.
  - `[low]` `[patch]` Added closed-surface guards for Settings Save and Create, preserved duplicate-name precedence at capacity, and surfaced successful Settings Save feedback on Start Screen.
  - `[low]` `[patch]` Kept utf8proc private to the domain target and added deterministic regression coverage for the hardened paths.
  - `[low]` `[patch]` Added render-plan and text-field tests covering the visible Settings surface, local validation, Unicode editing, and modal commit boundaries.
- delegated_review: Four synchronous read-only reviewers completed blind, edge-case, verification-gap, and intent-alignment passes. Findings outside the story contract or requiring native-window-only evidence were rejected; no human decision is required.

### 2026-08-29 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 6: (high 0, medium 3, low 3)
- defer: 0
- reject: 2: (high 0, medium 2, low 0)
- addressed_findings:
  - `[low]` `[patch]` Added a sorted-card selection regression through the Start Screen input boundary.
  - `[low]` `[patch]` Added service-level Unicode NFC and case-folded duplicate-name coverage.
  - `[medium]` `[patch]` Added SettingsPanel coverage for side bounds, total-cell overflow, and invalid intervals while preserving active settings.
  - `[medium]` `[patch]` Added an application story assertion that newly created sessions begin at a fresh configured interval.
  - `[medium]` `[patch]` Added modal Create-click coverage for invalid and duplicate names, including dialog/text retention.
  - `[low]` `[patch]` Added deterministic coverage for the Settings Save failure message.
- delegated_review: The verification-gap and intent-alignment reviewers returned. The blind and edge-case reviewers did not return within the bounded 10-minute wait; local inspection triaged the remaining review surface. The orchestrator-owned `sprint-status.yaml` bookkeeping was preserved and was not treated as an implementation defect; deferred-work ledger entries were not modified.

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

Status: done

Summary: Reviewed and hardened the typed in-memory Settings/session flow, configurable scheduling, Start Screen modal navigation, fixed-dimension sessions, and preserved Field behavior for Story 3.1. Added six deterministic regression checks without adding persistence or changing orchestrator bookkeeping.

Files changed:

- `cmake/dependencies.cmake` — adds the pinned utf8proc dependency for complete Unicode name normalization and case folding.
- `src/domain/{CMakeLists.txt,session/session-id.hpp,session/session-name.*,session/session.*}` — owns validated session names, identifiers, and fixed-dimension session aggregates.
- `src/foundation/error-code.hpp` — adds typed session/configuration error aliases while preserving existing values.
- `src/application/{CMakeLists.txt,commands/run-command.hpp,configuration/simulation-settings.*,configuration/settings-service.*,session/session-service.*,simulation/simulation-scheduler.*}` — implements validated global settings, in-memory session orchestration, and configurable timing.
- `src/presentation/{CMakeLists.txt,application/raylib-application.*,input/input-router.*,screens/start-screen.*,ui/{error-dialog,name-dialog,numeric-field,settings-panel,status-message,text-field}.*,ui/toolbar.hpp}` — integrates Start Screen, dialogs, fields, feedback, and session-backed Field navigation while retaining existing input ownership.
- `src/bootstrap/main.cpp` — composes the in-memory Settings/session services into the application.
- `tests/CMakeLists.txt` and `tests/unit/{domain,application,presentation}/*` — registers the story tests and adds validation, uniqueness, modal, card-selection, timing, and error-copy coverage.
- `README.md`, `_bmad-output/game-architecture.md`, `_bmad-output/implementation-artifacts/{epic-3-context.md,sprint-status.yaml}`, and `_bmad-output/project-context.md` — baseline project and orchestrator artifacts reviewed as part of the change; `sprint-status.yaml` was not modified in this pass.
- `_bmad-output/implementation-artifacts/spec-3-1-configure-settings-and-create-an-in-memory-session.md` — records this review pass, verification, and final result.

Review findings breakdown: 6 patches applied (0 high, 3 medium, 3 low), 0 items deferred, and 2 items rejected as window-only verification gaps under the project's deterministic, window-free test rules. The orchestrator-owned `sprint-status.yaml` change was preserved and not treated as a defect.

Follow-up review recommendation: `true` (patched findings: high 0, medium 3, low 3; score `3 × 3 + 3 = 12`).

Verification performed:

- `cmake --preset dev-debug` — passed.
- `cmake --build --preset dev-debug` — passed.
- `ctest --preset dev-debug --output-on-failure` — passed, 132/132.
- `cmake --preset dev-release` — passed.
- `cmake --build --preset dev-release` — passed.
- `ctest --preset dev-release --output-on-failure` — passed, 132/132.
- `git diff --check` — passed.
- The installed Homebrew clang-format 22.1.1 could not parse the repository `.clang-format` value `Standard: c++23`; no formatter rewrite was applied, and the changed tests were manually checked against the existing style.

Residual risks: native raygui rendering, OS DPI behavior, and full Start Screen visual output remain covered by deterministic render/input seams rather than a windowed test.

---
title: '1.2 Paint Live Cells'
type: 'feature'
created: '2026-08-26'
status: 'done'
baseline_revision: '2e21ae1c5f4e71fa22b69f72e0db9a502cdbd1d3'
review_loop_iteration: 0
followup_review_recommended: true
context:
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/1-2-paint-live-cells/_bmad-output/project-context.md'
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/1-2-paint-live-cells/_bmad-output/implementation-artifacts/epic-1-context.md'
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/1-2-paint-live-cells/_bmad-output/game-architecture.md'
warnings: []
deferred: []
---

## Intent

**Problem:** The Field MVP renders a finite board but does not accept direct player input, so its core manipulation loop cannot be demonstrated.

**Approach:** Translate logical mouse samples through one centralized coordinate converter and input router into typed Live-cell commands. Rasterize captured pointer movement across logical cells and execute only accepted in-bounds commands against the current dense Field.

## Boundaries & Constraints

**Always:** Keep Live selected by default and make every touched in-bounds cell live. Use zero-based half-open field geometry and logical client coordinates; reject negative, right-edge, bottom-edge, and gray out-of-field positions before conversion. Resolve ownership as active modal/overlay, toolbar, then field gesture; a field gesture captures until release. Rasterize sparse drag samples without gaps. Reuse `Field::setLive`, preserve the existing 1280×720/high-DPI logical viewport behavior, and keep raylib/raygui types in presentation.

**Block If:** The pinned raylib input APIs or the approved layer direction cannot compile without a new product or architecture decision.

**Never:** Add Die editing, simulation timing, camera navigation, sessions, Bank, Settings, persistence, input retention/replay, callbacks/events, per-cell UI objects, or a second DPI transform. Toolbar and modal-owned input must not mutate the Field.

## I/O & Edge-Case Matrix

| Scenario | Input / State | Expected Output / Behavior | Error Handling |
|----------|---------------|-----------------------------|----------------|
| HAPPY_PATH | Live press on an in-bounds logical point | Exactly its mapped cell changes from dead to live | No error expected |
| SPARSE_DRAG | Captured Live gesture with separated cell samples | Every deterministic rasterized cell between samples is live, with no gaps | Ignore repeated writes |
| OUTSIDE_OR_TOOLBAR | Point in gray out-of-field space, toolbar panel, or active modal | Field bytes remain byte-for-byte unchanged | Consume or ignore input without logging |
| HALF_OPEN_BOUNDARY | Point on field left/top edge, or exactly at right/bottom edge | Left/top maps to the correct cell; right/bottom maps to no cell | Reject invalid conversion without mutation |

## Code Map

- `src/domain/field/field.hpp:20-27` and `field.cpp:38-62` -- reuse bounds-safe `Field::setLive` and dense row-major state; no domain changes are needed.
- `src/presentation/rendering/field-renderer.hpp:9-24` and `field-renderer.cpp:42-65` -- authoritative render-plan rectangle and integer cell size; coordinate conversion must reuse this geometry instead of duplicating layout math.
- `src/presentation/camera/coordinate-converter.hpp/.cpp` -- convert normalized logical points to optional `CellCoordinate`, enforcing finite values and half-open rectangle bounds.
- `src/presentation/input/drag-rasterizer.hpp/.cpp` -- deterministic integer line rasterization between cell samples, returning every touched coordinate exactly once per segment.
- `src/application/commands/field-command.hpp` and `src/application/field-command-executor.hpp/.cpp` -- typed `PaintLive` command and application-side execution against `Field`; retain the approved dependency direction.
- `src/presentation/input/input-router.hpp/.cpp` -- sample press/down/release state, apply modal and `Toolbar::calculateLayout` ownership, capture field gestures through release, and emit typed commands only for in-bounds Live edits.
- `src/application/CMakeLists.txt`, `src/presentation/CMakeLists.txt`, and `tests/CMakeLists.txt` -- compile the new application/presentation sources and register window-free input/conversion/rasterization tests.
- `src/presentation/application/raylib-application.cpp:11-27` -- sample `GetMousePosition()` with `GetScreenWidth/Height` after any future simulation phase and execute commands before the single render; raylib already supplies logical mouse coordinates, so do not divide or multiply by DPI.
- `src/presentation/screens/field-screen.*` and `src/presentation/ui/toolbar.*` -- preserve the existing Field/Toolbar render composition and upper-right layout; toolbar panel geometry is the input ownership boundary.
- `tests/unit/presentation/field-renderer-test.cpp:9-44` and `tests/unit/domain/field-test.cpp:34-91` -- existing render geometry and byte-level mutation fixtures to extend without opening a window or changing `sprint-status.yaml`.

## Tasks & Acceptance

**Execution:**
- `src/presentation/camera/coordinate-converter.*` and `src/presentation/input/drag-rasterizer.*` -- implement reusable logical point conversion and gap-free cell rasterization -- make input deterministic and boundary-safe.
- `src/application/commands/field-command.hpp`, `src/application/field-command-executor.*`, and `src/application/CMakeLists.txt` -- define and execute typed Live commands -- keep mutation outside the input sampler and within the application layer.
- `src/presentation/input/input-router.*`, `src/presentation/CMakeLists.txt`, `src/presentation/application/raylib-application.*`, and `src/presentation/ui/toolbar.*` -- route raylib samples through ownership and captured Live painting -- prevent toolbar/modal click-through while keeping logical DPI behavior.
- `tests/unit/presentation/*` and `tests/CMakeLists.txt` -- cover coordinate boundaries, sparse horizontal/diagonal rasterization, one-cell painting, toolbar/modal/outside exclusion, and unchanged bytes on rejection -- prevent input regressions without window or wall-clock dependencies.

**Acceptance Criteria:**
- Given Live is selected, when the player presses an in-bounds visible cell, then exactly that logical cell is live in the current Field and unrelated bytes are unchanged.
- Given Live is selected and a field gesture remains held, when pointer samples jump between cells, then every cell on each rasterized segment becomes live without skipped gaps, and capture ends only on release.
- Given a pointer is in gray out-of-field space or on a toolbar/modal owner, when the input sample is processed, then no Field byte changes and no hidden cell receives the edit.
- Given the application runs on a high-DPI display, when a pointer sample is mapped, then the same logical visible point selects the same cell as at 1× DPI without a second scaling operation.

## Review Triage Log

### 2026-08-26 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 5: (high 0, medium 2, low 3)
- defer: 0
- reject: 13: (high 0, medium 5, low 8)
- addressed_findings:
  - `[medium]` `[patch]` Clear an active gesture when the button is up without a release edge, preventing stale capture after focus/input loss.
  - `[medium]` `[patch]` Add outside/re-entry and outside-release coverage, proving valid cells continue to rasterize and the next press starts a fresh gesture.
  - `[low]` `[patch]` Require coherent pressed-and-down activation and cover non-finite logical pointer values.
  - `[low]` `[patch]` Cover reverse horizontal and vertical rasterization directions.
  - `[low]` `[patch]` Harden drag rasterization arithmetic against signed overflow.

## Design Notes

The router emits one command per rasterized coordinate so execution remains a simple synchronous application call. A captured gesture retains its last valid cell while the pointer is outside the field; no cell changes occur outside the field, and re-entry continues the captured segment until release. When a toolbar or modal owns a captured sample, the router clears the rasterization anchor so re-entry starts a fresh segment and cannot paint through the overlay. The MVP’s visible Die button remains inert because Die behavior belongs to later editing stories.

## Verification

**Commands:**
- `cmake --preset dev-debug` -- expected: configure succeeds with the new presentation/application sources.
- `cmake --build --preset dev-debug` -- expected: the harness and all input tests compile under the project warning policy.
- `ctest --preset dev-debug --output-on-failure` -- expected: all existing tests plus deterministic coordinate, rasterizer, and router cases pass without a window.
- `cmake --preset dev-release && cmake --build --preset dev-release` -- expected: Release compilation succeeds.
- `git diff --check` -- expected: no whitespace errors.

**Manual checks (if no CLI):**
- Launch the harness and confirm one click paints one white cell, sparse drag paints a continuous line, toolbar clicks do not paint the field, gray space is inert, and close exits cleanly; record only the platform actually exercised.

### 2026-08-26 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 10: (high 1, medium 1, low 8)
- defer: 0
- reject: 12: (high 0, medium 7, low 5)
- addressed_findings:
  - `[high]` `[patch]` Prevent captured field gestures from painting through modal or toolbar ownership, with regression coverage.
  - `[medium]` `[patch]` Verify diagonal rasterization through the router and executor, not only in the rasterizer unit.
  - `[low]` `[patch]` Harden rasterizer coordinate and error arithmetic against signed overflow.
  - `[low]` `[patch]` Guard toolbar layout calculations against non-positive viewport dimensions.
  - `[low]` `[patch]` Add lost-release, final-boundary, relative-outside, steep/reverse, and same-cell regression coverage.
  - `[low]` `[patch]` Correct the prior triage log's addressed-finding count.

### 2026-08-26 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 9: (high 1, medium 1, low 7)
- defer: 0
- reject: 8: (high 0, medium 2, low 6)
- addressed_findings:
  - `[high]` `[patch]` Prevent captured gestures from interpolating through toolbar or modal ownership by clearing the rasterization anchor and starting a fresh re-entry segment.
  - `[medium]` `[patch]` Reject non-finite, non-positive-viewport, and outside-client logical pointer coordinates before field conversion.
  - `[low]` `[patch]` Bound rasterizer spans by the field safety limit to avoid pathological allocation and arithmetic ranges.
  - `[low]` `[patch]` Make reverse rasterization return the same cell set in reverse order and document the deterministic inclusive path contract.
  - `[low]` `[patch]` Add regression coverage for painting the final in-bounds cell on release.
  - `[low]` `[patch]` Add exact router command-sequence coverage to reject unintended diagonal overpainting.
  - `[low]` `[patch]` Add regression coverage proving rejected presses do not capture a gesture.
  - `[low]` `[patch]` Add application-boundary coverage proving invalid commands leave field bytes unchanged.
  - `[low]` `[patch]` Add rasterizer safety-limit coverage.

## Auto Run Result

Summary: Completed the Live-cell painting review pass. Logical pointer conversion now rejects points outside the client viewport, captured gestures cannot rasterize through toolbar or modal ownership, and rasterization is direction-stable and bounded by the field safety limit. Added regression coverage for release-time painting, overlay re-entry, rejected presses, exact command output, invalid commands, and client boundaries.

Files changed:
- `src/presentation/camera/coordinate-converter.cpp` -- rejects invalid logical viewport points before cell conversion.
- `src/presentation/input/drag-rasterizer.cpp` and `drag-rasterizer.hpp` -- bounds allocation/arithmetic, normalizes reverse paths, and documents the path contract.
- `src/presentation/input/input-router.cpp` -- resets the interpolation anchor when toolbar or modal ownership takes over.
- `tests/unit/presentation/coordinate-converter-test.cpp` -- covers client viewport rejection.
- `tests/unit/presentation/drag-rasterizer-test.cpp` -- covers safety-limit rejection and updated reverse-path expectations.
- `tests/unit/presentation/input-router-test.cpp` -- covers release, rejected-press, overlay re-entry, exact command, and invalid-command behavior.
- `spec-1-2-paint-live-cells.md` -- records this review, verification, and residual risks.
- `sprint-status.yaml` -- orchestrator-owned bookkeeping; intentionally not modified by this run.

Review findings breakdown: 9 patches applied (1 high, 1 medium, 7 low); 0 items deferred; 8 items rejected (0 high, 2 medium, 6 low).

Follow-up review recommendation: true. Patched findings by severity: high 1, medium 1, low 7. Score: `3 × 1 + 1 × 7 = 10`; the high-severity patch also requires `true`.

Verification performed:
- `cmake --preset dev-debug` -- passed.
- `cmake --build --preset dev-debug` -- passed.
- `ctest --preset dev-debug --output-on-failure` -- passed, 39/39 tests.
- `cmake --preset dev-release && cmake --build --preset dev-release` -- passed.
- `ctest --preset dev-release --output-on-failure` -- passed, 39/39 tests.
- `git diff --check` -- passed with no whitespace errors.
- `clang-format -i` -- unavailable for this repository configuration because the installed formatter rejects `Standard: c++23`; no formatting changes were made by the failed command, and changed code follows the existing `.clang-format` style.
- Manual visual and macOS/Linux high-DPI checks -- not performed in this headless run.

Residual risks: The real raylib event/render loop and platform-specific high-DPI behavior still need visual release-gate evidence on macOS and Linux. The orchestrator-owned `sprint-status.yaml` change remains outside this run's write scope.

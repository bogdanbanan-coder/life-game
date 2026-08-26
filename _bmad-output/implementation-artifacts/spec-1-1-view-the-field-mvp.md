---
title: '1.1 View the Field MVP'
type: 'feature'
created: '2026-08-26'
status: 'done'
baseline_revision: '49b02ed2c9b83528a670dadd981cde1b870c4793'
review_loop_iteration: 0
followup_review_recommended: false
context:
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/1-1-view-the-field-mvp/_bmad-output/project-context.md'
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/1-1-view-the-field-mvp/_bmad-output/implementation-artifacts/epic-1-context.md'
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/1-1-view-the-field-mvp/_bmad-output/game-architecture.md'
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/1-1-view-the-field-mvp/_bmad-output/planning-artifacts/ux-designs/ux-life-game-2026-08-19/EXPERIENCE.md'
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/1-1-view-the-field-mvp/_bmad-output/planning-artifacts/ux-designs/ux-life-game-2026-08-19/DESIGN.md'
warnings: []
deferred: []
---

## Intent

**Problem:** The foundation builds, but there is no launched Field surface for validating the first player-facing slice of Life Game.

**Approach:** Add a validating finite-field domain model and a minimal raylib Field Screen that opens at 1280×720, renders a dead 50×50 grid against gray out-of-field space, and shows the compact upper-right toolbar. Keep this story view-only so later stories can add editing and scheduled evolution without replacing the surface.

## Boundaries & Constraints

**Always:** Use a row-major byte field with validating dimensions, zero-based half-open cell geometry, black dead cells, white live cells, gray `#808080` out-of-field space, and 1 logical-pixel gray grid lines when cells are at least 4 logical pixels. Keep raylib/raygui in presentation, define `RAYGUI_IMPLEMENTATION` in exactly one source file, use the existing CMake target direction, and keep toolbar controls at least 32 logical pixels tall in the upper-right.

**Block If:** A required pinned raylib/raygui API cannot compile through the existing dependency targets, or the approved layer direction requires a product or architecture decision not present in the loaded sources.

**Never:** Add simulation timing, Life rules, editing commands, camera navigation, sessions, Bank, Settings, persistence, audio, a full-field texture, or per-cell UI objects. Do not modify `sprint-status.yaml`.

## I/O & Edge-Case Matrix

| Scenario | Input / State | Expected Output / Behavior | Error Handling |
|----------|---------------|-----------------------------|----------------|
| HAPPY_PATH | `Field::create(50, 50)` and a ready raylib window | The application renders all 2,500 in-field cells, with exact grid alignment and toolbar controls in the upper-right | No error expected |
| OUT_OF_BOUNDS_DOMAIN | Coordinate at or beyond width/height | The field reports no cell at that coordinate and does not mutate its byte buffer | Return a typed failure for invalid dimensions; out-of-bounds cell access is a no-op/false result |
| INVALID_DIMENSIONS | Zero, over-limit, or multiplication-overflow dimensions | No invalid Field is constructed or allocated | Return `ErrorCode::InvalidArgument` |

## Code Map

- `src/foundation/result.hpp:9` and `src/foundation/error-code.hpp:7` -- existing typed result/error contract reused by validating domain factories.
- `src/domain/CMakeLists.txt:1` -- currently interface-only; convert it to compile the new domain field sources while retaining its foundation dependency.
- `src/domain/field/field-dimensions.hpp/.cpp` -- validating 1..4096 dimensions, total-cell limit, and overflow-checked row-major count.
- `src/domain/field/cell-coordinate.hpp` and `src/domain/field/field.hpp/.cpp` -- zero-based coordinate value and dense dead/live byte storage with bounds-safe access.
- `src/presentation/CMakeLists.txt:1` -- currently interface-only; make it compile the Field Screen, renderer, toolbar, and the sole raygui implementation unit.
- `src/presentation/rendering/field-renderer.hpp/.cpp` -- calculate the logical field rectangle, fill gray outside it, draw black/white cells, and draw the conditional grid without a full-field texture.
- `src/presentation/ui/toolbar.hpp/.cpp` -- render the compact upper-right Live/Die/Pause/Highlight/Bank/Move/+/−/Exit controls and the textual Running state; controls are display-only in this story.
- `src/presentation/screens/field-screen.hpp/.cpp` -- compose field rendering and toolbar rendering in one Field-owned surface.
- `src/presentation/application/raylib-application.hpp/.cpp` -- own window creation, the render loop, and clean window shutdown without audio or gameplay rules.
- `src/presentation/raylib/raygui-implementation.cpp` -- the only `RAYGUI_IMPLEMENTATION` translation unit.
- `src/bootstrap/main.cpp:1` and `src/bootstrap/CMakeLists.txt:1` -- construct the default 50×50 Field and run the presentation composition root.
- `tests/unit/domain/field-dimensions-test.cpp` and `tests/unit/domain/field-test.cpp` -- deterministic validation, dimensions, default dead state, and boundary no-op coverage; no window or raylib initialization.
- `tests/CMakeLists.txt:1` -- register the new Catch2 domain tests alongside the existing foundation test.

## Tasks & Acceptance

**Execution:**
- `src/domain/field/*` and `src/domain/CMakeLists.txt` -- implement and compile the validating dense finite Field -- provide a safe model for the renderer and later stories.
- `src/presentation/rendering/*`, `src/presentation/ui/*`, `src/presentation/screens/*`, and `src/presentation/raylib/raygui-implementation.cpp` -- add the logical 50×50 Field surface and compact toolbar -- make the MVP visible with the approved palette and boundaries.
- `src/presentation/application/*`, `src/presentation/CMakeLists.txt`, `src/bootstrap/main.cpp`, and `src/bootstrap/CMakeLists.txt` -- wire a 1280×720 raylib render loop through the layer graph -- make the harness launch and close cleanly.
- `tests/unit/domain/*` and `tests/CMakeLists.txt` -- cover valid/default/boundary/invalid field behavior -- prevent invalid storage and boundary regressions without opening a window.

**Acceptance Criteria:**
- Given the Field MVP harness is launched, when the Field Screen renders, then a 50×50 finite field and compact upper-right controls with a visible Running state are present in the 1280×720 logical client area.
- Given a cell is rendered in the viewport, when its state is dead or live, then it occupies an exact aligned cell rectangle colored black or white respectively, and any visible area outside the finite field is gray rather than a third cell state.
- Given the MVP boundary, when the harness is run, then Bank, persistence, Settings, camera navigation, input editing, and simulation timing are not required or initialized, and the process exits cleanly when the window closes.

## Design Notes

The default layout uses an integer logical cell size computed from the viewport, leaving gray space around the finite board when appropriate; at 1280×720 the 50×50 board is a compact left-side matrix and the toolbar is isolated in the upper-right. The toolbar uses raygui mechanics and approved flat gray presentation, but its commands are intentionally inert until their owning stories add input and state transitions. A dead initial field still renders the complete grid; no sample live cells are invented for this view-only story.

## Verification

**Commands:**
- `cmake --preset dev-debug` -- expected: configure succeeds with the existing pinned dependencies and the new domain/presentation sources.
- `cmake --build --preset dev-debug` -- expected: domain tests and the `life-game` harness compile with the project warning policy.
- `ctest --preset dev-debug --output-on-failure` -- expected: foundation and field tests pass without a window, audio, real user-data access, or wall-clock timing.
- `cmake --preset dev-release && cmake --build --preset dev-release` -- expected: the harness compiles in Release.
- `git diff --check` -- expected: no whitespace errors.

**Manual checks (if no CLI):**
- Launch `life-game` on macOS and Linux and confirm a lined 50×50 black field on gray space, white/black palette support, upper-right controls, no audio initialization, and clean window-close exit; record platform/toolchain evidence without claiming the other platform from a local run.

## Review Triage Log

### 2026-08-26 — Review pass
- intent_gap: 0
- bad_spec: 0
- patch: 4: (high 0, medium 2, low 2)
- defer: 0
- reject: 20: (high 0, medium 5, low 15)
- addressed_findings:
  - `[medium]` `[patch]` Made the new static domain and presentation targets own their public include paths and C++23 feature requirements so their source files do not rely on accidental transitive configuration.
  - `[low]` `[patch]` Kept the raygui include dependency private to the presentation implementation target.
  - `[low]` `[patch]` Added legal minimum and maximum side-dimension tests for the validating factory.
  - `[medium]` `[patch]` Added full dead-buffer, independent row-major index, and unrelated-cell preservation assertions.

### 2026-08-26 — Review pass
- intent_gap: 0
- bad_spec: 0
- patch: 10: (high 3, medium 4, low 3)
- defer: 0
- reject: 5: (high 0, medium 3, low 2)
- addressed_findings:
  - `[low]` `[patch]` Removed an avoidable copy of the 50x50 cell buffer when transferring the validated Field into the application.
  - `[medium]` `[patch]` Removed the ambiguous `cellAt` API so out-of-bounds reads report failure through `readCell` instead of appearing dead.
  - `[high]` `[patch]` Limited cell drawing to the visible rows and columns so legal maximum fields do not issue millions of off-screen draw calls.
  - `[medium]` `[patch]` Moved the right and bottom grid boundaries inside the half-open field rectangle so they do not overwrite out-of-field pixels.
  - `[high]` `[patch]` Set the toolbar status label to the approved white text color so the visible Running state meets its contrast contract.
  - `[medium]` `[patch]` Kept Live's active fill as well as its two-pixel outline while rendering the persistent selected mode.
  - `[medium]` `[patch]` Replaced the unsupported Unicode minus glyph with the default-font-safe ASCII label.
  - `[low]` `[patch]` Corrected toolbar padding to the confirmed four-logical-pixel token.
  - `[high]` `[patch]` Enabled raylib high-DPI window support before window creation so logical sizing is scaled once by the platform layer.
  - `[low]` `[patch]` Added exact three-versus-four-pixel grid-threshold coverage and asserted the toolbar padding geometry.

### 2026-08-26 — Review pass
- intent_gap: 0
- bad_spec: 0
- patch: 0
- defer: 0
- reject: 21: (high 0, medium 10, low 11)
- addressed_findings:
  - none

## Auto Run Result

Status: done

Summary: Fresh follow-up review confirmed the validating finite Field, 1280x720 high-DPI raylib application, clipped cell/grid renderer, and upper-right view-only toolbar remain aligned with the story. No new code patches or deferred-work entries were warranted.

Files changed:
- `_bmad-output/implementation-artifacts/spec-1-1-view-the-field-mvp.md` -- recorded this follow-up review, verification evidence, and final status.

Review findings breakdown:
- Patches applied: 0.
- Items deferred: 0.
- Items rejected: 21, comprising orchestrator-owned board observations, unsupported-input hypotheticals, cross-cutting logging work not introduced by this story, and verification suggestions already covered proportionately by deterministic tests, code inspection, and a native runtime launch.

Follow-up review recommendation: false. Patched findings were high 0, medium 0, low 0; score `3 x 0 + 1 x 0 = 0`.

Verification performed:
- `cmake --preset dev-debug` -- passed with AppleClang and pinned raylib 6.0 configuration.
- `cmake --build --preset dev-debug` -- passed; no work remained.
- `ctest --preset dev-debug --output-on-failure` -- passed, 19/19 tests.
- `cmake --preset dev-release && cmake --build --preset dev-release` -- passed; no work remained.
- `git diff --check` -- passed.
- Native macOS runtime launch -- raylib initialized a 1280x720 logical screen with a 2560x1440 high-DPI render surface, audio remained unloaded, and the process was terminated cleanly after the launch check. Automated desktop capture was unavailable in the current session.

Residual risks: Linux build/runtime and pixel-level visual evidence were not reproduced in this macOS-only follow-up pass; the repository's CI and documented cross-platform manual release checks remain the owning gates.

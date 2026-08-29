---
title: '3.2 Move the Camera'
type: 'feature'
created: '2026-08-29'
status: 'done'
baseline_revision: 'cc2436b8c428d2bde999481561cdb4c343af0bf8'
review_loop_iteration: 0
followup_review_recommended: true
context:
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260828-234508-b520/worktrees/3-2-move-the-camera/_bmad-output/project-context.md'
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260828-234508-b520/worktrees/3-2-move-the-camera/_bmad-output/implementation-artifacts/epic-3-context.md'
warnings: []
deferred: []
---

<intent-contract>

## Intent

**Problem:** The Field currently fits the whole finite board to the viewport and has no camera state or Move interaction, so users cannot navigate large or rectangular sessions without painting cells.

**Approach:** Add one presentation-owned camera and a persistent Move mode to the existing Field flow. Use the same camera-aware render plan for drawing and coordinate conversion, route typed pan commands through the existing input boundary, and keep finite field data untouched.

## Boundaries & Constraints

**Always:** Keep camera values in logical cell units and clamp them so the viewport intersects at least one in-field cell. Fill visible out-of-field space gray; never simulate, edit, or persist it. A Move drag changes only the camera, captures until release, and preserves every field byte. Toolbar and modal ownership wins over field gestures, and out-of-field presses do nothing. Render and input mapping share one transform with half-open cell bounds and normalized logical coordinates. Reset camera and mode for each newly opened session while preserving simulation-before-input ordering.

**Block If:** The existing Epic 3 contract cannot determine a bounded pan, ownership, or coordinate-mapping behavior without a new product or architecture decision.

**Never:** Change domain Field geometry, wrapping, cell state, or simulation rules; add a second moving-state source, persistence, per-cell UI objects, full-field textures, callbacks, input queues, or changes to `sprint-status.yaml`.

## I/O & Edge-Case Matrix

| Scenario | Input / State | Expected Output / Behavior | Error Handling |
|----------|--------------|-----------------------------|----------------|
| SELECT_MOVE | Field open; Move toolbar control pressed | Move becomes the persistent active mode and is visibly identified | No error expected |
| PAN_FIELD | Move active; press, drag, and release inside the field | Camera pans within bounds; no paint command is emitted and field bytes are byte-for-byte unchanged | No error expected |
| PAN_BOUNDARY | Drag on rectangular, oversized, or already-clamped field | Camera clamps while at least one in-field cell remains visible | No error expected |
| REJECT_OWNERSHIP | Modal, toolbar, or gray out-of-field pointer input | No camera or field mutation; modal/toolbar input is not passed through | No error expected |
| MAP_TRANSFORM | Rendered cell and logical pointer use the same camera state | The pointer resolves to the cell shown at that location, with half-open boundaries preserved | No error expected |

</intent-contract>

## Code Map

- `src/presentation/rendering/field-renderer.hpp:9-29` and `.cpp:16-108` -- current fit-to-viewport `FieldRenderPlan`, gray fill, cell/grid drawing, and visible-range calculations; extend this shared plan with camera state rather than duplicating drawing math.
- `src/presentation/camera/coordinate-converter.hpp:10-23` and `.cpp:9-56` -- authoritative logical-point/cell conversion and cell-center calculation; consume the camera-aware render plan so input and rendering stay aligned.
- `src/application/commands/field-command.hpp:9-22` -- current Live/Die paint axis and paint variants; add the Move selection/pan command on the same typed application boundary without a parallel boolean state.
- `src/presentation/input/input-router.hpp:20-58` and `.cpp:66-178` -- toolbar/modal precedence and paint gesture capture; route Move selection and logical pointer deltas through a release-bounded camera gesture while retaining paint rasterization.
- `src/presentation/ui/toolbar.hpp:36-65` and `.cpp:93-204` -- nine-control upper-right layout already labels Move at index 5; expose its index, active styling, and status text without changing pointer-target sizing.
- `src/presentation/screens/field-screen.*` and `src/presentation/application/raylib-application.*:31-181` -- thread one camera through rendering/input, preserve simulation → input → render order, normalize pointer coordinates once, and reset camera/mode on session open/close.
- `src/presentation/camera/camera-controller.*` -- new presentation camera owner for logical-cell pan and intersection-preserving clamping; no domain or persistence dependency.
- `tests/unit/presentation/{coordinate-converter,field-renderer,input-router,toolbar,raylib-application}-test.cpp`, a focused camera test, and `tests/CMakeLists.txt:102-145` -- extend deterministic seams for transformed mapping, clamp behavior, ownership/capture, Move persistence, byte preservation, session reset, and target registration.

## Tasks & Acceptance

**Execution:**
- `src/presentation/camera/camera-controller.*` -- implement logical-cell camera state, bounded pan, and reset semantics -- keep the viewport intersecting at least one field cell for rectangular and oversized fields.
- `src/presentation/rendering/field-renderer.*` and `src/presentation/camera/coordinate-converter.*` -- share camera-aware render geometry for visible cells, gray out-of-field space, grid behavior, and half-open pointer mapping -- prevent render/input drift.
- `src/application/commands/field-command.hpp`, `src/presentation/input/input-router.*`, and `src/presentation/ui/toolbar.*` -- add persistent Move selection and release-bounded pan input with existing ownership precedence -- never emit paint commands in Move.
- `src/presentation/screens/field-screen.*` and `src/presentation/application/raylib-application.*` -- own and thread the camera, normalize logical input, apply pan commands, and reset navigation on session transitions -- preserve authoritative mode and loop phase ordering.
- `tests/unit/presentation/*` and `tests/CMakeLists.txt` -- add deterministic regression coverage for the matrix, transformed cells, clamping, capture/release, modal/toolbar/gray rejection, byte preservation, and session reset -- run without opening a window.

**Acceptance Criteria:**
- Given a Field is open, when the Move toolbar control is selected, then Move remains active until another field mode is selected and the toolbar visibly identifies it.
- Given Move is active, when the pointer is dragged across the Field, then only the camera changes, no paint command is emitted, and all field bytes remain unchanged.
- Given a Move drag crosses the toolbar, modal surface, gray space, or release edge, when ownership or release is observed, then input is not applied to a lower owner and the camera gesture ends cleanly.
- Given any legal rectangular or oversized Field, when the camera is panned to either extreme, then the clamped viewport still intersects at least one in-field cell and out-of-field space remains gray.
- Given a camera-aware render plan, when a logical pointer is converted or a cell center is projected, then both operations use the same transform, preserve half-open boundaries, and never map gray space to a cell.
- Given a session is created or opened, when its Field Screen becomes active, then the camera starts at its default position and Move/Live state is reset without changing the session’s dimensions or cells.

## Spec Change Log

## Review Triage Log

### 2026-08-29 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 9: (high 0, medium 3, low 6)
- defer: 0
- reject: 10: (high 0, medium 2, low 8)
- addressed_findings:
  - `[medium]` `[patch]` Snapped the shared camera-aware field rectangle to display pixels so rendered boundaries, hit-testing, and cell-center projection cannot diverge at fractional pan offsets.
  - `[medium]` `[patch]` Made camera mutation finite and bounded, rejecting non-finite or overflowing pan candidates before state changes.
  - `[low]` `[patch]` Removed mutable camera-controller access from application/screen surfaces and routed pans through the Field Screen owner.
  - `[low]` `[patch]` Removed unused duplicate field-command aliases/variants and kept Move selection on the existing typed mode boundary.
  - `[medium]` `[patch]` Prevented a captured paint gesture from emitting paint when the mode transitions to Move.
  - `[low]` `[patch]` Added non-default-camera input/mapping coverage and display-rectangle consistency assertions.
  - `[low]` `[patch]` Asserted stored camera state remains clamped after extreme pans.
  - `[low]` `[patch]` Asserted Move remains active after drag and release.
  - `[low]` `[patch]` Added the direct `<cstddef>` include required by the renderer’s public `std::size_t` declarations.

### 2026-08-29 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 6: (high 0, medium 1, low 5)
- defer: 0
- reject: 25: (high 0, medium 5, low 20)
- addressed_findings:
  - `[medium]` `[patch]` Kept captured Move drags tracking in-viewport gray coordinates so reversing at a camera boundary still produces logical pan deltas.
  - `[low]` `[patch]` Added initial-gray Move rejection and boundary-reversal regression coverage.
  - `[low]` `[patch]` Added camera-aware draw-call coverage for the visible range and transformed cell positions.
  - `[low]` `[patch]` Added application-level coverage that edits after navigation target the cell shown by the camera.
  - `[low]` `[patch]` Strengthened Move pan coverage to assert exact logical deltas.
  - `[low]` `[patch]` Recorded concrete verification outcomes in the auto-run result.

### 2026-08-29 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 7: (high 0, medium 2, low 5)
- defer: 0
- reject: 23: (high 0, medium 9, low 14)
- addressed_findings:
  - `[medium]` `[patch]` Preserved the selected Move mode when pausing and resuming; resuming from Die still returns to Live.
  - `[medium]` `[patch]` Added post-snap camera validation so a legal thin field retains a visible pixel at extreme camera values.
  - `[low]` `[patch]` Widened field-origin geometry intermediates to avoid signed overflow while calculating the presentation layout.
  - `[low]` `[patch]` Asserted the application-level pan direction and exact logical magnitude for a known Move drag.
  - `[low]` `[patch]` Extended toolbar coverage to prove Move is the sole active field mode in running and paused states.
  - `[low]` `[patch]` Added Move ownership coverage where the toolbar overlaps an in-field coordinate.
  - `[low]` `[patch]` Verified the final visible row and column are drawn at the transformed field boundary.

## Design Notes

Keep camera geometry as the single source of truth: rendering, cell hit-testing, and cell-center projections should all derive from one camera-aware plan. Panning in logical cell units avoids DPI-dependent behavior; the presentation may show gray beyond finite bounds, but no out-of-field coordinate becomes field data.

## Verification

**Commands:**
- `cmake --preset dev-debug` -- expected: configuration succeeds with camera sources and tests registered.
- `cmake --build --preset dev-debug` -- expected: all targets compile under the warning policy.
- `ctest --preset dev-debug --output-on-failure` -- expected: all existing and new camera/input/render tests pass.
- `cmake --preset dev-release && cmake --build --preset dev-release` -- expected: Release configuration and build succeed.
- `ctest --preset dev-release --output-on-failure` -- expected: the Release suite passes.
- `git diff --check` -- expected: no whitespace errors.

## Auto Run Result

Status: done

Summary: Completed Story 3.2 camera navigation with a presentation-owned logical-cell camera, bounded panning, persistent Move mode, ownership-safe pointer capture, shared render and mapping geometry, gray out-of-field space, and session navigation reset. The final review pass also preserves Move across pause/resume, keeps thin fields visible after display-pixel snapping, hardens wide geometry intermediates, and adds regression coverage for exact pan forwarding, toolbar ownership, sole-active Move styling, and far-edge rendering. Field dimensions, cell bytes, simulation semantics, persistence scope, and orchestrator bookkeeping remain unchanged by the implementation.

Files changed:

- `src/application/commands/field-command.hpp` -- adds Move mode and the typed pan command.
- `src/presentation/camera/camera-controller.*` -- owns finite, bounded logical camera state and reset/pan behavior.
- `src/presentation/camera/coordinate-converter.*` -- maps logical pointers and cell centers through shared camera-aware geometry.
- `src/presentation/input/input-router.*` -- adds persistent Move capture, typed pan commands, ownership precedence, and gray-space drag tracking.
- `src/presentation/rendering/field-renderer.*` -- renders camera-visible cells, gray out-of-field space, grid lines, snapped display geometry, and bounded field layout.
- `src/presentation/ui/toolbar.*` -- exposes Move’s active control and status cue.
- `src/presentation/screens/field-screen.*` and `src/presentation/application/raylib-application.*` -- own, apply, render, and reset camera navigation while preserving loop ordering.
- `src/presentation/CMakeLists.txt` and `tests/CMakeLists.txt` -- register camera implementation and tests.
- `tests/unit/presentation/camera-controller-test.cpp` -- covers clamping, non-finite input, transformed mapping, Move capture, ownership, thin fields, gray-space reversal, and mode selection.
- `tests/unit/presentation/field-renderer-test.cpp` -- covers camera-aware visible-cell draw positions and far-edge coverage.
- `tests/unit/presentation/raylib-application-test.cpp` and `raylib-application-story-test.cpp` -- cover application-level Move behavior, byte preservation, exact pan forwarding, persistence, post-pan mapping, and session reset.
- `tests/unit/presentation/toolbar-test.cpp` -- covers Move’s active styling in running and paused states.

Review findings breakdown: 7 patches applied (0 high, 2 medium, 5 low), 0 items deferred, and 23 items rejected (0 high, 9 medium, 14 low).

Follow-up review recommendation: true (patched findings: high 0, medium 2, low 5; score `3 × 2 + 5 = 11`). Remaining recommendations are non-blocking coverage or future contract-hardening items; no intent gap or product decision was found.

Verification performed:

- `cmake --preset dev-debug` -- passed.
- `cmake --build --preset dev-debug` -- passed.
- `ctest --preset dev-debug --output-on-failure` -- passed, 148/148.
- `cmake --preset dev-release` -- passed.
- `cmake --build --preset dev-release` -- passed.
- `ctest --preset dev-release --output-on-failure` -- passed, 148/148.
- `git diff --check` -- passed.
- The installed `clang-format` could not parse the repository’s `Standard: c++23` setting; final modified C++ follows the repository’s existing formatting and the whitespace check passed.

Residual risks: native raygui draw output, OS-specific DPI behavior, and sanitizer/windowed verification remain outside the deterministic test harness. The orchestrator-owned `sprint-status.yaml` delta was observed but not modified, staged, committed, or reverted.

---
title: '3.3 Change Discrete Zoom'
type: 'feature'
created: '2026-08-29'
status: 'done'
baseline_revision: 'ac05435bd33a900bd928797bcefc4ae4c7798412'
review_loop_iteration: 0
followup_review_recommended: false
context:
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260828-234508-b520/worktrees/3-3-change-discrete-zoom/_bmad-output/project-context.md'
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260828-234508-b520/worktrees/3-3-change-discrete-zoom/_bmad-output/implementation-artifacts/epic-3-context.md'
warnings: []
deferred: []
---

<intent-contract>

## Intent

**Problem:** The Field has camera panning but no zoom, so large and small finite sessions cannot be viewed at useful discrete scales and the existing `+`/`−` Toolbar controls do nothing.

**Approach:** Extend the presentation-owned camera and shared render plan with the seven approved zoom levels, route one zoom command per Toolbar press, and adjust the camera around the in-field pointer cell or viewport center while leaving field data and geometry unchanged.

## Boundaries & Constraints

**Always:** Support exactly `50%`, `75%`, `100%`, `150%`, `200%`, `300%`, and `400%` in that order; default and reset to `100%`; move exactly one level per press and clamp at either endpoint. Zoom is presentation-only and must preserve every field byte, fixed dimensions, logical cell coordinates, simulation order, and the camera rule that at least one in-field cell remains visible. Preserve the logical field position under an in-field pointer when zooming; use the logical position under the viewport center when the pointer is outside the field or owned by the Toolbar. Keep rendering, coordinate conversion, visible-cell selection, gray out-of-field fill, and grid-threshold evaluation on one camera-aware plan, with logical coordinates normalized before routing and half-open cell bounds retained. Toolbar and modal ownership wins over field input, and held input must not repeat a zoom request.

**Block If:** The approved level set, presentation-only ownership, anchor rule, or finite-field visibility invariant cannot be implemented without changing product or architecture decisions.

**Never:** Change domain `Field` dimensions, cell state, wrapping, or simulation rules; persist zoom before Epic 5; add wheel/keyboard shortcuts, callbacks, input queues, per-cell UI objects, full-field textures, or a second camera/zoom state; modify `sprint-status.yaml`.

## I/O & Edge-Case Matrix

| Scenario | Input / State | Expected Output / Behavior | Error Handling |
|----------|--------------|-----------------------------|----------------|
| ZOOM_IN | Field open; `+` pressed once | Zoom advances one approved level; camera keeps the anchor cell under the pointer or uses viewport center | No error expected |
| ZOOM_OUT | Field open; `−` pressed once | Zoom retreats one approved level with the same anchor rule | No error expected |
| ZOOM_LIMIT | At 50% or 400%; corresponding control pressed | Level and camera remain valid; no wraparound | No error expected |
| ZOOM_OWNERSHIP | Modal open, Toolbar-owned pointer, or held button | No lower-owner paint/pan command and no repeated zoom | No error expected |
| ZOOM_BOUNDARY | Rectangular/oversized field or pointer near an edge | Camera clamps while an in-field cell remains visible and gray space remains presentation-only | No error expected |

</intent-contract>

## Code Map

- `src/presentation/camera/camera-controller.hpp:7-35` and `.cpp:9-39` -- current logical `CameraState`, clamp/reset owner, and pan mutation; extend this owner with the ordered zoom state and anchor-preserving one-step changes.
- `src/presentation/rendering/field-renderer.hpp:19-59` and `.cpp:23-204` -- shared fit-to-viewport cell geometry, field rectangle, visible range, gray fill, and grid threshold; scale the 100% geometry through zoom, use pixel-safe edges, and clamp against zoomed dimensions.
- `src/presentation/camera/coordinate-converter.cpp:16-76` -- authoritative pointer/cell and cell-center transforms; consume the zoomed render plan so painting and anchor round trips stay aligned.
- `src/application/commands/field-command.hpp:5-29` -- typed field command boundary; add a directional zoom request without introducing presentation dependencies.
- `src/presentation/ui/toolbar.hpp:20-66` and `.cpp:149-206` -- controls 6/7 already render `+`/`−`; add named indices and retain the existing 32px layout and momentary styling.
- `src/presentation/input/input-router.hpp:15-68` and `.cpp:83-244` -- existing modal/Toolbar precedence and press-only command routing; emit one zoom request and prevent gesture fall-through.
- `src/presentation/screens/field-screen.*` and `src/presentation/application/raylib-application.*:76-208` -- thread zoom through the Field Screen/application, preserve simulation → input → render order, expose state for tests, and reset navigation on session transitions.
- `tests/unit/presentation/{field-renderer,coordinate-converter,camera-controller,input-router,toolbar,raylib-application,raylib-application-story}-test.cpp` and `tests/CMakeLists.txt:102-145` -- extend window-free seams for scale table, anchors, clamps, ownership, one-shot controls, byte preservation, and session reset; the standalone renderer target must remain linkable.

## Tasks & Acceptance

**Execution:**
- `src/presentation/camera/camera-controller.*` and `src/presentation/rendering/field-renderer.*` -- add discrete zoom state, scaled shared geometry, anchor-preserving camera adjustment, visibility clamping, and pixel-safe rendering -- keep zoom presentation-only and finite.
- `src/application/commands/field-command.hpp`, `src/presentation/input/input-router.*`, and `src/presentation/ui/toolbar.*` -- add directional zoom commands and named `+`/`−` routes -- preserve ownership precedence and one request per press.
- `src/presentation/camera/coordinate-converter.*`, `src/presentation/screens/field-screen.*`, and `src/presentation/application/raylib-application.*` -- thread zoom through mapping, rendering, application input, and navigation reset -- keep cell targeting and loop ordering authoritative.
- `tests/unit/presentation/*` and `tests/CMakeLists.txt` -- add deterministic regression coverage for all levels, endpoints, both anchor modes, rectangular/oversized fields, modal/Toolbar ownership, held input, unchanged bytes, and reset -- run without opening a window.

**Acceptance Criteria:**
- Given a Field at any approved level, when `+` or `−` is pressed once, then the level changes by exactly one adjacent level, clamps at 50%/400%, and does not repeat while held.
- Given an in-field pointer at zoom time, when the level changes, then the same logical cell remains under that pointer unless finite camera bounds require clamping; an outside-field or Toolbar-owned pointer anchors at the viewport center.
- Given any legal rectangular or oversized Field, when zoom reaches either endpoint, then the clamped render plan intersects at least one in-field cell and renders all non-field area gray.
- Given a zoomed render plan, when a cell is rendered, mapped, or projected to its center, then all operations use the same scaled geometry and preserve half-open boundaries without mapping gray space to a cell.
- Given a session is created, opened, or closed and reopened, when navigation state is inspected, then zoom is 100%, camera navigation is reset, and field dimensions/cells are unchanged.

## Spec Change Log

## Review Triage Log

### 2026-08-29 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 11: (high 0, medium 3, low 8)
- defer: 0
- reject: 11: (high 0, medium 4, low 7)
- addressed_findings:
  - `[medium]` `[patch]` Reconciled snapped zoom anchors for up to eight adjustments and stopped only after the original pointer cell remained selected or the center fallback converged.
  - `[medium]` `[patch]` Derived the display rectangle’s trailing edges from its snapped origin so fractional cell rasterization and field bounds share the same edges.
  - `[medium]` `[patch]` Clamped renderer screen positions and pixel spans before integer conversion to keep extreme legal camera states defined.
  - `[low]` `[patch]` Added fractional-zoom coordinate tests for cell centers, half-open boundaries, and gray-space rejection.
  - `[low]` `[patch]` Added fractional-zoom renderer tests proving adjacent cell draw edges remain contiguous.
  - `[low]` `[patch]` Added zoom-out and near-edge pointer-anchor regression coverage.
  - `[low]` `[patch]` Extended application coverage across every approved level, endpoint clamping, held input, and field-byte preservation.
  - `[low]` `[patch]` Added grid-threshold coverage for zoomed transitions.
  - `[low]` `[patch]` Added reset coverage after a session was actually zoomed.
  - `[low]` `[patch]` Added invalid-level normalization coverage for the existing approved-level fallback.
  - `[low]` `[patch]` Centralized scaled-cell-size calculation and added the direct `<cstddef>` include for the camera API.

### 2026-08-29 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 10: (high 0, medium 4, low 6)
- defer: 0
- reject: 23
- addressed_findings:
  - `[medium]` `[patch]` Preserved distinct 50% and 75% logical extents when the fit-to-viewport base cell is one display pixel, with fractional-scale coverage.
  - `[medium]` `[patch]` Kept thin finite fields visible at subpixel zoom by retaining pixel-safe trailing edges and endpoint coverage.
  - `[medium]` `[patch]` Aligned coordinate conversion with the renderer’s snapped half-open edges and rejected artificial snapped padding as gray space.
  - `[medium]` `[patch]` Selected pointer anchors only from actual mapped in-field cells, preventing snapped padding from becoming an anchor.
  - `[low]` `[patch]` Made invalid zoom directions a no-op and expanded non-default zoom movement coverage.
  - `[low]` `[patch]` Expanded held Toolbar ownership assertions and application coverage for center anchoring and post-zoom editing.

### 2026-08-29 — Review pass

- intent_gap: 0
- bad_spec: 0
- patch: 1: (high 0, medium 1, low 0)
- defer: 0
- reject: 18
- addressed_findings:
  - `[medium]` `[patch]` Routed a fresh Toolbar press before captured-gesture handling so a stale field gesture cannot swallow zoom or another Toolbar command; added a regression test.

## Design Notes

Use the existing 100% fit-to-viewport cell extent as the scale base. Keep the logical extent fractional when a percentage is not an integer multiple; rasterize cell edges from shared snapped boundaries so adjacent cells do not drift or gap, and evaluate the existing grid rule against the logical display extent. If clamping makes the exact anchor impossible at a finite edge, preserve the visibility invariant and the nearest valid camera state.

## Verification

**Commands:**
- `cmake --preset dev-debug` -- expected: configuration succeeds with zoom sources and tests registered.
- `cmake --build --preset dev-debug` -- expected: all targets compile under the warning policy.
- `ctest --preset dev-debug --output-on-failure` -- expected: all existing and zoom tests pass.
- `cmake --preset dev-release && cmake --build --preset dev-release` -- expected: Release configuration and build succeed.
- `ctest --preset dev-release --output-on-failure` -- expected: the Release suite passes.
- `git diff --check` -- expected: no whitespace errors.

## Auto Run Result

Status: done

Summary: Reviewed and hardened the presentation-owned discrete zoom implementation. A fresh Toolbar press now takes precedence over a stale captured field gesture, preserving one-shot zoom routing and preventing field-command fall-through.

Files changed:
- `src/application/commands/field-command.hpp` -- added the typed zoom direction and camera command.
- `src/presentation/camera`, `src/presentation/rendering`, and `src/presentation/camera/coordinate-converter.cpp` -- added discrete zoom state, anchor-aware navigation, shared scaled geometry, clamping, and mapping.
- `src/presentation/input/input-router.cpp`, `src/presentation/input/input-router.hpp`, and `src/presentation/ui/toolbar.hpp` -- routed Toolbar zoom commands with ownership precedence and named controls.
- `src/presentation/screens/field-screen.*` and `src/presentation/application/raylib-application.*` -- threaded zoom through the Field Screen/application and reset navigation on session transitions.
- `tests/unit/presentation/*` -- added zoom, geometry, ownership, application, and regression coverage, including the captured-gesture Toolbar case.

Review findings breakdown: 1 patch applied (1 medium), 0 items deferred, 18 items rejected. The orchestrator-owned `sprint-status.yaml` bookkeeping change was left untouched.

Follow-up review recommendation: false (patched findings: 0 high, 1 medium, 0 low; score 3).

Verification performed:
- `cmake --preset dev-debug` -- passed.
- `cmake --build --preset dev-debug` -- passed.
- `ctest --preset dev-debug --output-on-failure` -- passed, 167/167 tests.
- `cmake --preset dev-release` -- passed.
- `cmake --build --preset dev-release` -- passed.
- `ctest --preset dev-release --output-on-failure` -- passed, 167/167 tests.
- `git diff --check` -- passed.
- The committed `.clang-format` could not be loaded by the installed clang-format 22 because it rejects `Standard: c++23`; source formatting remains consistent with the existing project style.

Residual risks: live windowed visual evidence and cross-platform DPI behavior were not exercised by the window-free test suite. The orchestrator-owned `sprint-status.yaml` change remains outside this run's write scope.

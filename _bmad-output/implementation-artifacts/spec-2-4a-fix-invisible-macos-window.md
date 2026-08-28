---
title: '2.4a Fix Invisible macOS Window'
type: 'bugfix'
created: '2026-08-27'
status: 'done'
baseline_revision: '63879e50bf09699855ae2055cfdca92480d652a0'
review_loop_iteration: 0
followup_review_recommended: false
context:
  - '/Users/bogdan/core-dance/ai/life-game/_bmad-output/project-context.md'
  - '/Users/bogdan/core-dance/ai/life-game/_bmad-output/game-architecture.md'
  - '/Users/bogdan/core-dance/ai/life-game/_bmad-output/planning-artifacts/implementation-readiness-report-2026-08-24.md'
warnings: []
deferred: []
---

<intent-contract>

## Intent

**Problem:** On macOS with a Retina display, the current Life Game launch reports successful raylib/GLFW initialization but WindowServer exposes only 0×0 Life Game windows, so no usable window appears. The suspected `FLAG_WINDOW_HIGHDPI` configuration may be involved, but that is only a hypothesis.

**Approach:** Reproduce the baseline with native WindowServer evidence, run controlled window-configuration experiments that isolate the actual cause, then apply the smallest portable correction at the raylib window boundary. Preserve logical client coordinates and validate the Retina framebuffer rather than accepting raylib’s readiness flag as proof.

## Boundaries & Constraints

**Always:** Record baseline raylib/GLFW logs and a `CGWindowListCopyWindowInfo` observation tied to the launched process PID, including onscreen state, layer, and non-zero bounds. Test the existing HighDPI configuration against at least one controlled alternative before naming the cause. Keep the initial client area 1280×720 logical pixels, apply OS DPI scaling exactly once, and keep the renderer/input paths in logical pixels; a Retina framebuffer may be larger than the logical client area. Preserve normal close-button behavior and Escape-to-close. Run Debug and Release builds, all tests, and a native launch after the fix.

**Block If:** The evidence cannot distinguish window-creation configuration from an unrelated host/window-manager failure, or the correction requires upgrading/replacing the pinned raylib dependency or changing the approved logical-pixel contract.

**Never:** Treat `IsWindowReady()` or raylib logs alone as native proof; merge or implement Story 2.5 WIP; add a second DPI transform; change Life rules, input ownership, toolbar behavior, persistence, or unrelated stories; leave a platform-specific workaround that breaks the supported Linux path.

## I/O & Edge-Case Matrix

| Scenario | Input / State | Expected Output / Behavior | Error Handling |
|---|---|---|---|
| BASELINE_REPRODUCTION | Current Debug executable on Retina macOS | Controlled run reproduces or falsifies the reported 0×0 WindowServer result and identifies the relevant configuration variable | Preserve logs/evidence; do not infer cause from one run |
| CONFIGURATION_EXPERIMENT | Existing HighDPI setup and a minimal controlled alternative | The causal difference is demonstrated with raylib logs plus PID-scoped CGWindowList bounds | Reject inconclusive experiments and test the missing variable |
| RETINA_RENDERING | Fixed build on Retina macOS | One onscreen Life Game window has non-zero logical bounds, logical rendering remains 1280×720, and framebuffer dimensions reflect Retina scale without double-scaling input | Fail the acceptance check if bounds are zero, offscreen-only, or mapping is inconsistent |
| NORMAL_CLOSE | Visible fixed build, close action or Escape | The game loop exits and `CloseWindow()` completes with a normal process exit | Report non-zero/forced termination as a failure |

</intent-contract>

## Code Map

- `src/presentation/application/raylib-application.cpp:13-36` -- owns `SetConfigFlags`, `InitWindow(1280, 720, ...)`, readiness handling, frame loop, Escape/close polling, and `CloseWindow`; isolate the fix here or in a narrowly justified portable helper.
- `src/presentation/application/raylib-application.hpp` -- public application seam used by deterministic tests; keep window-specific state out of domain/application layers.
- `src/presentation/rendering/field-renderer.*` and `src/presentation/input/input-router.*` -- consume logical viewport/pointer coordinates; preserve their existing no-second-DPI-transform contract.
- `src/bootstrap/main.cpp:8-19` and `src/presentation/CMakeLists.txt` -- native executable composition and pinned raylib presentation boundary.
- `tests/unit/presentation/raylib-application-test.cpp`, `tests/unit/presentation/field-renderer-test.cpp`, and `tests/CMakeLists.txt` -- extend window-free seams where useful; native evidence must be a separate macOS launch check, not a mocked readiness assertion.
- `_bmad-output/project-context.md:128-136` and `_bmad-output/game-architecture.md:342-344` -- authoritative 1280×720 logical-pixel, one-time DPI, and macOS/Linux constraints.

## Tasks & Acceptance

**Execution:**
- `src/presentation/application/raylib-application.cpp` and any narrowly necessary presentation header/source -- reproduce the current lifecycle, isolate the causal window/DPI setting experimentally, and implement the smallest portable correction while preserving logical sizing, Retina framebuffer behavior, and normal close handling.
- `tests/unit/presentation/*` and build metadata only when required -- add deterministic regression coverage for the corrected configuration contract without initializing a native window in unit tests.
- Temporary macOS verification probe (not a product dependency) -- query `CGWindowListCopyWindowInfo` by launched PID/title and retain the native dimensions, onscreen/layer, responsiveness, close action, and Escape evidence in the run result.

**Acceptance Criteria:**
- Given the unmodified baseline on Retina macOS, when the controlled reproduction runs, then the actual cause of any 0×0 WindowServer window is identified from evidence rather than assumed from the HighDPI flag.
- Given the corrected Debug or Release executable on Retina macOS, when it launches, then CGWindowList evidence shows an onscreen non-zero-sized Life Game window; raylib initialization output may support but cannot replace that evidence.
- Given a Retina display, when the corrected window renders, then its client contract remains 1280×720 logical pixels, its framebuffer uses the platform scale correctly, and pointer/render coordinates are not scaled twice.
- Given the fixed executable, when the window close action or Escape is used, then the process closes normally and subsequent launches do not leave stale Life Game processes.
- Given the repository, when Debug and Release builds plus the full Debug CTest suite run, then configuration, compilation, and every discovered test pass.

## Design Notes

The native check must distinguish the WindowServer window rectangle from raylib’s internal readiness and screen-size values. A successful fix may retain HighDPI if the experiment proves it is correct; removing it is acceptable only if the comparison proves it is causal and the resulting Retina framebuffer/rendering contract remains correct.

## Verification

**Commands:**
- `cmake --preset dev-debug` -- expected: configure succeeds.
- `cmake --build --preset dev-debug` -- expected: Debug executable and tests compile.
- `cmake --preset dev-release` and `cmake --build --preset dev-release` -- expected: Release configure and compilation succeed.
- `ctest --preset dev-debug --output-on-failure` -- expected: every discovered Debug test passes.
- `git diff --check` -- expected: no whitespace errors.

**Manual checks:**
- Launch the fixed Debug executable natively on the Retina macOS desktop; use a PID-scoped CoreGraphics window probe to record onscreen, layer, width, height, responsiveness, and clean close via close action or Escape.

## Review Triage Log

### 2026-08-28 — Review pass
- intent_gap: 0
- bad_spec: 0
- patch: 1: (high 0, medium 0, low 1)
- defer: 0
- reject: 19
- addressed_findings:
  - `[low][patch]` Give `SUPPORT_CUSTOM_FRAME_CONTROL` a descriptive CMake cache help string; added the explanation while preserving the forced `OFF` value.

### 2026-08-28 — Review pass
- intent_gap: 0
- bad_spec: 0
- patch: 2: (high 0, medium 1, low 1)
- defer: 0
- reject: 20
- addressed_findings:
  - `[medium][patch]` Add deterministic regression coverage for the raylib frame-lifecycle setting; registered a window-free CTest that asserts the generated cache keeps `SUPPORT_CUSTOM_FRAME_CONTROL` `OFF`.
  - `[low][patch]` Make the intentional forced-off behavior explicit in the CMake cache help text.

## Auto Run Result

### Summary

The raylib 6.0 frame lifecycle correction remains enforced by setting `SUPPORT_CUSTOM_FRAME_CONTROL` to `OFF`, so `EndDrawing()` owns presentation, timing, and event polling while the existing 1280×720 logical and Retina rendering contract remains unchanged. This review pass added deterministic configuration regression coverage and clarified the cache option help text.

### Files changed

- `cmake/dependencies.cmake` -- explicitly describe and enforce standard raylib frame control.
- `tests/CMakeLists.txt` -- register the frame-lifecycle configuration check with CTest.
- `tests/cmake/raylib-frame-lifecycle-config-test.cmake` -- verify the generated CMake cache contains the required `OFF` setting without opening a native window.
- `_bmad-output/implementation-artifacts/spec-2-4a-fix-invisible-macos-window.md` -- record this review pass and final verification.

The orchestrator-owned `_bmad-output/implementation-artifacts/sprint-status.yaml` was not modified by this pass.

### Review findings breakdown

- Patches applied: 2 -- 1 medium-severity configuration regression-coverage gap and 1 low-severity help-text clarification.
- Items deferred: 0.
- Items rejected: 20, including lifecycle bookkeeping and suggestions already covered by the intent contract or the recorded native acceptance evidence.
- Follow-up review recommendation: `false` -- patched counts high 0, medium 1, low 1; score `4` (`3 × medium + low`), below the threshold of 5.

### Verification performed

- Baseline native Debug evidence recorded during the implementation pass: with `SUPPORT_CUSTOM_FRAME_CONTROL=ON`, raylib reported a HighDPI 1280×720 logical screen and 2560×1440 render dimensions, while the PID-scoped CoreGraphics probe reported Life Game windows with `onscreen=false` and `0×0` bounds.
- Controlled alternatives recorded during the implementation pass: removing `FLAG_WINDOW_HIGHDPI` did not change the zero-bounds result; native AppKit and raw GLFW probes were visible; raylib 6.0 inspection showed that custom frame control omits buffer swapping, frame timing, and event polling from `EndDrawing()`.
- `cmake --preset dev-debug` -- passed; the generated cache contains `SUPPORT_CUSTOM_FRAME_CONTROL:BOOL=OFF`.
- `cmake --build --preset dev-debug` -- passed.
- `ctest --preset dev-debug --output-on-failure` -- passed all 87/87 tests, including `life-game-raylib-frame-lifecycle-config`.
- `cmake --preset dev-release` -- passed; the generated cache also contains custom frame control `OFF`.
- `cmake --build --preset dev-release` -- passed.
- Fixed native Debug evidence recorded during the implementation pass: PID 6401 had `onscreen=true`, `layer=0`, and non-zero CoreGraphics bounds `1280×752` at `(116,115)`; raylib reported 1280×720 logical dimensions and a 2560×1440 Retina framebuffer.
- Escape closed the fixed process with exit code 0 and normal window/resource cleanup; a subsequent process check found no stale Life Game process.
- Source inspection confirmed `FLAG_WINDOW_HIGHDPI`, `InitWindow(1280, 720, ...)`, one `EndDrawing()` per frame, and no second DPI transform or competing swap/poll call remain changed.
- `git diff --check` -- passed.

### Residual risks

- Native Linux window execution was not available in this macOS-only environment; the correction is platform-neutral and the existing Linux CI path remains the cross-platform gate.
- The CoreGraphics probe remains temporary and was not added as a product dependency; native WindowServer evidence is recorded above while automated tests remain window-free.
- CMake emits raylib's existing macOS OpenGL deprecation developer warning; it does not prevent configure, build, rendering, or clean shutdown.

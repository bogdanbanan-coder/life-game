---
title: '1.3 Advance Conway Generations'
type: 'feature'
created: '2026-08-27'
status: 'done'
baseline_revision: '5719da04bd37e25aed8c19e5a0962f7cf71109ab'
review_loop_iteration: 0
followup_review_recommended: false
context:
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/1-3-advance-conway-generations/_bmad-output/project-context.md'
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/1-3-advance-conway-generations/_bmad-output/implementation-artifacts/epic-1-context.md'
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/1-3-advance-conway-generations/_bmad-output/game-architecture.md'
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-132320-8142/worktrees/1-3-advance-conway-generations/_bmad-output/planning-artifacts/story-specs-life-game-2026-08-25.md'
warnings: []
deferred: []
---

## Intent

**Problem:** The Field MVP can be painted but never evolves, so it cannot demonstrate deterministic Conway behavior.

**Approach:** Add framework-independent Conway generation evaluation and a bounded fixed-step scheduler. Integrate one iteration-start `steady_clock` snapshot into the existing raylib loop so complete generations run before input and the single final render.

## Boundaries & Constraints

**Always:** Use standard Conway rules on finite zero-based fields; treat out-of-bounds neighbors as permanently dead. Read one complete dense current buffer, write every cell to a separate next buffer, and publish only after the generation completes. Use a 250 ms interval, `steady_clock`, at most four due generations per iteration, and retain only the sub-interval remainder after discarding excess whole-interval backlog. Keep simulation on the main thread and preserve the phase order: clock snapshot/accumulation, simulation batch, raylib input translation, command execution, one render.

**Block If:** The approved layer direction or pinned C++23 toolchain cannot support a framework-independent domain simulation and synchronous application scheduler without a new architecture decision.

**Never:** Add alternate Life rules, wrapping or field expansion, configurable timing, pause/UI state, worker threads, callbacks/events/replay queues, per-cell objects, intermediate renders, or persistence. Do not modify `sprint-status.yaml`.

## I/O & Edge-Case Matrix

| Scenario | Input / State | Expected Output / Behavior | Error Handling |
|----------|---------------|-----------------------------|----------------|
| CANONICAL_FIXTURES | 3×3 lone cell at (1,1); 4×4 block at (1,1), (2,1), (1,2), (2,2); 5×5 vertical blinker at (2,1), (2,2), (2,3) | After one generation the lone cell is empty, the block is unchanged, and the blinker is horizontal at (1,2), (2,2), (3,2); after two generations the blinker returns to its original coordinates | No error expected |
| FINITE_BOUNDARY | Live cells near an edge or corner | Only in-field neighbors contribute; no wrapped or out-of-field cell becomes live | No error expected |
| FIXED_STEP | Elapsed duration supplied to a running scheduler | One step becomes due at 250 ms, with fractional remainder retained | Ignore non-positive elapsed durations |
| CATCH_UP | More than four intervals due at one iteration start | Execute exactly four sequential generations, discard remaining whole intervals, retain only the sub-interval remainder | No error expected |

## Code Map

- `src/domain/field/field.hpp:14-38` and `field.cpp:8-68` -- current validating row-major Field and bounds-safe access; extend storage with a persistent next-generation buffer without changing the public cell representation.
- `src/domain/simulation/` -- new framework-independent Conway evaluator; it must read the complete current buffer, write every next cell, and swap only after the pass.
- `src/domain/CMakeLists.txt:1-12` -- compile the simulation source into the existing domain target.
- `src/application/simulation/` -- new fixed-step scheduler that accepts one elapsed `steady_clock` duration, runs the bounded batch, and exposes deterministic step-count behavior for tests.
- `src/application/CMakeLists.txt:1-12` -- compile and export the scheduler through the application target.
- `src/presentation/application/raylib-application.hpp:9-19` and `.cpp:12-44` -- own the scheduler and snapshot `steady_clock` once at each loop start; invoke simulation before the existing input sampling and one render.
- `tests/unit/domain/field-test.cpp:15-99`, `tests/unit/presentation/raylib-application-test.cpp`, and `tests/CMakeLists.txt:12-54` -- preserve existing Field fixtures and register focused domain/application/presentation simulation tests without opening a window or reading runtime storage.
- `_bmad-output/game-architecture.md:300-332,480-482,1017-1025` -- authoritative double-buffer, fixed-step, bounded-catch-up, and ownership constraints.

## Tasks & Acceptance

**Execution:**
- `src/domain/field/*`, `src/domain/simulation/*`, and `src/domain/CMakeLists.txt` -- add allocation-free, double-buffered standard Conway evolution -- preserve validated dimensions and finite boundaries.
- `src/application/simulation/*`, `src/application/CMakeLists.txt`, and `src/presentation/application/*` -- add deterministic 250 ms accumulation and integrate the pre-input simulation batch -- ensure delayed iterations cap work and discard backlog correctly.
- `tests/unit/domain/*`, `tests/unit/application/*`, `tests/unit/presentation/raylib-application-test.cpp`, and `tests/CMakeLists.txt` -- cover canonical fixtures, edge behavior, double-buffer isolation, elapsed thresholds, fractional remainder, four-step cap, backlog discard, and pre-render input ordering -- prevent mathematical and timing regressions without a window.

**Acceptance Criteria:**
- Given a valid Field and a scheduled generation, when Conway evaluation runs, then every cell is derived from the prior complete buffer and the complete next buffer is published before rendering.
- Given the default scheduler and 0.25 seconds of elapsed time, when an iteration begins, then exactly one generation is eligible independently of rendering cadence.
- Given lone-cell, stable-block, and blinker fixtures, when the required generations run, then their canonical Conway outcomes are produced byte-for-byte.
- Given more than four intervals are due at one iteration start, when the loop processes the scheduler, then no more than four sequential generations run, excess whole intervals are discarded, and input is sampled only after the batch before one render.

## Spec Change Log

### 2026-08-27 — Review hardening

- Clarified the canonical fixture dimensions, coordinates, generation counts, and expected states.
- Recorded the deterministic pre-render application-iteration seam and its window-free verification surface.

## Review Triage Log

### 2026-08-27 — Review pass
- intent_gap: 0
- bad_spec: 0
- patch: 10: (high 0, medium 2, low 8)
- defer: 0
- reject: 8: (high 0, medium 3, low 5)
- addressed_findings:
  - `[medium]` `[patch]` Replaced saturating scheduler accumulation with overflow-safe whole-interval/remainder arithmetic so very large elapsed durations retain the correct fractional remainder.
  - `[medium]` `[patch]` Guarded zero-duration interval conversion before scheduler division and modulo operations.
  - `[low]` `[patch]` Bounded due-generation conversion before narrowing to `std::size_t`.
  - `[low]` `[patch]` Added capped-backlog coverage proving a fractional remainder survives discarded whole intervals.
  - `[low]` `[patch]` Added coverage proving non-positive elapsed durations preserve an existing fractional remainder.
  - `[low]` `[patch]` Added underpopulation, overpopulation, rectangular, and degenerate-field Conway coverage.
  - `[low]` `[patch]` Added repeated-generation coverage proving the reused next buffer is fully overwritten.
  - `[low]` `[patch]` Added post-batch command coverage proving edits are visible before the next scheduled generation only.
  - `[low]` `[patch]` Documented the public Conway and scheduler contracts in their headers.

### 2026-08-27 — Review pass
- intent_gap: 0
- bad_spec: 0
- patch: 6: (high 0, medium 1, low 5)
- defer: 0
- reject: 10: (high 0, medium 4, low 6)
- addressed_findings:
  - `[medium]` `[patch]` Added a deterministic, window-free application iteration path and regression coverage proving the scheduled generation completes before same-iteration input; the loop renders once afterward using the sampled viewport.
  - `[low]` `[patch]` Clarified canonical fixture dimensions, coordinates, generation counts, and expected states in the edge-case matrix.
  - `[low]` `[patch]` Added capped-catch-up coverage with a pre-existing fractional remainder.
  - `[low]` `[patch]` Corrected lone-cell coverage to assert the one-generation transition before checking stability.
  - `[low]` `[patch]` Added a stale-next-buffer regression that clears the current field before a subsequent generation.
  - `[low]` `[patch]` Added the direct `<vector>` include required by the Conway implementation.

## Design Notes

The Field owns the current and next dense byte buffers so a generation performs no per-generation allocation. The scheduler receives the elapsed duration captured by the application rather than sampling time during simulation; this makes due-count and final state deterministic in tests and ensures simulation work cannot enlarge the current batch. The existing Live edits remain in the post-batch command phase and therefore affect the next generation.

RaylibApplication::processIteration runs the scheduled batch and then applies a supplied deterministic FrameInput (or samples raylib input when omitted) before the caller performs the single render. This keeps the phase boundary testable without opening a window.

## Verification

**Commands:**
- `cmake --preset dev-debug` -- expected: configure succeeds with the existing pinned dependencies.
- `cmake --build --preset dev-debug` -- expected: domain, application, presentation, and simulation tests compile under the project warning policy.
- `ctest --preset dev-debug --output-on-failure` -- expected: all tests pass without opening a window or using wall-clock timing.
- `cmake --preset dev-release && cmake --build --preset dev-release` -- expected: Release compilation succeeds.
- `ctest --preset dev-release --output-on-failure` -- expected: the deterministic suite passes in Release.
- `git diff --check` -- expected: no whitespace errors.

### 2026-08-27 — Review pass
- intent_gap: 0
- bad_spec: 0
- patch: 0
- defer: 0
- reject: 19: (high 0, medium 3, low 16)
- addressed_findings:
  - none

## Auto Run Result

Summary: Reviewed the completed Conway generation and bounded fixed-step scheduler implementation; no actionable defects were found.

Files changed:
- `src/domain/field/*` and `src/domain/simulation/*` — double-buffered finite Conway evaluation.
- `src/application/simulation/*` — deterministic 250 ms scheduler with capped catch-up.
- `src/presentation/application/*` — pre-input simulation integration and testable iteration seam.
- `tests/*` and `tests/CMakeLists.txt` — domain, scheduler, and presentation regression coverage.
- `_bmad-output/implementation-artifacts/spec-1-3-advance-conway-generations.md` — review status and run result.
- `_bmad-output/implementation-artifacts/sprint-status.yaml` — existing orchestrator bookkeeping retained without edits.

Review findings breakdown: 0 patches applied, 0 items deferred, 19 items rejected after deduplication and triage (3 medium, 16 low).

Follow-up review recommendation: false (patched findings: 0 high, 0 medium, 0 low; score 0).

Verification performed:
- `cmake --preset dev-debug` — passed.
- `cmake --build --preset dev-debug` — passed; no work required.
- `ctest --preset dev-debug --output-on-failure` — passed, 57/57 tests.
- `cmake --preset dev-release` — passed.
- `cmake --build --preset dev-release` — passed; no work required.
- `ctest --preset dev-release --output-on-failure` — passed, 57/57 tests.
- `git diff --check` — passed.
- `clang-format --dry-run --Werror` on changed C++ files — unavailable as a verification check because the installed clang-format 22.1.1 rejects the repository's `Standard: c++23` setting; source formatting was manually inspected and no formatting changes were made.

Residual risks: The live raylib window/render loop is not exercised by the window-free unit suite; the production wiring was manually inspected. Cross-platform runtime behavior remains subject to the configured macOS/Linux CI environments.

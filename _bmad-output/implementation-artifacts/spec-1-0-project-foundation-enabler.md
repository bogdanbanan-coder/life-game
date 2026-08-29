---
title: '1.0 Project Foundation Enabler'
type: 'feature'
created: '2026-08-26'
status: done
baseline_revision: 'dc9644338580ccb4c99f9c57101771acb1eb2f42'
review_loop_iteration: 0
followup_review_recommended: true
context:
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-013604-f1ad/worktrees/1-0-project-foundation-enabler/_bmad-output/project-context.md'
  - '/Users/bogdan/core-dance/ai/life-game/.bmad-loop/runs/20260826-013604-f1ad/worktrees/1-0-project-foundation-enabler/_bmad-output/implementation-artifacts/epic-1-context.md'
warnings: []
deferred: []
operator_actions:
  - 'Run the committed GitHub Actions workflow on macos-15 and ubuntu-24.04 and confirm every Debug, sanitizer, and Release job passes.'
  - 'Confirm the macOS CI job reports AppleClang 17 and the Linux CI job uses Clang 18 with project warnings treated as errors.'
---

## Intent

**Problem:** The greenfield repository has no reproducible build, test, dependency, target, or CI baseline, so Field MVP work cannot start from a verified macOS/Linux foundation.

**Approach:** Add the smallest hand-authored CMake project that pins the approved dependencies, exposes the approved layer graph, implements the message-free foundation `Result` contract, and verifies it with a deterministic Catch2 test and composition-only executable.

## Boundaries & Constraints

**Always:** Use CMake 3.28-compatible presets, C++23 with extensions disabled, pinned release archives with verified SHA-256 hashes, explicit macOS/Linux toolchain presets, the dependency direction `foundation → domain → application → {adapters, presentation} → bootstrap`, and project warning flags `-Wall -Wextra -Wpedantic -Wsign-conversion`. Keep third-party warning policy separate from project targets.

**Block If:** A required dependency archive or its exact digest cannot be verified, or a required cross-platform/toolchain fact cannot be represented in the committed presets without inventing a project decision.

**Never:** Add gameplay, window/audio initialization, persistence, fake gameplay implementations, system-package fallbacks, floating dependency branches, generic utility modules, adapter-to-presentation dependencies, runtime state in the repository, or modifications to `sprint-status.yaml`.

## I/O & Edge-Case Matrix

| Scenario | Input / State | Expected Output / Behavior | Error Handling |
|----------|---------------|-----------------------------|----------------|
| HAPPY_PATH | Clean checkout; `dev-debug` configure, build, and CTest commands | All targets configure, the smoke executable builds, the foundation test is discovered and passes | No error expected |
| HASH_FAILURE | Pinned dependency archive bytes do not match its committed SHA-256 | Configuration fails rather than accepting altered bytes or using a system package | CMake reports the fetch/hash failure |
| FOUNDATION_ERROR | `Result<T, ErrorCode>` contains a typed error, or `Result<void, ErrorCode>` is unsuccessful | `hasValue`/error inspection and accessors preserve the typed outcome without message strings | Test asserts the error and avoids value access |

## Code Map

- `CMakeLists.txt` -- top-level project metadata, C++23 policy, testing setup, and inclusion of project options, dependencies, layers, and tests.
- `CMakePresets.json` -- committed CMake 3.28-compatible configure/build/test presets for `dev-debug`, `dev-debug-tools`, `dev-release`, and the four CI families.
- `cmake/project-options.cmake` -- typed project options and CI warning/sanitizer switches; `cmake/warnings.cmake` -- project-only warning flags; `cmake/sanitizers.cmake` -- Linux Debug ASan/UBSan; `cmake/dependencies.cmake` -- declarations and hashes before materialization.
- `src/CMakeLists.txt` and `src/{foundation,domain,application,adapters,presentation,bootstrap}/CMakeLists.txt` -- approved target graph and sibling boundaries.
- `src/foundation/error-code.hpp` -- message-free `lifeGame::foundation::ErrorCode`; `src/foundation/result.hpp` -- framework-independent `Result<T, ErrorCode>` and `Result<void, ErrorCode>` value/error contract; both are new API surfaces.
- `src/bootstrap/main.cpp` -- composition-only smoke executable that returns successfully without opening a window; `src/bootstrap/CMakeLists.txt` owns its link through the layer graph.
- `tests/CMakeLists.txt` and `tests/unit/foundation/result-test.cpp` -- Catch2 3.15.3 discovery and deterministic success/error tests; tests must not load raylib, access user data, or sample time.
- `.clang-format`, `.gitignore`, `README.md`, and `.github/workflows/ci.yml` -- existing formatting policy plus generated-artifact policy, preset-based developer instructions, and explicit macOS 15/Linux 24.04 CI.
- `_bmad-output/game-architecture.md:990-1013,1443-1487` and `_bmad-output/project-context.md` -- read-only architecture and project rules confirming target direction, pinned dependencies, presets, toolchains, and runtime boundaries.

## Tasks & Acceptance

**Execution:**
- `CMakeLists.txt`, `cmake/project-options.cmake`, `cmake/warnings.cmake`, and `cmake/sanitizers.cmake` -- establish the C++23 project policy, typed options, warning controls, and sanitizer controls -- keep policy explicit and scoped to project targets.
- `cmake/dependencies.cmake` -- declare raylib 6.0, raygui 5.0, SQLite 3.53.4, and Catch2 3.15.3 release archives with verified hashes before any materialization -- make configuration reproducible and prevent package fallback.
- `CMakePresets.json`, `.gitignore`, `.clang-format`, and `README.md` -- define all required preset families and document the supported workflow -- make clean-checkout use repeatable.
- `src/CMakeLists.txt`, layer CMake files, `src/bootstrap/main.cpp`, and `tests/CMakeLists.txt` -- create the approved graph, smoke executable, and Catch2 discovery -- provide extension points without gameplay scaffolding.
- `src/foundation/error-code.hpp` and `src/foundation/result.hpp` -- implement typed message-free error and value contracts, including `void` -- give later layers one project-owned result type.
- `tests/unit/foundation/result-test.cpp` -- cover successful values, typed errors, and `void` success/error outcomes -- prove the foundation contract deterministically.
- `.github/workflows/ci.yml` -- configure explicit macOS 15 AppleClang and Ubuntu 24.04 Clang 18 Debug/ASan/UBSan/Release jobs through committed presets -- make the cross-platform baseline reviewable.

**Acceptance Criteria:**
- Given a clean supported checkout, when `cmake --preset dev-debug` runs, then CMake 3.28+, C++23 with extensions disabled, CTest, the approved targets, and all four hashed release-archive dependencies configure without system-package fallback.
- Given a configured `dev-debug` tree, when `cmake --build --preset dev-debug` and `ctest --preset dev-debug --output-on-failure` run, then the foundation target, composition-only smoke executable, and deterministic foundation test compile and pass without a window, audio, real user-data directory, or wall-clock dependency.
- Given the committed CI workflow and presets, when the required jobs run, then `macos-15` uses the AppleClang baseline, `ubuntu-24.04` uses Clang 18 with Linux Debug ASan/UBSan, both compile Release, project warnings are errors in CI, third-party targets are not forced through that policy, and all seven named preset families exist.

## Spec Change Log

## Review Triage Log

### 2026-08-26 — Review pass
- intent_gap: 0
- bad_spec: 0
- patch: 7: (high 0, medium 5, low 2)
- defer: 0
- reject: 15: (high 0, medium 7, low 8)
- addressed_findings:
  - `[medium]` `[patch]` Forced FetchContent to retain the pinned archive path even when an existing cache requests package lookup.
  - `[medium]` `[patch]` Ignored `CMakeUserPresets.json` so machine-local presets cannot be committed accidentally.
  - `[medium]` `[patch]` Made release test presets tolerate intentionally disabled testing while preserving release compilation gates.
  - `[medium]` `[patch]` Registered the composition-only smoke executable with CTest.
  - `[medium]` `[patch]` Pinned macOS CI presets to the Apple compiler path and asserted both AppleClang 17 and the compiler selected by CMake.
  - `[low]` `[patch]` Made `Result` state inspection report an error only when the variant actually holds the error alternative.
  - `[low]` `[patch]` Updated the spec Code Map to describe the implemented files rather than the pre-implementation tree.

## Design Notes

The layer targets may be interface-only where no production source is owned yet. The smoke executable must link through the approved graph while remaining composition-only. Catch2 is fetched only when testing is enabled; raygui must not define `RAYGUI_IMPLEMENTATION` in this enabler. Dependency hashes must be obtained from the exact release bytes, never guessed.

## Verification

**Commands:**
- `cmake --preset dev-debug` -- expected: clean configuration with all required targets and pinned dependency verification.
- `cmake --build --preset dev-debug` -- expected: project targets and smoke executable build successfully with required warnings.
- `ctest --preset dev-debug --output-on-failure` -- expected: discovered foundation cases pass without opening a window or using wall-clock timing.
- `cmake --build --preset dev-release` -- expected: Release compilation succeeds without debug tools.
- `git diff --check` -- expected: no whitespace errors.

**Manual checks (if no CLI):**
- Inspect `.github/workflows/ci.yml` and `CMakePresets.json` to confirm explicit runner labels, compiler baselines, sanitizer settings, and absence of `*-latest` aliases.

## Auto Run Result

Status: awaiting-operator

Summary: Added the reproducible C++23/CMake foundation, pinned dependency acquisition, approved layer target graph, typed foundation result contract, deterministic Catch2 tests, composition-only smoke executable, committed presets, README, and explicit macOS/Linux CI workflow.

Files changed:

- `.gitignore` -- ignores out-of-source builds, local CMake presets, and platform/editor artifacts.
- `.github/workflows/ci.yml` -- defines macOS 15 and Ubuntu 24.04 Debug, sanitizer, and Release jobs.
- `CMakeLists.txt` -- configures the project standard, options, dependencies, layers, and tests.
- `CMakePresets.json` -- defines the seven required configure, build, and test preset families.
- `README.md` -- documents prerequisites and preset-based development commands.
- `cmake/dependencies.cmake` -- fetches the four pinned release archives with hashes and no package fallback.
- `cmake/project-options.cmake`, `cmake/warnings.cmake`, `cmake/sanitizers.cmake` -- define typed build policies.
- `src/CMakeLists.txt` and layer CMake files -- establish the approved dependency direction and executable.
- `src/foundation/error-code.hpp`, `src/foundation/result.hpp` -- implement message-free typed errors and `Result`, including `void`.
- `src/bootstrap/main.cpp` -- provides the composition-only smoke executable.
- `tests/CMakeLists.txt`, `tests/unit/foundation/result-test.cpp` -- discover and verify foundation and smoke tests.
- `_bmad-output/implementation-artifacts/epic-1-context.md` -- records compiled Epic 1 implementation context.
- `_bmad-output/implementation-artifacts/spec-1-0-project-foundation-enabler.md` -- records the implementation plan, review, verification, and operator handoff.

Review findings breakdown: 7 patch findings applied (5 medium, 2 low); 0 deferred; 15 rejected as out of scope, duplicate, or not actionable for this enabler.

Follow-up review recommendation: true. Patched severity counts are high 0, medium 5, low 2; score `3 × 5 + 2 = 17`.

Verification performed:

- `cmake --preset dev-debug` -- passed with CMake 4.2.1 locally; pinned archives configured successfully.
- `cmake --build --preset dev-debug` -- passed.
- `ctest --preset dev-debug --output-on-failure` -- passed, 5/5 including the smoke executable.
- `cmake --preset dev-release` and `cmake --build --preset dev-release` -- passed.
- `cmake --preset ci-macos-debug`, build, and CTest -- passed locally with AppleClang 21; this is not evidence for the required AppleClang 17 runner.
- `cmake --preset ci-macos-release`, build, and CTest -- passed locally; release CTest correctly has no registered tests.
- `cmake --list-presets`, `cmake --build --list-presets`, and `ctest --list-presets` -- all seven names recognized in each applicable family.
- `clang-format --dry-run --Werror` on all new C++ files and `git diff --check` -- passed.

Residual risks: GitHub Actions has not run in this environment; Ubuntu Clang 18 is unavailable locally, and the local macOS compiler is AppleClang 21. Cross-platform acceptance therefore remains operator-owned until the committed workflow passes on `macos-15` and `ubuntu-24.04`.

## Operator Confirmation

Confirmed 2026-08-29: the external actions this story owed were carried out.

- Run the committed GitHub Actions workflow on macos-15 and ubuntu-24.04 and confirm every Debug, sanitizer, and Release job passes.
- Confirm the macOS CI job reports AppleClang 17 and the Linux CI job uses Clang 18 with project warnings treated as errors.

_Appended by the bmad-loop orchestrator (`bmad-loop confirm`, #335): a human confirmed these external actions out of band, and the story was advanced from `awaiting-operator` to `done`._

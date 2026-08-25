# Story 1.0: Project Foundation Enabler

Status: ready-for-dev
Created: 2026-08-25
Story Key: 1-0-project-foundation-enabler
Trace: NFR1, NFR5

## Story

As the developer,
I can configure, build, and test the approved greenfield project baseline,
so that Field MVP implementation starts from a reproducible macOS/Linux C++23 foundation.

This is an implementation enabler inside Epic 1. It is required for development but does not add player-facing scope.

## Acceptance Criteria

### AC1 — Reproducible local configure and target baseline

Given a clean checkout on a supported development machine,
when `cmake --preset dev-debug` is run,
then CMake 3.28 or newer configures C++23 with compiler extensions disabled, the approved layer target graph is available, CTest is enabled, and all declared project dependencies use the pinned release archives and committed cryptographic hashes without system-package fallbacks.

The approved target names and dependency direction are:

```text
life-game-foundation
        ↓
 life-game-domain
        ↓
life-game-application
      ↙       ↘
life-game-adapters   life-game-presentation
      ↘       ↙
    life-game executable
```

The baseline may use CMake-only interface scaffolding for layers that have no production source yet. Do not add fake gameplay implementations merely to populate empty targets.

### AC2 — Minimal Debug build and deterministic test pass

Given the configured `dev-debug` build,
when `cmake --build --preset dev-debug` and `ctest --preset dev-debug --output-on-failure` are run,
then the foundation target, the composition-only smoke executable, and the initial deterministic foundation test compile and pass with the project warning policy.

The test process must not open a window, initialize raylib, initialize audio, access the real user-data directory, or depend on wall-clock timing.

### AC3 — Cross-platform CI baseline

Given the committed CI presets and workflow,
when CI runs on the required platforms,
then:

- `macos-15` uses the AppleClang 17 baseline and passes Debug tests plus Release compilation.
- `ubuntu-24.04` uses Clang 18, passes Debug tests with AddressSanitizer and UndefinedBehaviorSanitizer, and passes Release compilation.
- The workflow uses the explicit runner labels above; moving `*-latest` aliases are not used.
- Warnings are errors for project targets in CI, while fetched third-party targets are not forced through the project warning policy.
- The committed preset families are defined: `dev-debug`, `dev-debug-tools`, `dev-release`, `ci-linux-debug-asan`, `ci-linux-release`, `ci-macos-debug`, and `ci-macos-release`.

## Tasks / Subtasks

- [ ] Create the root build and repository baseline (AC: #1, #2)
  - [ ] Add `CMakeLists.txt` with `cmake_minimum_required(VERSION 3.28)`, project metadata, C++23 requirements, `CXX_STANDARD_REQUIRED ON`, and `CXX_EXTENSIONS OFF`.
  - [ ] Add `cmake/project-options.cmake` for typed project options such as `BUILD_TESTING`, `LIFE_GAME_ENABLE_DEBUG_TOOLS`, and the CI warnings-as-errors/sanitizer switches.
  - [ ] Add `cmake/warnings.cmake` with `-Wall`, `-Wextra`, `-Wpedantic`, and `-Wsign-conversion`; do not enable `-Wconversion`.
  - [ ] Add `cmake/sanitizers.cmake` for Linux Debug ASan/UBSan without affecting third-party targets.
  - [ ] Add `.gitignore` for out-of-source `build/`, CMake user presets, generated dependency trees, and platform-local artifacts.
  - [ ] Add the root `.clang-format`; it becomes the formatting source of truth for all C++ files.
  - [ ] Add a concise `README.md` containing the preset-based configure/build/test commands and supported platform/toolchain assumptions.

- [ ] Establish the approved target graph without pulling later gameplay into E1.0 (AC: #1, #2)
  - [ ] Add `src/CMakeLists.txt` and the layer CMake files under `src/foundation`, `src/domain`, `src/application`, `src/adapters`, and `src/presentation`.
  - [ ] Define `life-game-foundation`, `life-game-domain`, `life-game-application`, `life-game-adapters`, and `life-game-presentation` with the dependency direction in the architecture.
  - [ ] Add the composition-only `life-game` executable from `src/bootstrap/main.cpp`; it may return successfully without opening a window and must contain no gameplay rules or use cases.
  - [ ] Keep `domain` independent of raylib, raygui, SQLite, filesystem, logging, and application code.
  - [ ] Keep `application` dependent only on foundation/domain and owning ports, commands, state-machine types, and synchronous orchestration boundaries for later stories.
  - [ ] Keep `adapters` and `presentation` as sibling consumers of application; never introduce an adapter-to-presentation dependency.
  - [ ] Keep `sqlite3.h`, `raylib.h`, and `raygui.h` out of this story's foundation test and out of foundation/domain/application headers.

- [ ] Implement the minimal project-owned foundation contract (AC: #2)
  - [ ] Add `src/foundation/error-code.hpp` containing a message-free, typed `lifeGame::foundation::ErrorCode` representation. Do not store user-facing strings, exceptions, or logging context in the error code.
  - [ ] Add `src/foundation/result.hpp` containing the project-owned `Result<T, ErrorCode>` contract used by later layers, including success/error inspection, value access, error access, and the `void` result case needed by synchronous use cases.
  - [ ] Keep the implementation framework-independent and compatible with the common AppleClang 17/Clang 18 C++23 subset. Use the project-owned `Result` rather than introducing a dependency on an unverified library.
  - [ ] Add `tests/unit/foundation/result-test.cpp` covering success, error propagation, and `void` success/error behavior with deterministic values.
  - [ ] Link the test through `Catch2::Catch2WithMain` and register cases using `catch_discover_tests`.

- [ ] Add reproducible dependency acquisition (AC: #1, #2)
  - [ ] Add `cmake/dependencies.cmake` using `FetchContent` declarations for exact stable releases: raylib 6.0, raygui 5.0, SQLite 3.53.4, and Catch2 3.15.3.
  - [ ] Use release archive URLs and a committed SHA-256 hash for every declared archive. Verify each digest against the exact downloaded bytes; never invent a digest or reuse a digest from another asset.
  - [ ] Declare all dependency details before the first `FetchContent_MakeAvailable()` call, because CMake's first declaration for a dependency wins.
  - [ ] Prevent `FetchContent_MakeAvailable()` from satisfying these project dependencies through `find_package()` or system packages; the baseline must fail clearly if a pinned archive cannot be fetched or its hash does not match.
  - [ ] Disable third-party examples, documentation, self-tests, fuzzers, and unnecessary targets. Catch2 is fetched only when `BUILD_TESTING` is enabled.
  - [ ] Keep raygui as a presentation-only dependency. Do not define `RAYGUI_IMPLEMENTATION` in E1.0; the sole implementation unit will be added later at `src/presentation/raylib/raygui-implementation.cpp`.
  - [ ] Keep SQLite behind the future `adapters/persistence/sqlite/` boundary; E1.0 does not add a database connection or runtime storage.

- [ ] Add committed CMake presets (AC: #1, #3)
  - [ ] Add root `CMakePresets.json`, not `CMakeUserPresets.json`, for shared project presets.
  - [ ] Use the CMake 3.28-compatible preset schema version and declare `cmakeMinimumRequired` as 3.28; do not use a newer schema feature that breaks the minimum supported CMake.
  - [ ] Define configure, build, and test presets with stable out-of-source binary directories under `build/`.
  - [ ] Define the required `dev-debug` workflow with `BUILD_TESTING=ON` and project warnings enabled.
  - [ ] Define `dev-debug-tools` with `LIFE_GAME_ENABLE_DEBUG_TOOLS=ON`; this must not change normal Release builds.
  - [ ] Define `dev-release` with Release optimization and no debug tools.
  - [ ] Define `ci-linux-debug-asan` with Clang 18 and ASan/UBSan, `ci-linux-release` with Clang 18 Release, `ci-macos-debug` with the AppleClang 17 baseline and Debug tests, and `ci-macos-release` with Release compilation.
  - [ ] Use preset inheritance for shared cache variables and environment, without ad hoc compiler flags in README or CI commands.

- [ ] Add the CI workflow (AC: #3)
  - [ ] Add `.github/workflows/ci.yml` using explicit `macos-15` and `ubuntu-24.04` runners.
  - [ ] Run configure, build, and CTest through the corresponding committed presets.
  - [ ] Make project warnings errors in CI but do not impose those flags on FetchContent dependencies.
  - [ ] Run Linux ASan/UBSan Debug tests and both Release compilation paths.
  - [ ] Keep packaging, signing, notarization, distribution, networking, and runtime persistence out of this enabler.

- [ ] Verify from a clean build tree (AC: #1, #2, #3)
  - [ ] Configure, build, and test `dev-debug` from a clean checkout/build directory.
  - [ ] Confirm CTest discovers the foundation tests and reports no tests missing as a silent success.
  - [ ] Confirm the smoke executable links through the approved graph and exits successfully without opening a window.
  - [ ] Run `git diff --check` and format the new C++ files with the committed `.clang-format`.
  - [ ] Record the actual local platform/toolchain and the CI results in the Dev Agent Record; do not claim both platforms passed unless CI evidence exists.

## Dev Notes

### Current repository state

This is a greenfield repository. The current checkout contains planning/configuration artifacts and agent instructions but no runtime source tree, CMake files, CMake presets, tests, CI workflow, `.clang-format`, or `.gitignore`. There are therefore no existing implementation files to update and no previous story file or implementation pattern to inherit.

The architecture repository layout is the target structure, not evidence that those files already exist. Create only the foundation/build files required by this story; later stories own their feature source files.

### Epic and dependency context

E1.0 is the prerequisite for E1.1–E1.5. E1.1 will add the Field MVP development harness; E1.2–E1.5 add editing, simulation, boundary behavior, and canonical tests. E1.0 must leave a small, deterministic build/test path that later stories can extend without replacing the target graph or dependency policy.

Epic 1 deliberately excludes Bank, sessions, Settings, camera navigation, persistence, configurable timing, and window resizing. Do not introduce any of them as “scaffolding.”

### Foundation API guardrails

- `foundation/` owns only `Result` and message-free `ErrorCode` at this stage.
- Do not add `utils/`, `helpers/`, `common/`, `manager/`, service locators, global state, event buses, callbacks, command queues, replay queues, or a generic error-message system.
- Do not add `std::exception` subclasses or propagate human-readable error strings through domain/application interfaces.
- Later layers will use validating factories, direct synchronous calls, constructor injection, and typed `Result`/`ErrorCode` outcomes. The baseline should make those patterns easy without prematurely implementing them.
- Keep the root namespace `lifeGame` and use a layer namespace such as `lifeGame::foundation`.

### CMake and compiler guardrails

- Require C++23 with compiler extensions disabled. Use only features verified on both AppleClang 17 and Clang 18.
- Project targets use `-Wall -Wextra -Wpedantic -Wsign-conversion`; never add `-Wconversion`.
- Warnings-as-errors are a project/CI policy, not a flag applied to downloaded dependencies.
- Build out of source under ignored `build/`; do not create `vendor/` or write generated/runtime files into the repository.
- Use CMake presets for all documented workflows. The developer may use `CMakeUserPresets.json` locally, but it must be ignored and never committed.
- Keep `bootstrap/` composition-only. The E1.0 smoke executable must not contain gameplay rules, state transitions, raylib polling, or use-case logic.

### Dependency and library guardrails

The architecture pins these versions exactly:

| Dependency | Required release | E1.0 use | Boundary |
|---|---:|---|---|
| CMake | 3.28+ | Configure/build/test/presets | Build system |
| C++ | C++23 common subset | Foundation and smoke target | All project targets |
| raylib | 6.0 | Declared/pinned only; no Field yet | `presentation/` only |
| raygui | 5.0 | Declared/pinned only; no GUI yet | `presentation/` only |
| SQLite | 3.53.4 | Declared/pinned only; no database yet | `adapters/persistence/sqlite/` only |
| Catch2 | 3.15.3 | Foundation tests | `tests/` only |

Use release archives, not floating branches or system package discovery. Catch2's CMake integration should use `Catch2::Catch2WithMain` and the `Catch.cmake`/`catch_discover_tests` path. Do not use the deprecated `ParseAndAddCatchTests.cmake` path.

The project is pinning versions intentionally even if newer development versions exist. Do not upgrade versions, add a fallback, or change the architecture's dependency acquisition decision inside this story.

### Test design

The first test is a deterministic foundation contract test, not a Field test. It should prove that:

1. A successful `Result<T, ErrorCode>` exposes its value and reports success.
2. An error `Result<T, ErrorCode>` exposes its typed error and does not pretend to contain a value.
3. `Result<void, ErrorCode>` supports both successful completion and typed failure.
4. Test registration works through CTest and Catch2 discovery.

Do not open a window, load raylib, initialize audio, use a real filesystem data directory, sleep, sample wall time, or seed randomness in this test. Canonical Life fixtures belong to E1.5 and must not be faked in E1.0.

### CI evidence expectations

The CI workflow is part of the story's implementation, but local macOS execution alone is not proof of Linux success. The Dev Agent Record must distinguish:

- local configure/build/test evidence;
- macOS CI Debug/Release evidence;
- Linux Clang 18 Debug ASan/UBSan and Release evidence;
- any unavailable or failed platform evidence.

No “cross-platform complete” claim is valid without the required CI jobs passing.

### Project Structure Notes

Create new files in the following locations, following `kebab-case` naming:

```text
.github/workflows/ci.yml
.clang-format
.gitignore
CMakeLists.txt
CMakePresets.json
README.md
cmake/dependencies.cmake
cmake/project-options.cmake
cmake/sanitizers.cmake
cmake/warnings.cmake
src/CMakeLists.txt
src/foundation/CMakeLists.txt
src/foundation/error-code.hpp
src/foundation/result.hpp
src/domain/CMakeLists.txt
src/application/CMakeLists.txt
src/adapters/CMakeLists.txt
src/presentation/CMakeLists.txt
src/bootstrap/CMakeLists.txt
src/bootstrap/main.cpp
tests/CMakeLists.txt
tests/unit/foundation/result-test.cpp
```

If the final CMake decomposition requires a different file split, preserve the same ownership and target boundaries. Do not create a public `include/` mirror, generic utility directories, or feature files belonging to E1.1+.

### Project Context Rules

- Follow the repository's `AGENTS.md`; use `.clang-format` as the formatting source of truth, avoid copy-paste, and keep errors useful and actionable.
- Preserve `foundation → domain → application → {adapters, presentation} → bootstrap executable` dependency direction.
- Keep raylib/raygui types in presentation and SQLite headers in the SQLite adapter boundary.
- Use direct synchronous calls and deterministic fakes; no worker threads or asynchronous build/runtime architecture is needed.
- Keep runtime state out of the repository and installed asset directories. E1.0 has no runtime state and must not add any.
- Do not add audio, physics, networking, authentication, multiplayer, goals, scoring, progression, scripting, undo/redo, import/export, rotation, scaling, or alternate Life rules.

### Latest technical specifics

The following official documentation was checked during story preparation on 2026-08-25:

- CMake 3.28's preset schema supports schema version 8, `cmakeMinimumRequired`, configure/build/test presets, inheritance, and `cmake --preset`, `cmake --build --preset`, and `ctest --preset`. Use the CMake 3.28-compatible subset rather than current-only preset features.
- CMake `FetchContent` supports archive `URL_HASH`; declare dependency details before the first materialization call because the first declaration wins. Keep `FETCHCONTENT_TRY_FIND_PACKAGE_MODE` from introducing a system fallback.
- Catch2 3.15.3 exports `Catch2::Catch2WithMain`; `catch_discover_tests` registers individual test cases with CTest. `ParseAndAddCatchTests.cmake` is deprecated.
- raylib 6.0 and raygui 5.0 are the confirmed stable releases for this project. Do not follow raylib development-branch notes or newer unapproved versions.
- SQLite 3.53.4 is the confirmed pinned release; the official release history identifies it as a maintenance release. Record and verify the exact source archive hash used by the build rather than relying on a system SQLite.

External references:

- [CMake 3.28 presets](https://cmake.org/cmake/help/v3.28/manual/cmake-presets.7.html)
- [CMake 3.28 FetchContent](https://cmake.org/cmake/help/v3.28/module/FetchContent.html)
- [Catch2 CMake integration](https://catch2-temp.readthedocs.io/en/latest/cmake-integration.html)
- [Catch2 v3.15.3 release](https://github.com/catchorg/Catch2/releases/tag/v3.15.3)
- [raylib releases](https://github.com/raysan5/raylib/releases)
- [raygui releases](https://github.com/raysan5/raygui/releases)
- [SQLite release history](https://www.sqlite.org/changes.html)

### Out of scope

- Field rendering or simulation behavior.
- raylib window creation, raygui controls, input polling, camera conversion, assets, or audio.
- Session, Bank, Settings, SQLite schema/repositories, previews, logging adapters, or platform user-data paths.
- Conway rules, double-buffer field storage, fixed-step scheduling, and canonical Life fixtures beyond the fact that the later stories must be able to add them.
- Dependency auto-updates, package-manager fallback, external starter templates, or a broad application framework.

### References

- [Source: `_bmad-output/planning-artifacts/gdds/gdd-life-game-2026-08-19/epics.md` — Implementation Enabler / Story 1.0]
- [Source: `_bmad-output/planning-artifacts/story-specs-life-game-2026-08-25.md` — Conventions / E1.0 / Traceability Matrix]
- [Source: `_bmad-output/game-architecture.md` — Engine & Framework / Dependencies and Build System / Testing / Toolchains and Continuous Integration]
- [Source: `_bmad-output/game-architecture.md` — Project Structure / Repository Layout / Target Dependency Direction / Development Environment]
- [Source: `_bmad-output/project-context.md` — Technology Stack & Versions / Code Organization Rules / Testing Rules / Platform & Build Rules]
- [Source: `_bmad-output/planning-artifacts/sprint-change-proposal-2026-08-25.md` — Backlog and Story Structure / Technical Impact]
- [Source: `_bmad-output/planning-artifacts/implementation-readiness-report-2026-08-24.md` — Resolution Update / Final gate decision]
- [Source: `AGENTS.md` — Code style and implementation]

## Dev Agent Record

### Agent Model Used

Codex (GPT-5)

### Debug Log References

### Completion Notes List

- Ultimate context engine analysis completed - comprehensive developer guide created.
- Story is ready for implementation; no unresolved planning question blocks E1.0.

### File List

- `_bmad-output/implementation-artifacts/1-0-project-foundation-enabler.md` (created by story-context workflow)
- Implementation files will be recorded by the development agent.

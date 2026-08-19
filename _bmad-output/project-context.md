---
project_name: 'life-game'
user_name: 'bogdan'
date: '2026-08-20'
sections_completed: ['technology_stack', 'engine_specific_rules', 'performance_rules', 'code_organization_rules', 'testing_rules', 'platform_build_rules', 'critical_dont_miss_rules']
existing_patterns_found: 6
source_of_truth: '_bmad-output/game-architecture.md'
status: 'complete'
rule_count: 79
optimized_for_llm: true
---

# Project Context for AI Agents

_This file contains critical rules and patterns that AI agents must follow when implementing game code in this project. Focus on unobvious details that agents might otherwise miss._

---

## Technology Stack & Versions

- Use C++23 only where both AppleClang 17 and Clang 18 support the language and standard-library feature.
- Require CMake 3.28 or newer; use committed CMake presets rather than ad hoc compiler flags.
- Pin raylib 6.0, raygui 5.0, SQLite 3.53.4, and Catch2 3.15.3 to stable release archives with committed hashes.
- Never use floating branches, unverified archives, system-package fallbacks, or dependency auto-upgrades.
- Use `std::expected` only if it compiles on both required toolchains; otherwise use the single project-owned `Result` type.
- Context7 and Node.js 20.18.1 or newer are optional documentation tooling, never build or runtime dependencies.

## Critical Implementation Rules

### Engine-Specific Rules

- Keep every raylib and raygui type inside `presentation/`; domain, application, and persistence adapters must not include their headers.
- Define `RAYGUI_IMPLEMENTATION` in exactly `presentation/raylib/raygui-implementation.cpp`.
- Run simulation steps first, same-frame input commands second, and rendering last.
- Translate raylib polling into typed commands through the central input router; do not create callbacks, signals, or events.
- Resolve pointer ownership in this order: active modal or overlay, toolbar controls, field gesture. Consumed input never reaches a lower owner; capture a field gesture until release and rasterize Live or Die drags so fast movement cannot skip cells.
- Use logical cell coordinates with a top-left origin and half-open cell bounds. Only the centralized coordinate converter may translate screen, world, and cell positions.
- Keep `Camera2D` in logical cell units and render only the clamped visible cell range; never create per-cell UI objects or full-field textures.
- Preload static assets after window creation and release them before `CloseWindow`; missing optional visuals use the documented fallback.
- Route raylib diagnostics through the project logger. Do not initialize audio.
- Session previews render only the field through the saved camera and zoom; exclude controls, dialogs, selections, staged figures, and pointer state.

### Performance Rules

- Use two dense row-major `std::vector<std::uint8_t>` field buffers; a generation reads only current, writes all of next, then swaps.
- Enforce width and height `1..4096`, maximum total cells `4,194,304`, and maximum square field `2048×2048`; overflow-check multiplication before allocation.
- Keep simulation on the main thread. Do not introduce worker threads, ECS, object pooling, or per-cell objects.
- Drive simulation with `std::chrono::steady_clock`: default interval 250 ms, at most four catch-up generations per frame, then discard excess backlog.
- Clear accumulated time on pause and begin a fresh full interval on resume.
- Perform no allocation, logging, callbacks, or other fallible work inside per-cell generation or field-patch commit loops.
- Suppress grid lines below the UX-defined screen-size threshold.
- Lazily decode and cache only visible session-card previews; release their textures when leaving the browser.
- Do not add asset streaming, asynchronous loading, or hot reload.
- The debug overlay uses a fixed-size timing ring buffer without per-frame allocation.
- There is no formal FPS target; preserve deterministic behavior and measured bounds rather than inventing one.

### Code Organization Rules

- Preserve target direction: `foundation → domain → application → {adapters, presentation} → bootstrap executable`. Never add reverse or adapter-to-presentation dependencies.
- Keep `foundation/` limited to `Result` and message-free `ErrorCode`.
- Put invariants and value types in `domain/`; synchronous use cases, commands, state machines, and ports in `application/`; port implementations in `adapters/`; and all raylib or raygui code in `presentation/`.
- Keep `bootstrap/` as composition only: no gameplay rules or use cases in `main.cpp` or the composition root.
- Colocate private `.hpp` and `.cpp` files under their owning feature; do not create a public `include/` mirror.
- Do not add generic `utils`, `helpers`, `manager`, or `common` dumping-ground modules. Give each abstraction one explicit owner.
- Use `kebab-case` for files and multiword directories, `camelCase` for functions, variables, and namespaces, `PascalCase` for types, and `CAPS_LIKE_THIS` for constants.
- Use root namespace `lifeGame` with layer namespaces such as `lifeGame::domain`.
- Name CMake targets `life-game-foundation`, `life-game-domain`, `life-game-application`, `life-game-adapters`, `life-game-presentation`, and `life-game`.
- Group static assets by type under `assets/fonts`, `assets/icons`, `assets/images`, and `assets/styles`.
- Keep `sqlite3.h` inside `adapters/persistence/sqlite/`; keep `raylib.h` and `raygui.h` inside `presentation/`.

### Testing Rules

- Use Catch2 3.15.3 through CTest; name test files `<subject>-test.cpp`.
- Organize tests by scope first and owning layer second: `tests/unit/<layer>`, `tests/integration/<layer>`, and `tests/regression/<layer>`.
- Domain and application tests must run without opening a window, loading raylib, or accessing the real user-data directory.
- Prefer deterministic fakes over mocking frameworks: fake clock, in-memory repositories, recording logger, and sample-field fixtures.
- Use temporary SQLite databases for repository, migration, uniqueness, corruption, and rollback integration tests.
- Always cover lone-cell death, stable block, blinker period two, permanent-dead edges, double-buffer isolation, and dimension overflow.
- Test timing boundaries, four-step catch-up, backlog discard, pause clearing, and fresh-interval resume.
- Test every legal and illegal state transition, pointer ownership, click-through prevention, drag rasterization, and coordinate boundaries.
- For figure placement, assert both exact live or dead replacement and byte-for-byte unchanged state after every rejection path.
- Verify technical failures are logged exactly once at their origin and callers propagate only `ErrorCode`.
- Report deterministic random seeds on failure; never depend on wall-clock timing or test order.
- Every corrected defect requires a regression test in the owning layer.
- CI runs Debug tests on macOS and Linux, ASan and UBSan on Linux Debug, and a successful Release build on both platforms.

### Platform & Build Rules

- Treat macOS and Linux as equal targets; do not add Windows, web, mobile, or console code without a new product and architecture decision.
- Keep platform conditionals inside `adapters/platform/`; domain and application code must remain platform-independent.
- Require C++23 with compiler extensions disabled.
- Project targets use `-Wall -Wextra -Wpedantic -Wsign-conversion`; never enable `-Wconversion`. Treat warnings as errors for project targets in CI, never for fetched dependencies.
- Use explicit CI runners: `macos-15` with AppleClang 17 and `ubuntu-24.04` with Clang 18; do not use moving `*-latest` aliases.
- Use the committed preset families `dev-debug`, `dev-debug-tools`, `dev-release`, `ci-linux-debug-asan`, `ci-linux-release`, `ci-macos-debug`, and `ci-macos-release`.
- Disable unnecessary third-party examples and targets.
- Resolve installed assets relative to the executable, never the process working directory.
- Store SQLite data and rotating logs only in the platform user-data directory; never write runtime state into the repository or installed assets.
- Keep configuration typed: compile-time invariants, CMake build settings, composition-root defaults, validated SQLite settings, and debug-only CLI flags.
- Do not add required environment variables, JSON or YAML configuration, generic string-key settings, or remote configuration.
- Compile debug tools only with `LIFE_GAME_ENABLE_DEBUG_TOOLS`; exclude them from normal Release builds.

### Critical Don't-Miss Rules

- Log a technical failure exactly once where it occurs and full context exists. Translate third-party or standard exceptions there, then propagate only `ErrorCode`; never propagate, wrap, or reconstruct an error-message string. Retain the final application-loop exception boundary.
- Do not log expected validation outcomes such as duplicate names or out-of-bounds figure placement.
- Use direct synchronous calls only. Do not introduce events, observers, signals, message queues, service locators, command history, or replay infrastructure.
- Create invariant-bearing domain values through validating factories. Never construct temporarily invalid `Field`, `Figure`, dimensions, or names.
- Mutate `AppScreen`, `FieldMode`, and `RunState` only through named transition methods; never scatter raw enum assignments or replace states with boolean flags.
- A figure contains its entire rectangle, including dead cells. Validate the complete `FieldPatch` before the first write, then copy every byte.
- Rejected placement changes no field byte, is not logged, still exits Bank, selects Live, resumes, and starts a fresh interval.
- Save session cells, dimensions, camera, zoom, and field-only PNG preview in one transaction. Never persist paused or running state.
- Failed save or preview generation keeps the session open and paused. Do not retry automatically or silently fall back to in-memory storage.
- Never automatically repair, overwrite, or delete damaged persisted records. Database open or migration failure blocks startup.
- Use one main-thread SQLite connection with prepared statements and explicit transactions; do not add a pool or background database thread.
- The Bank is application-wide. Deleting a session must never delete its figures.
- Out-of-field coordinates are permanently dead and input outside the field never edits cells.
- The debug overlay is presentation-only and read-only; it cannot mutate state or call repositories.
- Do not invent deferred UX decisions: zoom values and limits, grid threshold, name syntax, length, or blank policy, Bank-delete confirmation, or preview pixel dimensions.
- Do not add audio, physics, networking, authentication, multiplayer, goals, scoring, progression, scripting, undo or redo, import or export, rotation, scaling, or alternate Life rules without updated product and architecture documents.

---

## Usage Guidelines

**For AI agents:**

- Read this file and `_bmad-output/game-architecture.md` before implementing game code.
- Follow every rule; when guidance is ambiguous, stop and resolve it instead of inventing behavior.
- Update this file only when an approved architecture or recurring implementation pattern changes.

**For humans:**

- Keep this file lean and focused on rules agents could otherwise miss.
- Update it when the stack, architecture, or product scope changes; periodically remove obsolete rules.

Last Updated: 2026-08-20

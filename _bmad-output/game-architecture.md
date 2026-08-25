---
title: 'Game Architecture'
project: 'life-game'
date: '2026-08-25'
author: 'bogdan'
version: '1.0'
stepsCompleted: [1, 2, 3, 4, 5, 6, 7, 8, 9]
status: 'complete'
engine: 'raylib 6.0 with raygui 5.0'
platform: 'macOS and Linux desktop'

# Source Documents
gdd: '_bmad-output/planning-artifacts/gdds/gdd-life-game-2026-08-19/gdd.md'
epics: '_bmad-output/planning-artifacts/gdds/gdd-life-game-2026-08-19/epics.md'
brief: '_bmad-output/planning-artifacts/briefs/brief-life-game-2026-08-15/brief.md'
ux_experience: '_bmad-output/planning-artifacts/ux-designs/ux-life-game-2026-08-19/EXPERIENCE.md'
ux_design: '_bmad-output/planning-artifacts/ux-designs/ux-life-game-2026-08-19/DESIGN.md'
---

# Game Architecture

## Document Status

This architecture document was completed through the GDS Architecture Workflow.

**Steps Completed:** 9 of 9 (Complete)

---

## Executive Summary

Life Game uses C++23, raylib 6.0, raygui 5.0, and SQLite 3.53.4 for native macOS and Linux. Its key decisions are deterministic double-buffered simulation, explicit state machines, direct synchronous commands, repository-isolated persistence, and message-free error propagation with origin-only logging. A hybrid layered structure covers nine core systems with six implementation patterns and is ready to guide implementation after the owning UX gates are resolved.

## Source Authority and Reconciliation

The architecture is governed by the following source order when documents appear to disagree:

1. Explicit user-approved decisions recorded in the applicable decision log and current Sprint Change Proposal.
2. The GDD for product scope, player-facing rules, game-state semantics, and exclusions.
3. `EXPERIENCE.md` and `DESIGN.md` for interaction behavior, screen/component ownership, visual tokens, accessibility, and UX acceptance constants.
4. This architecture for technical ownership, implementation mechanisms, safety limits, and non-conflicting platform constraints.
5. Epics and stories as delivery traceability; they may refine work breakdown but may not weaken a higher-level contract.

Architecture may resolve an implementation detail within its ownership only when it preserves the approved GDD and UX contracts. If a conflict is found, the owning agent must not choose silently: record a `UX-A#` or equivalent issue, resolve it through `gds-correct-course`, update every affected artifact and decision log, and rerun `gds-check-implementation-readiness` before the affected epic enters production. `_bmad-output/project-context.md` carries the reconciled rules for implementation agents; it does not replace the source documents.

## Project Context

### Game Overview

**Life Game** is a private, single-user Conway’s Game of Life study sandbox. The player edits a finite cellular field, observes deterministic evolution, captures exact rectangular constructions, and reuses them across persistent experimental sessions.

The current product scope deliberately excludes multiplayer, online services, progression, narrative, audio, scripting, infinite fields, and alternate cellular rules.

### Technical Scope

**Platform:** macOS and Linux desktop, with equal priority  
**Genre:** Sandbox / cellular-automaton simulation  
**Project Level:** Medium complexity  
**Technology:** C++23, CMake, raylib 6.0, raygui 5.0

The simulation and application layers remain independent of raylib, raygui, and SQLite. raylib and raygui belong only to presentation, while SQLite belongs only to persistence adapters.

### Core Systems

| System | Complexity | Design Reference |
| --- | --- | --- |
| Finite field model and permanent-dead boundaries | Medium | GDD: Cellular-Automaton Rules |
| Synchronous Life simulation and generation clock | Medium | GDD: Cellular-Automaton Rules; Performance and Correctness |
| Field rendering, camera, zoom, and coordinate conversion | Medium | GDD: Field and Camera |
| Tool and interaction state machine | High | GDD: Tools and Input |
| Rectangular figure capture and global Bank | Medium | GDD: Figure Capture; Bank Placement |
| Persistent sessions, saved views, and previews | High | GDD: Session Lifecycle |
| Immediate-mode application UI and modal workflows | Medium | GDD: Art Direction; Tools and Input |
| Cross-platform build and dependency integration | Medium | GDD: Technology and Platforms |
| Deterministic behavioral verification | Medium | GDD: Performance and Correctness Targets |

### Technical Requirements

- Use native C++23 with a feature subset verified on both selected toolchains.
- Build with CMake and pin raylib 6.0 and raygui 5.0 exactly.
- Keep simulation behavior independent of rendering cadence.
- Advance the default simulation once every 0.25 seconds.
- Compute each generation synchronously from one complete prior state.
- Treat every coordinate outside the fixed field as permanently non-live for simulation semantics; render visible out-of-field space gray rather than as black in-field dead cells.
- Support rectangular fields whose dimensions remain fixed for a session.
- Preserve exact cell state, dimensions, camera position, zoom, and last-view preview.
- Maintain one persistent figure Bank shared by all sessions.
- Store both live and dead cells in captured rectangular figures.
- Make invalid out-of-bounds placement atomic: no field cells may change.
- Keep mouse editing, camera movement, selection, dialogs, and staged placement behavior mutually unambiguous.

The GDD defines no user-facing frame-rate, resolution, or load-time promise. This architecture establishes explicit field-memory and internal verification bounds; window resizing remains optional.

### Networking Requirements

There is no networking, multiplayer, account, cloud, or synchronization requirement. All state is local to one user.

### Complexity Drivers

**High complexity:**

- Interaction-state transitions among editing, moving, highlighting, modal dialogs, Bank staging, pause, and resume.
- Durable session and Bank persistence, including damaged data, version evolution, atomic updates, and shared ownership rules.

**Medium complexity:**

- Separating simulation time from render frequency.
- Mapping pointer positions consistently through camera and zoom transforms.
- Exact rectangular capture and replacement semantics.
- Manual layout and modal behavior with raygui.
- Equivalent macOS and Linux builds and behavior.

**Novel concepts:**

The confirmed product scope contains no fundamentally novel technical subsystem. Exact rectangular replacement has unusual semantics but can use established patterns. Program replication and AI-assisted mathematical research are explicitly parked and must not influence the current architecture.

### Critical Architectural Decisions

1. Dense field representation and maximum supported dimensions.
2. Simulation buffering and delayed-frame catch-up policy.
3. Application/tool state-machine boundaries.
4. Camera, viewport, and cell-coordinate conventions.
5. Figure bitmap representation and placement transaction rules.
6. Persistence schema, versioning, and atomic-write strategy.
7. Session-preview representation and regeneration policy.
8. Core/application/presentation project structure.
9. Cross-platform build and verification strategy.
10. Same-frame ordering of scheduled simulation, input commands, and rendering.

### Intake Risks and Current Disposition

| Intake risk | Disposition |
| --- | --- |
| Unbounded field memory and rendering work | Resolved through dimension and total-cell limits plus visible-range rendering. |
| Delayed-frame simulation behavior | Resolved through a fixed-step accumulator and four-generation catch-up cap. |
| Persistence format, migration, corruption, and save failure | Resolved through validated SQLite repositories, transactional migrations, typed failures, and non-destructive recovery rules. |
| UX production constants and confirmation details | Resolved from the UX spine: discrete zoom levels, grid threshold, name validation, Bank-delete confirmation, and session-preview dimensions are fixed before their owning epics enter production. |
| Immediate-mode modal and tool conflicts | Mitigated through centralized input ownership and explicit state machines. |
| C++23 implementation differences | Mitigated through the verified common subset and required macOS/Linux Clang-family CI jobs. |
| Post-MVP scope entering the Field MVP | Controlled through epic boundaries and in-memory repository implementations for the MVP. |

## Engine & Framework

### Selected Framework

**raylib 6.0 with raygui 5.0**

**Rationale:** This combination provides the windowing, 2D rendering, camera transforms, pointer input, and compact immediate-mode controls required by Life Game without imposing an engine-owned scene or object model. That leaves the simulation, persistence, and application state under explicit project control, supporting both the learning goal and cross-platform requirements.

Both libraries must use stable release tags:

- raylib: `6.0`
- raygui: `5.0`

Development branches such as `master` must not be used as dependencies. raygui’s implementation macro must be defined in exactly one compilation unit.

Official sources:

- [raylib releases](https://github.com/raysan5/raylib/releases)
- [raygui releases](https://github.com/raysan5/raygui/releases)

### Project Initialization

**Starter:** No external starter template.

The project will use a minimal, hand-authored C++23 and CMake structure. It will not inherit the plain-C multi-screen architecture of the official raylib game template.

Once the initial project files exist, the baseline workflow will be:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

The dependency acquisition method, targets, directory structure, test framework, compiler warnings, and platform presets are defined in the subsequent architecture sections.

### Framework-Provided Architecture

| Component | Solution | Notes |
| --- | --- | --- |
| Application lifecycle | raylib window and frame loop | The application owns simulation scheduling and state transitions. |
| 2D rendering | raylib drawing and texture APIs | Render the field in batches rather than creating one UI object per cell. |
| Camera | raylib 2D camera transforms | Project code must define authoritative world-to-cell conversion rules. |
| Input polling | raylib mouse and keyboard APIs | Project code resolves tool ownership, capture, modal priority, and gesture semantics. |
| GUI controls | raygui immediate-mode widgets | Project code owns screen, dialog, focus, and exclusive-mode state. |
| Physics | Not used | Life evolution is a discrete simulation, not physical motion. |
| Audio | Not initialized | Audio is outside the confirmed product scope. |
| Scene management | Not provided | Application screens and Field modes require an explicit state model. |
| Persistence | Not provided | Sessions and the global Bank require a project-owned storage layer. |
| Build integration | raylib CMake support plus custom top-level CMake | Dependency versions remain pinned independently of system packages. |

### AI Documentation Integration

**Context7** is included as optional development-only tooling. It is not a runtime or build dependency.

Repository: [upstash/context7](https://github.com/upstash/context7)

Capabilities:

- Retrieve current, version-specific library documentation.
- Verify raylib and raygui API signatures.
- Find current CMake and C++ examples.
- Reduce reliance on model training data for dependency APIs.

Setup:

```bash
npx ctx7 setup
```

For a local MCP installation, use a compatible Node.js release; the current MCP package requires Node.js 20.18.1 or newer. An API key is recommended for higher limits. Developers may alternatively configure the hosted Context7 MCP endpoint.

### Decision Scope and Explicit Deferrals

| Topic | Status | Ownership and rationale |
| --- | --- | --- |
| External API and networking | Not applicable | The product is entirely local and has no remote consumers. |
| Authentication and authorization | Not applicable | There are no accounts, shared resources, or privilege boundaries. |
| Runtime deployment | Decided | Native macOS and Linux desktop executables; packaging, signing, and notarization are outside the current scope. |
| Zoom levels, limits, and grid threshold | Decided in UX | Use 50%, 75%, 100%, 150%, 200%, 300%, and 400% with a 100% new-session default; render grid lines at or above 4 logical display pixels per cell. |
| Session and figure name syntax, length, and blank-name policy | Decided in UX | Trim outer whitespace, normalize Unicode to NFC, require 1–64 Unicode code points, preserve internal whitespace/display case, and compare uniqueness case-insensitively. Validating domain factories enforce these rules. |
| Bank-figure delete confirmation | Decided in UX | Require explicit confirmation naming the figure before deletion. |
| Session-preview pixel dimensions | Decided in UX | Encode field-only previews as 256×256 PNGs using the saved camera and zoom; commit the image transactionally with session state. |
| Camera visibility boundary and out-of-field rendering | Decided in UX | Clamp camera movement so the viewport intersects at least one in-field cell. Render visible area outside the finite Field with the UX gray out-of-field fill; it is presentation-only and never a cell state or input target. |
| DPI, logical sizing, and pointer mapping | Decided | Use logical client pixels for all UI and Field presentation. Create an initial 1280×720 logical client area, enforce a 960×540 minimum logical viewport when resizing is enabled, apply OS DPI scaling once, and normalize pointer input to logical coordinates before UI, camera, or cell mapping. |
| Window resizing | Optional | It is non-blocking in the GDD; if enabled, the Field viewport absorbs added logical space while token-sized controls remain stable and the centralized coordinate converter remains the only input path. |
| Zoom anchor | Decided in UX | Zoom anchors on the Field cell under the pointer, or the viewport center when the pointer is outside the Field. |
| Settings Save/Cancel and invalid persisted Settings | Decided | Save atomically commits validated values; invalid input disables Save; Cancel discards edits. An invalid stored Settings record is preserved, safe defaults are used in memory, and explicit Save replaces the invalid record. |
| Pointer capture and staged placement | Decided in UX | Paint strokes interpolate samples; Move/Highlight capture until release; staged figures anchor by top-left cell, retain the last in-window preview, and show solid/dashed validity with text. |
| Recoverable error actions | Decided | Recoverable persistence/load operation failures use canonical text and Retry/Cancel; isolated damaged-record selection uses Acknowledge without mutation; fatal startup database failures use Acknowledge-to-exit. |
| Capacity and performance budgets | Decided | Maximum 512 sessions, 2048 Bank figures, and 4,194,304 field cells. Release-reference budgets are 16 ms for default-frame work, 250 ms for maximum-field generation, and 250 ms for default-session save plus preview. |
| Visual verification and accessibility scope | Decided in UX | Verify contrast, focus, text alternatives, line styles, modal capture, and DPI behavior on both platforms. Full keyboard navigation, screen-reader semantics, and alternative input are outside this mouse-first release. |

## Architectural Decisions

### Decision Summary

| Category                  | Decision                                                                                                 | Version                                  | Rationale                                                                                                                       |
| ------------------------- | -------------------------------------------------------------------------------------------------------- | ---------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------- |
| State management          | Explicit hierarchical state machines                                                                     | N/A                                      | Makes screen, tool, overlay, and simulation transitions visible and testable without global state.                              |
| Field storage             | Dense row-major byte buffers                                                                             | C++23                                    | Matches finite fields and exact rectangular operations while remaining simple and cache-friendly.                               |
| Simulation timing         | Fixed-step accumulator with bounded catch-up                                                             | C++23                                    | Preserves short timing delays without allowing long stalls to freeze the application.                                           |
| Frame sequencing          | Clock snapshot → scheduled simulation batch → input sampling/translation → accepted commands → rendering | N/A                                      | Gives edits immediate visual feedback while keeping the simulation order deterministic, without buffering or requiring a pause. |
| Coordinates and rendering | Logical cell space with visible-range rendering                                                          | raylib 6.0                               | Centralizes coordinate semantics and avoids rendering millions of off-screen cells.                                             |
| Input handling            | Central router producing typed commands                                                                  | raylib 6.0, raygui 5.0                   | Prevents click-through and makes command ordering deterministic.                                                                |
| Dependency acquisition    | Hashed release archives through CMake FetchContent                                                       | CMake 3.28+                              | Provides reproducible cross-platform dependencies without submodule workflow.                                                   |
| Persistence               | Transactional SQLite database behind repository interfaces                                               | SQLite 3.53.4                            | Supports atomic session and Bank changes, unique names, BLOB storage, and migrations.                                           |
| Session previews          | Field-only offscreen render encoded as PNG                                                               | raylib 6.0                               | Preserves the last camera view without transient UI or modal state.                                                             |
| Asset loading             | Preload the small static asset set                                                                       | raylib 6.0, raygui 5.0                   | Avoids runtime hitches and keeps resource ownership straightforward.                                                            |
| Testing                   | Catch2 with CTest discovery                                                                              | Catch2 3.15.3                            | Supports expressive deterministic tests and clean CMake integration.                                                            |
| Toolchains and CI         | AppleClang 17 on macOS; Clang 18 on Linux                                                                | GitHub Actions macOS 15 and Ubuntu 24.04 | Keeps compiler behavior relatively consistent while verifying both target platforms.                                            |

Versions were verified on 2026-08-19.

### State Management

**Approach:** Explicit hierarchical state machines with typed commands and injected services.

State is divided across three explicit axes:

- `AppScreen`: Start Screen or Field
- `FieldMode`: Live, Die, Move, Highlight, Capture Dialog, Bank List, or Bank Preview
- `RunState`: Running or Paused

The pure `Field` domain model contains cells, dimensions, and Life rules. Application state machines coordinate use cases and transitions. Presentation adapters translate raylib/raygui results into typed commands. Persistence, clock, and preview-rendering capabilities are injected through interfaces.

Global mutable state, service locators, ECS, and Redux-style whole-field snapshots are not used.

The centralized input router resolves pointer ownership in this order:

1. Active modal or overlay
2. Toolbar and UI controls
3. Field gesture

A consumed pointer press cannot reach a lower layer. Field gestures capture the pointer until release. Live and Die drags rasterize between sampled cells to prevent gaps. Commands carry logical coordinates rather than screen coordinates.

Each main-loop iteration has one deterministic phase order: snapshot the clock and accumulate elapsed time; execute the snapshotted simulation batch; sample the input state exposed by raylib and translate it into typed commands; execute accepted commands; render once. A running Live or Die edit mutates the current field during the command phase and is therefore visible in that render. It is not retroactively included in a scheduled step already processed earlier in the iteration. Input not exposed when the input state is sampled waits for a later iteration if it remains observable; the application adds no project-owned retention or replay queue. No user-facing contract assigns an exact wall-clock boundary to a particular rendered frame.

Pause clears accumulated simulation time. Opening Bank is an explicit pause transition: it enters `Bank List`, clears accumulated time, and keeps the main loop interactive for Bank controls while the Field remains paused. Selecting a figure enters `Bank Preview` and remains paused until placement resolves. Resume selects Live and begins a fresh interval. Capture completion and Bank exit without placement select Live and resume with a fresh interval. Invalid Bank placement changes no field cells but still exits Bank and resumes.

### Presentation Surface Ownership

The presentation layer uses explicit ownership boundaries. A session preview is a picture inside a session card; it is not a second Field screen.

| Surface or component | Owner and responsibility |
| --- | --- |
| Start Screen | `StartScreen` owns the application launch surface, the horizontal session browser, the Settings button, and navigation into a selected session. |
| Session browser | `SessionBrowserPanel` belongs to `StartScreen` and owns the scrolling `SessionCardList`, `SessionCard` rows, card actions, and field-preview images. Selecting a card opens the interactive `FieldScreen`. |
| Settings | `SettingsPanel` belongs to `StartScreen` and renders one global configuration in a two-column table: setting name on the left and editable value on the right. It owns Field width, Field height, and generation interval. Width and height become defaults for newly created sessions; a session's dimensions remain fixed after creation. |
| Field Screen | `FieldScreen` is the interactive game field reached after selecting or creating a session. It owns the field viewport, camera, overlays, and the upper-right `Toolbar`. |
| Toolbar | `Toolbar` is a panel owned by `FieldScreen`; it exposes field tools, pause/resume, zoom, Bank, and Exit commands. |
| Bank panel | `BankPanel` is opened by the Field Screen's Toolbar. It owns the one application-wide Bank list and figure actions; it is not owned by an individual session. |
| Name dialog | `NameDialog` is a reusable modal for session/figure creation and rename, with a text field, validation, Save, and Cancel. |
| Confirmation dialog | `ConfirmationDialog` is a reusable modal for destructive actions such as deleting a session or Bank figure. It names the target and offers explicit Confirm and Cancel actions; it is not used for ordinary navigation or dismissal. |
| Error dialog | `ErrorDialog` is a blocking modal for a failed persistence or load operation. Recoverable action failures present specific copy with `Retry` and `Cancel`; `Cancel` preserves the current valid state and leaves an open session paused. Selecting an isolated damaged record presents specific copy with `Acknowledge` and returns to the owning surface without mutation. A fatal database-open or migration failure presents `Acknowledge` and exits after acknowledgment. |
| Status message | `StatusMessage` is reusable non-modal feedback owned by the current Start Screen or Field Screen. It reports validation, success, failure, busy, and invalid-placement outcomes without changing navigation. |
| Text and numeric fields | `TextField` and `NumericField` are reusable presentation controls used by `NameDialog` and `SettingsPanel`; they retain invalid input and show field-local validation. |

### Field and Simulation

**Storage:** Two dense, row-major `std::vector<std::uint8_t>` buffers.

Each cell contains only `0` or `1`. A generation reads exclusively from the current buffer, writes the entire next buffer, and swaps only after completion.

Field limits:

- Width: `1..4096`
- Height: `1..4096`
- Maximum total cells: `4,194,304`
- Maximum square field: `2048×2048`
- Two maximum-sized simulation buffers: 8 MiB total

Dimension multiplication must be overflow-checked before allocation. Captured figures use the same row-major byte representation and cannot exceed field limits.

**Timing:** A fixed-step accumulator driven by `std::chrono::steady_clock`.

- Default generation interval: `250 ms`
- Accumulate time only while Running
- Use the complete iteration order: clock snapshot and accumulation → simulation batch → raylib-exposed input sampling and command translation → accepted command execution → one render
- Apply accepted running Live/Die edits synchronously to the current field; do not buffer them for a later frame
- Sample elapsed time once at the start of each main-loop iteration and determine the due-generation count from that snapshot
- Execute `min(floor(accumulator / interval), 4)` due generations total in that iteration, subtracting one interval after each generation
- After the batch, discard any remaining whole intervals with `accumulator %= interval`; retain the sub-interval remainder and never repay discarded intervals
- Never skip mathematical states between generations that are executed
- Do not render intermediate catch-up generations; after the catch-up batch, process input and render the final completed current buffer
- Time spent in the batch, input phase, and rendering is measured by the next iteration's clock sample; it cannot add steps to the current batch
- Pausing clears partial accumulated time
- Resuming begins a fresh full interval
- Rendering sees only a completed current buffer

A dedicated simulation thread is not used.

**Capacity and verification budgets:** The application accepts at most 512 persisted sessions and 2048 persisted Bank figures. These are safety limits, not pagination requirements. Release-reference verification targets are: default 50×50 simulation/input/render work within 16 ms; one legal maximum-field generation within 250 ms; and default-session save plus 256×256 preview generation within 250 ms. These budgets are measured in CI or an equivalent controlled harness and are not a user-facing FPS promise.

### Coordinate and Rendering Model

Cells use zero-based integer coordinates with the field origin at the upper-left. Positive X points right and positive Y points down.

Cell `(x, y)` occupies the half-open logical rectangle `[x, x+1) × [y, y+1)`. Right and bottom field boundaries are exclusive.

`Camera2D` operates in logical cell units. The application creates an initial 1280×720 logical-pixel client area and, when resizing is enabled, enforces a 960×540 minimum logical viewport; a smaller logical viewport is unsupported. UI layout, text, pointer targets, and Field screen geometry are specified in logical client pixels.

The platform/renderer applies OS DPI scaling exactly once. One presentation input service normalizes platform/framebuffer pointer coordinates to logical client coordinates before UI hit testing, camera conversion, or world-to-cell mapping. No presentation code may feed framebuffer pixels directly into hit testing or apply a second DPI scale. Invalid or out-of-field conversions return no cell.

Camera movement is clamped so the logical viewport rectangle always intersects at least one in-field cell rectangle. The camera may show space outside the finite Field, especially when the viewport is larger than the Field; the renderer fills that area with the UX `out-of-field` gray. Out-of-field pixels are presentation-only, are not part of the Field buffer or persisted field data, and cannot receive Live, Die, Highlight, Move, or Bank placement cell input. A saved session preview may contain the gray pixels when its camera view includes the boundary.

Rendering calculates a conservative visible cell range from transformed viewport corners, clamps it to field bounds, and draws only that range. `+` and `−` select the UX-defined levels `50%`, `75%`, `100%`, `150%`, `200%`, `300%`, and `400%`; new sessions start at `100%`. Grid lines render when a cell is at least 4 logical display pixels wide and tall and disappear below that threshold.

Highlight’s inclusive gesture endpoints are normalized into a half-open rectangle. Figure previews use an integer top-left anchor and must fit entirely inside the field.

Full-field render textures, per-cell UI objects, and duplicated coordinate-conversion code are prohibited.

### Data Persistence

**Save system:** One local SQLite 3.53.4 database in the platform’s user-data directory.

SQLite is introduced only when persistence enters scope. The Field MVP uses in-memory repository implementations.

Conceptual tables:

- `sessions`: display name, normalized case-folded uniqueness key, dimensions, cell BLOB, camera position, zoom, preview PNG, and metadata
- `figures`: display name, normalized case-folded uniqueness key, dimensions, and cell BLOB
- `settings`: the one global validated generation interval and new-session field width/height defaults
- schema version metadata through SQLite `user_version` or an equivalent metadata table

The application validates names before repository writes by trimming outer whitespace, normalizing Unicode to NFC, requiring 1–64 Unicode code points, preserving internal whitespace/display case, and deriving a case-folded uniqueness key. The database enforces a unique index on that key separately for sessions and figures, while retaining the validated display name. Cell BLOBs use the same row-major `0`/`1` byte representation as memory.

Save, rename, delete, and Bank mutations use explicit transactions. Deleting a session never removes figures. Paused/running state is not persisted.

Loading validates dimensions, total cell count, BLOB length, and every cell value before creating domain objects. Repository load results distinguish a fatal database/setup failure from an isolated damaged record: a damaged record returns a non-mutating summary with its kind, stable identity, available display name, and `ErrorCode` alongside valid records. Schema migrations run transactionally, with a recoverable backup created before destructive migration.

The global Settings record is validated separately from sessions and figures. If its stored width, height, or interval is invalid, the record is preserved and not overwritten; the application uses safe in-memory defaults of 50×50 and 250 ms, presents a warning/status message, and requires an explicit successful Save to replace the invalid record. Valid stored Settings load normally. A failed Settings Save preserves the previous valid record and the uncommitted edits remain available for Retry or Cancel.

An isolated damaged session is represented by a disabled session-card entry, while valid sessions remain usable. An isolated damaged figure is represented by a disabled Bank row, while valid figures and Bank operations remain usable. Neither damaged record is automatically repaired, overwritten, or deleted. Failure to open the database, complete schema migration, or establish the required schema is a fatal startup error; presentation shows the startup `ErrorDialog` and exits after acknowledgment without opening the Start Screen or creating a replacement database.

SQLite remains behind repository interfaces. Domain and application targets cannot include `sqlite3.h`. The application owns one connection and uses prepared statements; there is no pool or background database thread.

### Session Preview Management

Session previews are produced when leaving or closing a session.

A presentation-layer `SessionPreviewRenderer` renders the field using the saved camera and zoom into an offscreen target. It includes in-field black/white cells, gray out-of-field area where the saved camera view includes it, and applicable grid lines while excluding controls, dialogs, selections, Bank previews, and pointer state.

The preview is encoded as a field-only 256×256 PNG and committed in the same SQLite transaction as the session state. Its dimensions are stored with the image and validated on load.

The `SessionBrowserPanel` lazily decodes previews for visible `SessionCard` rows and releases their textures when leaving the Start Screen. These card previews are pictures only; they do not own an interactive Field state. Figure previews are rendered directly from figure cell data and are not stored as images.

### Asset Management

**Loading strategy:** Preload all static assets after window creation and before entering the application loop.

A presentation-layer `AssetStore` owns the interface font, optional icons, and raygui style data. It releases resources in reverse order before closing the raylib window.

Assets are resolved from an executable-relative installed asset directory configured by CMake, never from the process working directory. Explicit build and install rules copy required assets into their expected layouts.

If a custom font or icon fails to load, the UI falls back to raylib’s built-in font and text labels. Domain and application code reference semantic concepts rather than file paths or raylib handles.

No streaming, asynchronous loading, hot reload, or general-purpose resource manager is introduced.

### Dependencies and Build System

The project requires CMake 3.28 or newer and C++23 with compiler extensions disabled.

raylib 6.0, raygui 5.0, SQLite 3.53.4, and Catch2 3.15.3 are acquired from stable release archives through `FetchContent`. Each archive has a committed cryptographic hash, and configuration fails on mismatch.

Development branches, floating URLs, system-package fallbacks, and unpinned dependency versions are prohibited. Third-party examples and unnecessary targets are disabled.

raygui implementation is compiled in exactly one presentation-layer translation unit. Third-party headers cannot cross into domain or application targets.

`CMakePresets.json` defines consistent Debug, Release, and CI configurations.

### Testing

Catch2 3.15.3 is fetched only when `BUILD_TESTING` is enabled and registered through CTest using `catch_discover_tests`.

Domain and application tests run without opening a window or loading raylib. Tests use:

- A fake clock for simulation scheduling
- In-memory repositories for application workflows
- Temporary SQLite databases for persistence integration
- Deterministic random seeds reported on failure

Required coverage includes:

- Lone-cell death, stable block, blinker period two, and edge behavior
- Rectangular indexing and dimension validation
- Overflow and allocation-limit rejection
- Current/next buffer isolation
- Timing, pause, resume, and catch-up behavior
- Every legal and illegal state transition
- Pointer ownership and drag rasterization
- Logical sizing, one-time OS DPI scaling, high-DPI pointer normalization, and the 1280×720/960×540 viewport contract on macOS and Linux
- Camera clamping that keeps at least one in-field cell visible and gray out-of-field rendering/input exclusion
- Coordinate boundaries and invalid placement atomicity
- Session and Bank save/load round trips
- Unique-name enforcement
- Schema migration and transaction rollback
- Settings corruption recovery: preserve the invalid record, use safe in-memory defaults, and replace it only after explicit Save
- Recoverable Save failure Retry/Cancel behavior and fatal startup Acknowledge-to-exit behavior
- Visual verification of 21:1 cell contrast, UI contrast thresholds, focus rings, solid/dashed validity, textual errors, modal click-through prevention, and DPI scaling on macOS and Linux
- Internal verification budgets for default-frame work, maximum-field generation, and default-session save plus preview

Every corrected defect receives a regression test.

### Toolchains and Continuous Integration

GitHub Actions uses explicit runner labels:

- `macos-15`: AppleClang/LLVM 17 on ARM64
- `ubuntu-24.04`: Clang 18 on x86-64

Moving `*-latest` runner aliases are prohibited.

Both jobs configure and test Debug builds and compile Release builds. The Linux Debug job enables AddressSanitizer and UndefinedBehaviorSanitizer.

Project targets use:

- `-Wall`
- `-Wextra`
- `-Wpedantic`
- `-Wsign-conversion`

`-Wconversion` is explicitly prohibited.

Warnings are errors for project targets in CI but not for third-party dependencies. Tests run through CTest without opening a window. The presentation executable must still compile and link on both platforms.

Both required jobs must pass before merging. Packaging, signing, notarization, and distribution remain outside the current scope.

### Architecture Decision Records

#### ADR-001: Explicit State Machines Instead of Global State or ECS

The product has a small number of important modes and transitions but no entity population that warrants ECS. Explicit state machines make invalid transitions testable and prevent raylib callbacks from owning application behavior.

#### ADR-002: Dense Field Storage Instead of Sparse or Bit-Packed Storage

The field is finite, exact dead cells matter during figure replacement, and the learning goal favors readable algorithms. Dense byte buffers provide predictable indexing and adequate memory use under enforced limits.

#### ADR-003: Bounded Catch-Up Instead of Wall-Time Fidelity

Short frame delays should not change normal simulation speed, but a long stall must not trap the application in unbounded generation work. At the start of one main-loop iteration, the scheduler snapshots elapsed time and sets `steps = min(floor(accumulator / interval), 4)`. It executes exactly that many due generations sequentially without rendering intermediate states, subtracting one interval per step. It then discards any whole intervals still in the accumulator, retains only the sub-interval remainder, samples and translates raylib-exposed input, executes accepted commands, and renders once. Time spent executing the batch becomes elapsed time at the next iteration's clock sample and cannot enlarge the current batch. Discarded intervals are never repaid, so a long stall sacrifices wall-time fidelity without skipping a mathematical state between generations that are actually executed. Catch-up adds no project-owned input retention or replay queue.

#### ADR-004: Hashed FetchContent Dependencies

A fresh clone should obtain identical dependencies on macOS and Linux without requiring manual submodule commands or trusting system package versions.

#### ADR-005: SQLite for Transactional Local Persistence

Sessions and the shared Bank require uniqueness, atomic rename/delete behavior, schema evolution, and binary data. SQLite supplies those properties behind a narrow repository boundary.

#### ADR-006: Clang-Family Cross-Platform Baseline

AppleClang and upstream Clang reduce compiler-dialect variation while the two CI jobs still verify both operating systems and architectures. Only C++23 features compiling on both required jobs may enter the codebase.

#### ADR-007: Scheduled Simulation Before Same-Frame Input

The main loop snapshots the clock, processes the scheduled simulation batch, samples raylib-exposed input and translates it into commands, executes accepted commands, then renders. This preserves a single reproducible order and lets running edits appear in the current render without a special input buffer or a pause requirement. An input command cannot affect a scheduled step that has already completed earlier in that iteration. Input unavailable at the sampling phase is not retroactively replayed by project code. The exact wall-clock timer boundary at which a step is selected for an iteration is an implementation detail; the ordered phases and resulting state trace are the contract.

## Cross-cutting Concerns

These rules apply to every system and are mandatory for all implementations.

### Error Handling

**Strategy:** Typed result values with exception translation at system boundaries.

Expected failures return `Result<T, ErrorCode>`. Use `std::expected` only after it compiles under both required toolchains; otherwise provide one project-owned result type.

`ErrorCode` is a stable enum. Errors never propagate human-readable or technical message strings.

#### Error Categories

| Category | Handling |
| --- | --- |
| Expected validation outcome | Return a specific code or typed validation result; do not log |
| Recoverable technical failure | Log once at its origin, then propagate only `ErrorCode` |
| Fatal startup/runtime failure | Log once, present a concise mapped startup error when possible, exit after acknowledgment and return non-zero |
| Programmer error or broken invariant | Trigger a debug assertion and require a failing test |

#### Mandatory Rules

- Log each technical error exactly once, where it originates and where full context is available.
- Callers propagate or handle the error code without logging it again.
- Presentation maps `ErrorCode` to user-facing text.
- Never place preformatted message strings inside result objects.
- Invalid names and invalid placement are normal validation outcomes, not exceptions.
- Catch third-party, filesystem, SQLite, allocation, and standard-library exceptions at the nearest adapter that can translate them.
- A final exception boundary surrounds the application loop.
- Recoverable failures preserve the last valid domain state.
- A failed session save leaves the session open and paused.
- Missing optional visual assets use their documented fallback.
- No failed operation may leave partially mutated domain state.

#### Example

```cpp
auto SqliteSessionRepository::save(const Session& session)
    -> Result<void, ErrorCode>
{
    const int status = sqlite3_step(statement_);

    if (status != SQLITE_DONE) {
        logger_.error(
            "session.save_failed",
            {
                {"sqlite_status", status},
                {"session_id", session.id()}
            });

        return unexpected(ErrorCode::PersistenceWriteFailed);
    }

    return {};
}
```

The caller receives only `PersistenceWriteFailed`. It does not log it again. Presentation maps that code to an appropriate user-facing message.

### Logging

**Format:** Structured plain text  
**Destinations:** stderr and rotating local files  
**External telemetry:** None

Each entry contains:

1. UTC timestamp
2. Level
3. Stable event name
4. Human-readable diagnostic message
5. Escaped `key=value` context

#### Log Levels

| Level | Usage |
| --- | --- |
| `ERROR` | A technical operation failed |
| `WARN` | Unexpected behavior was handled through degradation or fallback |
| `INFO` | Application lifecycle and successful persistence milestones |
| `DEBUG` | State and performance diagnostics useful during development |
| `TRACE` | Explicitly enabled detailed development flow |

#### Mandatory Rules

- The project owns a framework-independent `Logger` interface.
- Domain functions do not log routine operations or expected validation results.
- The technical origin of an error logs it exactly once.
- Propagation sites must not duplicate error logs.
- Debug builds default to `DEBUG`; Release builds default to `INFO`.
- `TRACE` requires explicit developer configuration.
- raylib output is routed into the same logger by a presentation adapter.
- Never log cell BLOBs, complete fields, preview contents, or other large binary data.
- Never log inside the per-cell simulation loop.
- Aggregate performance data outside hot loops.
- Logging failures cannot terminate or alter application behavior.
- Retain three log files of at most 1 MiB each in the platform user-data directory.

#### Example

```text
2026-08-20T09:42:17Z INFO session.saved Saved session session_id=17 width=50 height=50
```

### Configuration

**Approach:** Typed layered configuration.

#### Configuration Structure

| Layer | Examples | Storage |
| --- | --- | --- |
| Compile-time invariants | Maximum dimensions, total-cell limit, catch-up limit, schema support | Named `constexpr` values |
| Build configuration | Dependency versions, warnings, debug tools, sanitizers | CMake and presets |
| Application defaults | Default generation interval, initial 100% zoom, and presentation thresholds | Typed structures at the composition root |
| Global settings | Generation interval and new-session field width/height defaults | Validated SQLite `settings` record |
| Session state | Fixed field dimensions and values belonging to one session | Session persistence |
| Platform values | User-data and installed-asset paths | Platform adapters |
| Developer settings | Log level and debug overlay | Debug-only command-line flags |

#### Mandatory Rules

- Every setting has exactly one owner, type, default, and validation function.
- Domain code receives validated typed values and never reads configuration storage.
- Invalid persisted Settings values are logged once at their loading origin, preserved in storage, and replaced only in memory with safe defaults until an explicit Save succeeds. Invalid session or figure records remain preserved and disabled when identifiable; they are never silently repaired or overwritten.
- The Settings panel writes one global settings record containing generation interval and new-session width/height defaults. Width/height changes affect creation only; existing session dimensions never change. The current global generation interval is read when a session is created or opened, and that transition clears the accumulator before the first interval.
- No generic string-key configuration interface is permitted.
- No JSON, YAML, remote configuration, or required environment variables are introduced.
- Runtime configuration cannot override architectural safety limits.
- Dependency versions and compiler settings remain build configuration.

#### Example

```cpp
struct SimulationSettings {
    std::chrono::milliseconds generationInterval{250};
};

auto validate(SimulationSettings settings)
    -> Result<SimulationSettings, ErrorCode>;
```

### Communication Between Systems

**Pattern:** Direct synchronous orchestration. There is no event system.

#### Synchronous operation feedback

Session load/save, Bank repository operations, and session-preview rendering/encoding are direct synchronous calls on the main thread. The owning surface may stage a named busy status before the call and must render the success or failure outcome after it returns. The entire window may stop rendering and accepting input while the call runs; the UX does not promise live progress, partial interactivity, or a progress animation. Changing that behavior requires a new asynchronous-execution architecture decision, including explicit threading and persistence-safety rules.

The flow is:

```text
raylib input
  → typed Command
  → ApplicationController::handle(command)
  → direct domain and repository calls
  → Result<Outcome, ErrorCode>
  → render current application state
```

#### Mandatory Rules

- Input commands are direct requests, not events.
- State transitions happen synchronously on the main thread.
- Successful operations return typed outcome data only when the caller needs it.
- Failures return only `ErrorCode`.
- Presentation renders current application state rather than subscribing to notifications.
- Persistence and preview generation are invoked directly by application use cases.
- No observer registry, signal system, event bus, message queue, event history, or replay mechanism.
- A future event mechanism requires a new architecture decision justified by multiple independent consumers or asynchronous execution.

#### Example

```cpp
auto ApplicationController::handle(const RenameFigure& command)
    -> Result<RenameFigureOutcome, ErrorCode>
{
    auto result = figureRepository_.rename(
        command.figureId,
        command.newName);

    if (!result) {
        return unexpected(result.error());
    }

    state_.bank.renameVisibleFigure(
        command.figureId,
        command.newName);

    return RenameFigureOutcome{command.figureId};
}
```

The repository logs any technical storage failure at its origin. The controller propagates only the code and does not log it again.

### Debug and Development Tools

**Approach:** Read-only diagnostic overlay in explicitly enabled development builds.

#### Activation

- Compile with `LIFE_GAME_ENABLE_DEBUG_TOOLS` through a development CMake preset.
- Launch with `--debug-overlay`.
- Toggle the enabled overlay with `F3`.
- Use `--log-level=trace` for detailed development logging.

Debug tools are excluded from normal Release builds.

#### Overlay Contents

- Current application screen
- Field mode and run state
- Field dimensions and live-cell count
- Generation number and configured interval
- Accumulator and catch-up count
- Camera position and zoom
- Visible cell bounds
- Render frame rate and recent frame duration

#### Mandatory Rules

- The overlay is presentation-only and read-only.
- It receives one immutable diagnostic snapshot after command processing.
- It cannot edit cells, alter timing, transition modes, or call repositories.
- Frame timing uses a fixed-size ring buffer without per-frame allocation.
- No debug console, command registry, cheat system, scripting interface, or editor is introduced.
- Tests, sanitizers, debugger tools, and logs remain the primary diagnostic mechanisms.

#### Example

```cpp
struct DiagnosticSnapshot {
    AppScreen screen;
    FieldMode fieldMode;
    RunState runState;
    FieldSize fieldSize;
    std::uint64_t generation;
    std::size_t liveCells;
    CameraState camera;
    VisibleCellRange visibleCells;
    FrameTiming frameTiming;
};
```

## Project Structure

### Repository Layout

```text
life-game/
├── .github/
│   └── workflows/
│       └── ci.yml
├── .agents/                         # Agent workflow definitions
├── _bmad/                           # BMAD configuration
├── _bmad-output/                    # Planning and architecture artifacts
├── for-human-context/               # Human-authored project guidance
├── assets/
│   ├── fonts/
│   ├── icons/
│   ├── images/
│   └── styles/
├── cmake/
│   ├── dependencies.cmake
│   ├── project-options.cmake
│   ├── sanitizers.cmake
│   └── warnings.cmake
├── src/
│   ├── CMakeLists.txt
│   ├── foundation/
│   │   ├── CMakeLists.txt
│   │   ├── error-code.hpp
│   │   └── result.hpp
│   ├── domain/
│   │   ├── CMakeLists.txt
│   │   ├── field/
│   │   │   ├── cell-coordinate.hpp
│   │   │   ├── cell-rectangle.cpp
│   │   │   ├── cell-rectangle.hpp
│   │   │   ├── field-dimensions.cpp
│   │   │   ├── field-dimensions.hpp
│   │   │   ├── field.cpp
│   │   │   └── field.hpp
│   │   ├── figure/
│   │   │   ├── figure-id.hpp
│   │   │   ├── figure-name.cpp
│   │   │   ├── figure-name.hpp
│   │   │   ├── figure.cpp
│   │   │   └── figure.hpp
│   │   ├── session/
│   │   │   ├── camera-state.hpp
│   │   │   ├── session-id.hpp
│   │   │   ├── session-name.cpp
│   │   │   ├── session-name.hpp
│   │   │   ├── session.cpp
│   │   │   └── session.hpp
│   │   └── simulation/
│   │       ├── life-stepper.cpp
│   │       └── life-stepper.hpp
│   ├── application/
│   │   ├── CMakeLists.txt
│   │   ├── bank/
│   │   │   ├── bank-service.cpp
│   │   │   └── bank-service.hpp
│   │   ├── commands/
│   │   │   ├── bank-command.hpp
│   │   │   ├── command.hpp
│   │   │   ├── field-command.hpp
│   │   │   └── session-command.hpp
│   │   ├── configuration/
│   │   │   ├── settings-service.cpp
│   │   │   ├── settings-service.hpp
│   │   │   ├── simulation-settings.hpp
│   │   │   └── user-settings.hpp
│   │   ├── controller/
│   │   │   ├── application-controller.cpp
│   │   │   └── application-controller.hpp
│   │   ├── diagnostics/
│   │   │   └── diagnostic-snapshot.hpp
│   │   ├── outcomes/
│   │   │   └── outcome.hpp
│   │   ├── ports/
│   │   │   ├── clock.hpp
│   │   │   ├── figure-repository.hpp
│   │   │   ├── logger.hpp
│   │   │   ├── session-preview-renderer.hpp
│   │   │   ├── session-repository.hpp
│   │   │   └── settings-repository.hpp
│   │   ├── session/
│   │   │   ├── session-service.cpp
│   │   │   └── session-service.hpp
│   │   ├── simulation/
│   │   │   ├── simulation-scheduler.cpp
│   │   │   └── simulation-scheduler.hpp
│   │   └── state/
│   │       ├── app-screen.hpp
│   │       ├── application-state.cpp
│   │       ├── application-state.hpp
│   │       ├── field-mode.hpp
│   │       └── run-state.hpp
│   ├── adapters/
│   │   ├── CMakeLists.txt
│   │   ├── clock/
│   │   │   ├── steady-clock.cpp
│   │   │   └── steady-clock.hpp
│   │   ├── logging/
│   │   │   ├── rotating-file-logger.cpp
│   │   │   └── rotating-file-logger.hpp
│   │   ├── persistence/
│   │   │   ├── memory/
│   │   │   │   ├── memory-figure-repository.cpp
│   │   │   │   ├── memory-figure-repository.hpp
│   │   │   │   ├── memory-session-repository.cpp
│   │   │   │   └── memory-session-repository.hpp
│   │   │   └── sqlite/
│   │   │       ├── cell-blob-codec.cpp
│   │   │       ├── cell-blob-codec.hpp
│   │   │       ├── schema-migrations.cpp
│   │   │       ├── schema-migrations.hpp
│   │   │       ├── sqlite-database.cpp
│   │   │       ├── sqlite-database.hpp
│   │   │       ├── sqlite-figure-repository.cpp
│   │   │       ├── sqlite-figure-repository.hpp
│   │   │       ├── sqlite-session-repository.cpp
│   │   │       ├── sqlite-session-repository.hpp
│   │   │       ├── sqlite-settings-repository.cpp
│   │   │       ├── sqlite-settings-repository.hpp
│   │   │       ├── sqlite-transaction.cpp
│   │   │       └── sqlite-transaction.hpp
│   │   └── platform/
│   │       ├── platform-paths.cpp
│   │       └── platform-paths.hpp
│   ├── presentation/
│   │   ├── CMakeLists.txt
│   │   ├── application/
│   │   │   ├── raylib-application.cpp
│   │   │   └── raylib-application.hpp
│   │   ├── assets/
│   │   │   ├── asset-store.cpp
│   │   │   └── asset-store.hpp
│   │   ├── camera/
│   │   │   ├── camera-controller.cpp
│   │   │   ├── camera-controller.hpp
│   │   │   ├── coordinate-converter.cpp
│   │   │   └── coordinate-converter.hpp
│   │   ├── diagnostics/
│   │   │   ├── debug-overlay.cpp
│   │   │   └── debug-overlay.hpp
│   │   ├── input/
│   │   │   ├── drag-rasterizer.cpp
│   │   │   ├── drag-rasterizer.hpp
│   │   │   ├── input-router.cpp
│   │   │   └── input-router.hpp
│   │   ├── raylib/
│   │   │   ├── raygui-implementation.cpp
│   │   │   ├── raylib-log-adapter.cpp
│   │   │   └── raylib-log-adapter.hpp
│   │   ├── rendering/
│   │   │   ├── field-renderer.cpp
│   │   │   ├── field-renderer.hpp
│   │   │   ├── figure-renderer.cpp
│   │   │   ├── figure-renderer.hpp
│   │   │   ├── session-preview-cache.cpp
│   │   │   ├── session-preview-cache.hpp
│   │   │   ├── session-preview-renderer.cpp
│   │   │   └── session-preview-renderer.hpp
│   │   ├── screens/
│   │   │   ├── field-screen.cpp
│   │   │   ├── field-screen.hpp
│   │   │   ├── start-screen.cpp
│   │   │   └── start-screen.hpp
│   │   └── ui/
│   │       ├── bank-panel.cpp
│   │       ├── bank-panel.hpp
│   │       ├── confirmation-dialog.cpp
│   │       ├── confirmation-dialog.hpp
│   │       ├── error-dialog.cpp
│   │       ├── error-dialog.hpp
│   │       ├── name-dialog.cpp
│   │       ├── name-dialog.hpp
│   │       ├── numeric-field.cpp
│   │       ├── numeric-field.hpp
│   │       ├── session-browser-panel.cpp
│   │       ├── session-browser-panel.hpp
│   │       ├── session-card.cpp
│   │       ├── session-card.hpp
│   │       ├── session-card-list.cpp
│   │       ├── session-card-list.hpp
│   │       ├── settings-panel.cpp
│   │       ├── settings-panel.hpp
│   │       ├── status-message.cpp
│   │       ├── status-message.hpp
│   │       ├── text-field.cpp
│   │       ├── text-field.hpp
│   │       ├── toolbar.cpp
│   │       └── toolbar.hpp
│   └── bootstrap/
│       ├── composition-root.cpp
│       ├── composition-root.hpp
│       ├── main.cpp
│       ├── runtime-options.cpp
│       └── runtime-options.hpp
├── tests/
│   ├── CMakeLists.txt
│   ├── support/
│   │   ├── fake-clock.hpp
│   │   ├── recording-logger.hpp
│   │   └── sample-fields.hpp
│   ├── unit/
│   │   ├── adapters/
│   │   │   ├── memory-figure-repository-test.cpp
│   │   │   └── memory-session-repository-test.cpp
│   │   ├── application/
│   │   │   ├── application-controller-test.cpp
│   │   │   ├── simulation-scheduler-test.cpp
│   │   │   └── state-transitions-test.cpp
│   │   ├── domain/
│   │   │   ├── cell-rectangle-test.cpp
│   │   │   ├── field-dimensions-test.cpp
│   │   │   ├── field-test.cpp
│   │   │   ├── figure-test.cpp
│   │   │   └── life-stepper-test.cpp
│   │   └── presentation/
│   │       ├── coordinate-converter-test.cpp
│   │       ├── drag-rasterizer-test.cpp
│   │       └── input-router-test.cpp
│   ├── integration/
│   │   ├── adapters/
│   │   │   ├── schema-migrations-test.cpp
│   │   │   ├── sqlite-figure-repository-test.cpp
│   │   │   ├── sqlite-session-repository-test.cpp
│   │   │   ├── sqlite-settings-repository-test.cpp
│   │   │   └── transaction-rollback-test.cpp
│   │   └── application/
│   │       ├── bank-workflow-test.cpp
│   │       └── session-workflow-test.cpp
│   └── regression/
│       ├── adapters/
│       ├── application/
│       ├── domain/
│       └── presentation/
├── .clang-format
├── .gitignore
├── CMakeLists.txt
├── CMakePresets.json
└── README.md
```

### Target Dependency Direction

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

- `foundation` contains only `Result` and message-free `ErrorCode`.
- `domain` has no raylib, SQLite, filesystem, logger, or application dependencies.
- `application` owns synchronous orchestration, commands, state machines, ports, and use cases.
- `adapters` implements persistence, logging, clock, and platform ports.
- `presentation` owns all raylib/raygui types and implements preview rendering.
- `bootstrap` is the composition root and contains no gameplay rules.
- No dependency is permitted between `adapters` and `presentation`.
- `sqlite3.h` stays inside `adapters/persistence/sqlite/`.
- `raylib.h` and `raygui.h` stay inside `presentation/`.
- `RAYGUI_IMPLEMENTATION` appears only in `raygui-implementation.cpp`.

### System Ownership

| System | Primary location |
| --- | --- |
| Field representation and boundaries | `domain/field/` |
| Conway generation algorithm | `domain/simulation/` |
| Figure data and exact replacement | `domain/figure/` |
| Session state and saved camera | `domain/session/` |
| Application, tool, and run state machines | `application/state/` |
| Fixed-step scheduling | `application/simulation/` |
| Session and Bank workflows | `application/session/`, `application/bank/` |
| Input ownership and drag capture | `presentation/input/` |
| Coordinate conversion and camera | `presentation/camera/` |
| Start Screen, session browser, and Settings ownership | `presentation/screens/start-screen`, `presentation/ui/session-browser-panel`, `presentation/ui/settings-panel` |
| Interactive Field Screen and Toolbar | `presentation/screens/field-screen`, `presentation/ui/toolbar` |
| Bank, dialogs, reusable fields, and status feedback | `presentation/ui/` |
| Field and UI rendering | `presentation/rendering/`, `presentation/screens/`, `presentation/ui/` |
| SQLite and in-memory storage | `adapters/persistence/` |
| Static runtime assets | `assets/` |
| Runtime construction | `bootstrap/` |

Session previews travel through the `SessionPreviewRenderer` application port. The application invokes the presentation implementation directly and then passes the resulting PNG bytes to the session repository. This does not create an adapter-to-presentation dependency or an event mechanism.

### Naming Rules

- Files and multiword directories: `kebab-case`
- Test files: `<subject>-test.cpp`
- Functions, variables, and namespaces: `camelCase`
- Types: `PascalCase`
- Constants: `CAPS_LIKE_THIS`
- Root namespace: `lifeGame`
- Layer namespaces: `lifeGame::domain`, `lifeGame::application`, `lifeGame::adapters`, and `lifeGame::presentation`
- CMake targets: `life-game-foundation`, `life-game-domain`, `life-game-application`, `life-game-adapters`, `life-game-presentation`, and `life-game`

### Filesystem Rules

- Builds are out-of-source under ignored `build/` directories.
- FetchContent dependencies remain under the build tree; there is no `vendor/` directory.
- `assets/` is read-only runtime content copied by CMake beside the executable and into the install layout.
- SQLite data, preview BLOBs, settings, and rotating logs live only in the platform user-data directory.
- Runtime code never writes into the source tree or installed asset directory.
- Regression directories gain files only when an actual defect is corrected; no placeholder tests are created.

## Implementation Patterns

These patterns ensure that future developers and AI agents produce compatible implementations.

### Novel Pattern

#### Validated Rectangular Field Patch

**Purpose:** Replace an exact rectangular field region—including live and dead cells—without partial mutation.

**Components:**

- `Figure` owns validated dimensions and contiguous row-major cell bytes.
- `FieldPatch` is an immutable synchronous view of a figure at an integer anchor.
- `Field::placeFigure(...)` validates and commits the patch.
- `BankService` handles the resulting application transition.

**Data flow:**

```text
Figure + anchor
  → immutable FieldPatch
  → validate complete representation and bounds
  ├── invalid → RejectedOutOfBounds; field unchanged
  └── valid   → non-failing row-copy commit → Placed
  → exit Bank
  → select Live
  → resume with a fresh simulation interval
```

**Implementation rules:**

- `Figure` guarantees positive dimensions, exact byte count, and only `0`/`1` values.
- Bounds are checked before the first field write.
- Bounds calculations must not use unchecked coordinate addition.
- The commit phase performs no allocation, callbacks, logging, or other fallible work.
- Every patch byte is copied; dead cells overwrite live cells.
- Rejection is an expected outcome and is not logged.
- Rejection leaves the complete field byte-for-byte unchanged.
- Placement does not advance the generation number.
- `FieldPatch` is not stored after the synchronous call.

**Example:**

```cpp
enum class PlacementOutcome {
    Placed,
    RejectedOutOfBounds
};

struct FieldPatch {
    CellCoordinate topLeft;
    FieldDimensions dimensions;
    std::span<const std::uint8_t> cells;
};

auto Field::placeFigure(const FieldPatch& patch) -> PlacementOutcome
{
    if (!fitsCompletely(patch)) {
        return PlacementOutcome::RejectedOutOfBounds;
    }

    const auto targetX = static_cast<std::size_t>(patch.topLeft.x);
    const auto targetY = static_cast<std::size_t>(patch.topLeft.y);

    for (std::size_t row = 0; row < patch.dimensions.height; ++row) {
        const auto sourceOffset = row * patch.dimensions.width;
        const auto targetOffset =
            (targetY + row) * dimensions_.width + targetX;

        const auto sourceRow = patch.cells.subspan(
            sourceOffset,
            patch.dimensions.width);

        auto targetCells = std::span<std::uint8_t>{
            cells_.data(),
            cells_.size()};
        auto targetRow = targetCells.subspan(
            targetOffset,
            patch.dimensions.width);

        std::ranges::copy(sourceRow, targetRow.begin());
    }

    return PlacementOutcome::Placed;
}
```

Tests must cover negative anchors, oversized figures, exact edge fits, every boundary, dead-cell replacement, maximum dimensions, and unchanged-field verification after rejection.

### Communication Pattern

**Pattern:** Constructor dependency injection with direct synchronous calls.

The composition root constructs concrete adapters and injects their interfaces. Dependencies are explicit references with lifetimes longer than their consumers.

```cpp
class ApplicationController {
public:
    ApplicationController(
        SessionRepository& sessionRepository,
        FigureRepository& figureRepository,
        SessionPreviewRenderer& previewRenderer,
        Clock& clock,
        Logger& logger);

    auto handle(const Command& command)
        -> Result<Outcome, ErrorCode>;

private:
    SessionRepository& sessionRepository_;
    FigureRepository& figureRepository_;
    SessionPreviewRenderer& previewRenderer_;
    Clock& clock_;
    Logger& logger_;
};
```

Rules:

- Do not introduce service locators, global registries, observers, or event buses.
- Commands are direct requests, not events.
- Calls complete synchronously on the main thread.
- A failed call propagates only `ErrorCode`.
- The component where a technical failure occurs logs it exactly once.
- Callers never log the propagated error again.

### Domain Object Creation Pattern

**Pattern:** Validating static factories for invariant-bearing types.

Constructors remain private when arbitrary values could create an invalid object. Invariant-free commands, outcomes, and snapshots remain aggregate structs.

```cpp
class FieldDimensions {
public:
    static auto create(
        std::size_t width,
        std::size_t height)
        -> Result<FieldDimensions, ErrorCode>
    {
        if (width == 0 || height == 0 ||
            width > MAX_FIELD_SIDE ||
            height > MAX_FIELD_SIDE ||
            width > MAX_TOTAL_CELLS / height) {
            return unexpected(ErrorCode::InvalidFieldDimensions);
        }

        return FieldDimensions{width, height};
    }

    std::size_t width;
    std::size_t height;

private:
    FieldDimensions(std::size_t widthValue, std::size_t heightValue)
        : width{widthValue},
          height{heightValue}
    {
    }
};
```

Rules:

- Invalid domain values cannot exist temporarily.
- Expected validation failures are not logged.
- Factories return typed values or message-free error codes.
- Builders and injected factory services are not used for simple domain objects.
- Field cells are bytes in a dense buffer, not individually allocated entities.

### State Transition Pattern

**Pattern:** Named, typed transition methods.

Only state-transition methods may mutate `AppScreen`, `FieldMode`, or `RunState`. Controllers must not assign their underlying enums directly.

```cpp
auto ApplicationController::handle(const PlaceFigure& command)
    -> Result<PlaceFigureOutcome, ErrorCode>
{
    const auto patch = makeFieldPatch(
        state_.selectedFigure(),
        command.topLeft);

    const auto placement = state_.field().placeFigure(patch);

    state_.resumeLive();
    simulationScheduler_.restart(clock_.now());

    return PlaceFigureOutcome{placement};
}
```

Rules:

- Each legal transition has one named implementation.
- Transition methods update all affected state axes together.
- Invalid transitions return a typed validation outcome and do not mutate state.
- Resume always selects Live and starts a fresh generation interval.
- Raw state setters are prohibited.
- Boolean combinations must not replace the approved state enums.
- Every legal and illegal transition requires a focused test.

### Data Access Pattern

**Pattern:** Injected repository ports.

Application code accesses persistent data exclusively through narrow interfaces. SQLite and in-memory implementations obey the same contract.

```cpp
class FigureRepository {
public:
    virtual ~FigureRepository() = default;

    virtual auto list()
        -> Result<std::vector<FigureSummary>, ErrorCode> = 0;

    virtual auto load(FigureId figureId)
        -> Result<Figure, ErrorCode> = 0;

    virtual auto save(const Figure& figure)
        -> Result<FigureId, ErrorCode> = 0;

    virtual auto rename(FigureId figureId, const FigureName& name)
        -> Result<void, ErrorCode> = 0;

    virtual auto remove(FigureId figureId)
        -> Result<void, ErrorCode> = 0;
};
```

Rules:

- Domain and presentation code never access SQLite or files.
- Repository interfaces expose domain values and typed summaries—not SQL rows or handles.
- Transactions are owned by concrete repository operations or an explicitly named application use case.
- SQLite adapters log technical failures at the failing SQLite operation.
- Repository callers propagate the returned code without logging it.
- Tests use in-memory repositories or temporary SQLite databases.
- Direct file access and global data managers are prohibited.

### Lifecycle and Recovery Pattern

**Pattern:** Preserve valid state, report once, and let the user retry.

Recoverable operations are synchronous and never retry automatically. A failure preserves the last valid state and returns only `ErrorCode`; presentation maps that code to user-facing text.

```cpp
auto ApplicationController::leaveSession()
    -> Result<void, ErrorCode>
{
    state_.pauseForSave();
    simulationScheduler_.clear();

    auto preview = previewRenderer_.render(state_.sessionSnapshot());
    if (!preview) {
        return unexpected(preview.error());
    }

    auto saved = sessionRepository_.save(
        state_.sessionSnapshot(),
        std::move(*preview));
    if (!saved) {
        return unexpected(saved.error());
    }

    state_.showSessionBrowser();
    return {};
}
```

Rules:

- Failed save or preview generation leaves the session open and paused.
- The user may retry the originating command; the system performs no automatic retry.
- Loading a damaged record does not mutate current application state.
- Damaged records are never automatically repaired, overwritten, or deleted.
- Failure to open or migrate the required database blocks startup and returns a mapped fatal error.
- A fatal database error shows the startup `ErrorDialog`; acknowledgment exits without opening the Start Screen or creating a replacement database. An isolated damaged session or figure remains visible as a disabled item when identifiable, while valid records remain usable.
- Optional font or icon failure uses the documented fallback and does not block startup.
- Closing an open session uses the same save path; save failure cancels closing and leaves the session paused.
- The technical origin logs the failure exactly once. Propagation sites do not log it.

### Consistency Rules

| Concern | Convention | Enforcement |
| --- | --- | --- |
| Files | `kebab-case.hpp/.cpp` | Review and `.clang-format` companion policy |
| Functions and variables | `camelCase` | Compiler review and code review |
| Types | `PascalCase` | Code review |
| Constants | `CAPS_LIKE_THIS` | Code review |
| Namespaces | `lifeGame::layerName` | Layer CMake targets |
| Dependencies | Constructor injection | Composition-root review |
| Communication | Direct synchronous commands | No event or observer infrastructure |
| Domain creation | Validating static factories | Private constructors and tests |
| State mutation | Named transition methods | No raw state setters |
| Data access | Repository ports | No SQLite headers outside its adapter |
| Recovery lifecycle | Preserve state; no automatic retry or destructive repair | Failure-path tests |
| Errors | Log once at origin; propagate only `ErrorCode` | Error-path tests and review |
| Field and figure format | Dense row-major bytes containing only `0` or `1` | Factory validation |
| Rectangles | Half-open bounds; complete validation before writes | Boundary tests |
| Third-party APIs | Contained in adapter or presentation targets | CMake dependency boundaries |
| Tests | Scope first, then owning architecture layer | Test directory and target layout |
| Regression tests | `<subject>-test.cpp` under the owning regression layer | Required for every corrected defect |

## Architecture Validation

### Validation Summary

| Check | Result | Notes |
| --- | --- | --- |
| Decision compatibility | PASS | Engine boundaries, direct communication, repositories, state machines, and error rules align. |
| GDD coverage | PASS | All nine core systems and technical requirement groups have architectural support. |
| Pattern completeness | PASS | Creation, communication, state, errors, data, lifecycle, and explicit no-event handling are defined. |
| Epic mapping | PASS | All six epics map to concrete modules and patterns. |
| Document completeness | PASS | Required sections, examples, versions, naming rules, and the complete source tree are present. |

### Core-system Coverage

| System | Architecture support | Status |
| --- | --- | --- |
| Finite field and dead boundaries | Dense validated `Field` domain model | PASS |
| Synchronous Life simulation | Double buffers and fixed-step scheduler | PASS |
| Rendering, camera, and zoom | Visible-range renderer and centralized conversion | PASS |
| Tools and input | Typed commands, ownership priority, and pointer capture | PASS |
| Figure capture and Bank | Figure domain, Bank service, repository, and validated patch | PASS |
| Persistent sessions | Session service and transactional SQLite repositories | PASS |
| Immediate-mode UI | Presentation screens, dialogs, toolbar, and explicit modes | PASS |
| Cross-platform build | Pinned CMake dependencies and two required CI jobs | PASS |
| Deterministic verification | Catch2, CTest, fake clock, and repository tests | PASS |

### Epic Mapping

| Epic | Primary locations | Governing patterns | Status |
| --- | --- | --- | --- |
| 1 — Field MVP | `domain/field`, `domain/simulation`, `presentation/rendering` | Factories and direct calls | PASS |
| 2 — Editing and Observation | `application/simulation`, `application/state`, `presentation/input` | Typed transitions | PASS |
| 3 — Field Navigation and Setup | `domain/session`, `presentation/camera` | Factories and direct commands | PASS |
| 4 — Figure Capture and Bank | `domain/figure`, `application/bank`, `presentation/ui` | Validated rectangular patch | PASS |
| 5 — Persistent Sessions | `application/session`, `adapters/persistence`, preview rendering | Repositories and recovery lifecycle | PASS |
| 6 — Cross-Platform Completion | `cmake`, `.github/workflows`, tests, and cross-cutting concerns | Error, logging, and consistency rules | PASS |

### Coverage Report

- Systems covered: **9/9**
- Epics mapped: **6/6**
- Patterns defined: **6**
- Major architectural decisions: **12**
- Unresolved architectural conflicts: **0**

### Issues Resolved

- Removed obsolete unresolved-decision language.
- Added the required executive summary.
- Documented non-applicable API and authentication concerns.
- Consolidated UX-owned deferrals with owning-epic gates.
- Added explicit persistence and damaged-data recovery behavior.
- Clarified third-party dependency boundaries.
- Corrected the patch example to remain compatible with `-Wsign-conversion`.
- Confirmed that `-Wconversion` is explicitly prohibited.
- Removed all template and placeholder text.
- Reconciled UX-A1 by making scheduled simulation → input → render the authoritative frame order and removing any current-generation inclusion promise.
- Reconciled UX-A2 by adopting the four-generation catch-up cap, discarding excess elapsed backlog, rendering only after the catch-up batch, and retiring contradictory slow-simulation UX promises.
- Reconciled UX-A3 by confirming the discrete zoom levels, 4-pixel grid threshold, shared name-validation contract, Bank-delete confirmation, and 256×256 session-preview size.
- Reconciled UX-A4 by making Start Screen, session-card preview, Settings panel, interactive Field Screen, Toolbar, Bank panel, dialogs, reusable fields, and status feedback explicit presentation owners.
- Reconciled UX-A5 by accepting synchronous blocking for session, Bank, and preview operations, defining pre-operation/post-operation status feedback without live-progress claims, and making Bank opening pause the Field with a fresh-interval resume path.
- Reconciled UX-A6 by replacing session-index/file recovery language with SQLite failure categories: fatal database-open/migration failure exits after a startup error acknowledgment, while isolated damaged session or figure records remain preserved and disabled without blocking valid records.
- Reconciled UX-A8 by making logical client sizing and one-time OS DPI scaling authoritative: the initial viewport is 1280×720 logical px, the minimum supported viewport is 960×540 logical px, and one centralized input service maps high-DPI pointer coordinates to logical UI and Field coordinates before hit testing.
- Reconciled UX-A9 by requiring camera bounds to keep at least one in-field cell visible and defining gray out-of-field rendering as presentation-only space that is neither a cell state nor an input target.
- Reconciled the 2026-08-24 readiness findings by assigning the Start Screen/session shell and global Settings to Epic 3, deferring SQLite persistence to Epic 5, adding explicit story specifications and FR/NFR traceability, and separating player-facing cross-platform stories from release gates.
- Reconciled invalid Settings recovery, recoverable Retry/Cancel actions, fixed session/Bank capacities, internal performance budgets, and visual verification evidence across UX, architecture, and story specifications.

### Validation Date

2026-08-25

**Overall Status:** PASS

## Development Environment

### Prerequisites

Common requirements:

- CMake 3.28 or newer
- Git
- A C++23 compiler and standard library matching the selected platform baseline

macOS development uses Xcode Command Line Tools and the AppleClang 17 baseline selected by this architecture:

```bash
xcode-select --install
```

Ubuntu 24.04 development uses Clang 18. Install the compiler, build tools, and the X11, Wayland, OpenGL, and ALSA development packages listed by the [official raylib Linux guide](https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux):

```bash
sudo apt update
sudo apt install build-essential git cmake clang-18 \
  libasound2-dev libx11-dev libxrandr-dev libxi-dev \
  libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev \
  libxinerama-dev libwayland-dev libxkbcommon-dev
```

The Linux presets set `CMAKE_C_COMPILER=clang-18` and `CMAKE_CXX_COMPILER=clang++-18`. macOS presets use the active Xcode command-line compiler.

### CMake Presets

`CMakePresets.json` defines matching configure, build, and test presets. The primary local workflow uses the [official CMake preset command forms](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html):

```bash
cmake --preset dev-debug
cmake --build --preset dev-debug
ctest --preset dev-debug --output-on-failure
```

The read-only diagnostic overlay uses a separate development preset:

```bash
cmake --preset dev-debug-tools
cmake --build --preset dev-debug-tools
ctest --preset dev-debug-tools --output-on-failure
```

The remaining required preset families are `dev-release`, `ci-linux-debug-asan`, `ci-linux-release`, `ci-macos-debug`, and `ci-macos-release`. CI Debug presets build and test; CI Release presets must compile successfully.

### Dependency Acquisition

Initial configuration downloads the pinned raylib, raygui, SQLite, and optional Catch2 release archives through CMake `FetchContent`. Archive hashes are verified during configuration. No dependency is installed globally and no system-package fallback is used.

### AI Documentation Tooling

Context7 is optional and development-only. It provides current library documentation but no raylib scene inspection or runtime control.

| Tool | Purpose | Install type |
| --- | --- | --- |
| Context7 | Version-aware raylib, raygui, CMake, and C++ documentation | Local or hosted MCP |

A local setup requires Node.js 20.18.1 or newer:

```bash
npx ctx7 setup
```

No engine-specific raylib MCP was selected.

### First Implementation Steps

1. Create the root CMake files, presets, and approved target structure.
2. Implement `foundation`, the field domain, and deterministic Life rules.
3. Wire the minimal raylib Field MVP without Bank or persistent sessions.
4. Pass the canonical Life, boundary, timing, and input-coordinate tests before adding post-MVP systems.

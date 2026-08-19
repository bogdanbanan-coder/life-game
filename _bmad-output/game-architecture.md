---
title: 'Game Architecture'
project: 'life-game'
date: '2026-08-19'
author: 'bogdan'
version: '1.0'
stepsCompleted: [1, 2, 3]
status: 'in-progress'

# Source Documents
gdd: '_bmad-output/planning-artifacts/gdds/gdd-life-game-2026-08-19/gdd.md'
epics: '_bmad-output/planning-artifacts/gdds/gdd-life-game-2026-08-19/epics.md'
brief: '_bmad-output/planning-artifacts/briefs/brief-life-game-2026-08-15/brief.md'
---

# Game Architecture

## Document Status

This architecture document is being created through the GDS Architecture Workflow.

**Steps Completed:** 3 of 9 (Engine & Starter Selection)

---

_Content will be added as we progress through the workflow._

## Project Context

### Game Overview

**Life Game** is a private, single-user Conway’s Game of Life study sandbox. The player edits a finite cellular field, observes deterministic evolution, captures exact rectangular constructions, and reuses them across persistent experimental sessions.

The current product scope deliberately excludes multiplayer, online services, progression, narrative, audio, scripting, infinite fields, and alternate cellular rules.

### Technical Scope

**Platform:** macOS and Linux desktop, with equal priority  
**Genre:** Sandbox / cellular-automaton simulation  
**Project Level:** Medium complexity  
**Technology:** C++23, CMake, raylib 6.0, raygui 5.0

The simulation and persistence domains must remain independent of raylib and raygui. Framework dependencies belong at the application and presentation boundary.

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
- Treat every coordinate outside the fixed field as permanently dead.
- Support rectangular fields whose dimensions remain fixed for a session.
- Preserve exact cell state, dimensions, camera position, zoom, and last-view preview.
- Maintain one persistent figure Bank shared by all sessions.
- Store both live and dead cells in captured rectangular figures.
- Make invalid out-of-bounds placement atomic: no field cells may change.
- Keep mouse editing, camera movement, selection, dialogs, and staged placement behavior mutually unambiguous.

No formal frame-rate, resolution, memory, or load-time target is currently defined. Window resizing is optional.

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

### Technical Risks

- Maximum field dimensions are undefined, preventing a final memory and rendering bound.
- Generation catch-up behavior after a delayed frame is unspecified.
- Persistence format, migration, corruption recovery, and save-failure behavior remain undefined.
- Zoom levels, limits, naming rules, and some confirmation behavior remain deferred.
- The UX documents are currently placeholders, so architecture must avoid inventing finalized interaction details.
- raygui’s immediate-mode model requires disciplined ownership of modal and tool state.
- C++23 compiler and standard-library support may differ between the macOS and Linux toolchains.
- MVP scope may expand prematurely into Bank and persistent-session infrastructure.

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

The dependency acquisition method, targets, directory structure, test framework, compiler warnings, and platform presets remain explicit architectural decisions.

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

### Remaining Architectural Decisions

The following decisions still require explicit treatment:

1. Dependency acquisition and pinning strategy.
2. Source, test, asset, and platform directory structure.
3. Domain boundaries and target dependency directions.
4. Dense field representation and maximum dimensions.
5. Simulation buffering and timing policy.
6. Application-screen and Field-mode state machines.
7. Camera and cell-coordinate conventions.
8. Rendering strategy for grid, cells, selection, and previews.
9. Persistence schema, versioning, and atomic writes.
10. Error handling and damaged-data recovery.
11. Test framework and deterministic verification seams.
12. macOS and Linux compiler, preset, and CI policy.

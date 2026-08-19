---
title: "Life Game — Development Epics"
game_type: sandbox
secondary_descriptor: simulation
status: final
created: 2026-08-19
updated: 2026-08-19
---

# Life Game — Development Epics

## Sequence and Traceability

| Epic | Pillars delivered | Depends on |
|---|---|---|
| 1. Field MVP | Direct Manipulation, Deterministic Evolution | None |
| 2. Editing and Observation | Direct Manipulation, Deterministic Evolution | Epic 1 |
| 3. Field Navigation and Setup | Direct Manipulation | Epic 1 |
| 4. Figure Capture and Bank | Preservation, Composition | Epics 2–3 |
| 5. Persistent Sessions | Preservation, Composition | Epic 4 |
| 6. Cross-Platform Completion | All pillars | Epics 1–5 |

## Epic 1 — Field MVP

**Goal:** Deliver the smallest complete Life experiment: a visible finite field, direct Live input, and correct scheduled evolution.

**High-level stories:**

1. As the player, I see a small lined **50×50** field and compact upper-right controls when the application starts.
2. As the player, I click a visible cell in Live mode and that exact cell becomes alive.
3. As the player, I watch the whole field advance synchronously under standard Conway rules every **0.25 seconds**.
4. As the player, I observe permanent-dead boundaries rather than wrapping or field expansion.
5. As the designer, I can demonstrate lone-cell death, stable block behavior, blinker period two, and correct edge behavior.

**Completion outcome:** Field runs on macOS and Linux development environments without Bank, sessions, settings, camera navigation, or persistence.

## Epic 2 — Editing and Observation

**Goal:** Let the player deliberately change and inspect a running experiment.

**High-level stories:**

1. As the player, I draw across cells with Live to make them alive.
2. As the player, I draw across cells with Die to make them dead.
3. As the player, I pause generation updates without losing field state.
4. As the player, I resume updates and return automatically to Live.
5. As the player, I configure a positive generation interval whose default is **0.25 seconds**.
6. As the player, I can always see which persistent tool is selected.

**Completion outcome:** Direct editing, observation, and timing control work without introducing goals, scores, or progression.

## Epic 3 — Field Navigation and Setup

**Goal:** Support fields larger than the current view while keeping field geometry stable.

**High-level stories:**

1. As the player, I create a session with independently configured width and height.
2. As the player, I press Move and drag the camera without editing cell state.
3. As the player, I press `+` or `−` to change zoom by one level.
4. As the player, I navigate without changing field dimensions or cell coordinates.
5. As the player, I see useful grid lines at editing zoom levels without excessive visual noise when zoomed out.

**Completion outcome:** Camera movement and zoom work on fixed square or rectangular fields without editing cells or changing field geometry.

## Epic 4 — Figure Capture and Bank

**Goal:** Preserve exact rectangular constructions and compose them onto any session field.

**High-level stories:**

1. As the player, I drag Highlight between two cells to select the inclusive rectangle and pause simulation.
2. As the player, I save the rectangle under a unique name or cancel without changing Bank.
3. As the player, I open the application-wide Bank and browse figures by name.
4. As the player, I rename or delete a saved figure through dedicated controls.
5. As the player, I select a figure and move its translucent preview across the field.
6. As the player, I press Resume to replace every underlying live/dead cell in a valid target rectangle.
7. As the player, I receive no placement when any part of the staged rectangle is outside the field; Bank closes, Live returns, and simulation resumes.

**Completion outcome:** Figures are exact reusable rectangular bitmaps shared across the application.

## Epic 5 — Persistent Sessions

**Goal:** Preserve multiple named experimental workspaces and one shared Bank across application restarts.

**High-level stories:**

1. As the player, I browse session cards horizontally by unique name and last-view preview.
2. As the player, I create a session through a naming dialog that rejects duplicate names.
3. As the player, I rename a session through its dedicated control without creating a duplicate.
4. As the player, I delete a session only after confirmation.
5. As the player, I leave a session and automatically save cells, dimensions, camera position, zoom, and preview.
6. As the player, I reopen a session at its saved field and view while generation updates begin immediately.
7. As the player, I use one persistent Bank across all sessions, and deleting a session never deletes its figures.

**Completion outcome:** Session and Bank state survive application restart without cross-session ownership errors.

## Epic 6 — Cross-Platform Completion

**Goal:** Make the confirmed product scope dependable and visually coherent on macOS and Linux.

**High-level stories:**

1. As the player, I get equivalent Life rules, timing semantics, input behavior, and persistence on both platforms.
2. As the player, I can distinguish live cells, dead cells, selection, staged figures, and the active tool at a glance.
3. As the player, I encounter clear failure behavior for duplicate names, invalid placement, missing or damaged saved data, and unsupported actions.
4. As the developer, I can build with pinned raylib/raygui dependencies and the verified C++23 subset on both target toolchains.
5. As the designer, I can verify every product success metric without relying on narrative, audio, multiplayer, or performance-at-scale features.

**Completion outcome:** The complete local single-user product is ready for continued personal experimentation.

## Deferred Beyond These Epics

- Program/source replication triggered by `flex`.
- AI-assisted mathematical reasoning and evidence.
- A semantic/versioned construction workbench with provenance and replayable experiments.
- Any future feature currently listed as explicitly out of scope in the GDD.

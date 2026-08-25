---
title: "Life Game — Development Epics"
game_type: sandbox
secondary_descriptor: simulation
status: final
created: 2026-08-19
updated: 2026-08-24
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
5. As the player, I configure the global positive generation interval whose default is **0.25 seconds**.
6. As the player, I can always see which persistent tool is selected.

**Completion outcome:** Direct editing, observation, and timing control work without introducing goals, scores, or progression.

#### UX-A1 — Same-frame input versus scheduled generation acceptance criteria

These criteria apply to Epic 1 Story 3, Epic 2 Stories 1–2, and Epic 6 Story 1:

- **Given** a Running field in an iteration where the scheduler executes a generation, **when** the iteration is processed, **then** the clock snapshot and scheduled simulation batch complete before raylib-exposed input is sampled and translated, accepted Live/Die commands execute afterward, and rendering occurs last.
- **Given** a Running field and an accepted Live or Die edit, **when** the input command is processed, **then** the current field changes immediately and the edit is visible in that frame; the player is not required to pause and the edit is not placed in a special buffer.
- **Given** a scheduled generation and an accepted edit in the same rendered frame, **when** the ordered phases complete, **then** the generation uses the field state that existed before the input phase, the edit is applied to the current field afterward, and no user-facing behavior promises that the edit affected the generation already processed.
- **Given** the same initial field, controlled clock trace, and input trace, **when** the run is repeated, **then** the generation sequence, post-input field state, and rendered result are identical. The test does not require a particular wall-clock assignment at an exact timer boundary.
- **Given** equivalent controlled traces on macOS and Linux, **when** the frame loop is exercised, **then** both platforms produce the same simulation → input → render ordering and visible result.

#### UX-A2 — Delayed-frame catch-up acceptance criteria

These criteria apply to Epic 1 Story 3, Epic 2 Stories 1–2 and 5, and Epic 6 Story 1:

- **Given** a Running field at the start of a main-loop iteration, **when** elapsed time is sampled, **then** the scheduler snapshots `steps = min(floor(accumulator / interval), 4)` and does not enlarge that batch using time elapsed later in the same iteration.
- **Given** a snapshot containing one to four complete generation intervals, **when** the iteration runs, **then** exactly that many due generations execute sequentially before input and rendering, and only the completed field after the batch is rendered.
- **Given** a snapshot containing more than four complete generation intervals, **when** the iteration runs, **then** exactly four due generations execute, additional whole-generation timing debt is discarded with `accumulator %= interval`, and the sub-interval remainder is retained.
- **Given** a catch-up batch, **when** each generation executes, **then** it consumes the preceding completed field state and publishes a complete next state; no executed mathematical state is merged, partially applied, or skipped.
- **Given** a catch-up batch, **when** the batch completes, **then** the application samples raylib-exposed input, translates it into commands, executes accepted commands, and renders once. An accepted Live or Die command mutates the resulting current field and affects only future scheduled generations; input not exposed at that sampling phase receives no project-owned retention or replay guarantee.
- **Given** discarded timing debt, **when** subsequent iterations run normally, **then** the scheduler does not repay the discarded intervals and the application does not display `Running slower than interval.` or a replacement slow-simulation message.

#### UX-A3 — UX production-gate acceptance criteria

These criteria apply to Epic 3 Stories 3 and 5, Epic 4 Stories 2 and 4, Epic 5 Stories 1–6, and Epic 6 Stories 2–3:

- **Given** a Field with the zoom controls available, **when** the player presses `+` or `−`, **then** zoom changes by exactly one level in `50%`, `75%`, `100%`, `150%`, `200%`, `300%`, `400%`, remains clamped to that set, and a new session starts at `100%`.
- **Given** the current zoom, **when** a cell is at least 4 logical display pixels wide and tall, **then** the Field renders grid lines; below 4 logical pixels, grid lines are hidden while cell state remains visible.
- **Given** a session or figure name, **when** it is validated, **then** leading/trailing whitespace is removed, Unicode is normalized to NFC, the result contains 1–64 Unicode code points, internal whitespace and display case are preserved, and uniqueness comparison is case-insensitive.
- **Given** a player deletes a saved Bank figure, **when** Delete is activated, **then** a confirmation names the figure and no removal occurs until the player explicitly confirms.
- **Given** a session is left or the application closes with a session open, **when** the preview is generated, **then** the field-only saved camera view is encoded as a 256×256 PNG and committed transactionally with the session state.

#### UX-A4 — Presentation ownership and Start Screen acceptance criteria

These criteria apply to Epic 3 Story 1, Epic 4 Stories 2–4, Epic 5 Stories 1–5, and Epic 6 Stories 2–3:

- **Given** the application is at the Start Screen, **when** it renders, **then** the Start Screen owns the horizontally scrolling session browser, Create action, and Settings action; a session-card preview is an inert picture and does not own an interactive Field.
- **Given** the player opens Settings, **when** the panel renders, **then** it shows one global configuration in a two-column table with setting names on the left and editable values on the right for Field width (cells), Field height (cells), and generation interval.
- **Given** valid Settings width and height values, **when** the player creates a session through the Name dialog, **then** the new session uses those values, its dimensions remain fixed thereafter, and the interactive Field Screen opens.
- **Given** the interactive Field Screen is open, **when** the player uses the upper-right Toolbar, **then** field tools, pause/resume, zoom, Bank, and Exit are routed through that Toolbar; Bank opens the one application-wide Bank panel.
- **Given** a destructive session or Bank action, **when** Delete is activated, **then** the Confirmation dialog names the target and offers Confirm and Cancel; Cancel changes nothing and Confirm performs the deletion.
- **Given** a blocking persistence or load failure, **when** the application reports it, **then** an Error dialog presents a specific message and Acknowledge returns to the owning surface; reusable Text and Numeric fields retain invalid input with field-local validation, and Status messages provide non-modal feedback.

#### UX-A5 — Synchronous operation feedback and Bank pause acceptance criteria

These criteria apply to Epic 4 Stories 3–7 and Epic 5 Stories 2, 5–6:

- **Given** a session, Bank, or preview operation is requested, **when** its synchronous main-thread call runs, **then** the owning surface may stage a named busy status before the call, the whole window may block while it runs, and the UI makes no live-progress or partial-interactivity promise; after return, it shows the success or failure result.
- **Given** a Field is Running, **when** the player opens Bank from the Toolbar, **then** RunState becomes Paused, accumulated simulation time is cleared, no generation advances while Bank is open, and Bank controls remain interactive.
- **Given** Bank is open without a staged placement, **when** the player closes or cancels it, **then** no cells change, Live becomes selected, and simulation resumes with a fresh interval. A selected figure remains in paused Bank Preview until Resume or invalid-placement handling resolves it.
- **Given** a future implementation would keep the window interactive during a synchronous persistence or preview call, **when** that behavior is proposed, **then** it requires a separate asynchronous-execution architecture decision rather than a UX-only change.

#### UX-A6 — Persistence failure categories and recovery acceptance criteria

These criteria apply to Epic 5 Stories 1–6 and Epic 6 Stories 1, 3, and 5:

- **Given** the required SQLite database cannot open, complete migration, or establish its required schema, **when** startup handles the failure, **then** a specific startup Error dialog is shown, the Start Screen is not opened, no replacement database is created, and Acknowledge exits the application.
- **Given** the database opens but one session record fails validation, **when** the Start Screen loads, **then** the database is preserved, valid session cards remain usable, and the damaged session remains visible as a disabled card when its identity is available; selecting it shows a specific error and never opens Field.
- **Given** the database opens but one Bank figure record fails validation, **when** Bank loads, **then** valid figures and Bank operations remain usable, the damaged figure remains visible as a disabled row when its identity is available, and selecting it shows a specific error without staging it.
- **Given** any damaged persisted record is detected, **when** the application reports it, **then** it does not automatically repair, overwrite, delete, or silently replace the record with empty data.

#### UX-A7 — Global Settings acceptance criteria

These criteria apply to Epic 2 Story 5, Epic 3 Story 1, Epic 5 Stories 2 and 6, and Epic 6 Story 3:

- **Given** the player opens Settings, **when** the panel loads, **then** Field width, Field height, and generation interval are read from and written to one global Settings record; there is no per-session Settings record for these values.
- **Given** valid global Field width and height values are saved, **when** a new session is created, **then** those values supply its dimensions; changing them never resizes an existing session, whose dimensions remain fixed.
- **Given** a valid global generation interval is saved, **when** a session is created or opened afterward, **then** that session uses the saved interval and begins from a fresh simulation interval.
- **Given** Settings Save fails or contains invalid input, **when** the player remains in Settings, **then** the global stored values and every existing session remain unchanged, invalid fields retain their input, and no partial global configuration is applied.

#### UX-A8 — Platform scaling and input acceptance criteria

These criteria apply to Epic 2 Stories 1–6, Epic 3 Stories 2–5, Epic 4 Stories 2–7, Epic 5 Stories 1–7, and Epic 6 Stories 1–3:

- **Given** Life Game runs on macOS or Linux at any supported OS DPI scale, **when** the UI renders, **then** layout, text, pointer targets, and Field geometry use logical client pixels, OS scaling is applied exactly once, body text is 16 logical px, compact text is 14 logical px, and pointer targets are at least 32×32 logical px.
- **Given** the player clicks or drags on a high-DPI display, **when** input is routed, **then** platform/framebuffer coordinates are normalized to logical client coordinates before UI hit testing, camera conversion, or Field cell mapping, so the same visible point selects the same control or cell at every supported DPI scale.
- **Given** the application starts or a resizable window is used, **when** the client area is established, **then** the initial logical size is 1280×720 and the minimum supported logical viewport is 960×540; resizing adds space to the Field viewport while token-sized controls remain stable, and a smaller logical viewport is unsupported.
- **Given** the release candidate is checked for Cross-Platform Completion, **when** the platform gate runs, **then** high-DPI rendering, pointer mapping, viewport limits, and 4-logical-pixel grid-threshold behavior pass on both macOS and Linux.

#### UX-A9 — Camera boundary and out-of-field rendering acceptance criteria

These criteria apply to Epic 3 Stories 2–5, Epic 5 Stories 5–6, and Epic 6 Stories 1–2:

- **Given** the player moves or zooms the camera, **when** the viewport is updated, **then** its logical rectangle intersects at least one in-field cell; the finite Field can never disappear completely from view.
- **Given** the viewport extends beyond the finite Field, **when** the Field renders, **then** visible out-of-field area is gray and visually distinct from black dead cells and white live cells.
- **Given** the pointer is over gray out-of-field area, **when** the player uses Live, Die, Highlight, Move, or Bank placement, **then** no in-field cell is selected or changed by that out-of-field position.
- **Given** a session preview is rendered from a camera view that includes the boundary, **when** the 256×256 preview is encoded, **then** it preserves black dead cells, white live cells, and gray out-of-field area without persisting gray as field data.

## Epic 3 — Field Navigation and Setup

**Goal:** Support fields larger than the current view while keeping field geometry stable.

**High-level stories:**

1. As the player, I configure default field width and height in the Start Screen Settings table and create a session using those dimensions.
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
3. As the player, I open the application-wide Bank, pause the simulation, and browse figures by name.
4. As the player, I rename or delete a saved figure through dedicated controls.
5. As the player, I select a figure and move its translucent preview across the field.
6. As the player, I press Resume to replace every underlying live/dead cell in a valid target rectangle.
7. As the player, I receive no placement when any part of the staged rectangle is outside the field; Bank closes, Live returns, and simulation resumes.

**Completion outcome:** Figures are exact reusable rectangular bitmaps shared across the application.

## Epic 5 — Persistent Sessions

**Goal:** Preserve multiple named experimental workspaces and one shared Bank across application restarts.

**High-level stories:**

1. As the player, I browse the Start Screen's horizontal session cards by unique name and picture-only last-view preview.
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
3. As the player, I encounter clear failure behavior for duplicate names, invalid placement, fatal database startup failure, isolated damaged saved records, and unsupported actions.
4. As the developer, I can build with pinned raylib/raygui dependencies and the verified C++23 subset on both target toolchains.
5. As the designer, I can verify every product success metric without relying on narrative, audio, multiplayer, or performance-at-scale features.

**Completion outcome:** The complete local single-user product is ready for continued personal experimentation.

## Deferred Beyond These Epics

- Program/source replication triggered by `flex`.
- AI-assisted mathematical reasoning and evidence.
- A semantic/versioned construction workbench with provenance and replayable experiments.
- Any future feature currently listed as explicitly out of scope in the GDD.

---
title: "Life Game — Development Epics"
game_type: sandbox
secondary_descriptor: simulation
status: final
created: 2026-08-19
updated: 2026-08-25
---

# Life Game — Development Epics

## Implementation Enabler

The following enabler is delivery work required to make the first player-facing epic implementable. It is not a product epic and does not expand player scope.

### Story 1.0 — Project Foundation Enabler

As the developer, I can configure, build, and test the approved greenfield project baseline before implementing Field MVP.

**Acceptance criteria:**

- Given a clean checkout, when the `dev-debug` preset is configured, then CMake 3.28+, C++23, the approved target structure, pinned dependency hashes, and CTest are configured without system-package fallbacks.
- Given the configured project, when the Debug build and CTest run, then the foundation target and an initial deterministic test pass on the development platform.
- Given the CI presets, when macOS and Linux jobs run, then the explicit runners, warnings-as-errors policy, sanitizers, and Release compilation paths are defined and validate the baseline.

The full enabler context and gate evidence are in [`story-specs-life-game-2026-08-25.md`](../../story-specs-life-game-2026-08-25.md).

## Sequence and Traceability

| Epic | Pillars delivered | Depends on |
|---|---|---|
| 1. Field MVP | Direct Manipulation, Deterministic Evolution | Foundation enabler 1.0 |
| 2. Editing and Observation | Direct Manipulation, Deterministic Evolution | Epic 1 |
| 3. Field Navigation and Setup | Direct Manipulation | Epic 1 |
| 4. Figure Capture and Bank | Preservation, Composition | Epics 2–3 |
| 5. Persistent Sessions | Preservation, Composition | Epics 3–4 |
| 6. Cross-Platform Completion | All pillars | Epics 1–5 |

Story-level Given/When/Then acceptance criteria, source links, and the complete FR1–FR27/NFR1–NFR13 traceability matrix are maintained in [`story-specs-life-game-2026-08-25.md`](../../story-specs-life-game-2026-08-25.md). The headings below are the delivery story index; the companion is the canonical implementation handoff.

## Epic 1 — Field MVP

**Goal:** Deliver the smallest complete Life experiment: a visible finite field, direct Live input, and correct scheduled evolution.

**High-level stories:**

### Story 1.1 — View the Field MVP

As the player, I see a small lined **50×50** field and compact upper-right controls in the Field MVP development harness.

### Story 1.2 — Paint Live Cells

As the player, I click a visible cell in Live mode and that exact cell becomes alive.

### Story 1.3 — Advance Conway Generations

As the player, I watch the whole field advance synchronously under standard Conway rules every **0.25 seconds**.

### Story 1.4 — Respect Permanent-Dead Boundaries

As the player, I observe permanent-dead boundaries rather than wrapping or field expansion.

### Story 1.5 — Verify Canonical Field Behavior

As the developer, I can demonstrate lone-cell death, stable block behavior, blinker period two, and correct edge behavior.

**Completion outcome:** In the Field MVP development harness, Field runs on macOS and Linux development environments without Bank, sessions, Settings, camera navigation, or persistence. Epic 3 supplies the product Start Screen/session shell around this field foundation.

## Epic 2 — Editing and Observation

**Goal:** Let the player deliberately change and inspect a running experiment.

**High-level stories:**

### Story 2.1 — Paint Live by Drag

As the player, I draw across cells with Live to make them alive.

### Story 2.2 — Paint Dead by Drag

As the player, I draw across cells with Die to make them dead.

### Story 2.3 — Pause Without Losing State

As the player, I pause generation updates without losing field state.

### Story 2.4 — Resume and Return to Live

As the player, I resume updates and return automatically to Live.

### Story 2.5 — Show the Active Tool

As the player, I can always see which persistent tool is selected.

**Completion outcome:** Direct editing, observation, pause/resume, and selected-tool feedback work at the default **0.25-second** interval without introducing goals, scores, or progression. Global Settings are introduced in Epic 3 with the Start Screen/session shell.

#### UX-A1 — Same-frame input versus scheduled generation acceptance criteria

These criteria apply to Epic 1 Story 3, Epic 2 Stories 1–2, and Epic 6 Story 1:

- **Given** a Running field in an iteration where the scheduler executes a generation, **when** the iteration is processed, **then** the clock snapshot and scheduled simulation batch complete before raylib-exposed input is sampled and translated, accepted Live/Die commands execute afterward, and rendering occurs last.
- **Given** a Running field and an accepted Live or Die edit, **when** the input command is processed, **then** the current field changes immediately and the edit is visible in that frame; the player is not required to pause and the edit is not placed in a special buffer.
- **Given** a scheduled generation and an accepted edit in the same rendered frame, **when** the ordered phases complete, **then** the generation uses the field state that existed before the input phase, the edit is applied to the current field afterward, and no user-facing behavior promises that the edit affected the generation already processed.
- **Given** the same initial field, controlled clock trace, and input trace, **when** the run is repeated, **then** the generation sequence, post-input field state, and rendered result are identical. The test does not require a particular wall-clock assignment at an exact timer boundary.
- **Given** equivalent controlled traces on macOS and Linux, **when** the frame loop is exercised, **then** both platforms produce the same simulation → input → render ordering and visible result.

#### UX-A2 — Delayed-frame catch-up acceptance criteria

These criteria apply to Epic 1 Story 3, Epic 2 Stories 1–2, Epic 3 Story 1, and Epic 6 Story 1:

- **Given** a Running field at the start of a main-loop iteration, **when** elapsed time is sampled, **then** the scheduler snapshots `steps = min(floor(accumulator / interval), 4)` and does not enlarge that batch using time elapsed later in the same iteration.
- **Given** a snapshot containing one to four complete generation intervals, **when** the iteration runs, **then** exactly that many due generations execute sequentially before input and rendering, and only the completed field after the batch is rendered.
- **Given** a snapshot containing more than four complete generation intervals, **when** the iteration runs, **then** exactly four due generations execute, additional whole-generation timing debt is discarded with `accumulator %= interval`, and the sub-interval remainder is retained.
- **Given** a catch-up batch, **when** each generation executes, **then** it consumes the preceding completed field state and publishes a complete next state; no executed mathematical state is merged, partially applied, or skipped.
- **Given** a catch-up batch, **when** the batch completes, **then** the application samples raylib-exposed input, translates it into commands, executes accepted commands, and renders once. An accepted Live or Die command mutates the resulting current field and affects only future scheduled generations; input not exposed at that sampling phase receives no project-owned retention or replay guarantee.
- **Given** discarded timing debt, **when** subsequent iterations run normally, **then** the scheduler does not repay the discarded intervals and the application does not display `Running slower than interval.` or a replacement slow-simulation message.

#### UX-A3 — UX production-gate acceptance criteria

These criteria apply to Epic 3 Stories 3 and 5, Epic 4 Stories 2 and 4, Epic 5 Stories 1–7, and Epic 6 Stories 2–3:

- **Given** a Field with the zoom controls available, **when** the player presses `+` or `−`, **then** zoom changes by exactly one level in `50%`, `75%`, `100%`, `150%`, `200%`, `300%`, `400%`, remains clamped to that set, and a new session starts at `100%`.
- **Given** the current zoom, **when** a cell is at least 4 logical display pixels wide and tall, **then** the Field renders grid lines; below 4 logical pixels, grid lines are hidden while cell state remains visible.
- **Given** a session or figure name, **when** it is validated, **then** leading/trailing whitespace is removed, Unicode is normalized to NFC, the result contains 1–64 Unicode code points, internal whitespace and display case are preserved, and uniqueness comparison is case-insensitive.
- **Given** a player deletes a saved Bank figure, **when** Delete is activated, **then** a confirmation names the figure and no removal occurs until the player explicitly confirms.
- **Given** a session is left or the application closes with a session open, **when** the preview is generated, **then** the field-only saved camera view is encoded as a 256×256 PNG and committed transactionally with the session state.

#### UX-A4 — Presentation ownership and Start Screen acceptance criteria

These criteria apply to Epic 3 Story 1, Epic 4 Stories 2–4, Epic 5 Stories 1–7, and Epic 6 Stories 2–3:

- **Given** the application is at the Start Screen, **when** it renders, **then** the Start Screen owns the horizontally scrolling session browser, Create action, and Settings action; a session-card preview is an inert picture and does not own an interactive Field.
- **Given** the player opens Settings, **when** the panel renders, **then** it shows one global configuration in a two-column table with setting names on the left and editable values on the right for Field width (cells), Field height (cells), and generation interval.
- **Given** valid Settings width and height values, **when** the player creates a session through the Name dialog, **then** the new session uses those values, its dimensions remain fixed thereafter, and the interactive Field Screen opens.
- **Given** the interactive Field Screen is open, **when** the player uses the upper-right Toolbar, **then** field tools, pause/resume, zoom, Bank, and Exit are routed through that Toolbar; Bank opens the one application-wide Bank panel.
- **Given** a destructive session or Bank action, **when** Delete is activated, **then** the Confirmation dialog names the target and offers Confirm and Cancel; Cancel changes nothing and Confirm performs the deletion.
- **Given** a recoverable persistence or load operation failure, **when** the application reports it, **then** an Error dialog presents specific copy with Retry and Cancel; Cancel preserves the current valid state and returns to the owning surface, while Retry repeats the originating operation. **Given** a fatal database-open or migration failure, **when** it is reported, **then** the startup Error dialog offers Acknowledge and exits after acknowledgment. Isolated damaged-record selection uses the Acknowledge path defined in UX-A6. Reusable Text and Numeric fields retain invalid input with field-local validation, and Status messages provide non-modal feedback.

#### UX-A5 — Synchronous operation feedback and Bank pause acceptance criteria

These criteria apply to Epic 4 Stories 3–7, Epic 5 Stories 1–7, and Epic 6 Story 3:

- **Given** a session, Bank, or preview operation is requested, **when** its synchronous main-thread call runs, **then** the owning surface may stage a named busy status before the call, the whole window may block while it runs, and the UI makes no live-progress or partial-interactivity promise; after return, it shows the success or failure result.
- **Given** a Field is Running, **when** the player opens Bank from the Toolbar, **then** RunState becomes Paused, accumulated simulation time is cleared, no generation advances while Bank is open, and Bank controls remain interactive.
- **Given** Bank is open without a staged placement, **when** the player closes or cancels it, **then** no cells change, Live becomes selected, and simulation resumes with a fresh interval. A selected figure remains in paused Bank Preview until Resume or invalid-placement handling resolves it.
- **Given** a future implementation would keep the window interactive during a synchronous persistence or preview call, **when** that behavior is proposed, **then** it requires a separate asynchronous-execution architecture decision rather than a UX-only change.

#### UX-A6 — Persistence failure categories and recovery acceptance criteria

These criteria apply to Epic 5 Stories 1–7 and Epic 6 Stories 1 and 3:

- **Given** the required SQLite database cannot open, complete migration, or establish its required schema, **when** startup handles the failure, **then** a specific startup Error dialog is shown, the Start Screen is not opened, no replacement database is created, and Acknowledge exits the application.
- **Given** the database opens but one session record fails validation, **when** the Start Screen loads, **then** the database is preserved, valid session cards remain usable, and the damaged session remains visible as a disabled card when its identity is available; selecting it shows a specific error and never opens Field.
- **Given** the database opens but one Bank figure record fails validation, **when** Bank loads, **then** valid figures and Bank operations remain usable, the damaged figure remains visible as a disabled row when its identity is available, and selecting it shows a specific error without staging it.
- **Given** any damaged persisted record is detected, **when** the application reports it, **then** it does not automatically repair, overwrite, delete, or silently replace the record with empty data.
- **Given** a recoverable save or load operation failure, **when** the Error dialog is shown, **then** Retry repeats only the originating operation and Cancel preserves the current valid state; an open session remains paused. A damaged-record selection is acknowledged without mutation rather than retried.

#### UX-A7 — Global Settings acceptance criteria

These criteria apply to Epic 3 Story 1, Epic 5 Stories 1–2 and 6, and Epic 6 Story 3:

- **Given** the player opens Settings, **when** the panel loads, **then** Field width, Field height, and generation interval are read from and written to one global Settings record; there is no per-session Settings record for these values.
- **Given** valid global Field width and height values are saved, **when** a new session is created, **then** those values supply its dimensions; changing them never resizes an existing session, whose dimensions remain fixed.
- **Given** a valid global generation interval is saved, **when** a session is created or opened afterward, **then** that session uses the saved interval and begins from a fresh simulation interval.
- **Given** Settings Save fails or contains invalid input, **when** the player remains in Settings, **then** the global stored values and every existing session remain unchanged, invalid fields retain their input, and no partial global configuration is applied.
- **Given** a stored Settings record is invalid, **when** the application loads it, **then** the record remains preserved and unmodified, safe in-memory defaults of 50×50 and 0.25 seconds are used, a warning is shown, and only an explicit successful Save replaces the invalid record.

#### UX-A8 — Platform scaling and input acceptance criteria

These criteria apply to Epic 2 Stories 1–5, Epic 3 Stories 1–5, Epic 4 Stories 2–7, Epic 5 Stories 1–7, and Epic 6 Stories 1–3:

- **Given** Life Game runs on macOS or Linux at any supported OS DPI scale, **when** the UI renders, **then** layout, text, pointer targets, and Field geometry use logical client pixels, OS scaling is applied exactly once, body text is 16 logical px, compact text is 14 logical px, and pointer targets are at least 32×32 logical px.
- **Given** the player clicks or drags on a high-DPI display, **when** input is routed, **then** platform/framebuffer coordinates are normalized to logical client coordinates before UI hit testing, camera conversion, or Field cell mapping, so the same visible point selects the same control or cell at every supported DPI scale.
- **Given** the application starts or a resizable window is used, **when** the client area is established, **then** the initial logical size is 1280×720 and the minimum supported logical viewport is 960×540; resizing adds space to the Field viewport while token-sized controls remain stable, and a smaller logical viewport is unsupported.
- **Given** the release candidate is checked for Cross-Platform Completion, **when** the platform gate runs, **then** high-DPI rendering, pointer mapping, viewport limits, and 4-logical-pixel grid-threshold behavior pass on both macOS and Linux.

#### UX-A9 — Camera boundary and out-of-field rendering acceptance criteria

These criteria apply to Epic 3 Stories 2–5, Epic 5 Stories 1 and 5–6, and Epic 6 Stories 1–2:

- **Given** the player moves or zooms the camera, **when** the viewport is updated, **then** its logical rectangle intersects at least one in-field cell; the finite Field can never disappear completely from view.
- **Given** the viewport extends beyond the finite Field, **when** the Field renders, **then** visible out-of-field area is gray and visually distinct from black dead cells and white live cells.
- **Given** the pointer is over gray out-of-field area, **when** the player uses Live, Die, Highlight, Move, or Bank placement, **then** no in-field cell is selected or changed by that out-of-field position.
- **Given** a session preview is rendered from a camera view that includes the boundary, **when** the 256×256 preview is encoded, **then** it preserves black dead cells, white live cells, and gray out-of-field area without persisting gray as field data.

## Epic 3 — Field Navigation and Setup

**Goal:** Introduce the Start Screen/session shell and global Settings in memory, then support fields larger than the current view while keeping field geometry stable. Persistence is deliberately deferred to Epic 5.

**High-level stories:**

### Story 3.1 — Configure Settings and Create an In-Memory Session

As the player, I use the Start Screen Settings table to configure global Field width, Field height, and generation interval, then create an in-memory named session using those defaults.

### Story 3.2 — Move the Camera

As the player, I press Move and drag the camera without editing cell state.

### Story 3.3 — Change Discrete Zoom

As the player, I press `+` or `−` to change zoom by one level.

### Story 3.4 — Preserve Fixed Field Geometry While Navigating

As the player, I navigate without changing field dimensions or cell coordinates.

### Story 3.5 — Apply the Grid Visibility Threshold

As the player, I see useful grid lines at editing zoom levels without excessive visual noise when zoomed out.

**Completion outcome:** The Start Screen, global Settings, in-memory session shell, camera movement, and zoom work on fixed square or rectangular fields without SQLite persistence, editing cells accidentally, or changing field geometry. Epic 5 upgrades this shell to durable storage.

## Epic 4 — Figure Capture and Bank

**Goal:** Preserve exact rectangular constructions and compose them onto any session field.

**High-level stories:**

### Story 4.1 — Capture an Inclusive Rectangle

As the player, I drag Highlight between two cells to select the inclusive rectangle and pause simulation.

### Story 4.2 — Save or Cancel a Figure

As the player, I save the rectangle under a unique name or cancel without changing Bank.

### Story 4.3 — Open and Browse the Shared Bank

As the player, I open the application-wide Bank, pause the simulation, and browse figures by name.

### Story 4.4 — Rename or Delete a Figure

As the player, I rename or delete a saved figure through dedicated controls.

### Story 4.5 — Stage a Figure Preview

As the player, I select a figure and move its translucent preview across the field.

### Story 4.6 — Commit a Valid Figure Placement

As the player, I press Resume to replace every underlying live/dead cell in a valid target rectangle.

### Story 4.7 — Reject an Invalid Figure Placement

As the player, I receive no placement when any part of the staged rectangle is outside the field; Bank closes, Live returns, and simulation resumes.

**Completion outcome:** Figures are exact reusable rectangular bitmaps shared across the application.

## Epic 5 — Persistent Sessions

**Goal:** Preserve multiple named experimental workspaces and one shared Bank across application restarts.

**High-level stories:**

### Story 5.1 — Browse Persisted Session Cards

As the player, I browse the Epic 3 session shell after it is upgraded to SQLite-backed horizontal session cards by unique name and picture-only last-view preview, with the global Settings record loaded alongside it.

### Story 5.2 — Create a Persisted Session

As the player, I create a session through a naming dialog that rejects duplicate names.

### Story 5.3 — Rename a Persisted Session

As the player, I rename a session through its dedicated control without creating a duplicate.

### Story 5.4 — Delete a Persisted Session with Confirmation

As the player, I delete a session only after confirmation.

### Story 5.5 — Save Session State and Preview

As the player, I leave a session and automatically save cells, dimensions, camera position, zoom, and preview.

### Story 5.6 — Restore a Persisted Session

As the player, I reopen a session at its saved field and view while generation updates begin immediately.

### Story 5.7 — Share Bank Across Sessions

As the player, I use one persistent Bank across all sessions, and deleting a session never deletes its figures.

**Completion outcome:** Session, global Settings, and Bank state survive application restart without cross-session ownership errors; invalid Settings remain preserved while safe defaults are used in memory until explicit Save.

## Epic 6 — Cross-Platform Completion

**Goal:** Make the confirmed product scope dependable and visually coherent on macOS and Linux.

**High-level stories:**

### Story 6.1 — Match Life Rules and Timing Across Platforms

As the player, I get equivalent Life rules and timing semantics on macOS and Linux.

### Story 6.2 — Match Input, Camera, Viewport, and Visual States Across Platforms

As the player, I get equivalent input, camera, viewport, and visual-state behavior on macOS and Linux.

### Story 6.3 — Match Validation and Persistence Recovery Across Platforms

As the player, I receive the same validation and persistence-failure outcomes on both platforms, including fatal startup failure and isolated damaged records.

**Completion outcome:** The complete local single-user product is ready for continued personal experimentation.

## Release and Quality Gates

These are release gates and implementation enablers, not player-facing stories:

- **RG1 — Build and CI gate:** pinned raylib/raygui/SQLite/Catch2 dependencies, verified C++23 subset, explicit macOS/Linux runners, warning policy, sanitizers, Debug tests, and Release builds pass.
- **RG2 — Visual and accessibility gate:** black/white/gray cell semantics, contrast, focus rings, solid/dashed placement cues, non-color-only state/error cues, pointer targets, modal click-through prevention, and logical-pixel typography pass on both platforms.
- **RG3 — Determinism and performance gate:** canonical Life behaviors, fixed-step/catch-up behavior, input ordering, default 50×50 internal budgets, maximum-field generation budget, and default session save/preview budget pass without changing the no-user-facing-FPS-promise contract.
- **RG4 — Traceability gate:** every FR/NFR maps to at least one story acceptance criterion, every story has an implementation path, and every open decision is either resolved in the source artifacts or explicitly recorded as out of scope.

## Deferred Beyond These Epics

- Program/source replication triggered by `flex`.
- AI-assisted mathematical reasoning and evidence.
- A semantic/versioned construction workbench with provenance and replayable experiments.
- Any future feature currently listed as explicitly out of scope in the GDD.

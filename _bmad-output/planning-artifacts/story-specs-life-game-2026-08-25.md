---
title: "Life Game — Story Specifications and Traceability"
project: life-game
status: ready-for-dev
date: 2026-08-25
source_epics: gdds/gdd-life-game-2026-08-19/epics.md
---

# Life Game — Story Specifications and Traceability

This document is the canonical story-level companion to `epics.md`. Every delivery story has a bounded outcome, explicit acceptance criteria, and traceability to the functional (`FR`) and non-functional (`NFR`) requirement identifiers defined in the implementation-readiness assessment.

## Conventions

- `E1.0` is an implementation enabler inside Field MVP; it is required for development but is not product scope.
- `RG1`–`RG4` are release/quality gates, not user stories.
- Acceptance criteria use Given/When/Then semantics. A story is ready for development only when its criteria and linked source contracts are available.
- The product remains a private, single-user, local sandbox; these stories do not add goals, progression, networking, audio, or alternate Life rules.

## Epic 1 — Field MVP

### E1.0 — Project Foundation Enabler

**Outcome:** A clean checkout can configure, build, test, and run the approved greenfield baseline.

**Trace:** NFR1, NFR5.

**Acceptance criteria:**

- Given a clean checkout, when `dev-debug` is configured, then CMake 3.28+, C++23, pinned dependency archives and hashes, the approved layer targets, and CTest are available without system-package fallbacks.
- Given the Debug configuration, when the foundation target and initial tests are built, then compilation uses the approved warning policy and tests pass without opening a window.
- Given the CI presets, when the macOS and Linux jobs are configured, then the explicit runners, Linux sanitizers, Debug tests, and Release compilation paths are defined.

### E1.1 — View the Field MVP

**Outcome:** The Field MVP development harness shows a small lined 50×50 finite field and compact controls; the final Start Screen/session shell is introduced by E3.

**Trace:** FR5, NFR1, NFR7.

**Acceptance criteria:**

- Given the Field MVP is launched, when the Field surface renders, then a 50×50 field and upper-right controls are visible.
- Given a cell is in the viewport, when it is rendered, then its grid alignment is exact and the in-field dead/live palette remains distinct from gray out-of-field space.
- Given the MVP boundary, when the Field runs, then Bank, persistence, Settings, and camera navigation are not required to demonstrate this story.

### E1.2 — Paint Live Cells

**Outcome:** A Live click or drag changes every touched in-bounds cell to live.

**Trace:** FR9, NFR2, NFR11.

**Acceptance criteria:**

- Given Live is selected, when the player presses an in-bounds cell, then exactly that logical cell becomes live in the current field.
- Given Live is selected, when the player drags across cells, then rasterized touched in-bounds cells become live without gaps caused by pointer-sample spacing.
- Given the pointer is outside the field or on a toolbar/modal owner, when input is received, then no hidden field cell changes.

### E1.3 — Advance Conway Generations

**Outcome:** The field advances synchronously under standard Conway rules at the fixed MVP interval.

**Trace:** FR6, FR8, FR27, NFR3, NFR4.

**Acceptance criteria:**

- Given a field and a scheduled generation, when the generation runs, then every cell reads the prior complete buffer and publishes a complete next buffer before rendering.
- Given the default interval, when 0.25 seconds elapse, then one due generation is eligible independently of rendering cadence.
- Given a lone cell, stable block, or blinker fixture, when the required generations run, then the canonical expected result is produced.

### E1.4 — Respect Permanent-Dead Boundaries

**Outcome:** The finite field does not wrap or expand and out-of-bounds coordinates remain non-live.

**Trace:** FR7, FR14, NFR3, NFR7.

**Acceptance criteria:**

- Given a neighbor query at an out-of-bounds coordinate, when Conway rules are evaluated, then the coordinate contributes no live neighbor and cannot become live.
- Given the camera shows beyond the finite field, when the renderer draws, then outside space is gray and is not an editable cell state.
- Given input lands in gray outside space, when Live, Die, or selection input is processed, then field bytes remain unchanged.

### E1.5 — Verify Canonical Field Behavior

**Outcome:** The Field MVP has deterministic evidence for canonical Life and edge behavior.

**Trace:** FR27, NFR4.

**Acceptance criteria:**

- Given lone-cell, block, blinker, and edge fixtures, when the deterministic tests run, then all expected outcomes pass.
- Given the same initial state and controlled clock/input trace, when the run is repeated, then the generation and post-input states match byte-for-byte.

## Epic 2 — Editing and Observation

### E2.1 — Paint Live by Drag

**Outcome:** Live remains useful during deliberate running-field editing.

**Trace:** FR9, NFR2, NFR11.

**Acceptance criteria:**

- Given a Running or Paused field with Live selected, when the player drags, then every touched in-bounds cell becomes live in the current field.
- Given a generation is scheduled in the same iteration, when input is processed, then the edit occurs after the scheduled batch and is visible in the resulting render.

### E2.2 — Paint Dead by Drag

**Outcome:** Die sets touched cells dead, including cells that were live.

**Trace:** FR10, NFR2, NFR11.

**Acceptance criteria:**

- Given Die is selected, when the player presses or drags across in-bounds cells, then every touched cell becomes dead.
- Given the pointer is outside the field, when Die input is processed, then no field byte changes.

### E2.3 — Pause Without Losing State

**Outcome:** The player can stop evolution while preserving the current field.

**Trace:** FR11, NFR3.

**Acceptance criteria:**

- Given a Running field, when Pause is activated, then RunState becomes Paused and no generation advances while paused.
- Given a paused field, when time elapses, then the field bytes remain unchanged and the accumulated simulation time is cleared.

### E2.4 — Resume and Return to Live

**Outcome:** Resume restarts evolution with a fresh interval and selects Live.

**Trace:** FR11, NFR3.

**Acceptance criteria:**

- Given a paused field, when Resume is activated, then RunState becomes Running, Live is selected, and a fresh full interval begins.
- Given Resume has returned control to the Field, when the player edits, then Live semantics apply without requiring another tool selection.

### E2.5 — Show the Active Tool

**Outcome:** The selected persistent mode is always visible and unambiguous.

**Trace:** FR11, NFR7, NFR11.

**Acceptance criteria:**

- Given Live, Die, or Move is selected, when the Toolbar renders, then the active mode has a persistent visual outline and a textual cue.
- Given selection, placement validity, or error state changes, when the state is shown, then meaning does not depend on gray shade alone.

## Epic 3 — Field Navigation and Setup

### E3.1 — Configure Settings and Create an In-Memory Session

**Outcome:** The Start Screen owns a global Settings panel and creates a named in-memory session without persistence.

**Trace:** FR2, FR3, FR4, FR5, FR8, NFR1, NFR2, NFR9.

**Acceptance criteria:**

- Given the Start Screen is open, when Settings is opened, then a two-column table exposes global Field width, Field height, and generation interval.
- Given invalid numeric input, when the player edits Settings, then the input remains visible, field-local validation is shown, and Save is disabled.
- Given valid Settings values, when Save is activated, then width, height, and interval are validated and committed atomically to the in-memory global configuration.
- Given a Settings Save failure, when the Error dialog is shown, then the previous valid configuration remains active, edits remain visible, and Retry/Cancel are available.
- Given valid Settings and a unique name, when Create is committed, then a fixed-dimension in-memory session opens in Field with Live selected and the saved interval.
- Given Cancel is activated, when Settings or Create is open, then no global value or session is changed.
- Given global width/height are changed after a session exists, when the current session is viewed, then its dimensions remain fixed.

### E3.2 — Move the Camera

**Outcome:** Move mode changes camera position without editing cells.

**Trace:** FR12, FR14, NFR2.

**Acceptance criteria:**

- Given Move is selected, when the player drags the Field, then camera position changes and field bytes remain unchanged.
- Given the pointer is over a toolbar, modal, or outside-field area, when it is dragged, then ownership prevents accidental cell edits.

### E3.3 — Change Discrete Zoom

**Outcome:** `+` and `−` move through the confirmed zoom levels.

**Trace:** FR13, NFR8.

**Acceptance criteria:**

- Given any zoom level, when `+` or `−` is pressed, then zoom changes by exactly one level in `50%`, `75%`, `100%`, `150%`, `200%`, `300%`, `400%` and clamps at the ends.
- Given a new session, when Field opens, then zoom starts at 100%.
- Given the pointer is over an in-field cell, when zoom changes, then that cell remains under the pointer; outside-field zoom uses viewport center.

### E3.4 — Preserve Fixed Field Geometry While Navigating

**Outcome:** Camera movement and zoom never change dimensions or logical cell coordinates.

**Trace:** FR5, FR12, FR14, NFR2.

**Acceptance criteria:**

- Given a rectangular field, when Move or zoom is used, then width, height, and cell coordinates remain unchanged.
- Given a viewport smaller or larger than the field, when the camera is clamped, then at least one in-field cell remains visible and remaining space is gray.

### E3.5 — Apply the Grid Visibility Threshold

**Outcome:** Grid lines aid editing without obscuring zoomed-out cell state.

**Trace:** NFR7, NFR8.

**Acceptance criteria:**

- Given a cell is at least 4 logical display pixels wide and tall, when Field renders, then a 1px grid line is visible.
- Given a cell is below 4 logical display pixels, when Field renders, then grid lines are hidden while live/dead state remains visible.

## Epic 4 — Figure Capture and Bank

### E4.1 — Capture an Inclusive Rectangle

**Outcome:** Highlight captures an exact rectangular live/dead region and pauses simulation.

**Trace:** FR15, NFR2.

**Acceptance criteria:**

- Given Highlight is selected, when the player presses, drags, and releases on two in-bounds cells, then the inclusive rectangle is shown and simulation pauses.
- Given the gesture does not end on a valid second cell, when it releases, then capture does not open and field bytes remain unchanged.

### E4.2 — Save or Cancel a Figure

**Outcome:** The player can name an exact rectangle or cancel without Bank mutation.

**Trace:** FR16, NFR9.

**Acceptance criteria:**

- Given a valid unique name, when Save is committed, then the full rectangle including dead cells is stored in Bank and the dialog closes to Running Live.
- Given a duplicate or invalid name, when Save is attempted, then the selection and typed input remain, Save is blocked, and Bank is unchanged.
- Given Cancel or outside click on the capture dialog, when the action completes, then Bank is unchanged and the Field resumes in Live.

### E4.3 — Open and Browse the Shared Bank

**Outcome:** Bank pauses the Field and exposes all valid saved figures.

**Trace:** FR17, NFR6.

**Acceptance criteria:**

- Given a Running Field, when Bank opens, then RunState becomes Paused, accumulated time is cleared, and Bank controls remain interactive.
- Given valid figures exist, when Bank renders, then rows appear in stable case-insensitive ascending order by display name.
- Given no figures exist, when Bank renders, then an empty-state explanation and Close action remain available.

### E4.4 — Rename or Delete a Figure

**Outcome:** Bank management preserves uniqueness and requires explicit destructive confirmation.

**Trace:** FR21, NFR9.

**Acceptance criteria:**

- Given a valid new name, when Rename is committed, then the display name changes without changing the bitmap.
- Given a duplicate/invalid name, when Rename is attempted, then the edit remains visible, the action is blocked, and the figure is unchanged.
- Given Delete is activated, when the Confirmation dialog is shown, then Cancel preserves the figure and explicit Confirm removes it.

### E4.5 — Stage a Figure Preview

**Outcome:** A selected figure becomes a translucent cell-snapped preview while paused.

**Trace:** FR18, NFR7.

**Acceptance criteria:**

- Given a valid Bank row, when it is selected, then Bank closes and the full live/dead rectangle follows the pointer at whole-cell positions.
- Given the pointer leaves the window, when the preview is rendered, then the last in-window position is retained.
- Given the preview fits, when it renders, then its outline is solid; if it does not fit, the outline is dashed and `Outside field` is shown.

### E4.6 — Commit a Valid Figure Placement

**Outcome:** Resume atomically replaces a valid rectangle and leaves outside cells untouched.

**Trace:** FR19, NFR6.

**Acceptance criteria:**

- Given every staged cell is in bounds, when Resume is pressed, then every stored live and dead byte replaces the target rectangle in one commit.
- Given a valid commit, when the transition completes, then Bank exits, Live is selected, the generation number is unchanged, and a fresh interval begins.

### E4.7 — Reject an Invalid Figure Placement

**Outcome:** An out-of-bounds staged figure changes no field byte and returns safely to Running Live.

**Trace:** FR20, NFR6.

**Acceptance criteria:**

- Given any staged cell is outside the field, when Resume is pressed, then the field is byte-for-byte unchanged.
- Given the invalid placement resolves, when the UI returns to Field, then Bank exits, Live is selected, a fresh interval begins, and `Outside field. Nothing placed.` is visible.

## Epic 5 — Persistent Sessions

### E5.1 — Browse Persisted Session Cards

**Outcome:** The Epic 3 session shell loads persisted sessions and field-only previews.

**Trace:** FR2, FR23, FR24, NFR6, NFR10.

**Acceptance criteria:**

- Given the SQLite database opens, when Start Screen loads, then valid sessions appear as stable case-insensitive ascending cards with names and lazy 256×256 field-only previews.
- Given the SQLite database opens, when Start Screen loads, then the global Settings record is loaded with the session browser; valid Settings populate the Settings panel and an invalid Settings record is preserved while safe 50×50/0.25-second defaults are used in memory with a warning.
- Given there are no sessions, when Start Screen renders, then the empty state keeps Create available.
- Given a damaged session identity is available, when the browser loads, then the card is visible but disabled and selecting it shows `Session data could not be read.` without opening Field.

### E5.2 — Create a Persisted Session

**Outcome:** A unique validated name creates one persisted fixed-dimension session using global defaults.

**Trace:** FR3, FR4, FR5, NFR6, NFR9.

**Acceptance criteria:**

- Given a valid unique name and global Settings, when Create is committed, then the session is saved transactionally and Field opens with fixed dimensions and the current interval.
- Given valid global Settings, when the Settings panel is saved, then width, height, and interval replace the persisted Settings record atomically; existing session dimensions remain unchanged and later create/open operations use the saved interval.
- Given a duplicate or invalid name, when Create is attempted, then no card or database record is created and the entered value remains visible.
- Given persistence fails, when Create returns, then no partial session exists and the error offers Retry or Cancel.

### E5.3 — Rename a Persisted Session

**Outcome:** Session Rename updates only the validated display name and uniqueness key.

**Trace:** FR2, FR3, NFR6, NFR9.

**Acceptance criteria:**

- Given a valid unique replacement name, when Rename is committed, then the session card changes transactionally.
- Given a duplicate/invalid name or persistence failure, when Rename is attempted, then the session remains unchanged and Retry/Cancel behavior is explicit.

### E5.4 — Delete a Persisted Session with Confirmation

**Outcome:** A session is removed only after explicit confirmation and its Bank figures remain.

**Trace:** FR2, FR24, NFR6.

**Acceptance criteria:**

- Given Delete is activated, when Confirmation names the session, then Cancel makes no change and Confirm deletes only that session transactionally.
- Given a delete persistence failure, when the operation returns, then the session remains and Retry/Cancel is available.
- Given figures exist in the shared Bank, when a session is deleted, then no figure is removed.

### E5.5 — Save Session State and Preview

**Outcome:** Leaving or closing a session commits exact state and field-only preview in one transaction.

**Trace:** FR22, FR25, NFR6, NFR10.

**Acceptance criteria:**

- Given a session is left or the window closes, when save runs, then cells, dimensions, camera, zoom, and 256×256 field-only preview commit atomically; paused/running state is not persisted.
- Given save or preview generation fails, when the error returns, then the session stays open and paused, no partial write is accepted, and Retry/Cancel is offered.
- Given the camera includes the finite boundary, when preview is encoded, then gray outside space is preserved without controls, dialogs, selection, staged figure, or pointer state.

### E5.6 — Restore a Persisted Session

**Outcome:** Opening a valid session restores exact state and begins evolution immediately.

**Trace:** FR23, FR25, FR26, NFR6.

**Acceptance criteria:**

- Given a valid persisted record, when it opens, then cells, dimensions, camera, and zoom restore exactly and generation starts immediately using the current global interval.
- Given a damaged record, when it is selected, then Field does not open and the record remains preserved and disabled.
- Given a recoverable load operation failure, when ErrorDialog appears, then Retry repeats only the originating load and Cancel returns to Start Screen with valid state preserved. Given a damaged record is selected, then Acknowledge returns to Start Screen without mutation.

### E5.7 — Share Bank Across Sessions

**Outcome:** One persistent Bank remains available regardless of the active session.

**Trace:** FR17, FR24, NFR6.

**Acceptance criteria:**

- Given a figure saved in one session, when another session opens Bank, then the same valid figure is available.
- Given a session is deleted, when Bank is opened afterward, then its figures remain available.

## Epic 6 — Cross-Platform Completion

### E6.1 — Match Life Rules and Timing Across Platforms

**Outcome:** macOS and Linux produce the same deterministic Life and scheduler results.

**Trace:** FR6, FR8, FR27, NFR1, NFR3, NFR4.

**Acceptance criteria:**

- Given equivalent field, clock, and input traces, when Debug tests run on macOS and Linux, then generation, catch-up, pause, and resume results match.
- Given the canonical fixtures, when both platform builds run, then all four correctness behaviors pass.

### E6.2 — Match Input, Camera, Viewport, and Visual States Across Platforms

**Outcome:** The same visible point, mode, camera boundary, and state cues work on both platforms.

**Trace:** FR12, FR13, FR14, NFR2, NFR7, NFR8, NFR11.

**Acceptance criteria:**

- Given equivalent DPI scales and pointer traces, when input is normalized, then the same control or cell is selected on both platforms.
- Given the same zoom/camera state, when Field renders, then black dead, white live, gray outside, grid threshold, selection, staging, focus, and error cues match the UX contract.
- Given a modal or toolbar owns the pointer, when a click occurs, then no lower Field owner receives it on either platform.

### E6.3 — Match Validation and Persistence Recovery Across Platforms

**Outcome:** Duplicate validation, atomic placement, save/load, fatal database failure, damaged records, and Settings fallback have the same outcomes on both platforms.

**Trace:** FR16, FR20, FR25, FR26, NFR6, NFR9.

**Acceptance criteria:**

- Given equivalent valid and invalid names/placements, when actions run, then both platforms produce the same no-op or commit result and the same explicit feedback.
- Given fatal database setup failure, when startup handles it, then both platforms show the startup error, do not open Start Screen, and exit after Acknowledge.
- Given one damaged session/figure or invalid Settings record, when the database loads, then valid records remain usable, damaged records are preserved, Settings uses safe defaults in memory, and no record is silently overwritten.
- Given a recoverable save or load operation failure, when Retry or Cancel is selected, then Retry repeats only the originating operation and Cancel preserves the current valid state; an open session remains paused. Given an isolated damaged record is selected, when Acknowledge is selected, then the record and current state remain unchanged.

## Release and Quality Gates

### RG1 — Build and CI Gate

**Trace:** NFR1, NFR5.

**Acceptance criteria:**

- Both explicit CI platforms configure and build the approved targets.
- Debug tests, Linux ASan/UBSan, and both Release builds pass.
- Dependency archives and hashes are pinned and no forbidden fallback is used.

### RG2 — Visual and Accessibility Gate

**Trace:** NFR7, NFR8, NFR9, NFR10, NFR11.

**Acceptance criteria:**

- Manual or presentation-level checks verify 21:1 black/white cell contrast, minimum 4.5:1 text/background contrast, 3:1 focus/boundary contrast, and non-color-only state/error cues.
- Focus rings, solid/dashed placement cues, modal click-through prevention, logical typography, and 32×32 targets pass on macOS and Linux.

### RG3 — Determinism and Performance Gate

**Trace:** NFR3, NFR4, NFR6.

**Acceptance criteria:**

- Default 50×50 simulation/input/render work targets 16 ms in Release on reference CI environments.
- A legal maximum-field generation targets 250 ms in Release; field-memory and overflow limits remain enforced.
- Default session save plus 256×256 preview targets 250 ms; larger operations remain synchronous and correctness-bound without a user-facing progress promise.
- Catch-up, backlog discard, intermediate-render suppression, and no-repayment behavior pass deterministic tests.

### RG4 — Traceability Gate

**Trace:** all FR/NFR identifiers.

**Acceptance criteria:**

- Every FR and NFR appears in the matrix below and maps to at least one story or gate.
- Every story has a bounded outcome, acceptance criteria, and source path.
- No implementation-relevant UX Open Decisions remain unresolved.

## Requirement Traceability Matrix

| Requirement | Story or gate |
|---|---|
| FR1 | E1.1, E4.2, E5.7, E6.3 |
| FR2 | E3.1, E5.1, E5.3, E5.4 |
| FR3 | E3.1, E5.2, E5.6 |
| FR4 | E3.1, E5.2, E5.6 |
| FR5 | E1.1, E3.1, E3.4, E5.2 |
| FR6 | E1.3, E6.1 |
| FR7 | E1.4, E3.4, E6.2 |
| FR8 | E1.3, E3.1, E6.1 |
| FR9 | E1.2, E2.1 |
| FR10 | E2.2 |
| FR11 | E2.3, E2.4, E2.5 |
| FR12 | E3.2, E3.4, E6.2 |
| FR13 | E3.3, E6.2 |
| FR14 | E1.4, E3.4, E6.2 |
| FR15 | E4.1 |
| FR16 | E4.2, E5.2, E6.3 |
| FR17 | E4.3, E5.7 |
| FR18 | E4.5 |
| FR19 | E4.6 |
| FR20 | E4.7, E6.3 |
| FR21 | E4.4 |
| FR22 | E5.5 |
| FR23 | E5.1, E5.6 |
| FR24 | E5.4, E5.7 |
| FR25 | E5.5, E5.6, E6.3 |
| FR26 | E5.1, E5.6, E6.3 |
| FR27 | E1.3, E1.5, E6.1 |
| NFR1 | E1.0, E1.1, E6.1, RG1 |
| NFR2 | E1.2, E2.1, E2.2, E3.2, E3.4, E6.2 |
| NFR3 | E1.3, E2.3, E2.4, E6.1, RG3 |
| NFR4 | E1.5, E6.1, RG3 |
| NFR5 | E1.0, RG1 |
| NFR6 | E4.3, E4.6, E4.7, E5.1–E5.7, E6.3, RG3 |
| NFR7 | E1.1, E1.4, E2.5, E3.5, E4.5, E6.2, RG2 |
| NFR8 | E3.3, E3.5, E6.2, RG2 |
| NFR9 | E3.1, E4.2, E4.4, E5.2, E5.3, E6.3, RG2 |
| NFR10 | E5.1, E5.5, RG2 |
| NFR11 | E1.2, E2.1, E2.2, E2.5, E6.2, RG2 |
| NFR12 | E6.2, RG2, RG4 |
| NFR13 | E6.1, E6.2, E6.3, RG4 |

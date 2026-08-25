---
title: "Life Game — Game Design Document"
game_type: sandbox
secondary_descriptor: simulation
platforms:
  - macOS
  - Linux
status: final
created: 2026-08-19
updated: 2026-08-24
---

# Life Game — Game Design Document

## Executive Summary

### Core Concept

Life Game is a private, single-user Conway's Game of Life study sandbox. The player directly seeds a finite cellular field, watches deterministic patterns emerge, preserves exact rectangular constructions, and reuses them across experiments.

The application imposes no goal, challenge, progression, score, or reward. Its intended player is its creator: a technically curious “nerd” who finds value in manipulating and observing the system without external motivation.

### Unique Value

- Direct cell manipulation with synchronous, predictable Life evolution.
- Exact capture and recomposition of rectangular constructions.
- Multiple persistent experimental sessions sharing one figure Bank.
- A deliberately restrained learning scope with Field as the first complete milestone.

## Goals and Context

### Project Goals

1. Learn C++ desktop game/application development by building a correct, interactive Life sandbox.
2. Make cellular experiments easy to create, observe, preserve, and recombine.
3. Establish a clean design foundation for later program-replication and AI-assisted mathematical research without pulling either into the initial product.

### Background and Rationale

This is a solo personal-study project with no commercial deadline. Existing Life applications already provide similar capabilities; originality and market differentiation are not success criteria. Reimplementation is part of the learning goal.

### Design References

- **Golly** is a reference for deterministic simulation, dependable editing, and respect for expert workflows. Its multi-rule breadth, scripting, layers, and infinite-universe scope are intentionally excluded.
- **Conway Canvas** is a reference for readable selection, preview, and reusable-pattern workflows. Its browser/PWA positioning and broader feature set are intentionally excluded.

## Target Platforms

- **Targets:** macOS and Linux desktop, with equal design priority.
- **Interaction:** mouse-driven native window.
- **Window resizing:** optional and explicitly non-blocking.
- **Logical window baseline:** the initial client area is 1280×720 logical pixels; the minimum supported logical viewport is 960×540. UI text, pointer targets, and Field presentation scale through the operating system's DPI setting exactly once, with pointer input normalized back to logical coordinates before hit testing.

## Core Gameplay

### Game Pillars

1. **Direct Manipulation** — Live and Die tools set the state of cells touched by the player.
2. **Deterministic Evolution** — every generation applies Conway's rules synchronously and independently of rendering frequency.
3. **Preservation** — a highlighted rectangle can be named and saved as a reusable figure.
4. **Composition** — a saved figure can be staged transparently and committed as an exact rectangular replacement.

### Core Gameplay Loop

1. Choose or create a named session; its field and last camera view appear, and generation updates begin immediately.
2. Draw live or dead cells, pause or resume, and observe successive generations.
3. Highlight a useful rectangle and either save it under a unique name or cancel.
4. Open Bank, stage a saved figure, and press Resume to commit the entire rectangle and continue evolution.
5. Leave the session to save its field and view, then reuse figures in the same or another session.

### Win and Loss Conditions

There are no win, loss, score, challenge, achievement, or completion conditions. A session ends only when the player leaves it or closes the application.

## Game Mechanics

### Cellular-Automaton Rules

The field uses standard Conway Life rules:

- A live cell survives when it has exactly 2 or 3 live neighbors.
- A dead cell becomes live when it has exactly 3 live neighbors.
- Every other cell becomes or remains dead.
- All cells read the same previous generation and change simultaneously.
- Every out-of-bounds coordinate is permanently non-live for simulation, never contributes as a live neighbor, and cannot become alive; visible out-of-field space is rendered gray rather than as a black in-field dead cell.

The MVP advances one generation every **0.25 seconds**. Global Settings allow a positive generation interval in seconds, retaining 0.25 seconds as the default; a session created or opened after Save uses the current global interval.

### Field and Camera

- The default field is **50×50 cells**.
- Session setup may configure width and height independently, including rectangular fields.
- Field dimensions never change during a session.
- The field does not wrap or automatically expand.
- A dedicated Move control activates camera movement; dragging on the field moves the camera without editing cells.
- Dedicated `+` and `−` controls change zoom by one level per press.
- Camera movement is clamped so the viewport always shows at least one in-field cell. Any visible area outside the finite Field is gray, not black or white; it is not a cell state, cannot be edited, and is not persisted as field data. A saved session preview may include gray boundary pixels.

### Tools and Input

One tool is always selected. Live is selected by default.

| Tool | Player input | Result |
|---|---|---|
| Live | Press or drag across field cells | Touched cells become alive. |
| Die | Press or drag across field cells | Touched cells become dead. |
| Pause/Resume | Press the control | Generation updates stop or restart. After the command, Live becomes selected. |
| Highlight | Press on one cell, drag to another, release | Select the inclusive rectangular region and open figure capture. |
| Bank | Press the control | Pause simulation immediately and open the global figure list. After Bank ends without placement, Live becomes selected and a fresh interval begins. |
| Move | Press the control, then drag the field | Move the camera without changing cells. |
| Zoom | Press `+` or `−` | Increase or decrease zoom by one level. |

Input outside the field does not change cell state.

### Figure Capture

Completing a Highlight gesture pauses simulation and opens a small dialog with a figure-name field, Save, and Cancel.

- Save succeeds only when the name is unique within Bank.
- Save stores the full rectangular bitmap, including both live and dead cells.
- Cancel or clicking outside the dialog closes it without saving.
- Either outcome closes Highlight and resumes simulation.

### Bank Placement

Opening Bank pauses simulation immediately, clears accumulated simulation time, and lists saved figures by unique name. Selecting a figure closes the list and stages a translucent preview over the field while simulation remains paused. The player moves the preview with the pointer.

Pressing Resume commits a valid preview: every live and dead cell in the saved rectangle replaces the corresponding field cell, while cells outside the rectangle remain unchanged. If any part extends beyond the field, Resume places nothing, exits Bank, returns to Live, and resumes simulation.

Bank provides dedicated Rename and Delete controls. Rename rejects duplicate names.

### Session Lifecycle

The Start Screen contains a horizontally scrolling list of session cards, a Create action, and a Settings action. Each card shows the unique session name, a picture-only preview of its last camera view, and dedicated Rename and Delete controls. Settings opens a two-column table for one global configuration: its left column names the setting and its right column edits its value; it includes Field width and Field height in cells plus the generation interval. Create opens a name-entry dialog and uses the validated global width and height settings as defaults for the new fixed-dimension session. Existing session dimensions never change when global width or height is edited. A session created or opened after Save uses the current global generation interval. Duplicate session names are rejected. Delete requires confirmation.

Leaving a session automatically saves:

- Complete cell state.
- Field dimensions.
- Camera position.
- Zoom level.
- Preview of the last camera view.

Opening a saved session restores those values and begins generation updates immediately. Paused state is not restored. One persistent Bank is shared by all sessions; deleting a session never deletes Bank figures.

### Persistence Failure Behavior

If the required SQLite database cannot open, complete migration, or establish its schema, Life Game shows a specific startup error dialog, does not open the Start Screen, does not create a replacement database, and exits after acknowledgment. If the database opens but one session or Bank figure record fails validation, the database is preserved, valid sessions and figures remain usable, and the affected session card or Bank row remains visible but disabled when its identity is available. Selecting the damaged item shows a specific error. No saved record is automatically repaired, overwritten, or deleted.

## Sandbox-Specific Design

### Creation Tools

Creation consists of direct Live/Die painting, exact rectangular Highlight capture, and Bank reuse. Grid cells provide the only placement space. There is no free-position placement between cells, object rotation, scaling, undo/redo, import/export, or scripting in the confirmed scope.

### System Simulation

Conway's synchronous rules are the sole simulated system. Construction interaction emerges from cell placement and generational evolution; there are no physics materials, structural-integrity rules, environmental systems, or alternate cellular rules.

### Sharing and Community

No sharing, gallery, workshop, ratings, collaboration, accounts, networking, moderation, or multiplayer features are planned. Bank is local and belongs to the single user.

### Constraints and Rules

The player has unlimited editing access and no resource budget. Creative constraints come from the finite field, permanent-dead boundary, exact Life rules, and exact rectangular Bank placement. There is one creative mode and no challenge mode or victory condition.

### Tools and Editing

The tool pool intentionally remains small. Testing and preview occur through Pause/Resume and staged Bank placement. Advanced logic, animation, terrain, weather, lighting, scripting, and multi-rule editing are excluded.

### Emergent Gameplay

Emergence is observed rather than scored. Stable structures, oscillators, moving patterns, destruction, and unexpected interactions may occur, but the application does not classify, reward, teach, or publish them. The player defines what is interesting.

## Progression and Balance

### Player Progression

There is no character, account, unlock, skill-tree, content, or meta progression. The only accumulation is the player's own knowledge plus the shared Bank of saved figures.

### Difficulty Curve

There is no designed difficulty curve. Complexity increases only when the player chooses larger fields or more complex constructions.

### Economy and Resources

There is no currency, cost, inventory limit, energy, score, or resource economy.

## Level Design Framework

### Level Types

There are no authored levels. Each named session is a persistent experimental workspace with a fixed finite field.

### Session Progression

Sessions do not unlock or order one another. The launch browser provides direct access to every saved session, and the global Bank permits construction reuse between them.

## Art and Audio Direction

### Art Style

- Dark field and background.
- Bright live cells and subdued dead cells.
- Gray out-of-field area is visually distinct from black dead cells and white live cells.
- Faint grid lines visible only at useful zoom levels; the UX contract renders them at or above 4 logical display pixels per cell.
- Distinct selection accent.
- Translucent staged Bank figure that preserves visibility of underlying cells.
- Small, flat, functional controls in the upper-right region of Field.
- No world art, characters, narrative presentation, or decorative animation requirement.

### Audio and Music

No music, ambience, sound effects, or audio feedback.

## Technical Specifications

### Technology and Platforms

- Native C++23 desktop application.
- CMake build.
- raylib 6.0 and raygui 5.0, pinned to exact versions.
- macOS and Linux support.
- Only language and library features verified on both chosen toolchains.

### Performance and Correctness Targets

No frames-per-second target is defined. The default 50×50 field must complete each synchronous generation before the next scheduled 0.25-second update.

Correctness is demonstrated by canonical behaviors:

- A lone live cell dies after one generation.
- A 2×2 block remains stable.
- A three-cell blinker returns to its starting orientation after two generations.
- Out-of-bounds positions remain dead and affect edge patterns accordingly.

### Asset Requirements

Only a readable interface font, simple tool icons or labels, grid rendering, selection styling, figure transparency, and session-preview images are required. There are no audio assets or content-production pipeline requirements.

## Development Epics

| Sequence | Epic | Product outcome | Release boundary |
|---:|---|---|---|
| 1 | Field MVP | A 50×50 lined field accepts Live input and advances correct Life generations every 0.25 seconds. | MVP |
| 2 | Editing and Observation | Die, Pause/Resume, generation timing settings, and clear selected-tool behavior. | Post-MVP |
| 3 | Field Navigation and Setup | Move mode, `+`/`−` zoom, and configurable square or rectangular session fields. | Post-MVP |
| 4 | Figure Capture and Bank | Highlight, naming, exact figure storage, Bank management, staging, and replacement. | Post-MVP |
| 5 | Persistent Sessions | Session browser, Create/Rename/Delete, automatic state saving, previews, and global Bank persistence. | Product target |
| 6 | Cross-Platform Completion | Consistent macOS/Linux behavior, visual polish, failure handling, and release verification. | Product target |

Detailed epic goals and high-level stories live in `epics.md`.

## Success Metrics

### MVP Success

- All four canonical Life behaviors pass on the 50×50 field.
- Clicking any visible cell in Live mode makes the intended cell alive.
- Generations advance at the fixed 0.25-second interval independently of rendering cadence.
- The application runs on at least one macOS and one Linux development environment.
- No post-MVP subsystem is required to demonstrate Field.

### Product Success

- Sessions restore exact cell state, dimensions, camera position, and zoom.
- Bank figures retain exact live/dead rectangles across sessions.
- Valid placement replaces exactly the target rectangle; invalid placement changes no cells.
- The player can create, preserve, and recombine constructions without a program-defined objective.

## Out of Scope

### Explicitly Excluded

- Multiplayer, accounts, online services, community sharing, moderation, and collaboration.
- Authored challenges, goals, progression, achievements, scoring, and economy.
- Infinite or wrapping boards.
- Multiple cellular-automaton rules, scripting, layers, physics, narrative, and audio.
- Mobile, web/PWA, Windows, and console releases.
- Undo/redo, import/export, rotation, or scaling unless added through a future GDD update.

### Parked Research

- `flex`-triggered program/source replication in which the application creates a copy of itself “next to” the original.
- AI-assisted mathematical reasoning and evidence for more complex constructions.
- An uncommitted semantic construction workbench with versioning, provenance, and replayable experiment records.

None of these research directions belongs to the current development epics. Each requires a future GDD update before entering product scope.

## Assumptions and Dependencies

- raylib 6.0 and raygui 5.0 remain available for both target platforms.
- The selected C++23 feature subset compiles consistently on the chosen macOS and Linux toolchains.
- A 50×50 default field provides enough visible space for early experiments; changing this later requires an explicit design update.
- UX production constants are confirmed in the Experience spine: zoom levels are 50%, 75%, 100%, 150%, 200%, 300%, and 400% with a 100% default; names are trimmed, NFC-normalized, 1–64 Unicode code points, and case-insensitive for uniqueness; Bank deletion requires confirmation; session previews are 256×256 PNGs. Persistence format remains architectural.
- Maximum configurable field dimensions are deferred to architecture and must preserve the fixed-dimensions-per-session rule.
- Window resizing remains optional.

## Deferred Design Notes

UX-owned zoom, naming, Bank-delete, grid, and session-preview constants are confirmed in the Experience and Design spines. Remaining deferred presentation decisions continue to be tracked there and in the architecture document.

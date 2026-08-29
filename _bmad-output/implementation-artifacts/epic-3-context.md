# Epic 3 Context: Field Navigation and Setup

<!-- Generated from planning artifacts. Regenerate with compile-epic-context if planning docs change. -->

## Goal

Introduce the Start Screen/session shell and one global, in-memory Settings configuration, then let each newly created finite field be navigated at multiple scales without changing its geometry. Players can configure dimensions and timing, create a named session, move and zoom its camera, and retain useful grid visibility. Persistence and durable previews are deferred to Epic 5; figure capture and Bank workflows belong to Epic 4.

## Stories

- Story 3.1: Configure Settings and Create an In-Memory Session
- Story 3.2: Move the Camera
- Story 3.3: Change Discrete Zoom
- Story 3.4: Preserve Fixed Field Geometry While Navigating
- Story 3.5: Apply the Grid Visibility Threshold

## Requirements & Constraints

- The Start Screen owns the session shell, Create action, and Settings gateway. Settings exposes one global Field width, Field height, and positive generation interval. Defaults are 50×50 cells and 0.25 seconds.
- Settings Save validates and commits all three values atomically. Invalid numeric input remains visible with field-local validation and disables Save; Cancel makes no change. A failed Save preserves the prior valid configuration, retains edits, and offers Retry/Cancel.
- Create requires a valid unique name. Trim outer whitespace, normalize names to NFC, allow 1–64 Unicode code points, preserve internal whitespace/display case, and compare uniqueness case-insensitively. A successful create uses the saved width/height, opens Field with Live selected, and starts at the saved interval.
- Width and height apply only to newly created sessions. Each session keeps fixed dimensions; rectangular fields are valid. The finite field never wraps or expands, and out-of-field positions cannot edit or become live.
- Move and zoom change only the camera presentation. The viewport must continue to intersect at least one in-field cell; visible outside area is gray and is not field data. Use zoom levels 50%, 75%, 100%, 150%, 200%, 300%, and 400%, clamped at the ends, with new sessions at 100%. Show a 1px grid line at or above 4 logical display pixels per cell and hide it below that threshold while retaining cell state.
- Use logical client pixels consistently on macOS and Linux: normalize pointer coordinates before UI, camera, or cell mapping; initial logical viewport is 1280×720 and minimum supported viewport is 960×540. Pointer targets are at least 32×32 logical px.

## Technical Decisions

- Keep the field/session model independent of raylib and raygui; use C++23, CMake, raylib 6.0, and raygui 5.0 at the application/presentation boundary. Render the field as a board or texture rather than a UI object per cell.
- Use explicit application state machines and a central presentation input router that emits typed logical-coordinate commands. Keep presentation ownership clear: Start Screen owns the browser and Settings, while Field Screen owns the interactive field and Toolbar.
- Use pinned utf8proc 2.11.2 for complete Unicode NFC normalization and case folding; do not fall back to platform or system Unicode libraries.
- Keep global Settings distinct from per-session state. Session dimensions are captured at creation and are never resized by later Settings edits; a session created after Save uses the current interval and begins with a fresh simulation interval.
- Preserve deterministic scheduling: sample the elapsed-time snapshot at iteration start, run up to four due generations before input and rendering, discard excess timing debt, then process input and render once. Camera transforms and zoom must not alter logical cell coordinates.
- Enforce the configured field safety limits: width and height 1–4096, with no more than 4,194,304 total cells. There is no user-facing FPS target.
- Keep Epic 3 storage in memory; do not introduce SQLite, durable previews, or persistent Bank behavior before Epic 5.

## UX & Interaction Patterns

- The Start Screen is the session browser and Settings entry point, not a separate title screen. Its session list is horizontal, stably case-insensitive sorted, and keeps Create available when empty. A session-card preview, when shown, is an inert picture; selecting the card/name opens the interactive Field Screen.
- Settings is a two-column table with labels on the left and aligned numeric fields on the right. Numeric fields retain invalid text rather than silently clamping or rounding. Dialogs capture pointer input so controls cannot click through to the Field; the interface is mouse-first, with keyboard input for text and numeric entry.
- Field controls live in the compact upper-right Toolbar. Move is a persistent mode and dragging changes camera only. `+`/`−` are commands: zoom anchors on the in-field cell under the pointer, or the viewport center when the pointer is outside the field.
- Render in-field dead cells black and live cells white; render out-of-field space gray. Keep active, focus, and validation/error meaning available through labels, borders, or line styles rather than color alone. Use flat, rectilinear controls and readable 16px body/14px compact or numeric text.

## Cross-Story Dependencies

- Epic 3 depends on the Epic 1 Field MVP and foundation: its field state, finite boundaries, deterministic generations, and rendering are the base for the session shell. Story 3.1 establishes the session/configuration state consumed by camera and zoom stories.
- Stories 3.2 and 3.3 must obey Story 3.4’s fixed-geometry and camera-boundary invariant. Story 3.5 derives grid visibility from the zoomed cell size without changing cell state.
- Epic 4 assumes Epic 3’s fixed fields and Field Screen. Epic 5 upgrades this in-memory shell to SQLite persistence while retaining global Settings and fixed per-session dimensions. Epic 6 verifies the same navigation, scaling, input mapping, and visual behavior across macOS and Linux.

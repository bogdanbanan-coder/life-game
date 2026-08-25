---
name: "Life Game"
description: "The interaction, navigation, state, and accessibility contract for a native Life-study sandbox."
status: final
project: life-game
created: 2026-08-19
updated: 2026-08-24
sources:
  - ../../briefs/brief-life-game-2026-08-15/
  - ../../gdds/gdd-life-game-2026-08-19/
---

# Life Game — Experience Spine

## Foundation

Life Game is a native macOS and Linux desktop application with equal platform priority. It is mouse-first and runs in a native desktop window. The implementation foundation is C++23, raylib 6.0, raygui 5.0, and CMake with pinned dependency versions. raygui supplies immediate-mode controls; this spine specifies the behavioral delta and `DESIGN.md` specifies the visual delta.

The application is a private mathematical study tool, not a story game. The task persona is `Nerd`, whose only supplied motivation is interest in mathematics. There are no goals, scores, progression, achievements, narrative beats, or prescribed completion states.

`DESIGN.md` is the visual-identity authority. This spine and `DESIGN.md` win on conflict with any mock, wireframe, or import. No creative artifacts or visual mocks were produced on the fast path.

`[ASSUMPTION]` Keyboard input is supported for text and numeric entry only; full keyboard navigation and shortcuts are not part of this draft. `[ASSUMPTION]` A trackpad is supported only through ordinary pointer emulation. Controller, touch, motion, and platform button-glyph adaptation are out of scope with the confirmed desktop/mouse foundation.

## Information Architecture

### Navigable screens and dialogs

| Surface | Reached from | Purpose | Leaves to |
|---|---|---|---|
| Start Screen | Application launch; Field Screen `Exit` | Owns the session browser and Settings gateway. Shows the horizontal session-card list and Create/Settings controls. | Name dialog, Settings panel, selected Field Screen, session rename/delete dialogs, OS close. |
| Session card preview | Start Screen → session-card list | Inert 256×256 picture of the saved field view. It is not an interactive Field screen. | Interactive Field Screen when the card is selected. |
| Name dialog | Start Screen → Create/Rename; Field → figure capture/Bank rename | Enter or edit one session or figure name with validation. | Calling surface on Save or Cancel. |
| Settings panel | Start Screen → Settings | Shows the global settings in a two-column table with setting names on the left and editable values on the right: Field width (cells), Field height (cells), and generation interval. | Start Screen on Save or Cancel. |
| Field Screen | Select a session card; create a session | Interactive finite fixed-dimension Life field. Restores cells, dimensions, camera position, and zoom for saved sessions, then begins generation updates immediately. | Figure-capture dialog, Bank panel, staged-figure overlay, Start Screen via `Exit`, OS close. |
| Figure-capture dialog | Complete an inclusive Highlight drag | Name and save the exact live/dead rectangle, or cancel it. Simulation remains paused while open. | Field running in Live after Save, Cancel, or outside click. |
| Bank panel | Field Screen → Toolbar → Bank | Pause simulation immediately when opened; browse the one application-wide figure list by unique name; select, rename, or delete a figure. | Staged-figure overlay on select; Field running in Live on close with a fresh interval. |
| Confirmation dialog | Session delete or Bank delete | Modal “Confirm / Cancel” prompt naming the destructive target. No deletion happens until Confirm. | Calling surface after Confirm or Cancel. |
| Error dialog | Recoverable persistence/load failure or fatal startup database failure | Modal explanation of a specific error with Acknowledge. A recoverable record error returns to its owning surface; a fatal startup error exits the application after acknowledgment. | Owning surface after acknowledgment, or application exit for a fatal startup error. |

### Field regions, overlays, and feedback

| Element | Present when | Purpose | Ends when |
|---|---|---|---|
| Toolbar | Field Screen is open | Upper-right panel exposing Live, Die, Pause/Resume, Highlight, Bank, Move, `+`, `−`, and `Exit`; makes the active persistent mode and run state visible. | Field Screen closes. |
| Staged-figure overlay | A Bank figure is selected | Move a translucent preview of the exact rectangle over the Field while simulation remains paused. | Resume commits or rejects placement and returns to running Live. |
| Status feedback | Duplicate/invalid input, invalid placement, save/rename failure, busy operation | Give non-audio, non-color-only feedback without creating another navigation level. Busy feedback is pre-operation or post-operation because the underlying calls are synchronous. | Remains on the current surface. |

The Start Screen is both the launch session browser and the Settings gateway. It is not a separate title presentation. A session card preview is only a picture; selecting the card opens the interactive Field Screen. Pressing `Exit` in Field Screen automatically saves the session state and last-camera-view preview, then returns to the Start Screen. Closing the application window exits the application; `[ASSUMPTION]` when a session is open, closing the window first attempts the same automatic save as `Exit` and does not add an extra quit-confirmation surface.

`[ASSUMPTION]` When no sessions exist, the Start Screen shows an empty `session-card-list` with a direct Create action.

Settings uses explicit Save and Cancel actions. It stores one global validated configuration containing Field width, Field height, and generation interval. Width and height are defaults for newly created sessions; an existing session remains fixed. The current global generation interval is used whenever a session is created or opened after the settings are saved.

`[ASSUMPTION]` The New Session name dialog starts blank. Create stays unavailable while the name is invalid. Cancel does not create a session. A successful create uses the validated Settings width and height, opens the new Field Screen immediately, and starts simulation at the configured interval.

`[ASSUMPTION]` The `bank-list` and `session-card-list` sort names by a stable case-insensitive ascending order. Closing or canceling Bank does not place a figure; it returns to Live and resumes the simulation with a fresh interval.

## Voice and Tone

`[ASSUMPTION]` Microcopy is brief, literal, and mathematical-tool-like. The table below proposes exact copy where the sources define an outcome but not its wording. Brand posture lives in `DESIGN.md`.

| Do | Don't |
|---|---|
| `Outside field. Nothing placed.` | `Oops! Try again!` |
| `Name already exists.` | `Something went wrong.` |
| `Session data could not be read.` | Invent a fictional or emotional explanation. |
| Use exact action labels: `Save`, `Cancel`, `Delete`, `Exit`, `Resume`. | Use motivational, reward, narrative, or playful copy. |
| State the result of a destructive or failed action. | Rely on a shade change or silent no-op. |

`[ASSUMPTION]` The shipping interface language is English. User-entered names may use Unicode as defined in Naming & Validation.

## Component Patterns

Behavioral rules only; visual specifications live under the identical component identifiers in `DESIGN.md` Components.

`[ASSUMPTION]` Any precise behavior below for activation, focus, dismissal, empty states, or error retention that is not explicitly inherited from the sources is a fast-path proposal.

| Component | Behavioral specification |
|---|---|
| `button` | `[ASSUMPTION]` Hovering does not activate a `button`; pressing and releasing inside the `button` activates it once; disabled actions neither mutate state nor dismiss their surface. Persistent-mode buttons remain selected until another persistent mode is chosen. |
| `panel` | `[ASSUMPTION]` Groups related controls without becoming a navigation target. Pointer events captured by a panel never reach Field cells beneath it. |
| `field-grid` | Maps the pointer to exact integer cell coordinates. Live/Die paint only in-bounds cells. Highlight returns an inclusive cell rectangle. Camera transforms never alter field coordinates or cell state; visible out-of-field area is gray and cannot receive cell input. |
| `toolbar` | Upper-right panel on the interactive Field Screen. Shows Live, Die, Pause/Resume, Highlight, Bank, Move, `+`, `−`, and `Exit`, plus the active persistent mode and running/paused/staged status. Modal workflows disable commands that are not part of that workflow. |
| `session-card-list` | Scrolls horizontally when cards exceed available width. `[ASSUMPTION]` Pointer wheel scrolls horizontally while hovered; a visible scrollbar also supports dragging. Empty state keeps Create available. |
| `session-card` | Shows the session name, an inert 256×256 PNG field preview, and dedicated Rename/Delete controls. Clicking the preview/name area opens the interactive Field Screen and starts generation immediately. Dedicated actions do not open the session. Delete always opens the confirmation dialog. A damaged session record remains visible as a disabled card when its identity is available; selecting it shows a specific error instead of opening Field. |
| `field-preview` | Picture-only session preview rendered from the saved camera view. It never receives Field editing input and never owns simulation state. |
| `settings-table` | Two-column table owned by the Start Screen's Settings panel. It edits one global configuration: the left column names Field width, Field height, or generation interval; the right column contains the corresponding `numeric-field`. Width and height never resize an existing session. |
| `dialog` | raygui in-application modal. Captures all pointer input. Confirmation and error variants require an explicit action. Figure capture is the only dialog where outside click is defined as Cancel. `[ASSUMPTION]` Outside click does not dismiss any other dialog. |
| `text-field` | Accepts and displays a candidate session or figure name. It retains invalid input and shows validation adjacent to the field. Save/Create/Rename cannot commit an invalid or duplicate name. |
| `numeric-field` | Accepts dimensions or positive seconds. `[ASSUMPTION]` It retains invalid input for correction and never silently clamps or rounds it. |
| `bank-list` | Lists the shared Bank by unique name while the Field is paused. Selecting a valid row stages that figure; dedicated Rename/Delete actions do not stage it. A damaged figure record remains visible as a disabled row when its identity is available; selecting it shows a specific error. `[ASSUMPTION]` Empty state explains that Highlight saves figures here. |
| `staged-figure-overlay` | Snaps to Field cells and follows pointer movement while paused. Resume is the only commit command. It replaces the entire valid stored rectangle, including stored dead cells; outside cells are unchanged. Invalid Resume commits nothing, exits Bank, selects Live, and resumes. |
| `status-message` | Announces success, failure, invalid placement, or busy state in text. Busy text may be staged before a synchronous call and replaced with success or failure after it returns; the whole window may block during the call. It never promises live progress or partial interactivity. `[ASSUMPTION]` Feedback remains until the next relevant action or explicit dismissal and never vanishes on a short timer. |

## State Patterns

| Surface | Required states and treatment |
|---|---|
| Start Screen | **Cold load:** stage `Loading sessions…` before the synchronous metadata/preview load when a busy indication is useful; the window may block until the call returns. **Empty:** Create remains available. **Populated:** horizontal cards. **Fatal database failure:** show a specific startup `error-dialog`; do not show the browser or create a replacement database, and exit after Acknowledge. **Damaged session record:** preserve the database, keep valid cards usable, and show the affected session as a disabled card with a specific error on selection. |
| New Session name dialog | **Pristine:** name blank. **Invalid:** field-local text, Create disabled. **Duplicate:** `Name already exists.` **Create failure:** blocking error; no partial card appears. **Cancel:** no mutation. Validated Settings width and height supply the new session dimensions. |
| Settings panel | **Clean:** current global Field width, Field height, and generation interval in a two-column table. **Dirty:** Save enabled. **Invalid:** field-local validation and Save disabled. **Save failure:** retain edits and show blocking error. **Cancel:** discard uncommitted edits. Width/height changes affect new sessions only; the current global interval is used by sessions created or opened after Save. Save/Cancel interaction details remain `[ASSUMPTION]`. |
| Field Screen / Toolbar | **Running:** generation updates active; selected persistent mode visible. **Paused:** no generation updates; Resume visible. **Painting:** touched in-bounds cells change to Live/Die. **Moving:** drag changes camera only. **Bank opened:** RunState changes to Paused, accumulated simulation time is cleared, and Bank controls remain interactive. `[ASSUMPTION]` **Modal/capture/Bank:** unrelated controls are inert. `[ASSUMPTION]` **Auto-save failure:** blocking error before returning to Start Screen; recovery action remains architecture-dependent. |
| Figure-capture dialog | **Open:** selection visible, simulation paused. **Invalid/duplicate name:** selection retained, Save blocked. **Saved:** figure added, dialog closes, Live selected, simulation resumes. **Cancel/outside click:** no Bank mutation, dialog closes, Live selected, simulation resumes. Return-to-Live is `[ASSUMPTION]`. |
| Bank list | **Loading:** stage a named busy `status-message` before the synchronous load; the whole window may block while it runs, then show the result or an error. **Empty:** explanation plus Close. **Populated:** valid rows are selectable; a damaged figure row is visible but disabled and reports a specific error when selected. **Rename invalid/duplicate:** retain edit. **Delete:** confirm before removal. **Persistence failure:** block the mutating action and retain the list. **Close:** no placement, Live selected, simulation resumes with a fresh interval. |
| Staged-figure overlay | **Valid:** exact rectangle visible with solid outline. **Invalid:** exact rectangle visible with dashed outline and `Outside field`. **Resume valid:** commit exact replacement, Live selected, resume. **Resume invalid:** place nothing, show `Outside field. Nothing placed.`, Live selected, resume. **Pointer outside window:** `[ASSUMPTION]` preserve the last in-window preview position. |
| Confirmation dialog | **Confirm:** destructive action and target are named. **Cancel:** no mutation. No nested modal stack. |
| Error dialog | **Recoverable action or damaged-record failure:** specific error is shown and Acknowledge returns to the calling surface. **Fatal database open/migration failure:** specific startup error is shown and Acknowledge exits the application. No nested modal stack. |
| Status feedback | **Success:** exact completed action. **Validation failure:** adjacent to invalid input. **Invalid placement:** visible after return to Live. **Busy:** names the blocked action. Every state is textual and visual because the product has no audio. |

Persisted data is never silently treated as valid empty data. If the database cannot open or migrate, the failure is fatal at startup: the application preserves the database, shows a specific error, and exits after acknowledgment. If one session or figure record fails validation after the database opens, the application preserves it, keeps valid records usable, and renders the affected session card or Bank row disabled when its identity is available; selecting it shows a specific error. No record is automatically repaired, overwritten, or deleted.

## Interaction Primitives

### Pointer and Field

- Use Live or Die by pressing or dragging over Field cells. Every touched in-bounds cell is set directly to the active state; input outside the Field changes no cell.
- `[ASSUMPTION]` Painting interpolates between pointer samples in cell coordinates so a fast drag does not leave unintended gaps. Leaving the Field while held stops cell changes; re-entering while still held continues the same stroke.
- `[ASSUMPTION]` Pointer capture lasts through a Move or Highlight drag until release, including when the pointer briefly leaves the Field. Only in-bounds endpoints contribute to Highlight; releasing without a valid second in-bounds cell cancels without opening capture.
- The main-loop order is deterministic: clock and simulation work run first; raylib-exposed input is then sampled and translated; accepted Live/Die commands execute next; rendering runs last. A running edit is visible in that render and is not promised to affect a generation already processed in the iteration. Catch-up adds no special input-retention or replay queue, so input not exposed at the sampling phase is outside the accepted-input guarantee. No pause is required, and the exact timer-boundary-to-render assignment remains an implementation detail.
- Move changes only camera position. `[ASSUMPTION]` Live, Die, and Move are persistent modes; Highlight and Bank launch modal workflows; Pause/Resume, zoom, and Exit are commands. Highlight ends in Live after Save/Cancel. Opening Bank pauses the Field; Bank ends in Live with a fresh interval unless a staged placement is still awaiting Resume.

### Zoom and camera

- `+` and `−` change exactly one discrete zoom level per press.
- Levels are 50%, 75%, 100%, 150%, 200%, 300%, and 400%; 100% is the new-session default.
- `[ASSUMPTION]` Zoom anchors on the Field cell under the pointer; if the pointer is outside the Field, zoom anchors on the viewport center.
- Camera movement is clamped so the viewport always shows at least one in-field cell. Exact centering when the Field is smaller than the viewport is implementation-owned; any remaining viewport area is gray out-of-field space.
- Grid lines display when a cell is at least 4 logical display pixels across, using the `DESIGN.md` `field-grid` rule.

### Capture and Bank placement

- Highlight press, drag, and release selects the inclusive rectangle between the two cells. Completing it pauses generation and opens the figure-capture dialog.
- Save stores the full live/dead rectangle. Selecting Cancel or clicking outside saves nothing. Either outcome resumes generation.
- Bank opening pauses generation. Selecting a figure closes Bank and stages its translucent rectangle over Field.
- `[ASSUMPTION]` The stored rectangle's top-left cell anchors to the cell under the pointer. The preview snaps one whole cell at a time.
- Resume commits only when every staged cell is within the finite Field. A valid commit replaces every underlying live and dead cell in the rectangle and leaves outside cells unchanged. An invalid commit changes nothing, exits Bank, returns to Live, and resumes.

### Destructive and modal actions

- Session Delete requires confirmation and names the session in the message.
- Bank-figure Delete requires confirmation because there is no undo/redo and Bank is the only accumulating in-product record of saved constructions.
- Rename uses the same `dialog`, normalization, and uniqueness rules as creation/capture and makes no change until Save.
- Figure-capture outside click is equivalent to Cancel. Other outside clicks do not dismiss dialogs.

## HUD & Non-diegetic UI

All Life Game UI is non-diegetic application UI; there is no fictional world or in-world display. The Field is primary. The compact `toolbar` occupies the upper-right region, above the Field render layer but below any modal `dialog`.

Information hierarchy:

1. Cell state and exact grid position.
2. Active persistent mode and simulation state: Running, Paused, or Staging.
3. Available commands in the Field toolbar.
4. Transient `status-message` feedback.

The active mode never fades. `[ASSUMPTION]` The remaining hierarchy is: cell state first, active mode/run state second, available commands third, and feedback fourth. Running/Paused/Staging appears as a short text label in the `toolbar`; the Pause/Resume label alone is not treated as sufficient status. The Field includes no generation counter, coordinates, zoom value, field dimensions, or interval readout because the confirmed sources require none.

The Field grid hides only when zoom makes it noisy. The cell states themselves never hide. During capture or staged placement the source rectangle/preview stays visible and the underlying Field remains readable. Modal surfaces block Field edits until resolved.

## Input Schemes

| Scheme | Contract |
|---|---|
| Mouse | Primary and complete pointer scheme for menus, buttons, Field editing, Move, Highlight, Bank staging, and horizontal scrolling. |
| Keyboard | `[ASSUMPTION]` Used for focused text and numeric entry only. Enter/Escape shortcuts and full focus traversal are not committed. |
| Trackpad | `[ASSUMPTION]` Ordinary operating-system pointer emulation only; no gestures. |
| Controller / touch / motion | Out of scope. No glyph adaptation or remapping screen is required. |

Pointer cursors are `[ASSUMPTION]`: arrow over menus, crosshair over editable Field in Live/Die/Highlight, open hand in Move, closed hand while moving, and rectangle preview during staging. Cursor shape reinforces the active operation without becoming its only cue.

## Game Feel & Feedback

Feedback is immediate, exact, visual, and quiet. Changes made with Live or Die appear on the same rendered frame as the accepted input. This visual immediacy does not promise that the edit is included in a generation scheduled for that frame; the ordered frame phases define the result. Tool selection changes at activation. Pause stops generation updates without freezing camera or UI. Resume restarts at the configured interval.

There is no music, ambience, sound effect, or audio feedback. `[ASSUMPTION]` There is also no screen shake, hit-stop, haptic feedback, easing animation, animated toast, or decorative transition. These would obscure inspection or add unsupported sensory channels. Status changes use stable text, line style, and direct Field updates.

Capture and Bank operations prioritize proof of exactness: selection bounds stay aligned to the cell grid; staging shows the whole stored rectangle; valid commit changes the complete rectangle at once. An invalid commit still follows the source-defined transition back to running Live, but leaves `Outside field. Nothing placed.` visible in `status-message` so the no-op is understandable.

## Accessibility Floor

Behavioral floor; contrast and visual tokens live in `DESIGN.md`.

- Live/dead states use black/white and 21:1 contrast. Selection, staging, validity, errors, and active mode add outline style and text; color/shade is never the sole cue.
- Out-of-field space is gray and is never treated as a third cell state: it is not simulated, editable, or persisted as field data. A saved session preview may include its gray pixels when the camera view includes the boundary. In-field dead cells remain black and in-field live cells remain white.
- Body UI text is 16 logical px and compact labels are 14 logical px; the UI follows operating-system DPI scaling on macOS and Linux without bitmap blurring.
- Pointer targets are at least 32×32 logical px. This is the minimum for the explicitly compact upper-right controls.
- `[ASSUMPTION]` Every pointer-active overlay captures input so clicks cannot mutate hidden Field cells.
- Text and numeric inputs retain invalid values for correction, pair errors directly with the responsible field, and never rely on audio.
- `[ASSUMPTION]` No Reduce Motion setting is needed because the draft defines no non-essential motion. If motion is added later, reduced-motion behavior becomes required.
- `[ASSUMPTION]` Full keyboard-only navigation, screen-reader semantics, and alternative input support are not committed for this private mouse-first release. This is an accessibility limitation, not an implicit capability.
- The Field remains inspectable when paused; camera and zoom remain available unless a modal workflow needs exclusive pointer capture.

## Inspiration & Anti-patterns

- **Golly:** retain deterministic simulation, dependable editing, and respect for expert workflows. Do not inherit multiple rules, scripting, layers, or infinite-universe scope.
- **Conway Canvas:** retain readable selection, preview, and reusable-pattern workflows. Do not inherit browser/PWA positioning or broader features.
- Research references to stored buffers, timelines, interchange, provenance, classification, and reproducibility are not present-product requirements.
- Avoid challenge lists, scores, achievements, onboarding theatrics, reward feedback, narrative framing, community UI, accounts, inventory/map metaphors, import/export, undo/redo, rotation, and scaling controls.
- Avoid visual spectacle, silent failures, hidden cell mutations, and any placement behavior that does not preserve the exact live/dead rectangle.

## Responsive & Platform

macOS and Linux receive the same surface map, labels, pointer semantics, Life timing semantics, and persistence behavior. Platform-native window chrome may differ; all in-application dialogs remain raygui surfaces rather than native OS dialogs.

Window resizing is optional and non-blocking. The initial client area is 1280×720 logical px and the minimum supported logical viewport is 960×540; a smaller logical viewport is unsupported. If resizing is implemented, the Field viewport absorbs added space while the upper-right `toolbar`, dialogs, and Start Screen controls retain token sizing. No mobile, web, Windows, console, handheld, TV, or VR adaptation is planned.

All UI layout, text, pointer targets, and Field screen geometry use logical client pixels. The platform/renderer applies OS DPI scaling exactly once; pointer input is normalized back to logical client coordinates before UI hit testing, camera conversion, or Field cell mapping. The 4-pixel grid threshold is measured in logical display pixels. High-DPI behavior must be verified on both target platforms before Cross-Platform Completion.

## Naming & Validation

Unique names are required separately across sessions and across the application-wide Bank. Names use the following shared validation contract:

- Trim leading and trailing whitespace; a result with zero characters is invalid.
- Accept Unicode, normalize to NFC, and allow 1–64 Unicode code points.
- Uniqueness comparison is case-insensitive after trimming and normalization, so `Blinker` and `blinker` conflict.
- Keep the user's internal whitespace and displayed case after validation.
- Validation runs during editing, but validation feedback appears only after the field has been touched or a commit is attempted. Duplicate feedback is `Name already exists.`
- `[ASSUMPTION]` Global Settings Field width and height accept positive base-10 whole numbers and each default to 50 cells; architecture supplies the maximum. Values above that maximum are rejected, not clamped. Saved session dimensions do not change when these global defaults are edited.
- `[ASSUMPTION]` The generation interval field accepts a positive base-10 number of seconds with `.` as the decimal separator; the default is 0.25. Zero, negative, nonnumeric, NaN, and infinity are rejected.
- `[ASSUMPTION]` The UI is English-only for this personal release, but Unicode names persist round-trip on both platforms.

## Performance-facing UX

Correctness wins over apparent speed. A generation is synchronous and completes before the next scheduled update; rendering frequency never changes Life results. The default 50×50 field must finish each generation before the next 0.25-second update.

- Field input and camera feedback should remain visibly responsive while simulation runs; no FPS value is promised.
- When the application falls behind, one main-loop iteration executes at most four due simulation generations total before processing input and rendering once. Intermediate generations are not displayed. Excess whole-generation timing backlog is discarded rather than carried into later iterations; every generation that is executed remains complete and sequential. The application does not show a slow-simulation status message.
- Session, Bank, and preview operations use named pre-operation busy feedback or post-operation success/failure feedback. Because these operations are synchronous on the main thread, the whole window may block while a call runs; the UX makes no promise of live progress, partial interactivity, or a progress animation. A new asynchronous architecture decision would be required to change that contract.
- Successful persistence is defined by exact restoration, not by displaying a progress animation. Failed saves/loads never silently discard, replace, or reinterpret data.
- Maximum field dimensions, session count, Bank size, figure dimensions, and acceptable operation latency are architecture-owned limits and remain performance-validation concerns. Session previews use 256×256 PNGs; synchronous operations are measured against those limits rather than presented as live-progress work.

## Key Flows

### Flow 1 — Nerd configures and observes a first experiment

Nerd is a non-story task persona interested only in mathematics. `[ASSUMPTION]` The concrete experiment is a three-cell blinker because no individual construction was supplied.

1. Nerd launches Life Game. The Start Screen loads the horizontal `session-card-list`; if no sessions exist, Create remains available.
2. Nerd opens Settings, changes or verifies Field width, Field height, and the positive generation interval, then saves. Invalid input remains in the corresponding `numeric-field` with a direct error; nothing is silently coerced.
3. Nerd selects Create. The New Session name dialog opens. Nerd enters a unique name and creates a session using the saved width and height defaults.
4. The interactive Field Screen opens and begins generations immediately. Live is visibly selected; the running state is explicit.
5. Nerd presses Pause. On the lined Field, Nerd uses Live to paint three cells and Die to correct one accidental cell, confirming that only touched in-bounds cells change. Nerd selects Move to inspect another region without editing, uses `+`/`−` by discrete levels, and observes black in-field dead cells against the gray out-of-field boundary.
6. Nerd presses Resume; Live becomes selected and synchronous updates continue at the configured interval.
7. **Climax — `[ASSUMPTION]`:** the three-cell line alternates orientation and returns to its starting orientation after two generations, giving Nerd the mathematical observation sought.

Failure path: a duplicate session name blocks creation with `Name already exists.` and preserves every entered field. A width/height beyond the architecture limit remains editable and creates no partial session.

### Flow 2 — Nerd captures and exactly reuses a construction

`[ASSUMPTION]` The concrete construction is the observed blinker; exact preservation and reuse are the climax because the stakeholder supplied no individual experiment.

1. From a running Field, Nerd selects Highlight, presses on one cell, drags to another, and releases. The inclusive rectangle is outlined, simulation pauses, and the figure-capture `dialog` opens.
2. Nerd enters a unique figure name and presses Save. The complete rectangle, including live and dead cells, enters the application-wide Bank. The dialog closes, Live is selected, and simulation resumes.
3. Nerd opens Bank. Simulation pauses; the `bank-list` shows the saved figure by name.
4. Nerd renames the figure through its dedicated action, then selects it. Bank closes and its translucent `staged-figure-overlay` follows the pointer, snapped to cells over the still-visible Field.
5. Nerd positions the full rectangle inside the Field and presses Resume.
6. **Climax — `[ASSUMPTION]`:** every stored live and dead cell replaces the corresponding cell in the target rectangle at once, cells outside the target rectangle remain unchanged, Live returns, and evolution resumes with an exact reusable copy.

Failure paths:

1. Duplicate figure name: Save remains blocked, selection and typed name remain, simulation stays paused; Cancel or outside click saves nothing and resumes.
2. Invalid placement: if any staged cell is outside the Field, Resume changes no cell, closes Bank placement, selects Live, resumes simulation, and shows `Outside field. Nothing placed.`
3. Bank Delete names the figure in a confirmation `dialog`; Cancel preserves it, while a persistence error blocks deletion and retains the row.

### Flow 3 — Nerd leaves, manages, and restores a session

`[ASSUMPTION]` Exact restoration of a prior mathematical workspace is the climax because no more specific session goal was supplied.

1. From Field, Nerd presses `Exit`.
2. Life Game automatically saves the exact cells, fixed dimensions, camera position, zoom, and last-camera-view preview, then returns to the Start Screen.
3. Nerd sees the session's unique name and picture-only preview in its `session-card`, renames it through the dedicated action, then opens Delete and cancels the required confirmation `dialog`; the session remains.
4. Nerd selects the preserved session card. The saved field and view are restored; the paused state is not restored, and generations begin immediately using the current global generation interval.
5. **Climax — `[ASSUMPTION]`:** the exact construction and view are present and evolving again, while figures captured earlier remain available from the shared Bank.
6. Nerd closes the application window; the application exits. `[ASSUMPTION]` If a session is open, the same automatic-save attempt runs before shutdown.

Failure path: `[ASSUMPTION]` If session data is missing or damaged, Life Game opens a specific error `dialog`, does not silently create an empty replacement, and returns Nerd to the Start Screen after acknowledgment. A save failure before `Exit` blocks the transition, so the current in-memory Field is not silently abandoned; exact recovery actions remain architecture decisions.

### Source-flow and requirement coverage

| Inherited flow / requirement | Key Flow coverage |
|---|---|
| `Core Loop` / `Core Gameplay Loop`; Epics 1–3, `Field MVP`, `Editing and Observation`, `Field Navigation and Setup` | Flow 1: configure, create, edit, navigate, and observe. |
| `Highlight and Save`; `Bank and Paste`; Epic 4, `Figure Capture and Bank` | Flow 2: capture and exact reuse, including duplicate-name and invalid-placement failures. |
| `Session Lifecycle`; Epic 5, `Persistent Sessions` | Flow 3: leave, manage, restore, and close. |
| Epic 6, `Cross-Platform Completion` | All flows use the same macOS/Linux behaviors; failure and state tables cover duplicate names, invalid placement, and damaged persisted data. |

## Confirmed UX Production Constants

The following values are no longer assumptions and are gates for the owning epics:

| Area | Confirmed contract |
|---|---|
| Zoom | Discrete levels are 50%, 75%, 100%, 150%, 200%, 300%, and 400%; a new session starts at 100%. |
| Grid | Render grid lines when a cell is at least 4 logical display pixels across; hide them below that threshold. |
| Names | Trim outer whitespace, normalize Unicode to NFC, require 1–64 Unicode code points, preserve internal whitespace/display case, and compare uniqueness case-insensitively. |
| Bank deletion | Deleting a saved figure requires explicit confirmation naming the figure. |
| Session preview | Store and display a field-only 256×256 PNG of the saved camera view. |
| Camera boundary | Clamp the camera so the viewport always shows at least one in-field cell; render any visible area outside the finite Field as gray `{colors.out-of-field}` (`#808080`), not as a cell state. |

## Open Decisions

These tagged assumptions are implementation-relevant and should be confirmed or replaced before their owning feature enters production:

1. Zoom anchor.
2. Settings Save/Cancel interaction semantics.
3. Window resizing and keyboard/accessibility scope.
4. Bank/session ordering and scrolling; empty states.
5. Pointer interpolation, capture outside Field, staging anchor, and invalid-preview presentation.
6. Exact error copy and retry affordances for recoverable persistence failures.
7. Architecture-owned maxima and performance budgets for fields, figures, sessions, Bank, and persistence.

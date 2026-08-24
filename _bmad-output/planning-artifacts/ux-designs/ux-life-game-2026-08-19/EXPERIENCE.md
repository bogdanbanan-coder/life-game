---
name: "Life Game"
description: "The interaction, navigation, state, and accessibility contract for a native Life-study sandbox."
status: final
project: life-game
created: 2026-08-19
updated: 2026-08-22
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
| Start Menu | Application launch; Field `Exit` | Session browser and Settings gateway. Shows the horizontal session-card list and Create/Settings controls. | New Session dialog, Settings, selected Field, session rename/delete dialogs, OS close. |
| New Session dialog | Start Menu → Create | Enter a unique session name plus independent width and height; both dimensions default to 50. | New Field on success; Start Menu on Cancel or dismiss. |
| Settings | Start Menu → Settings | Configure the generation interval as a positive number of seconds; the default is 0.25 seconds. Other parameters were mentioned by the stakeholder but are not yet named. | Start Menu on Save or Cancel. |
| Field | Select a session card; create a session | Run and edit the finite fixed-dimension Life field. Restores cells, dimensions, camera position, and zoom for saved sessions, then begins generation updates immediately. | Figure-capture dialog, Bank list, staged-figure overlay, Start Menu via `Exit`, OS close. |
| Figure-capture dialog | Complete an inclusive Highlight drag | Name and save the exact live/dead rectangle, or cancel it. Simulation remains paused while open. | Field running in Live after Save, Cancel, or outside click. |
| Bank list | Field → Bank | Pause simulation; browse the application-wide figure list by unique name; select, rename, or delete a figure. | Staged-figure overlay on select; Field running in Live on close. |
| Confirmation/error dialog | Session delete; `[ASSUMPTION]` Bank delete and blocking persistence failure | Confirm an irreversible deletion or explain a blocking failure. | Calling surface after confirm/acknowledge/cancel. |

### Field regions, overlays, and feedback

| Element | Present when | Purpose | Ends when |
|---|---|---|---|
| Field tool pool | Field is open | Expose Live, Die, Pause/Resume, Highlight, Bank, Move, `+`, `−`, and `Exit`; make the active persistent mode and run state visible. | Field closes. |
| Staged-figure overlay | A Bank figure is selected | Move a translucent preview of the exact rectangle over the Field while simulation remains paused. | Resume commits or rejects placement and returns to running Live. |
| Status feedback | Duplicate/invalid input, invalid placement, save/rename failure, busy operation | `[ASSUMPTION]` Give non-audio, non-color-only feedback without creating another navigation level. | Remains on the current surface. |

The Start Menu is both the launch session browser and the Settings gateway. It is not a separate title presentation. A session card opens its Field directly. Pressing `Exit` in Field automatically saves the session state and last-camera-view preview, then returns to the Start Menu. Closing the application window exits the application; `[ASSUMPTION]` when a session is open, closing the window first attempts the same automatic save as `Exit` and does not add an extra quit-confirmation surface.

`[ASSUMPTION]` When no sessions exist, the Start Menu shows an empty `session-card-list` with a direct Create action. `[ASSUMPTION]` Settings uses explicit Save and Cancel actions; the generation interval is application-wide and applies to every session opened after saving. No unnamed “other parameters” are added until the stakeholder or an upstream source identifies them.

`[ASSUMPTION]` The New Session dialog presets width and height to 50 and permits positive whole numbers up to an architecture-owned maximum. Create stays unavailable while any field is invalid. Cancel does not create a session. A successful create opens the new Field immediately and simulation begins at the configured interval.

`[ASSUMPTION]` The `bank-list` and `session-card-list` sort names by a stable case-insensitive ascending order. Closing or canceling Bank does not place a figure; it returns to Live and resumes the simulation. These ordering and close rules are not source-defined.

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
| `field-grid` | Maps the pointer to exact integer cell coordinates. Live/Die paint only in-bounds cells. Highlight returns an inclusive cell rectangle. Camera transforms never alter field coordinates or cell state. |
| `tool-pool` | Shows Live, Die, Pause/Resume, Highlight, Bank, Move, `+`, `−`, and `Exit`. It exposes the active persistent mode and running/paused/staged status at all times. Modal workflows disable commands that are not part of that workflow. |
| `session-card-list` | Scrolls horizontally when cards exceed available width. `[ASSUMPTION]` Pointer wheel scrolls horizontally while hovered; a visible scrollbar also supports dragging. Empty state keeps Create available. |
| `session-card` | Clicking its main preview/name area opens the session and starts generation immediately. Dedicated Rename/Delete actions do not open the session. Delete always opens confirmation. |
| `dialog` | raygui in-application modal. Captures all pointer input. Confirmation/error dialogs require an explicit action. Figure capture is the only dialog where outside click is defined as Cancel. `[ASSUMPTION]` Outside click does not dismiss any other dialog. |
| `text-field` | Accepts and displays a candidate session or figure name. `[ASSUMPTION]` It retains invalid input and shows validation adjacent to the field. Save/Create/Rename cannot commit an invalid or duplicate name. |
| `numeric-field` | Accepts dimensions or positive seconds. `[ASSUMPTION]` It retains invalid input for correction and never silently clamps or rounds it. |
| `bank-list` | Lists the shared Bank by unique name. Selecting a row stages that figure; dedicated Rename/Delete actions do not stage it. `[ASSUMPTION]` Empty state explains that Highlight saves figures here. |
| `staged-figure-overlay` | Snaps to Field cells and follows pointer movement while paused. Resume is the only commit command. It replaces the entire valid stored rectangle, including stored dead cells; outside cells are unchanged. Invalid Resume commits nothing, exits Bank, selects Live, and resumes. |
| `status-message` | Announces success, failure, invalid placement, or busy state in text. `[ASSUMPTION]` Non-blocking feedback remains until the next relevant action or explicit dismissal; it never vanishes on a short timer. |

## State Patterns

| Surface | Required states and treatment |
|---|---|
| Start Menu | **Cold load:** load session metadata and previews. `[ASSUMPTION]` Show `Loading sessions…` in `status-message` if not ready on the first rendered frame. **Empty:** Create remains available. **Populated:** horizontal cards. `[ASSUMPTION]` **Damaged/missing index:** show a blocking `dialog`, preserve files, then display an empty or read-only browser after acknowledgment instead of silently overwriting. |
| New Session dialog | **Pristine:** name blank, width 50, height 50. **Invalid:** field-local text, Create disabled. **Duplicate:** `Name already exists.` **Create failure:** blocking error; no partial card appears. **Cancel:** no mutation. |
| Settings | **Clean:** current interval. **Dirty:** Save enabled. **Invalid:** positive-seconds message, Save disabled. **Save failure:** retain edit and show blocking error. **Cancel:** discard uncommitted edit. All Save/Cancel semantics are `[ASSUMPTION]`. |
| Field / Field tool pool | **Running:** generation updates active; selected persistent mode visible. **Paused:** no generation updates; Resume visible. **Painting:** touched in-bounds cells change to Live/Die. **Moving:** drag changes camera only. `[ASSUMPTION]` **Modal/capture/Bank:** unrelated controls are inert. `[ASSUMPTION]` **Auto-save failure:** blocking error before returning to Start Menu; recovery action remains architecture-dependent. |
| Figure-capture dialog | **Open:** selection visible, simulation paused. **Invalid/duplicate name:** selection retained, Save blocked. **Saved:** figure added, dialog closes, Live selected, simulation resumes. **Cancel/outside click:** no Bank mutation, dialog closes, Live selected, simulation resumes. Return-to-Live is `[ASSUMPTION]`. |
| Bank list | **Loading:** `[ASSUMPTION]` busy `status-message`. **Empty:** explanation plus Close. **Populated:** selectable rows. **Rename invalid/duplicate:** retain edit. **Delete:** `[ASSUMPTION]` confirm before removal. **Persistence failure:** block the mutating action and retain the list. **Close:** `[ASSUMPTION]` no placement, Live selected, simulation resumes. |
| Staged-figure overlay | **Valid:** exact rectangle visible with solid outline. **Invalid:** exact rectangle visible with dashed outline and `Outside field`. **Resume valid:** commit exact replacement, Live selected, resume. **Resume invalid:** place nothing, show `Outside field. Nothing placed.`, Live selected, resume. **Pointer outside window:** `[ASSUMPTION]` preserve the last in-window preview position. |
| Confirmation/error dialog | `[ASSUMPTION]` **Confirm:** destructive action is named. **Cancel:** no mutation. **Action failure:** dialog remains or is replaced with a specific error. **Acknowledge:** returns to the calling surface. No nested modal stack. |
| Status feedback | **Success:** exact completed action. **Validation failure:** adjacent to invalid input. **Invalid placement:** visible after return to Live. **Busy:** names the blocked action. Every state is textual and visual because the product has no audio. |

`[ASSUMPTION]` Missing or damaged persisted data is never silently treated as valid empty data. A damaged session remains visible but cannot open; selecting it shows a blocking error and returns to Start Menu. A damaged Bank file disables Bank mutations until the error is acknowledged; recovery and repair remain architecture decisions.

## Interaction Primitives

### Pointer and Field

- Use Live or Die by pressing or dragging over Field cells. Every touched in-bounds cell is set directly to the active state; input outside the Field changes no cell.
- `[ASSUMPTION]` Painting interpolates between pointer samples in cell coordinates so a fast drag does not leave unintended gaps. Leaving the Field while held stops cell changes; re-entering while still held continues the same stroke.
- `[ASSUMPTION]` Pointer capture lasts through a Move or Highlight drag until release, including when the pointer briefly leaves the Field. Only in-bounds endpoints contribute to Highlight; releasing without a valid second in-bounds cell cancels without opening capture.
- The frame order is deterministic: scheduled simulation steps run first, accepted Live/Die input commands run second, and rendering runs last. A running edit mutates the current field during the input phase and is visible in that frame; it is not promised to affect a generation already processed earlier in that frame. No pause or special input buffer is required, and the exact timer-boundary-to-frame assignment remains an implementation detail.
- Move changes only camera position. `[ASSUMPTION]` Live, Die, and Move are persistent modes; Highlight and Bank launch modal workflows; Pause/Resume, zoom, and Exit are commands. Highlight ends in Live after Save/Cancel.

### Zoom and camera

- `+` and `−` change exactly one discrete zoom level per press.
- `[ASSUMPTION]` Levels are 50%, 75%, 100%, 150%, 200%, 300%, and 400%; 100% is the new-session default.
- `[ASSUMPTION]` Zoom anchors on the Field cell under the pointer; if the pointer is outside the Field, zoom anchors on the viewport center.
- `[ASSUMPTION]` Camera movement is clamped so the finite Field cannot be lost entirely off-screen. Exact centering when the Field is smaller than the viewport is implementation-owned.
- `[ASSUMPTION]` Grid lines display when a cell is at least 4 screen pixels across, using the `DESIGN.md` `field-grid` rule.

### Capture and Bank placement

- Highlight press, drag, and release selects the inclusive rectangle between the two cells. Completing it pauses generation and opens the figure-capture dialog.
- Save stores the full live/dead rectangle. Selecting Cancel or clicking outside saves nothing. Either outcome resumes generation.
- Bank opening pauses generation. Selecting a figure closes Bank and stages its translucent rectangle over Field.
- `[ASSUMPTION]` The stored rectangle's top-left cell anchors to the cell under the pointer. The preview snaps one whole cell at a time.
- Resume commits only when every staged cell is within the finite Field. A valid commit replaces every underlying live and dead cell in the rectangle and leaves outside cells unchanged. An invalid commit changes nothing, exits Bank, returns to Live, and resumes.

### Destructive and modal actions

- Session Delete requires confirmation and names the session in the message.
- `[ASSUMPTION]` Bank-figure Delete also requires confirmation because there is no undo/redo and Bank is the only accumulating in-product record of saved constructions.
- `[ASSUMPTION]` Rename uses the same `dialog`, normalization, and uniqueness rules as creation/capture and makes no change until Save.
- Figure-capture outside click is equivalent to Cancel. Other outside clicks do not dismiss dialogs.

## HUD & Non-diegetic UI

All Life Game UI is non-diegetic application UI; there is no fictional world or in-world display. The Field is primary. The compact `tool-pool` occupies the upper-right region, above the Field render layer but below any modal `dialog`.

Information hierarchy:

1. Cell state and exact grid position.
2. Active persistent mode and simulation state: Running, Paused, or Staging.
3. Available commands in the Field tool pool.
4. Transient `status-message` feedback.

The active mode never fades. `[ASSUMPTION]` The remaining hierarchy is: cell state first, active mode/run state second, available commands third, and feedback fourth. Running/Paused/Staging appears as a short text label in the `tool-pool`; the Pause/Resume label alone is not treated as sufficient status. The Field includes no generation counter, coordinates, zoom value, field dimensions, or interval readout because the confirmed sources require none.

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
- `[ASSUMPTION]` Body UI text begins at 16px and compact labels at 14px; UI follows operating-system DPI scaling on macOS and Linux without bitmap blurring.
- `[ASSUMPTION]` Pointer targets are at least 32×32px. This is the proposed compromise for the explicitly compact upper-right controls and remains open for usability testing.
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

Window resizing is optional and non-blocking. `[ASSUMPTION]` The first product release may use one fixed logical window size; that size and minimum supported display resolution remain unresolved. If resizing is implemented, the Field viewport absorbs added space while the upper-right `tool-pool`, dialogs, and Start Menu controls retain token sizing. No mobile, web, Windows, console, handheld, TV, or VR adaptation is planned.

`[ASSUMPTION]` OS DPI scale affects the full UI consistently, including pointer hit mapping and Field grid thickness. High-DPI correctness must be verified on both target platforms before Cross-Platform Completion.

## Naming & Validation

Unique names are required separately across sessions and across the application-wide Bank. The following details are fast-path proposals, not upstream decisions:

- `[ASSUMPTION]` Trim leading and trailing whitespace; a result with zero characters is invalid.
- `[ASSUMPTION]` Accept Unicode, normalize to NFC, and allow 1–64 Unicode code points.
- `[ASSUMPTION]` Uniqueness comparison is case-insensitive after trimming and normalization, so `Blinker` and `blinker` conflict.
- `[ASSUMPTION]` Keep the user's internal whitespace and displayed case after validation.
- `[ASSUMPTION]` Validation runs during editing, but validation feedback appears only after the field has been touched or a commit is attempted. Duplicate feedback is `Name already exists.`
- `[ASSUMPTION]` Width and height accept positive base-10 whole numbers and each default to 50; architecture supplies the maximum. Values above that maximum are rejected, not clamped.
- `[ASSUMPTION]` The generation interval field accepts a positive base-10 number of seconds with `.` as the decimal separator; the default is 0.25. Zero, negative, nonnumeric, NaN, and infinity are rejected.
- `[ASSUMPTION]` The UI is English-only for this personal release, but Unicode names persist round-trip on both platforms.

## Performance-facing UX

Correctness wins over apparent speed. A generation is synchronous and completes before the next scheduled update; rendering frequency never changes Life results. The default 50×50 field must finish each generation before the next 0.25-second update.

- Field input and camera feedback should remain visibly responsive while simulation runs; no FPS value is promised.
- `[ASSUMPTION]` When a configured field cannot sustain the requested interval, the application never skips, merges, or partially displays a generation. It completes the current generation, delays the next, and shows `Running slower than interval.` in `status-message` until cadence recovers.
- `[ASSUMPTION]` Session, Bank, or preview operations that cannot complete interactively show a named busy `status-message` and block only the mutating control, not the entire window, unless data consistency requires a modal block.
- Successful persistence is defined by exact restoration, not by displaying a progress animation. Failed saves/loads never silently discard, replace, or reinterpret data.
- Maximum field dimensions, session count, Bank size, figure dimensions, preview format/size, and acceptable operation latency are architecture-owned limits and remain phase-relevant gaps.

## Key Flows

### Flow 1 — Nerd configures and observes a first experiment

Nerd is a non-story task persona interested only in mathematics. `[ASSUMPTION]` The concrete experiment is a three-cell blinker because no individual construction was supplied.

1. Nerd launches Life Game. The Start Menu loads the horizontal `session-card-list`; if no sessions exist, Create remains available.
2. Nerd opens Settings, changes or verifies the positive generation interval, and saves. Invalid input remains in the `numeric-field` with a direct error; nothing is silently coerced.
3. Nerd selects Create. The New Session dialog opens with width 50 and height 50. Nerd enters a unique name and creates the session.
4. The Field opens and begins generations immediately. Live is visibly selected; the running state is explicit.
5. Nerd presses Pause. On the lined Field, Nerd uses Live to paint three cells and Die to correct one accidental cell, confirming that only touched in-bounds cells change. Nerd selects Move to inspect another region without editing, uses `+`/`−` by discrete levels, and observes that the finite boundary stays dead.
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
3. `[ASSUMPTION]` Bank Delete names the figure in a confirmation `dialog`; Cancel preserves it, while a persistence error blocks deletion and retains the row.

### Flow 3 — Nerd leaves, manages, and restores a session

`[ASSUMPTION]` Exact restoration of a prior mathematical workspace is the climax because no more specific session goal was supplied.

1. From Field, Nerd presses `Exit`.
2. Life Game automatically saves the exact cells, fixed dimensions, camera position, zoom, and last-camera-view preview, then returns to the Start Menu.
3. Nerd sees the session's unique name and preview in its `session-card`, renames it through the dedicated action, then opens Delete and cancels the required confirmation `dialog`; the session remains.
4. Nerd selects the preserved session card. The saved field and view are restored; the paused state is not restored, and generations begin immediately.
5. **Climax — `[ASSUMPTION]`:** the exact construction and view are present and evolving again, while figures captured earlier remain available from the shared Bank.
6. Nerd closes the application window; the application exits. `[ASSUMPTION]` If a session is open, the same automatic-save attempt runs before shutdown.

Failure path: `[ASSUMPTION]` If session data is missing or damaged, Life Game opens a specific error `dialog`, does not silently create an empty replacement, and returns Nerd to the Start Menu after acknowledgment. A save failure before `Exit` blocks the transition, so the current in-memory Field is not silently abandoned; exact recovery actions remain architecture decisions.

### Source-flow and requirement coverage

| Inherited flow / requirement | Key Flow coverage |
|---|---|
| `Core Loop` / `Core Gameplay Loop`; Epics 1–3, `Field MVP`, `Editing and Observation`, `Field Navigation and Setup` | Flow 1: configure, create, edit, navigate, and observe. |
| `Highlight and Save`; `Bank and Paste`; Epic 4, `Figure Capture and Bank` | Flow 2: capture and exact reuse, including duplicate-name and invalid-placement failures. |
| `Session Lifecycle`; Epic 5, `Persistent Sessions` | Flow 3: leave, manage, restore, and close. |
| Epic 6, `Cross-Platform Completion` | All flows use the same macOS/Linux behaviors; failure and state tables cover duplicate names, invalid placement, and damaged persisted data. |

## Open Decisions

These tagged assumptions are implementation-relevant and should be confirmed or replaced before their owning feature enters production:

1. Zoom levels, anchor, camera clamp, and grid-visibility threshold.
2. Name trimming, Unicode normalization, length, case sensitivity, and validation timing.
3. Bank-figure deletion confirmation.
4. Settings Save/Cancel semantics and whether generation interval is global or per session.
5. Window size/resizing, DPI scaling, pointer-target floor, and keyboard/accessibility scope.
6. Bank/session ordering and scrolling; Bank close behavior; empty states.
7. Pointer interpolation, capture outside Field, staging anchor, and invalid-preview presentation.
8. Persistence-error recovery and busy/slow-simulation feedback.
9. Other parameters mentioned for Settings but not yet named.
10. Architecture-owned maxima and performance budgets for fields, figures, sessions, Bank, persistence, and previews.

---
title: "Life Game UX Source Extraction — Planning Inputs"
status: working
created: 2026-08-19
updated: 2026-08-19
sources:
  - ../../briefs/brief-life-game-2026-08-15/brief.md
  - ../../briefs/brief-life-game-2026-08-15/addendum.md
  - ../../gdds/gdd-life-game-2026-08-19/gdd.md
  - ../../gdds/gdd-life-game-2026-08-19/epics.md
  - ../../gdds/gdd-life-game-2026-08-19/decision-log.md
---

# Life Game UX Source Extraction — Planning Inputs

## Reading Rules

- **Explicit fact** means the source states or formally resolves the point.
- **Inference** means a likely UX consequence derived from explicit facts. It is not yet a product decision.
- **Open gap** means the source intentionally defers the decision or does not provide enough detail to specify behavior.
- Citations use `filename.md` → “Heading”. All citations refer to the two confirmed source sets only.

## Product and Experience Foundation

### Explicit Facts

- Life Game is a private, single-user Conway's Game of Life study sandbox. Its promise is to let the player seed a finite field directly, watch deterministic patterns, preserve exact rectangular constructions, and reuse them in later experiments. (`brief.md` → “Core Concept”; `gdd.md` → “Executive Summary → Core Concept”)
- It is a solo personal-study and learning project, not a commercial product. There is no fixed commercial deadline; originality and broad-market appeal are not success criteria. (`brief.md` → “Purpose, Audience, and Boundaries”; `brief.md` → “Scope and MVP → Technical Boundary”; `gdd.md` → “Goals and Context”)
- The intended audience is the creator, described as a technically curious “nerd” willing to manipulate and observe Life cells without external motivation. (`brief.md` → “Purpose, Audience, and Boundaries”; `gdd.md` → “Executive Summary → Core Concept”)
- The application supplies no goals, challenges, score, reward, progression, achievements, win/loss state, or prescribed completion. A session may continue until the player leaves or closes the application. (`brief.md` → “Intended Experience”; `gdd.md` → “Core Gameplay → Win and Loss Conditions”; `gdd.md` → “Progression and Balance”)
- The four canonical pillars are **Direct Manipulation**, **Deterministic Evolution**, **Preservation**, and **Composition**. (`brief.md` → “Design Pillars”; `gdd.md` → “Core Gameplay → Game Pillars”)
- Field is both the MVP and first implementation milestone. MVP deliberately excludes Bank, saved sessions, configuration, configurable timing, navigation, resizing, and storage. (`brief.md` → “Scope and MVP”; `gdd.md` → “Development Epics”; `epics.md` → “Epic 1 — Field MVP”)
- The full target product adds editing/observation, field navigation/setup, figure capture and Bank, persistent sessions, and cross-platform completion in that order. (`gdd.md` → “Development Epics”; `epics.md` → “Sequence and Traceability”)

### Inferences to Validate

- The UX should optimize for uninterrupted experimentation and legibility rather than onboarding theatrics, reward presentation, or retention mechanics. This follows from the expert-curious audience and explicit absence of goals/rewards, but the desired onboarding depth is not stated.
- Bank contents function as the only accumulating in-product record of the player's discoveries, although the source explicitly says the only “progression” is knowledge plus the Bank. This makes preservation errors disproportionately costly. (`gdd.md` → “Progression and Balance → Player Progression”)

## Stakes, Audience, Platform, Engine, and Input

### Explicit Facts

- Stakes: hobby/personal-study/learning project, single developer and single user, no commercial deadline. (`brief.md` → “Purpose, Audience, and Boundaries”; `gdd.md` → “Goals and Context”)
- Platforms: macOS and Linux desktop have equal design priority. Mobile, web/PWA, Windows, and console are out of scope. (`gdd.md` → “Target Platforms”; `gdd.md` → “Out of Scope → Explicitly Excluded”)
- Form factor: a native desktop window. Window resizing is optional and non-blocking; it may remain unsupported if costly. (`addendum.md` → “Interaction Detail → Board Boundary and View”; `gdd.md` → “Target Platforms”)
- Primary input: mouse. Field interactions are expressed as clicks, presses, pointer movement, and drags. (`gdd.md` → “Target Platforms”; `gdd.md` → “Game Mechanics → Tools and Input”)
- Technology/UI system: C++23, CMake, raylib 6.0, and raygui 5.0 with exact versions pinned. raylib/raygui sit at the application/presentation boundary; simulation and persistence remain framework-independent. (`brief.md` → “Scope and MVP → Technical Boundary”; `addendum.md` → “C++ Framework Evaluation → Selected: raylib + raygui”; `gdd.md` → “Technical Specifications → Technology and Platforms”)
- raygui provides immediate-mode buttons, toggles, text boxes, lists, scroll panels, message boxes, and text-input dialogs. Layout is manual and dialogs appear inside the application rather than as native OS windows. (`addendum.md` → “C++ Framework Evaluation → Selected: raylib + raygui”)
- No keyboard, keyboard-shortcut, trackpad-specific, controller, touch, or assistive-input behavior is specified.

### Inferences to Validate

- “Mouse-driven” likely includes ordinary trackpad pointer emulation on laptops, but gestures and trackpad-specific affordances are not confirmed.
- Since window resizing may be unsupported, one stable desktop layout could cover the product, but minimum window size, scaling behavior, and high-DPI handling still need decisions.
- Since the UI is immediate-mode and manually laid out, focus order, keyboard text entry, modal focus capture, and accessible semantics will require deliberate behavior rather than arriving automatically.

## Player Journeys and Loops

### Exact Named-Journey Evidence

- **No named-protagonist player journey exists in the confirmed sources.** There is no journey name or persona such as “Rosa” to mirror verbatim.
- The sources provide generic flows under the exact headings **“Core Loop”** (`brief.md`), **“Core Gameplay Loop”** (`gdd.md`), **“Highlight and Save”** (`addendum.md`), **“Bank and Paste”** (`addendum.md`), **“Session Lifecycle”** (`addendum.md` and `gdd.md`), and epic-level “As the player” stories (`epics.md`). These are source flows, not named-protagonist journeys.

### Explicit Generic Core Flow

1. Choose or create a named session; restore its field and last camera view; simulation begins immediately.
2. Draw live/dead cells, pause/resume, and observe generations.
3. Highlight an inclusive rectangle; name and save it, or cancel.
4. Open Bank; choose and stage a saved figure; press Resume to commit its full rectangle and resume evolution.
5. Leave the session; automatically save its state and last-view preview; reuse Bank figures in any session.

(`brief.md` → “Core Loop”; `gdd.md` → “Core Gameplay → Core Gameplay Loop”)

### Journey Details That Must Be Preserved

- Reopening a session restores cells, dimensions, camera position, and zoom, but never restores paused state; generations begin immediately. (`addendum.md` → “Interaction Detail → Session Lifecycle”; `gdd.md` → “Game Mechanics → Session Lifecycle”)
- Highlight begins with press on one cell, drag to another, and release; the selected rectangle is inclusive. Capture pauses the simulation. Save or Cancel ends capture and resumes simulation. Clicking outside the capture dialog is equivalent to Cancel. (`addendum.md` → “Interaction Detail → Highlight and Save”; `gdd.md` → “Game Mechanics → Figure Capture”)
- Opening Bank pauses simulation. Selecting a figure closes the list and stages a translucent preview. Pressing Resume is the commit action, not a separate Paste action. (`addendum.md` → “Interaction Detail → Bank and Paste”; `gdd.md` → “Game Mechanics → Bank Placement”)
- A committed figure is exact rectangular replacement: both stored live and stored dead cells replace underlying cells; outside cells remain unchanged. (`brief.md` → “Core Loop”; `addendum.md` → “Interaction Detail → Bank and Paste”; `gdd.md` → “Game Mechanics → Bank Placement”)
- An invalid Bank placement—any part outside the finite field—commits nothing, exits Bank, returns to Live, and resumes simulation. (`gdd.md` → “Game Mechanics → Bank Placement”; `epics.md` → “Epic 4 — Figure Capture and Bank”)

## Information Architecture and Surfaces

### Explicit Surfaces

| Surface | Explicit content and behavior | Source |
|---|---|---|
| Launch screen / session browser | Horizontally scrolling session-card list. Each card shows a unique session name, last-camera-view preview, Rename, and Delete. Create starts new-session naming. | `addendum.md` → “Interaction Detail → Session Lifecycle”; `gdd.md` → “Game Mechanics → Session Lifecycle”; `epics.md` → “Epic 5 — Persistent Sessions” |
| New-session dialog | Name entry; the product supports independent width and height setup; duplicate session names are rejected. | `addendum.md` → “Interaction Detail → Board Boundary and View” and “Session Lifecycle”; `gdd.md` → “Game Mechanics → Field and Camera” and “Session Lifecycle” |
| Session-delete confirmation | Deletion requires confirmation. | `addendum.md` → “Interaction Detail → Session Lifecycle”; `gdd.md` → “Game Mechanics → Session Lifecycle” |
| Field | Finite, fixed-dimension square or rectangle; dark field, visible cells, grid, and editable cell coordinates. Default is 50×50. | `gdd.md` → “Game Mechanics → Field and Camera”; `gdd.md` → “Art and Audio Direction → Art Style” |
| Field controls / tool pool | Compact, flat controls in the upper-right region. Live, Die, Pause/Resume, Highlight, Bank, Move, `+`, and `−` are required behaviors/controls. Active persistent tool must be visible. | `brief.md` → “Scope and MVP”; `gdd.md` → “Game Mechanics → Tools and Input”; `gdd.md` → “Art and Audio Direction → Art Style”; `epics.md` → “Epic 2 — Editing and Observation” |
| Figure-capture dialog | Figure-name field, Save, Cancel; click outside cancels. Duplicate names are rejected. | `addendum.md` → “Interaction Detail → Highlight and Save”; `gdd.md` → “Game Mechanics → Figure Capture” |
| Bank list | Application-wide figure list by unique name, with dedicated Rename and Delete controls. Selecting a figure returns to Field with a staged preview. | `gdd.md` → “Game Mechanics → Bank Placement”; `epics.md` → “Epic 4 — Figure Capture and Bank” |
| Staged figure overlay | Translucent rectangle follows pointer over Field while simulation is paused; underlying cells remain visible; Resume commits if fully in bounds. | `addendum.md` → “Interaction Detail → Bank and Paste”; `gdd.md` → “Game Mechanics → Bank Placement”; `gdd.md` → “Art and Audio Direction → Art Style” |
| Generation timing settings | Post-MVP behavior permits a positive interval in seconds, default 0.25 seconds. No containing screen or control form is specified. | `gdd.md` → “Game Mechanics → Cellular-Automaton Rules”; `epics.md` → “Epic 2 — Editing and Observation” |
| Last-view preview | A stored image of the last camera view appears on the corresponding session card. | `addendum.md` → “Interaction Detail → Session Lifecycle”; `gdd.md` → “Game Mechanics → Session Lifecycle” |

### Explicitly Absent or Excluded Surfaces

- No authored level selection, challenge list, score screen, achievement screen, progression surface, account/community surface, sharing/gallery/workshop UI, multiplayer lobby, narrative UI, inventory, map, import/export, scripting, undo/redo, rotation, or scaling UI is in scope. (`gdd.md` → “Sandbox-Specific Design”; `gdd.md` → “Progression and Balance”; `gdd.md` → “Out of Scope”)
- No title-screen presentation, pause menu, options-menu structure, help/tutorial, about screen, quit flow, or explicit main-navigation model is specified.

### IA Closure Gaps

- How the player leaves Field and returns to the session browser is not defined.
- The app's first-run/zero-session state is not defined.
- The new-session form's exact relationship among name, width, height, defaults, validation, cancel, and creation is not defined.
- The location and access path for generation-interval settings are not defined.
- Bank browsing behavior beyond “lists by name” is open: layout, selection affordance, empty state, scrolling, ordering, and whether search is needed.
- Rename flows for sessions and Bank figures are required, but dialog behavior, cancellation, validation presentation, and collision recovery are not defined.
- Delete behavior is resolved only for sessions. Whether deleting a Bank figure requires confirmation is explicitly deferred. (`gdd.md` → “Deferred Design Notes”; `decision-log.md` → “Discipline validation”)
- Missing/damaged persisted data must have clear failure behavior, but no recovery surface or message behavior is specified. (`epics.md` → “Epic 6 — Cross-Platform Completion”)

## HUD, Diegesis, and Information Hierarchy

### Explicit Facts

- There is no fictional world, character, or narrative presentation; the Field and controls are application UI, not in-world/diegetic UI. (`brief.md` → “Visual and Audio Direction”; `gdd.md` → “Art and Audio Direction”)
- The Field requires small flat controls in its upper-right region. (`brief.md` → “Scope and MVP”; `gdd.md` → “Art and Audio Direction → Art Style”)
- One tool is always selected; Live is the default. The currently selected persistent tool must be visible at all times. (`addendum.md` → “Interaction Detail → Tool Pool”; `gdd.md` → “Game Mechanics → Tools and Input”; `epics.md` → “Epic 2 — Editing and Observation”)
- Pause/Resume and Bank are transient commands. After they end, Live is selected. Highlight also ends by returning to resumed simulation after Save or Cancel. (`addendum.md` → “Interaction Detail → Tool Pool” and “Highlight and Save”; `gdd.md` → “Game Mechanics → Tools and Input” and “Figure Capture”)
- The user must distinguish live cells, dead cells, selection, staged figures, and active tool at a glance. (`epics.md` → “Epic 6 — Cross-Platform Completion”)

### Open HUD/Status Gaps

- No generation number/counter is required or excluded.
- No explicit paused/running status indicator is specified beyond Pause/Resume control behavior.
- No field dimensions, camera coordinates, zoom level, generation interval, figure dimensions, or placement-validity readout is specified.
- It is unclear whether Move is a persistent selected tool, a one-shot command, or a temporary mode that returns to Live; the source only says it “activates camera movement.”
- The phrase “one tool is always selected” coexists with Pause/Resume, Bank, and Zoom commands. UX terminology should distinguish persistent modes from transient commands so the active-state model is unambiguous.
- No rule states which controls remain available, disabled, hidden, or transformed during capture dialogs, Bank browsing, staged placement, or persistence failure.

## Interaction Primitives and State Behavior

### Explicit Facts

- Live and Die use press-or-drag across cells; every touched cell is set directly to the selected state. Interaction outside the field changes no cell. (`gdd.md` → “Game Mechanics → Tools and Input”)
- Simulation generations are synchronous and independent of rendering frequency. Default interval is 0.25 seconds; later settings accept a positive interval in seconds. (`gdd.md` → “Game Mechanics → Cellular-Automaton Rules”)
- Move uses a dedicated control followed by field drag; it changes camera only. Zoom uses dedicated `+`/`−` controls and changes one level per press. (`gdd.md` → “Game Mechanics → Field and Camera”; `epics.md` → “Epic 3 — Field Navigation and Setup”)
- Highlight selection is a pointer drag between cell endpoints, producing an inclusive rectangular selection. (`gdd.md` → “Game Mechanics → Tools and Input”)
- Dialog dismissal by outside click is explicitly defined for figure capture only. (`addendum.md` → “Interaction Detail → Highlight and Save”)
- Duplicate session names and duplicate Bank figure names are rejected. Both object types have Rename and Delete controls. (`gdd.md` → “Game Mechanics → Figure Capture”, “Bank Placement”, and “Session Lifecycle”)

### Open Interaction Gaps

- Exact hit targets, control labels/icons, hover/pressed/focus/disabled states, cursor states, drag thresholds, pointer capture, and behavior when the pointer leaves the field/window mid-drag are unspecified.
- Painting interpolation is unspecified: the sources require all “touched” cells to change, but do not define how fast pointer motion avoids gaps.
- It is unclear what happens when editing input and a generation update occur at the same instant.
- Pause/Resume behavior while Bank placement is staged is overloaded as a commit action; the displayed label, enabled state, and invalid-placement feedback are not defined.
- Preview anchoring is unspecified: which saved-rectangle cell aligns to the pointer, whether the preview snaps by cell, and how it behaves near edges.
- Zoom increment, min/max levels, zoom anchor, visible-grid threshold, camera bounds, and panning at field edges are explicitly deferred or absent. (`gdd.md` → “Assumptions and Dependencies”; `gdd.md` → “Deferred Design Notes”)
- Blank names, allowed characters, maximum length, whitespace normalization, case sensitivity, and inline validation behavior are explicitly deferred. (`gdd.md` → “Assumptions and Dependencies”; `gdd.md` → “Deferred Design Notes”)
- No keyboard equivalent, shortcut system, focus model, tab order, Enter/Escape behavior, or clipboard behavior is specified.
- No undo/redo is in scope, so destructive edits and exact replacement have no specified recovery path. (`gdd.md` → “Sandbox-Specific Design → Creation Tools”)

## Game Feel, Feedback, and Motion

### Explicit Facts

- The game has no music, ambience, sound effects, or audio feedback. (`brief.md` → “Visual and Audio Direction”; `gdd.md` → “Art and Audio Direction → Audio and Music”)
- Simulation updates occur on a fixed cadence and must remain independent of render cadence. (`brief.md` → “Design Pillars”; `gdd.md` → “Game Mechanics → Cellular-Automaton Rules”)
- No decorative animation is required. (`gdd.md` → “Art and Audio Direction → Art Style”)
- Staged Bank figures are translucent; selection has a distinct accent; live/dead/selection/staging/tool state must remain immediately distinguishable. (`gdd.md` → “Art and Audio Direction → Art Style”; `epics.md` → “Epic 6 — Cross-Platform Completion”)

### Inferences to Validate

- Because there is no audio, all state changes, errors, and confirmations need sufficient visual feedback. The precise feedback pattern is not specified.
- Screen shake, hit-stop, haptics, and spectacle-oriented “juice” do not fit any stated requirement and have no platform/input support in scope. Their exclusion is an inference, not a recorded decision.
- Transition motion should not obscure exact cell-state inspection, but motion duration and reduced-motion behavior remain open.

### Open Feedback Gaps

- No visual response timing is defined for cell paint, generation step, tool switch, save success, auto-save, rename, delete, duplicate-name rejection, invalid placement, or damaged persistence.
- No distinction is defined between destructive confirmation, recoverable inline error, toast/status feedback, and blocking modal feedback.
- There is no specified visualization of the next generation, recently changed cells, or simulation timing; none should be assumed.

## Accessibility, Internationalization, and Content Density

### Explicit Accessibility-Adjacent Facts

- A readable interface font is required. (`gdd.md` → “Technical Specifications → Asset Requirements”)
- Live cells, dead cells, selection, staged figures, and the active tool must be distinguishable at a glance. (`epics.md` → “Epic 6 — Cross-Platform Completion”)
- Grid lines should be visible at useful/editing zoom levels without excessive visual noise when zoomed out. (`brief.md` → “Visual and Audio Direction”; `epics.md` → “Epic 3 — Field Navigation and Setup”)
- The UI is mouse-driven and supplies no audio feedback. (`gdd.md` → “Target Platforms”; `gdd.md` → “Art and Audio Direction → Audio and Music”)

### Open Accessibility Gaps

- No contrast ratios, non-color state cues, color-vision strategy, UI scaling, text-size floor, target-size floor, reduced-motion option, keyboard-only path, focus indicator, screen-reader semantics, or assistive technology support is specified.
- Distinguishability is required, but it is not stated whether color alone may encode live/dead/selection/staged/valid/invalid states.
- Dense 50×50 content and small upper-right controls create a potential legibility/target-size tension, especially on high-DPI or smaller laptop displays. This is a risk inference, not a source decision.
- A fully silent UI makes visible confirmation critical, but there is no alternate sensory channel requirement.

### Internationalization Facts and Gaps

- No localization languages, locale behavior, string expansion, font glyph coverage, text direction, number formatting, or decimal separator behavior is specified.
- Figure and session names are user-entered, but Unicode support and normalization are unspecified.
- The GDD describes intervals “in seconds”; locale-aware decimal entry and formatting are unspecified.
- English control labels appear in the design sources, but the shipping language is not formally declared.

## Visual Identity and Brand

### Explicit Facts

- The intended style is a restrained mathematical-tool aesthetic. (`brief.md` → “Visual and Audio Direction”)
- Surfaces use a dark field/background, bright live cells, subdued dead cells, faint grid lines shown only at useful zooms, a distinct selection accent, and translucent staged figures that preserve underlying-cell visibility. (`brief.md` → “Visual and Audio Direction”; `gdd.md` → “Art and Audio Direction → Art Style”)
- Controls are simple, flat, functional, small, and located in the upper-right region of Field. (`brief.md` → “Scope and MVP” and “Visual and Audio Direction”; `gdd.md` → “Art and Audio Direction → Art Style”)
- Required assets are limited to a readable interface font, simple icons or labels, grid rendering, selection styling, transparency, and session previews. (`gdd.md` → “Technical Specifications → Asset Requirements”)
- There is no world art, character art, narrative presentation, decorative animation requirement, music, or sound. (`brief.md` → “Visual and Audio Direction”; `gdd.md` → “Art and Audio Direction”)

### Open Visual-System Gaps

- No color values, contrast levels, typography family/scale, spacing scale, corner radii, borders, elevation, opacity values, icon style, or component-state tokens are specified.
- “Small” controls and “readable” font are qualitative only; dimensions and scaling behavior remain unresolved.
- The relative hierarchy among field, tool pool, simulation status, dialogs, and Bank is not visually specified.
- Session-card proportions, preview aspect ratio, horizontal-scroll mechanics, dialog size, Bank item density, and empty/error states are open.
- It is unclear how live and dead pixels inside a translucent staged rectangle remain distinguishable from the underlying live/dead cells while also communicating placement validity.

## Inspirations and Anti-Patterns

### Explicit Inspirations

- **Golly:** take deterministic simulation, dependable editing, and respect for expert workflows. (`brief.md` → “References”; `gdd.md` → “Goals and Context → Design References”)
- **Conway Canvas:** take readable selection, preview, and reusable-pattern workflows. (`brief.md` → “References”; `gdd.md` → “Goals and Context → Design References”)
- The addendum also records Alan Dewar's Life simulator as precedent for named stored pattern buffers, Slice of Life for large-pattern navigation/timeline/preview ideas, and LifeWiki/Catagolue for interchange, identity, provenance, classification, and reproducibility expectations. These are research observations, not confirmed UX requirements. (`addendum.md` → “Discovery Research Snapshot”)

### Explicit Anti-Patterns / Take-Leave Boundaries

- Do not inherit Golly's multi-rule breadth, scripting, layers, or infinite-universe scope. (`brief.md` → “References”; `gdd.md` → “Goals and Context → Design References”)
- Do not inherit Conway Canvas's browser/PWA positioning or broader feature scope. (`brief.md` → “References”; `gdd.md` → “Goals and Context → Design References”)
- Avoid feature drift into multiplayer, accounts, community, authored challenges, progression, achievements, narrative, audio, infinite/wrapping fields, multiple rules, scripting, layers, physics, import/export, undo/redo, rotation, or scaling. (`gdd.md` → “Out of Scope”)
- Advanced semantic/versioned pattern workbench ideas are uncommitted research, not a mandate for the current Bank UX. (`addendum.md` → “Discovery Research Snapshot”; `gdd.md` → “Out of Scope → Parked Research”)

## Performance and Correctness Constraints

### Explicit Facts

- The default field is 50×50. Each synchronous generation must complete before the next scheduled 0.25-second update. No FPS target is defined. (`gdd.md` → “Game Mechanics → Field and Camera”; `gdd.md` → “Technical Specifications → Performance and Correctness Targets”)
- Rendering cadence must not affect simulation timing or results. (`brief.md` → “Design Pillars”; `gdd.md` → “Game Mechanics → Cellular-Automaton Rules”)
- Canonical correctness cases are lone-cell death after one generation, stable 2×2 block, period-two three-cell blinker, and permanent-dead out-of-bounds behavior. (`gdd.md` → “Technical Specifications → Performance and Correctness Targets”)
- Fields may be rectangular and larger than the viewport, but maximum configurable dimensions are deferred to architecture. (`gdd.md` → “Game Mechanics → Field and Camera”; `gdd.md` → “Assumptions and Dependencies”)
- Session persistence must restore exact cells, dimensions, camera position, and zoom. Bank persistence and placement must preserve exact live/dead rectangles. (`gdd.md` → “Success Metrics → Product Success”)
- The Life model should use current/next buffers and whole-board/texture rendering instead of one UI object per cell. These are explicit architecture inputs rather than UX requirements. (`addendum.md` → “C++ Framework Evaluation”; `decision-log.md` → “Source reconciliation”)

### Open Performance UX Gaps

- No responsiveness budget is set for input latency, camera motion, Bank opening, session loading/saving, preview generation, or large-field simulation.
- No maximum field dimensions, Bank size, session count, figure dimensions, preview file size, or persistence scale is defined.
- No loading, busy, save-progress, dropped-update, overload, or graceful-degradation behavior is defined.
- High-DPI rendering and cross-platform font/scale consistency are not addressed.

## Source Tensions and Resolutions

### Resolved Evolution, Not Contradictions

- The brief defers default field dimensions; the finalized GDD resolves the default to **50×50** under explicit user delegation. (`brief.md` → “Interaction Detail → Board Boundary and View”; `decision-log.md` → “Express gap resolution” and “Source reconciliation”)
- The brief initially leaves session/figure naming, renaming, and duplicate behavior open; the GDD resolves unique names plus Rename/Delete for both. Detailed validation rules remain deferred. (`brief.md` → “Risks and Open Questions”; `decision-log.md` → “Express gap resolution”; `gdd.md` → “Game Mechanics”)
- The addendum describes five tools, while the GDD adds Move and Zoom controls. This is an authorized later expansion, not a contradiction. (`addendum.md` → “Interaction Detail → Tool Pool”; `decision-log.md` → “Express gap resolution”; `gdd.md` → “Game Mechanics → Tools and Input”)
- “Application starts with Field” applies to the Field-only MVP, while the session browser is the post-MVP/product launch surface. The release boundaries reconcile the apparent difference. (`epics.md` → “Epic 1 — Field MVP”; `epics.md` → “Epic 5 — Persistent Sessions”)

### UX Tensions Still Requiring Decisions

- **Tool vs. command semantics:** “one tool is always selected,” but Pause/Resume, Bank, and Zoom behave as commands and Highlight initiates a modal workflow. The persistent-mode model needs explicit UX language and state rules.
- **Automatic evolution vs. safe re-entry:** sessions resume immediately when opened. The UX must preserve that source rule while making the running state unmistakable and preventing surprise edits or lost observation time.
- **Small controls vs. accessibility:** compact upper-right controls are required, but no target-size or scale floor exists.
- **Exact replacement vs. limited recovery:** Bank placement overwrites live and dead cells exactly, while undo/redo is excluded. Preview validity, commit intent, and error prevention therefore carry high UX importance.
- **Invalid placement behavior vs. feedback:** invalid Resume intentionally exits Bank and resumes without placing anything. The required outcome is explicit, but how the user understands why nothing happened is unspecified.
- **Dialog outside-click cancel vs. destructive effort:** figure capture cancels when clicking outside its dialog. Whether entered names/selection are lost without warning is implicit but not discussed.

## Consolidated Open UX Decisions

### Phase-Relevant Decisions Already Assigned to UX

1. Zoom increment, minimum/maximum zoom, anchor behavior, and grid-visibility thresholds.
2. Blank-name validity, character set, maximum length, whitespace/case normalization, and validation feedback for sessions and figures.
3. Whether Bank-figure deletion requires confirmation.

(`gdd.md` → “Deferred Design Notes”; `decision-log.md` → “Discipline validation”)

### Additional Gaps Exposed by Surface Closure

1. Define the screen/navigation map: first run, session browser, create, open, leave/return, settings, Bank, dialogs, and quit/close behavior.
2. Define mode/command state transitions for Live, Die, Move, Highlight, Pause/Resume, Bank, staged placement, Zoom, and modal interruptions.
3. Define Field status hierarchy: at minimum active tool and running/paused/placement state; decide whether generation, interval, zoom, or field size needs persistent display.
4. Define pointer details: hit targets, drag continuity, cursor feedback, pointer capture, edge behavior, preview anchor, and cell snapping.
5. Define feedback and recovery for duplicates, blank/invalid names, invalid Bank placement, persistence failures, corrupted data, save/rename/delete success, and unavailable actions.
6. Define session-card and Bank-list ordering, scrolling, empty states, content density, and selected/hover/focus behavior.
7. Define exact visual tokens and component states while preserving the restrained mathematical-tool direction.
8. Set an accessibility floor: contrast, non-color cues, readable scale, control targets, keyboard/focus policy, high-DPI behavior, and reduced-motion policy.
9. Decide localization scope and Unicode/name handling, even if the answer is explicitly English-only for the personal-study release.
10. Define performance-facing UX for large configured fields and growing session/Bank collections once architecture supplies limits.
11. Author at least one named-protagonist journey grounded in the desktop/mouse form factor; none exists in source, and the GDS UX contract requires journeys to close the IA.

## Non-Current Directions to Keep Out of This UX Scope

- `flex`-triggered program/source replication is parked and technically undefined. (`brief.md` → “Longer-Term Direction”; `addendum.md` → “Parked Future Directions → Quine”; `gdd.md` → “Out of Scope → Parked Research”)
- AI-assisted mathematical reasoning/evidence is parked; its role and rigor are unresolved. (`brief.md` → “Longer-Term Direction”; `addendum.md` → “Parked Future Directions → AI-Assisted Mathematical Work”; `gdd.md` → “Out of Scope → Parked Research”)
- The semantic/versioned construction workbench with provenance and replayable experiments is uncommitted research. (`addendum.md` → “Discovery Research Snapshot”; `gdd.md` → “Out of Scope → Parked Research”)

These directions should not add present-day screens, controls, or information architecture without a future GDD update.

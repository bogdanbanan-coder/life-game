# Epic 2 Context: Editing and Observation

<!-- Generated from planning artifacts. Regenerate with compile-epic-context if planning docs change. -->

## Goal

Let the player deliberately alter and inspect a running Conway experiment by painting cells alive or dead, pausing without changing the field, resuming from the preserved state, and always seeing the selected editing mode and simulation state. This extends the Field MVP into a responsive observation tool while retaining deterministic evolution at the default 0.25-second interval and introducing no goals, scores, progression, Settings, sessions, or persistence.

## Stories

- Story 2.1: Paint Live by Drag
- Story 2.2: Paint Dead by Drag
- Story 2.3: Pause Without Losing State
- Story 2.4: Resume and Return to Live
- Story 2.5: Show the Active Tool

## Requirements & Constraints

- Live and Die must set every touched in-bounds cell directly to alive or dead during either a press or drag. Painting works while the field is Running or Paused and must not require pausing first.
- A fast drag must not leave unintended gaps: interpolate between pointer samples in integer cell coordinates. Leaving the field while held stops mutations; re-entering while still held continues the stroke. Input outside the finite field changes no cell.
- Accepted painting input mutates the current field immediately and is visible in the same rendered frame. If generation work is also due in that iteration, the edit applies after the completed scheduled batch and affects only future generations.
- Pause must preserve the current field exactly, stop all generation updates, and clear accumulated simulation time. Time spent paused must not create catch-up work.
- Resume must switch back to Running, select Live, and begin a fresh full generation interval. Pause/Resume is a transient command rather than a persistent editing mode.
- Exactly one persistent mode is selected at a time. The active mode must remain continuously visible, and Running or Paused must have an explicit textual indicator; a changing Pause/Resume button label alone is insufficient.
- Editing and toolbar feedback must remain responsive at the default 50×50 field. Release-reference simulation/input/render work for that default field has a 16 ms verification budget, but there is no user-facing FPS promise.
- macOS and Linux must use the same editing, timing, and presentation semantics. Coordinates must be normalized to logical client pixels before hit testing or field-cell mapping so the same visible point selects the same cell at every supported DPI scale.

## Technical Decisions

- Preserve the authoritative per-iteration phase order: snapshot the clock and execute the scheduled simulation batch, sample and translate exposed input, execute accepted commands, then render. Repeating the same controlled clock and input traces must produce the same generation sequence, post-input field, and rendered result.
- Snapshot due work as `min(floor(accumulator / interval), 4)`. Execute that many complete sequential generations before input; render only the final generation. When more than four are due, discard excess whole-interval debt while retaining the sub-interval remainder. Do not replay input missed before the sampling phase or show a slow-simulation message.
- Keep simulation run state distinct from persistent tool selection. Running/Paused controls scheduler advancement; Live/Die controls field mutation. Activating Resume updates both state dimensions deliberately by entering Running and selecting Live.
- Route editing through logical-coordinate conversion and exact field-cell mapping. Toolbar or other UI-owned pointer events must never fall through and mutate cells beneath them.
- The Life model remains framework-independent. Generations use separate current and next state buffers so all cells read one consistent prior generation; editing commands mutate the resulting current state only after scheduled generation work completes.

## UX & Interaction Patterns

- The compact upper-right toolbar exposes Live, Die, and Pause/Resume and shows both the active persistent mode and the short Running/Paused status text. Live is the default mode.
- Persistent-mode buttons remain selected until another persistent mode is chosen. The active button uses the active fill plus a 2 logical px white inset outline and a textual cue, so selection never depends on gray shade alone.
- Controls use short literal labels (`Live`, `Die`, `Pause`, `Resume`), remain visually stable when Pause changes to Resume, and provide pointer targets of at least 32×32 logical px. Body text is 16 logical px and compact labels are 14 logical px.
- Use a crosshair over editable cells in Live and Die. Tool selection changes immediately on activation; cell changes appear without animation or audio. Pausing stops simulation but does not freeze the UI or prevent field inspection and editing.
- The Field remains the visual priority: dead cells are black, live cells are white, and active mode/run state is secondary. Do not add generation counters, coordinate readouts, zoom values, field dimensions, interval readouts, decorative motion, or audio feedback for this epic.

## Cross-Story Dependencies

- Epic 2 depends on Epic 1 for the visible finite 50×50 field, direct Live cell mutation, fixed-step Conway scheduler, permanent-dead boundaries, and deterministic current/next generation model.
- Stories 2.1 and 2.2 share drag rasterization, coordinate normalization, in-bounds filtering, and the simulation → input → render contract.
- Stories 2.3 and 2.4 share run-state and accumulator transitions; Story 2.4 also establishes the post-command Live selection consumed by subsequent editing.
- Story 2.5 presents the tool and run states established by the other stories. Later camera and Bank workflows add more persistent modes and transient commands, so this epic's state model must remain extensible without changing its Live/Die/Pause/Resume semantics.

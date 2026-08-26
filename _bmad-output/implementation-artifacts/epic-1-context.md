# Epic 1 Context: Field MVP

<!-- Generated from planning artifacts. Regenerate with compile-epic-context if planning docs change. -->

## Goal

Deliver the smallest complete Life experiment: a visible finite field, direct Live input, and correct scheduled Conway evolution. This validates the core manipulation and deterministic-evolution pillars in a focused Field MVP development harness before later session, Bank, navigation, and persistence systems are introduced.

## Stories

- Story 1.1: View the Field MVP
- Story 1.2: Paint Live Cells
- Story 1.3: Advance Conway Generations
- Story 1.4: Respect Permanent-Dead Boundaries
- Story 1.5: Verify Canonical Field Behavior

## Requirements & Constraints

The harness must show a lined 50×50 finite field with compact upper-right controls. Live is the default tool. Clicking or dragging over visible in-bounds cells makes exactly the touched logical cells live; drag rasterization must not skip cells between pointer samples. Toolbar, modal, and out-of-field input must not mutate field state. The MVP uses standard Conway rules: live cells survive with 2 or 3 live neighbors, dead cells become live with exactly 3, and every cell is computed synchronously from one complete prior generation.

Generation updates use the fixed 0.25-second MVP interval independently of rendering cadence. The finite field never wraps or expands. Out-of-bounds coordinates are permanently non-live, do not contribute neighbors, and cannot be edited; visible space beyond the field is presentation-only. Canonical evidence must cover lone-cell death, a stable 2×2 block, a period-two blinker, and edge behavior, plus repeatability for the same controlled clock and input trace. The default 50×50 simulation/input/render work has a 16 ms Release-reference verification budget, with no user-facing FPS promise. The baseline must run on at least one macOS and one Linux development environment.

Bank, persistence, Settings, configurable timing, camera navigation, and the final Start Screen/session shell are outside this epic’s MVP harness scope.

## Technical Decisions

Use the approved C++23, CMake, raylib 6.0, and raygui 5.0 stack with pinned dependency versions on macOS and Linux. Keep the Life model framework-independent. Store cells in dense row-major byte buffers: compute each generation with separate current and next buffers, then publish the complete next state. Render the field as one board or visible range rather than one UI object per cell.

Each main-loop iteration snapshots elapsed time and the due-generation count before later work. The ordered phases are: execute the scheduled simulation batch, sample and translate input, apply accepted edits, then render. A delayed frame runs `min(floor(accumulator / interval), 4)` complete generations sequentially; excess whole-generation debt is discarded while the sub-interval remainder is retained. Input after the batch changes the current field for future generations only, and the completed batch is rendered once.

## UX & Interaction Patterns

Dead in-field cells are black, live cells are white, and out-of-field space is gray (`#808080`); the grid uses the approved gray line token and is hidden below 4 logical display pixels per cell. Field geometry aligns exactly with simulation cells. Use logical client pixels for rendering and pointer mapping, applying OS DPI scaling once and normalizing coordinates before cell hit testing. The compact toolbar remains in the upper-right, with flat controls and persistent visual/textual state cues; modal or toolbar ownership prevents click-through to the field.

## Cross-Story Dependencies

Foundation enabler Story 1.0 must establish the clean-checkout CMake, dependency, target, build, test, and CI baseline before Field work is considered implementable. Within this epic, the field surface and coordinate mapping established by Story 1.1 underpin Live editing, simulation, boundary handling, and deterministic verification. Epic 1 is the prerequisite for Editing and Observation (Epic 2) and Field Navigation and Setup (Epic 3); Epic 3 later supplies the product Start Screen/session shell around this field foundation.

---
title: "Game Brief: Life Game"
status: final
created: 2026-08-15
updated: 2026-08-18
---

# Game Brief: Life Game

## Core Concept

Life Game is a private, single-user Conway's Game of Life study sandbox for macOS and Linux.

**Core promise:** Directly seed a finite Life field, watch deterministic patterns emerge, preserve exact constructions, and reuse them across experiments.

## Purpose, Audience, and Boundaries

This is a personal study project for its creator, who describes the intended player simply as a “nerd”: someone willing to manipulate and observe Life cells without external motivation. Its value comes from learning through implementation and from supporting personal mathematical experimentation.

Originality and broad-market appeal are not success criteria. Existing Life tools may be used as learning references. Multiplayer, accounts, community systems, authored challenges, progression, achievements, and prescribed goals are outside the vision. The player supplies their own curiosity and decides what is worth constructing or preserving.

## Design Pillars

- **Direct manipulation:** Live and Die tools set cell state through drawing on the field.
- **Deterministic evolution:** each generation applies Conway's rules synchronously, independent of rendering speed.
- **Preservation:** a highlighted rectangle can be named and saved as a reusable figure.
- **Composition:** a saved figure can be staged transparently and committed as exact rectangular replacement.

## Intended Experience

The player works with a finite board whose dimensions remain fixed during a session. In each simulation generation, every cell updates synchronously under Conway's standard rules: a live cell survives if it has two or three live neighbors; a dead cell becomes live if it has exactly three live neighbors; and all other cells become or remain dead. The simulation treats positions beyond the board boundary as permanently dead, so patterns may be clipped or destroyed at an edge.

The experience is deliberately a sandbox rather than a conventional challenge game. A session may run indefinitely until the player leaves it or closes the application.

## Core Loop

1. Choose or create a named session; its field and last camera view appear, and simulation begins immediately.
2. Draw live or dead cells, pause or resume evolution, and observe successive generations.
3. Highlight a useful rectangle and either save it under a name or cancel the capture.
4. Open Bank, stage a saved figure transparently, then press Resume to replace the entire rectangle beneath it and continue simulation.
5. Leave the session to save its state and last-view preview, then reuse Bank figures in any other session.

## Scope and MVP

The MVP and first implementation milestone are the same: **Field**. It validates rendering, input mapping, mutable cell state, and synchronous Life updates without pulling later application systems into the first build.

Field is complete when:

- The application displays a small, finite, lined square field with small tool buttons in the upper-right corner.
- Clicking inside a cell in the default Live mode makes that cell alive.
- The field advances synchronously under standard Conway rules at a fixed interval of 0.25 seconds.
- Coordinates outside the field remain permanently dead.

Bank, persistent sessions, configuration menus, configurable timing, camera navigation, window resizing, and storage are outside the MVP.

### Target Product Scope After MVP

- **Simulation and navigation:** Live, Die, and Pause/Resume; configurable generation interval; square or rectangular fixed-dimension fields; camera movement and zoom. Window resizing remains optional.
- **Construction tools:** drag-to-highlight capture with automatic pause, naming, Save, and Cancel; one application-wide Bank; translucent staging; exact rectangular commit when simulation resumes.
- **Sessions and persistence:** multiple automatically saved sessions containing cell states, camera position, zoom level, and a preview of the last camera view; a horizontally scrolling session browser with Create and Delete actions, where Delete requires confirmation.

### Technical Boundary

The confirmed stack is C++23, CMake, raylib 6.0, and raygui 5.0, with dependency versions pinned. Project code uses only C++23 features verified on both selected macOS and Linux toolchains. Simulation and persistence remain framework-independent; raylib and raygui stay at the application and presentation boundary.

There is no fixed commercial deadline. Persistence format, exact default dimensions, and performance limits are downstream design or implementation concerns rather than brief blockers.

## References

- **[Golly](https://golly.sourceforge.io/):** take deterministic simulation, dependable editing, and respect for expert workflows; leave its multi-rule breadth, scripting, layers, and infinite-universe scope outside this project.
- **[Conway Canvas](https://conwaycanvas.com/):** take readable selection, preview, and reusable-pattern workflows; leave its browser/PWA positioning and feature breadth outside the native personal-study scope.

The project does not need to surpass these tools. Reimplementation is part of the learning goal.

## Visual and Audio Direction

Use a restrained mathematical-tool aesthetic: dark field, bright live cells, subdued dead cells, faint grid lines visible only at useful zoom levels, a distinct selection accent, translucent staged figures, and simple flat controls. The project has no world, narrative presentation, decorative character art, music, or sound effects.

## Longer-Term Direction

- Investigate program/source replication triggered by the input `flex`, where the program creates a copy of itself “next to” itself. This is not a self-replicating Life pattern, and its technical meaning remains intentionally deferred.
- Use AI-assisted mathematical reasoning and evidence to help develop more complex constructions.

Neither direction belongs to the MVP.

## Risks and Open Questions

- Scope may drift from Field toward the complete application; the explicit MVP boundary controls that risk.
- Synchronous simulation must remain independent of render frequency and consistent across both target platforms.
- Session and figure naming, renaming, and duplicate-name behavior remain open for the GDD.
- Camera gestures must not conflict with drawing and Highlight input.
- C++23 compiler and standard-library coverage varies, so cross-platform builds must validate every adopted feature.

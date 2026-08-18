---
title: "Life Game Brief Addendum"
status: final
created: 2026-08-15
updated: 2026-08-18
---

# Life Game Brief Addendum

## Interaction Detail

The companion brief is canonical for the visual direction and the Field MVP boundary. This section preserves post-MVP interaction details for the GDD.

### Board Boundary and View

The board is finite. The simulation treats any coordinate beyond the board boundary as permanently dead: it does not count as a live neighbor and cannot become alive. The board does not wrap or automatically expand.

Field dimensions remain unchanged during a session. A separate camera can move across and zoom into or out of the field without affecting field coordinates or cell state. Application-window resizing is not required and may remain unsupported if costly.

A new session begins with a small square field by default. Session setup can configure width and height independently, so rectangular fields are supported. Exact default dimensions are intentionally deferred.

### Session Lifecycle

The application supports multiple saved sessions. When the player leaves a session, its current state is saved automatically so the session can be reopened later. Saved state includes the cell grid, camera position, and zoom level. When the session is reopened, those values are restored and generation updates begin immediately; any previous paused state is not restored.

The launch screen presents every session in a horizontally scrolling list. A session entry contains:

- Its name.
- A preview image of the last camera view.
- An adjacent Delete button.

Create opens a small window in which the player enters the new session's name. Delete opens a confirmation window before removing the session.

The figure bank is application-wide rather than owned by an individual session. A figure captured in one session is available in every other session, and deleting a session does not remove figures from the bank.

### Tools and Figure Bank

#### Tool Pool

The interface exposes five tools: Live, Die, Pause/Resume, Highlight, and Bank. One tool is always selected; Live is the default.

- **Live:** drawing over the field makes affected cells alive.
- **Die:** drawing over the field makes affected cells dead.
- **Pause/Resume:** pauses or resumes generation updates.
- **Highlight:** begins rectangular figure capture.
- **Bank:** opens the saved-figure workflow.

Pause/Resume and Bank are transient commands. After either command completes, Live becomes selected. The next press or drag over field cells makes those cells alive; interaction that does not touch a field cell does not change the board.

#### Highlight and Save

The player points at a cell, presses the mouse button, drags to another cell, and releases. The rectangular area between those cells becomes highlighted. The simulation automatically pauses, and a small window appears with a name field and Save and Cancel buttons.

Save stores the selected figure in Bank under the entered name. Choosing Cancel or clicking anywhere outside the small window dismisses the operation without adding the figure. After either outcome, the simulation resumes.

#### Bank and Paste

Opening Bank pauses the simulation and displays every saved figure with its name. Clicking a figure closes the menu and stages that figure as a transparent preview over the field. The player can move the staged figure to the intended location. Pressing Resume commits the figure and restarts the simulation.

A saved figure is a rectangular bitmap containing both live and dead cells. On commit, every cell in the stored rectangle replaces the corresponding board cell exactly: stored live cells make underlying cells live, and stored dead cells make underlying cells dead. Cells outside the placed rectangle remain unchanged.

## C++ Framework Evaluation

C++ is suitable for the project. The Life model should remain framework-independent and use separate buffers for the current and next cell states so each generation is computed from one consistent prior state. The implementation should render the field as one board or texture rather than creating a UI object for every cell.

### Selected: raylib + raygui

[raylib](https://github.com/raysan5/raylib) supports C++, macOS, Linux, CMake, camera transforms, coordinate conversion, mouse input, and straightforward 2D drawing. [raygui](https://github.com/raysan5/raygui) adds immediate-mode buttons, toggles, text boxes, lists, scroll panels, message boxes, and text-input dialogs. Together they cover the board, toolbar, modal flows, Bank, and session browser without a large application framework.

Tradeoffs: the APIs are C-flavored, layout is manual, dialogs are drawn inside the application rather than in native operating-system windows, and the horizontally scrolling session-card layout requires a custom implementation. Exact library versions should be pinned because raygui's development branch may track breaking changes in raylib.

The chosen baseline is raylib 6.0 plus raygui 5.0. The project compiles in C++23 mode. C++23 is a published standard, but [Clang's official implementation status](https://clang.llvm.org/cxx_status) still identifies C++23 support as partial, and [GCC tracks language and library support separately](https://gcc.gnu.org/projects/cxx-status.html). Use C++23 features only after verifying them on both target toolchains.

### Strong Alternative: Qt 6 Widgets

[Qt Widgets](https://doc.qt.io/qt-6/qtwidgets-index.html) is the stronger choice if the learning priority becomes a conventional desktop application with ready-made toolbars, dialogs, model/view lists, and richer layout. A single custom-drawn `QGraphicsView` board can provide camera movement and zoom. Qt is considerably heavier to install and deploy, introduces generated Meta-Object Compiler code, and complicates the boundary of a later source-replication experiment.

### Less Suitable Here

[SFML](https://www.sfml-dev.org/) offers a clean native C++ graphics API and camera view but no built-in application GUI. [SDL](https://wiki.libsdl.org/SDL3/FrontPage) is a strong low-level portability layer but requires more custom camera and interface plumbing. Both are reasonable learning choices, but neither shortens this project as much as raylib plus raygui.

## Discovery Research Snapshot

The initial editor and construction-bank mechanics are valuable but established in the Life-tool landscape:

- [Golly](https://golly.sourceforge.io/) is the expert benchmark for high-performance simulation, selections, transformed pastes, pattern libraries, interoperability, scripting, and very large universes.
- [Conway Canvas](https://conwaycanvas.com/) is a close browser/PWA comparison with selection-to-stamp saving, searchable custom and favorite stamps, previews, transforms, and touch support.
- [Alan Dewar's Life simulator](https://cuug.ab.ca/dewara/life/life.html) demonstrates that named, browser-stored pattern buffers and selection-based editing are longstanding conventions.
- [Slice of Life](https://www.conwaygame.life/) explores large-pattern navigation, timeline controls, and visual previews of future evolution.
- [LifeWiki](https://conwaylife.com/wiki/) and [Catagolue](https://conwaylife.com/wiki/Catagolue) set strong expectations for pattern interchange, canonical identity, provenance, classification, and reproducible discovery.

Established functionality is acceptable because this is a personal study project rather than a market-facing product. A possible later learning direction is a construction workbench in which saved patterns become semantic, versioned components with provenance and replayable experiment results. This is a research-derived option, not a confirmed product decision.

The earlier web/PWA hypothesis is superseded by the selected native C++23, raylib, raygui, and CMake stack.

## Parked Future Directions

### Quine

The intended future quine is program replication, not a self-replicating Game of Life construction. Given the input `flex`, the program should create a copy of itself “next to” the original. The user also envisions each source file participating in this behavior but has not defined its exact form.

Later technical design must distinguish among source-text generation, copying files, reconstructing a complete source tree, producing a neighboring application bundle, and spawning another running instance. It must also determine whether each source file reproduces independently or the program reproduces itself as a coordinated whole.

Copying a compiled executable is different from reproducing C++ sources. Source reproduction requires the program to embed, generate, or otherwise obtain those sources; third-party framework code and generated build artifacts should not implicitly become part of the replication target. Keep this experiment isolated from the simulation and UI.

### AI-Assisted Mathematical Work

The user intends to explore mathematical reasoning and evidence with AI to create more complex constructions. Open design questions include the roles AI might play—conjecture partner, pattern-search tool, explainer, verifier, construction generator, or a combination—and the level of mathematical rigor that “evidence” implies.

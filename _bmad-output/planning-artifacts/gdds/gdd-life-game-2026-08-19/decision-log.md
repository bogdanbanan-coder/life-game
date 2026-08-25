# Life Game GDD Decision Log

## 2026-08-19 — GDD initialized

- **Intent:** Create a new GDD from the finalized game brief and addendum.
- **Workspace:** `_bmad-output/planning-artifacts/gdds/gdd-life-game-2026-08-19/`
- **Platforms:** macOS and Linux, inherited from the confirmed brief.
- **Status:** Discovery.
- **Pending:** Confirm the matched game type before loading its genre-specific design guide.

## 2026-08-19 — Source extraction complete

- **Inputs:** Finalized `brief.md` and `addendum.md` from `brief-life-game-2026-08-15`.
- **Confirmed foundation:** Personal single-user Life sandbox; four pillars (direct manipulation, deterministic evolution, preservation, composition); Field-only MVP; macOS/Linux; no challenges, progression, narrative, multiplayer, or audio.
- **Detailed input retained:** Finite-board rules, session lifecycle, tools, exact Bank replacement, visual direction, post-MVP scope, and parked quine/AI directions.
- **Game-type match:** Sandbox is the proposed primary type; Simulation is the secondary descriptor.
- **Reason:** Creative construction and minimal objectives define the player experience, while deterministic simulation describes the underlying system.
- **Status:** Awaiting user confirmation before loading the Sandbox genre guide.

## 2026-08-19 — Game type confirmed

- **Decision:** Classify Life Game as **Sandbox** with **Simulation** as a secondary descriptor.
- **Reason:** The user confirmed the proposed match. Creative freedom, construction, reusable figures, and minimal objectives define the experience; deterministic cellular-automaton evolution defines the simulated system.
- **Complexity:** Low according to the installed Sandbox guide, right-sized to the solo study scope.
- **Genre guide adaptation:** Keep Creation Tools, Constraints and Rules, Tools and Editing, and Emergent Gameplay. Reinterpret Physics and Building Systems as Conway simulation behavior. Mark Sharing and Community explicitly out of scope.
- **Narrative:** No dedicated narrative workflow is indicated.

## 2026-08-19 — Working mode

- **Decision:** Use Express mode.
- **Reason:** Explicit user choice; the finalized brief already resolves the vision, pillars, core loop, MVP boundary, major mechanics, platforms, and art/audio direction.
- **Consequence:** Resolve remaining architecture-facing design gaps in one compact batch, then draft the complete GDD and epics.

## 2026-08-19 — Express gap resolution

- **Default field:** 50×50 cells, selected by the facilitator under explicit user delegation. This preserves a small complete-grid view while leaving room for basic Life constructions.
- **Camera:** A dedicated Move button activates camera-drag behavior. Dedicated `+` and `−` buttons control zoom.
- **Invalid Bank placement:** If the staged figure extends beyond the finite board when Resume is pressed, place nothing, exit Bank mode, restore Live, and resume simulation.
- **Names:** Duplicate session names are forbidden among sessions; duplicate figure names are forbidden within the global Bank. Both object types support Rename and Delete through dedicated buttons.
- **Performance:** No frames-per-second target. Field success is defined through correct scheduled generations and responsive input, consistent with the lightweight personal-study scope.
- **Deferred tuning:** Exact zoom ratios/limits, name character/length rules, and Bank-delete confirmation are downstream UX details unless validation finds them phase-blocking.

## 2026-08-19 — First full draft

- **Decision:** Draft the complete GDD and development epics from the confirmed brief, extracted addendum, Sandbox guide, and Express resolutions.
- **Status:** Drafting complete; finalization audit and validation remain.

## 2026-08-19 — Draft accounting approved

- **Decision:** The user approved the draft accounting without changes.
- **Included:** Confirmed game vision, rules, tools, Bank and session behavior, finite-field boundary semantics, camera controls, platform/toolchain targets, Field MVP, and development epics.
- **Parked:** Program self-replication (quine direction) and AI-assisted mathematical discovery remain future research rather than committed gameplay scope.
- **Deferred:** Zoom tuning, blank-name handling, Bank-delete confirmation behavior, persistence format, and window-resize behavior remain downstream UX or architecture decisions.
- **Excluded:** Multiplayer, authored challenges, progression, objectives, narrative, audio, and performance-as-spectacle remain outside the agreed scope.
- **Status:** Approved for reconciliation and discipline validation.

## 2026-08-19 — Source reconciliation

- **Result:** The finalized brief and addendum reconcile with the GDD and epics without gameplay contradictions.
- **Resolved-size note:** The brief originally deferred the exact field size; the later Express decision validly resolves it to 50×50 under the user's explicit delegation.
- **Preservation edit:** Added the Golly and Conway Canvas reference rationale, including the intended take/leave boundaries.
- **Classification edit:** Changed frontmatter to canonical `game_type: sandbox` and retained Simulation as a secondary descriptor.
- **Research accounting:** Explicitly set aside the uncommitted semantic/versioned construction-workbench idea alongside the two confirmed future research directions.
- **Architecture handoff:** Preserve framework independence for the Life model and persisted domain state. The addendum's double-buffering and whole-board rendering notes are architecture inputs, not player-facing GDD requirements, and are intentionally omitted here to avoid implementation leakage.

## 2026-08-19 — Discipline validation

- **Result:** 14 pass, 2 warn, 1 fail, and 1 not-applicable before correction; no issue blocks Game Architecture.
- **Required correction:** Epic 3 no longer claims compatibility with Die or Highlight before those tools are delivered. Its outcome now concerns camera navigation without cell edits or geometry changes.
- **Terminology correction:** Standardized the editable space as **field** and renamed Epic 3 to **Field Navigation and Setup** in both documents.
- **Measurability clarification:** The 50×50 field must complete a generation before the next scheduled 0.25-second update. Maximum configurable dimensions remain an explicit architecture decision.
- **Deferred warnings accepted:** Exact zoom tuning, detailed naming rules, and Bank-delete confirmation are non-blocking for architecture and remain assigned to later UX/design work.

## 2026-08-19 — Finalization

- **Post-correction validation:** 16 pass, 1 low-severity warning, 1 not-applicable, and 0 fail.
- **Open-items review:** Field maxima, zoom tuning, detailed naming rules, and Bank-delete confirmation are phase-gated downstream decisions; none blocks Game Architecture.
- **Document standards:** No additional project-specific polish standards are configured.
- **Narrative:** No narrative flag or narrative handoff applies.
- **External handoffs:** None are configured.
- **Status:** `gdd.md` and `epics.md` are final.

## 2026-08-19 — Workflow handoff

- **Optional next step:** `[CU]` `gds-ux`, because the game has several concrete interaction flows but no settled layout or detailed UI behavior.
- **Next required step:** `[GA]` `gds-game-architecture`, followed by `[CE]` `gds-create-epics-and-stories` and `[IR]` `gds-check-implementation-readiness`.
- **Architecture input reminder:** Include the finalized brief addendum so framework independence, synchronous-state buffering, and whole-field rendering constraints are considered without embedding implementation design in the GDD.

## 2026-08-24 — UX-A3 production gates resolved

- **Decision:** Adopt the UX production constants for the owning Epics 3–5: seven discrete zoom levels (`50%`, `75%`, `100%`, `150%`, `200%`, `300%`, `400%`) with a `100%` default; grid lines at or above 4 logical display pixels per cell; trimmed, NFC-normalized 1–64-code-point names with case-insensitive uniqueness; explicit confirmation for Bank-figure deletion; and 256×256 field-only PNG session previews.
- **Impact:** The formerly deferred UX gates are now reflected in `gdd.md`, `epics.md`, `game-architecture.md`, the UX spines, and project context. Persistence schema and remaining camera/settings decisions stay downstream-owned.

## 2026-08-24 — UX-A4 presentation ownership resolved

- **Decision:** The Start Screen owns the scrolling session browser, Create action, and Settings panel. Session-card field previews are inert pictures; selecting a card opens the interactive Field Screen, whose upper-right Toolbar owns field commands and opens the single application-wide Bank panel.
- **Settings contract:** The Settings table shows Field width and Field height in cells plus generation interval, with labels in the left column and editable values in the right. Width and height apply as defaults to newly created sessions; session dimensions remain fixed after creation.
- **Dialog contract:** Name dialog handles text entry, Confirmation dialog handles explicit Confirm/Cancel deletion, Error dialog handles blocking failures, and reusable fields/status feedback own validation and non-modal messages.

## 2026-08-24 — UX-A5 synchronous feedback and Bank pause resolved

- **Decision:** Keep session, Bank, and preview operations synchronous on the main thread. Busy feedback may be staged before a call and success/failure is shown after it returns; the whole window may block during the call, with no live-progress promise.
- **Bank contract:** Opening Bank pauses the active Field and clears accumulated time. Bank controls remain usable while paused; closing or canceling without placement returns to Live with a fresh interval, while a selected figure remains paused until placement resolves.
- **Impact:** UX-A5 is resolved without adding asynchronous execution or changing the deterministic Life model. The behavior is reflected in the UX, architecture, project context, and epic acceptance criteria.

## 2026-08-24 — UX-A6 persistence failure categories resolved

- **Decision:** Treat inability to open the SQLite database, establish its schema, or complete migration as a fatal startup failure. Show a specific Error dialog, do not create a replacement database or open the Start Screen, and exit after acknowledgment.
- **Record behavior:** When the database opens but an individual session or Bank figure record fails validation, preserve it and keep valid records usable. Show the damaged item as disabled when identifiable and report a specific error on selection; do not automatically repair, overwrite, delete, or replace it with empty data.
- **Impact:** UX-A6 now matches the architecture's single-database model and provides distinct recovery behavior for fatal storage failure versus isolated record damage.

## 2026-08-24 — UX-A7 global Settings resolved

- **Decision:** Treat Field width, Field height, and generation interval as one global Settings configuration edited on the Start Screen.
- **Activation:** Global width and height provide defaults for newly created sessions only; saved session dimensions never change. A session created or opened after Settings Save uses the current global generation interval and starts a fresh interval.
- **Impact:** The former user-settings versus session-settings ambiguity is removed without changing fixed session geometry or adding per-session configuration controls.

## 2026-08-24 — UX-A8 platform scaling and input resolved

- **Decision:** Use logical client pixels for UI, pointer targets, and Field presentation on macOS and Linux. The initial logical client area is 1280×720 and the minimum supported logical viewport is 960×540. Body text is 16 logical px, compact/numeric text is 14 logical px, and pointer targets are at least 32×32 logical px.
- **Input contract:** OS DPI scaling is applied once; pointer coordinates are normalized to logical client coordinates before UI hit testing, camera conversion, or Field cell mapping. The 4-pixel grid threshold is measured in logical display pixels.
- **Impact:** Cross-platform presentation and input now have explicit implementation and Epic 6 verification gates without changing the supported desktop platforms or Field coordinate model.

## 2026-08-24 — UX-A9 camera boundary and out-of-field rendering resolved

- **Decision:** Clamp the camera so the viewport always intersects at least one in-field cell. Any visible area outside the finite Field is rendered gray (`#808080`) rather than black or white.
- **Semantics:** Gray out-of-field space is presentation-only. It is not a third cell state, does not participate in simulation, cannot receive cell input, and is not persisted as field data; a saved session preview may include its gray pixels. Out-of-field coordinates remain non-live for Life rules.
- **Impact:** Camera behavior and finite-boundary visuals are now explicit without changing the finite Field model or cell-state storage.

## 2026-08-24 — UX-A10 source authority and reconciliation resolved

- **Decision:** User-approved decisions and the current Sprint Change Proposal take precedence, followed by GDD product scope and semantics, UX interaction/visual contracts, architecture implementation details, and epics/stories as delivery traceability.
- **Reconciliation:** A conflict is recorded as a `UX-A#` issue and resolved through `gds-correct-course`; all affected artifacts and decision logs are updated together, then `gds-check-implementation-readiness` is rerun before the owning epic enters production.
- **Impact:** Architecture now has explicit UX sources and a repeatable governance path, without changing the GDD's product authority.

---
title: "Sprint Change Proposal — Readiness Findings Resolution"
project: life-game
date: 2026-08-25
status: applied
scope: moderate
trigger: implementation-readiness-report-2026-08-24.md
---

# Sprint Change Proposal — Readiness Findings Resolution

## 1. Issue Summary

The 2026-08-24 implementation-readiness assessment found no product-scope contradiction, functional-requirement coverage gap, circular dependency, or explicit forward dependency. It did find planning defects that prevented production readiness:

- project bootstrap and build/test enablement were absent from the delivery plan;
- most epic stories had no story-specific acceptance criteria or explicit FR/NFR trace;
- Settings, the Start Screen, in-memory session creation, and persistent session browsing had overlapping or unassigned ownership;
- Epic 6 mixed platform behavior, technical enablement, failure handling, and release verification in oversized stories;
- UX anchor, validation, recovery, capacity, responsiveness, and visual-verification decisions remained open;
- invalid persisted global Settings behavior was not clearly bounded.

The user authorized resolving all contradictions, problems, and concerns in the readiness report as one batch correction.

## 2. Impact Analysis

### Epic Impact

- Epic 1 gains a non-player `Story 1.0` foundation enabler for CMake, pinned dependencies, target structure, CTest, and CI baseline.
- Epic 2 remains the self-contained editing/observation slice and no longer owns global Settings interval configuration.
- Epic 3 owns the Start Screen/session shell and one global Settings surface in memory; it remains independent of persistence.
- Epic 4 is unchanged in product scope and depends on Epics 2–3.
- Epic 5 explicitly depends on Epic 3 and Epic 4, upgrading the session shell to SQLite-backed persistent sessions and previews.
- Epic 6 retains only player-facing cross-platform parity and persistence-failure behavior. Build and release verification move to explicit quality gates.

### Artifact Impact

- `epics.md`: dependency graph, story slices, implementation enabler, release gates, story-specific acceptance criteria, and FR/NFR traceability are added.
- `story-specs-life-game-2026-08-25.md`: canonical story specifications and acceptance criteria are added.
- `EXPERIENCE.md` and `DESIGN.md`: remaining UX assumptions are confirmed, error/retry behavior is specified, and visual verification is made explicit.
- `game-architecture.md`: Settings recovery, capacity limits, internal performance budgets, error actions, and visual/accessibility verification are made authoritative.
- `gdd.md`: product-facing persistence and confirmed planning constraints are reconciled without changing the Field MVP or out-of-scope product scope.
- `project-context.md`: implementation rules are updated to carry the new decisions.
- GDD and UX decision logs record the reconciliation as UX-A11; the 2026-08-24 sprint proposal remains historical and this proposal records the follow-up correction.
- The 2026-08-24 readiness report receives a resolution update after verification.

### Technical Impact

No runtime code or product scope is changed. The correction changes planning contracts, backlog slicing, verification evidence, and architecture-owned safety/quality limits.

## 3. Recommended Approach

**Selected path: Option 1 — Direct Adjustment, with a documentation/quality-gate hybrid.**

Rollback is unnecessary because the existing GDD, UX, and architecture decisions are coherent. An MVP reduction is unnecessary because the findings concern delivery structure and unresolved downstream contracts rather than excessive product scope. The direct adjustment preserves the learning goal and player value while making implementation slices and gates explicit.

Effort is moderate at the planning level, risk is low-to-moderate, and the timeline impact is limited to backlog reorganization and verification planning before production begins.

## 4. Detailed Change Proposals

### Backlog and Story Structure

- Add `Epic 1 / Story 1.0 — Project Foundation Enabler` for the greenfield CMake/dependency/test/CI baseline.
- Move global Settings ownership from Epic 2 to Epic 3, where the Start Screen and in-memory session shell are introduced together.
- Make Epic 5 depend on Epic 3 and Epic 4; it owns the persistence upgrade rather than recreating the Start Screen.
- Reduce Epic 6 to three coherent player-facing stories and move build, visual/accessibility, performance, and traceability verification into named release gates.
- Add story-specific Given/When/Then acceptance criteria and FR/NFR links in the canonical story-spec artifact.

### UX Decisions Confirmed

- Zoom anchors on the in-field cell beneath the pointer; outside-field zoom uses viewport center.
- Settings Save is atomic; invalid input disables Save and remains visible; Cancel discards edits; Save failure preserves the previous stored configuration and keeps the panel open.
- Resizing remains optional and non-blocking. The release supports mouse/trackpad pointer input plus text/numeric entry; full keyboard navigation, screen-reader semantics, and alternative input are explicitly outside this private mouse-first scope.
- Session and Bank lists use stable case-insensitive ascending ordering; horizontal scrolling uses pointer-wheel/trackpad scrolling and a draggable scrollbar; empty states retain the primary action.
- Pointer drags rasterize between cell samples, capture until release, and use the figure's top-left cell as the staging anchor. A preview leaving the window retains its last in-window position; invalid placement uses a dashed outline and `Outside field` text.
- Recoverable failures use explicit Retry/Cancel actions; fatal startup failure uses Acknowledge and exits. Canonical messages are recorded in the UX spine.
- Invalid persisted Settings are preserved, not overwritten; safe defaults (50×50 and 0.25 seconds) are used in memory with a visible warning, and an explicit Save is required to replace the invalid record.

### Architecture and Verification Decisions

- Capacity limits are 512 sessions and 2,048 Bank figures; field and figure cell limits remain the existing validated limits.
- Internal reference budgets are added without creating a user-facing FPS promise: default 50×50 simulation/input/render work targets 16 ms in Release, a legal maximum-field generation targets 250 ms, and default session save plus preview targets 250 ms on the reference CI environments.
- Visual verification covers black/white/gray state contrast, focus rings, solid/dashed placement cues, non-color-only error/active-state cues, and modal click-through on both platforms.
- The architecture and project context explicitly distinguish invalid Settings fallback from non-repairable damaged session/figure records.

## 5. Implementation Handoff

**Change classification:** Moderate — backlog reorganization and cross-artifact planning correction.

- Product/Architecture: maintain the confirmed scope and the new ownership, recovery, capacity, and budget decisions.
- UX: implement the confirmed interaction and visual contracts in `EXPERIENCE.md` and `DESIGN.md`.
- Delivery/Developer: use `story-specs-life-game-2026-08-25.md` as the implementation context and create/update sprint tracking from the revised epics.
- QA/Test: use the story acceptance criteria and release gates for deterministic, cross-platform, persistence, visual, and accessibility verification.

Success criteria:

1. Every delivery story has explicit acceptance criteria and FR/NFR traceability.
2. The Start Screen/Settings/session-shell dependency is unambiguous.
3. Epic 6 contains no oversized technical or release-verification story.
4. UX has no unresolved implementation-relevant Open Decisions.
5. Invalid Settings recovery is explicit and non-destructive.
6. The readiness report is regenerated and no longer reports the resolved findings as open.

## Checklist Status

- [x] Trigger and evidence identified from `implementation-readiness-report-2026-08-24.md`.
- [x] Epic impact assessed across Epics 1–6.
- [x] GDD, UX, architecture, project context, decision logs, and supporting proposals reviewed.
- [x] Direct-adjustment/hybrid path selected.
- [x] Artifact edits defined and applied in batch under the user's instruction.
- [x] Story and release-gate handoff defined.
- [x] Readiness verification completed after edits: the sprint parser recognized 6 epics and 33 delivery stories with zero warnings; the story companion contains 37 acceptance/trace sections including RG1–RG4; all 27 FRs and 13 NFRs are present in the matrix; and `git diff --check` passes.

## Approval and Handoff

The user's instruction to resolve all findings is treated as approval to apply this batch correction. No runtime implementation is authorized by this proposal; the revised planning gate now passes, and implementation may proceed against the story specifications and RG1–RG4 gates.

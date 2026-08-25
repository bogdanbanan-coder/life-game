---
title: "Sprint Change Proposal — UX-A6 Persistence Failure Categories"
project: life-game
date: 2026-08-24
status: applied
scope: minor
---

# Sprint Change Proposal — UX-A6 Persistence Failure Categories

## 1. Issue Summary

UX-A6 identified a mismatch between the UX recovery model and the approved SQLite architecture. The UX referred to a missing or damaged session index, an empty/read-only browser after acknowledgment, and a damaged file. The architecture has one SQLite database, no session index, and treats database-open or migration failure as fatal at startup.

The correction was applied under the user's explicit instruction to apply the suggested UX-A6 solution.

## 2. Impact Analysis

- **UX:** Replace index/file language with two concrete states: fatal database failure and isolated damaged record. Define disabled damaged session cards and Bank rows, specific errors, and non-destructive behavior.
- **Architecture:** Preserve the existing SQLite topology and typed-error rules. Distinguish fatal database/setup failures from record-level validation failures and expose non-mutating damaged-record summaries.
- **GDD:** Document startup failure and isolated session/figure recovery behavior.
- **Epics:** Add UX-A6 acceptance criteria for fatal database failure, damaged session records, damaged Bank figures, and the no-automatic-repair rule.
- **Project context:** Make the recovery contract an implementation rule.
- **Technical impact:** No threading, schema replacement, automatic repair, or runtime code change is introduced. Implementation needs disabled-item rendering and mapped error outcomes.

## 3. Recommended Approach

Use a direct adjustment within the existing plan. Keep database open, schema establishment, and migration failures fatal at startup. When the database is usable but one record fails validation, preserve the record, keep valid records usable, and render the damaged item as disabled when its identity is available.

This is the lowest-risk approach because it follows the existing architecture's single-database, typed-error, preserve-valid-state rules. Effort is minor at the planning level; implementation risk is limited to repository result modeling and disabled-item presentation.

## 4. Detailed Change Proposals

### UX / UI

**Old:** A damaged or missing session index may produce an empty/read-only browser; a damaged Bank file disables Bank mutations.

**New:** A fatal database-open, schema, or migration error shows a startup Error dialog, does not open the Start Screen, does not create a replacement database, and exits after acknowledgment. An isolated damaged session or figure remains preserved and disabled when identifiable; valid records remain usable, and selecting the damaged item shows a specific error.

**Rationale:** The UI now describes the actual SQLite storage model and separates service-level failure from record-level damage.

### Architecture

**Old:** Architecture defined validation and fatal startup behavior but did not specify presentation outcomes for an isolated damaged record.

**New:** Repository load results distinguish fatal database/setup failure from isolated damaged-record summaries. Damaged sessions render as disabled session cards; damaged figures render as disabled Bank rows. Neither is automatically repaired, overwritten, deleted, or replaced with empty data.

**Rationale:** This preserves valid data and makes error handling testable without introducing a session index or recovery subsystem.

### GDD and Epics

**New acceptance coverage:**

- Fatal database open, schema, or migration failure blocks the Start Screen and exits after acknowledgment.
- A damaged session leaves valid session cards usable and never opens Field.
- A damaged Bank figure leaves valid Bank operations usable and is never staged.
- No damaged record is silently replaced, automatically repaired, overwritten, or deleted.

## 5. Implementation Handoff

**Change classification:** Minor documentation and backlog correction.

**Handoff:**

- **Developer:** Implement typed repository outcomes, disabled damaged-item presentation, startup-fatal Error dialog flow, and tests for fatal versus isolated record failures.
- **Test/QA:** Verify database-open/migration failure, valid records alongside one damaged session, valid figures alongside one damaged Bank record, preservation, and no automatic replacement.
- **Product/UX:** Supply final error copy and retry affordances for recoverable save failures; these remain separate from the resolved A6 storage-category decision.

**Success criteria:** UX, GDD, epics, architecture, project context, and readiness report use the same SQLite failure categories; valid records remain usable after isolated damage; fatal startup failures never create replacement data; `git diff --check` passes.

## Addendum — UX-A7 Global Settings

### Issue

The UX placed Field width, Field height, and generation interval in the Start Screen Settings menu, but the architecture still described a user-settings/session-settings split and did not define when a saved interval became active.

### Applied resolution

- One global SQLite `settings` record owns Field width, Field height, and generation interval.
- Width and height are global defaults for new sessions only; existing session dimensions remain fixed.
- The current global generation interval applies when a session is created or opened after Save, and that transition starts a fresh interval.
- Settings Save failure leaves global values and all sessions unchanged; invalid fields retain their input.

### Handoff

This remains a minor documentation/backlog correction. Implementation should use one `SettingsService`/repository record, pass validated global values into session creation/opening, and cover new-session defaults, fixed existing dimensions, interval activation, invalid input, and transactional Save failure with tests.

## Addendum — UX-A8 Platform Scaling and Input

### Issue

UX required consistent DPI scaling, high-DPI pointer mapping, 14/16px text, and 32×32px targets, while the architecture did not define logical sizing, an initial window size, or a minimum supported viewport.

### Applied resolution

- UI, pointer targets, and Field presentation use logical client pixels on macOS and Linux.
- Body text is 16 logical px, compact/numeric text is 14 logical px, and pointer targets are at least 32×32 logical px.
- The initial client area is 1280×720 logical px; the minimum supported logical viewport is 960×540. Resizing remains optional and gives extra space to the Field viewport.
- OS DPI scaling is applied once. One centralized input service normalizes platform/framebuffer pointer coordinates to logical client coordinates before UI hit testing, camera conversion, or Field cell mapping.
- Epic 6 now verifies high-DPI rendering, pointer mapping, viewport limits, and the 4-logical-pixel grid threshold on both target platforms.

### Handoff

This remains a minor documentation and backlog correction. Implementation should centralize logical-viewport/DPI conversion in the presentation layer, keep UI constants in logical pixels, and add macOS/Linux coverage for high-DPI rendering, pointer mapping, and the initial/minimum viewport contract. No runtime code was changed by this planning correction.

## Addendum — UX-A9 Camera Boundary and Out-of-Field Rendering

### Issue

UX assumed camera clamping would prevent the finite Field from disappearing, but architecture defined only coordinate conversion and visible-range clamping. The visual contract also needed to explain what appears outside the finite Field.

### Applied resolution

- Clamp camera movement so the viewport always intersects at least one in-field cell.
- Render visible space outside the finite Field with the gray `{colors.out-of-field}` fill (`#808080`).
- Treat gray out-of-field space as presentation-only: it is not a third cell state, not simulated, not editable, and not persisted as field data. A saved session preview may include its gray pixels. Out-of-field coordinates remain non-live for Life rules.
- Keep in-field dead cells black and in-field live cells white.
- Extend Epic 3/Epic 6 acceptance coverage and camera/rendering regression tests to verify the visibility boundary, gray fill, and input exclusion.

### Handoff

This is a minor documentation and backlog correction. Implementation should enforce the camera intersection invariant in the camera service, fill the viewport outside the finite Field with the gray UX token, and ensure pointer-to-cell conversion returns no cell for gray space. No runtime code was changed by this planning correction.

## Addendum — UX-A10 Source Authority and Reconciliation

### Issue

Architecture referenced the GDD, epics, and brief but omitted the UX spines even though it delegated production gates to UX. There was also no explicit rule for resolving conflicts between product scope, UX behavior, architecture, and delivery stories.

### Applied resolution

- Architecture frontmatter now lists both UX source files: `EXPERIENCE.md` and `DESIGN.md`.
- Authority order is explicit: user-approved decisions/current proposal; GDD scope and semantics; UX interaction/visual/accessibility contracts; architecture implementation details; epics/stories as delivery traceability.
- Conflicts must not be resolved silently. Record a `UX-A#` issue, use `gds-correct-course`, update all affected artifacts and decision logs, and rerun `gds-check-implementation-readiness` before the owning epic enters production.
- `project-context.md` carries the reconciled implementation rules but does not replace the source documents.

### BMad workflow improvement from `bmad-help`

Use `bmad-help` in a fresh context when the next BMad action is unclear. It should inspect the installed catalog and current artifacts, identify the current phase and required gate, and recommend the next skill with its menu code. For this project, the better sequence after a source-governance correction is:

1. Run `gds-check-implementation-readiness` (`[IR]`) to regenerate the readiness gate from the now-reconciled sources.
2. If the gate is ready, run `gds-sprint-planning` (`[SP]`) to create/update sprint tracking.
3. Use `gds-create-story` (`[CS]`) and `gds-dev-story` (`[DS]`) only after the relevant epic is implementation-ready.

### Handoff

This is a minor documentation and workflow-governance correction. No runtime code changed. Future conflicts should follow the authority order and reconciliation sequence above instead of editing one artifact in isolation.

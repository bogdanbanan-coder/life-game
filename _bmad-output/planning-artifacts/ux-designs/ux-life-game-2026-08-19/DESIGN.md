---
name: "Life Game"
description: "A restrained grayscale visual system for a native mathematical Life-study tool."
status: final
project: life-game
created: 2026-08-19
updated: 2026-08-19
sources:
  - ../../briefs/brief-life-game-2026-08-15/
  - ../../gdds/gdd-life-game-2026-08-19/
colors:
  dead-cell: '#000000'
  live-cell: '#FFFFFF'
  surface-base: '#202020'
  surface-raised: '#303030'
  control-rest: '#666666'
  control-hover: '#777777'
  control-active: '#4A4A4A'
  text-primary: '#FFFFFF'
  text-secondary: '#D0D0D0'
  border: '#8A8A8A'
  grid-line: '#5A5A5A'
  focus-ring: '#FFFFFF'
  overlay-scrim: '#000000'
typography:
  ui:
    fontFamily: "raygui default UI font"
    fontSize: 16px
    fontWeight: '400'
    lineHeight: '1.4'
  label:
    fontFamily: "raygui default UI font"
    fontSize: 14px
    fontWeight: '400'
    lineHeight: '1.3'
  numeric:
    fontFamily: "raygui default monospace-capable UI font"
    fontSize: 14px
    fontWeight: '400'
    lineHeight: '1.3'
rounded:
  none: 0px
  sm: 2px
spacing:
  '1': 4px
  '2': 8px
  '3': 12px
  '4': 16px
  '5': 24px
components:
  button:
    fill: '{colors.control-rest}'
    fillHover: '{colors.control-hover}'
    fillActive: '{colors.control-active}'
    text: '{colors.text-primary}'
    border: '{colors.border}'
    radius: '{rounded.none}'
    minHeight: 32px
    paddingInline: '{spacing.3}'
  panel:
    fill: '{colors.surface-base}'
    border: '{colors.border}'
    radius: '{rounded.none}'
    padding: '{spacing.3}'
  field-grid:
    deadFill: '{colors.dead-cell}'
    liveFill: '{colors.live-cell}'
    gridLine: '{colors.grid-line}'
    selectionLine: '{colors.live-cell}'
    selectionCounterline: '{colors.dead-cell}'
    lineWidth: 1px
  tool-pool:
    fill: '{colors.surface-base}'
    border: '{colors.border}'
    gap: '{spacing.1}'
    padding: '{spacing.1}'
  session-card-list:
    fill: '{colors.surface-base}'
    gap: '{spacing.3}'
    padding: '{spacing.3}'
  session-card:
    fill: '{colors.surface-raised}'
    border: '{colors.border}'
    text: '{colors.text-primary}'
    radius: '{rounded.none}'
    padding: '{spacing.3}'
  dialog:
    fill: '{colors.surface-raised}'
    scrim: '{colors.overlay-scrim}'
    border: '{colors.border}'
    text: '{colors.text-primary}'
    radius: '{rounded.sm}'
    padding: '{spacing.4}'
  text-field:
    fill: '{colors.surface-base}'
    border: '{colors.border}'
    text: '{colors.text-primary}'
    radius: '{rounded.none}'
    minHeight: 32px
  numeric-field:
    fill: '{colors.surface-base}'
    border: '{colors.border}'
    text: '{colors.text-primary}'
    radius: '{rounded.none}'
    minHeight: 32px
  bank-list:
    fill: '{colors.surface-base}'
    rowFill: '{colors.surface-raised}'
    border: '{colors.border}'
    gap: '{spacing.1}'
  staged-figure-overlay:
    liveFill: '{colors.live-cell}'
    deadFill: '{colors.dead-cell}'
    outline: '{colors.live-cell}'
    opacity: '55%'
  status-message:
    fill: '{colors.surface-raised}'
    border: '{colors.focus-ring}'
    text: '{colors.text-primary}'
    radius: '{rounded.none}'
    padding: '{spacing.2}'
---

# Life Game — Design Spine

## Brand & Style

Life Game looks like a mathematical instrument, not a narrative game. The Field is a visibly lined matrix. A dead cell is black; a live cell is white. Menus and buttons are gray, simple, flat, and functional. `[ASSUMPTION]` The interface adds no decorative color, illustration, texture, gradient, animation, or game-like reward treatment.

raygui 5.0 supplies the immediate-mode control foundation. This spine overrides raygui's palette, spacing, typography roles, square shape language, and component-state appearance; raygui retains its rendering and interaction mechanics. `[ASSUMPTION]` The embedded raygui font is the starting UI font. A readable bundled fallback is required only if the font renders inconsistently across platforms or lacks required Unicode glyphs.

## Colors

| Role | Token | Rule |
|---|---|---|
| Dead cell / Field base | `{colors.dead-cell}` | Stakeholder anchor. Dead cells are black. |
| Live cell / strongest mark | `{colors.live-cell}` | Stakeholder anchor. Live cells are white. `[ASSUMPTION]` White also supplies the strongest focus and selection mark without adding a hue. |
| Menu base | `{colors.surface-base}` | `[ASSUMPTION]` Exact dark gray chosen to keep gray menus distinct from the black Field. |
| Raised surface | `{colors.surface-raised}` | `[ASSUMPTION]` Exact gray for cards, dialogs, and list rows. |
| Button rest | `{colors.control-rest}` | `[ASSUMPTION]` Exact middle gray for simple buttons. |
| Button hover | `{colors.control-hover}` | `[ASSUMPTION]` Exact gray; hover is lighter as well as pointer-cued. |
| Button active / selected | `{colors.control-active}` | `[ASSUMPTION]` Exact gray; selected controls additionally use a white outline so selection never depends on shade alone. |
| Primary text | `{colors.text-primary}` | `[ASSUMPTION]` White text and primary labels. |
| Secondary text | `{colors.text-secondary}` | `[ASSUMPTION]` Exact light gray for supporting metadata only. |
| Structural border | `{colors.border}` | `[ASSUMPTION]` Exact gray for control, card, and dialog boundaries. |
| Grid line | `{colors.grid-line}` | `[ASSUMPTION]` Exact gray. Lines appear only when the current zoom makes cells useful to inspect. |
| Focus / state outline | `{colors.focus-ring}` | `[ASSUMPTION]` White ring or line paired with a label or line style. |
| Modal scrim | `{colors.overlay-scrim}` | `[ASSUMPTION]` Black at reduced opacity behind in-application dialogs. |

The contrast ratio between live and dead cells is 21:1. `[ASSUMPTION]` Text/background combinations must meet at least 4.5:1; focus rings and load-bearing boundaries must meet at least 3:1 against adjacent surfaces. Selection, staging validity, errors, and active-tool state always add line style or text and never rely on gray value alone.

## Typography

There is one functional sans-serif UI voice and one fixed-width numeric role. No display face, decorative type, italic emphasis, or oversized title typography is needed.

`[ASSUMPTION]` `{typography.ui}` is the 16px base for dialogs, settings, and session names; `{typography.label}` is the 14px compact-control role; `{typography.numeric}` is the 14px fixed-width role for dimensions and intervals. All three begin with the raygui default UI font. If its glyph coverage is insufficient, replacement must remain plain, readable, and metrically stable on both macOS and Linux.

Labels use sentence case except literal controls `Live`, `Die`, `Pause`, `Resume`, `Highlight`, `Bank`, `Move`, `+`, `−`, and `Exit`. Numeric fields use tabular or fixed-width digits so edited values do not shift.

## Layout & Spacing

The Field owns the available canvas. Its cells align exactly to the simulation grid; UI panels never overlap editable cells except the compact upper-right `tool-pool` and modal overlays. The Start Menu uses one horizontal `session-card-list`. `[ASSUMPTION]` Dialogs form one centered layer and never stack.

`[ASSUMPTION]` The spacing scale is 4 / 8 / 12 / 16 / 24px (`{spacing.1}` through `{spacing.5}`). Compact Field controls use 4px internal gaps; forms use 8–12px gaps between related controls; separate form groups use 16–24px gaps. `[ASSUMPTION]` The minimum pointer target is 32×32px to preserve the requested small controls while keeping a stable desktop target.

The grid line belongs to the Field geometry, not to page decoration. `[ASSUMPTION]` Render `{colors.grid-line}` only when a cell is at least 4 screen pixels wide and tall; hide the line below that threshold to prevent gray noise from obscuring cell state.

## Elevation & Depth

`[ASSUMPTION]` Depth comes from borders, tonal separation, and the modal scrim. There are no shadows, bevels, glow, glass, or textured surfaces. A dialog uses `{colors.overlay-scrim}` at 55% opacity behind it; only one dialog may be active.

The `staged-figure-overlay` is the sole translucent content layer. Its transparency exists to compare the saved rectangle with the underlying Field, not as decorative depth.

## Shapes

Field cells and controls are rectilinear. `{rounded.none}` is the default for buttons, panels, fields, lists, and cards. `[ASSUMPTION]` `{rounded.sm}` (2px) is permitted only on the outer edge of an in-application dialog so the modal boundary is legible; no pills, circles, or ornamental silhouettes are introduced.

`[ASSUMPTION]` Solid outlines mean selectable or valid; dashed outlines mean invalid placement. The difference is structural and remains readable without color.

## Components

The names in this table are the canonical component identifiers shared with `EXPERIENCE.md`.

`[ASSUMPTION]` Except where a row restates the stakeholder's requirements for black/white cells, gray simplicity, or a lined Field, or cites an upstream requirement, the row's exact border width, target size, opacity, line style, padding, and state shade are fast-path proposals.

| Component | Visual specification |
|---|---|
| `button` | Flat `{colors.control-rest}` rectangle with `{colors.text-primary}`. Hover uses `{colors.control-hover}`; pressed uses `{colors.control-active}`. `[ASSUMPTION]` A persistent active mode keeps the active fill plus a 2px `{colors.focus-ring}` inset outline; minimum height is 32px; no icon is required where a short text label exists. |
| `panel` | `{colors.surface-base}` with a 1px `{colors.border}` boundary and square corners. Used only to group controls or form content. |
| `field-grid` | `{colors.dead-cell}` cells, `{colors.live-cell}` cells, and 1px `{colors.grid-line}` at useful zoom. `[ASSUMPTION]` Selection is a double white/black rectangle around the inclusive cell region so its structure remains visible across both cell states. |
| `tool-pool` | Compact `{colors.surface-base}` group in the upper-right of Field, with 1px `{colors.border}` and `{spacing.1}` gaps. All controls remain visually stable when labels change between Pause and Resume. |
| `session-card-list` | Single horizontal strip on the Start Menu, using `{spacing.3}` between cards. No carousel decoration, pagination dots, or cover-flow effects. |
| `session-card` | `{colors.surface-raised}` rectangle with preview, session name, and dedicated Rename/Delete `button` controls. The preview has a 1px `{colors.border}` and preserves Field black/white values. |
| `dialog` | Centered `{colors.surface-raised}` surface over `{colors.overlay-scrim}`. Title, message/form content, then actions. Confirmation and error variants use explicit words and line structure, not chromatic fills. |
| `text-field` | `{colors.surface-base}` input with `{colors.text-primary}`, 1px `{colors.border}`, visible caret, and white focus outline. Validation text sits directly below. |
| `numeric-field` | Same frame as `text-field`, using `{typography.numeric}` and an adjacent unit or dimension label. Invalid text is retained for correction rather than silently coerced. |
| `bank-list` | `{colors.surface-base}` list with `{colors.surface-raised}` rows separated by `{spacing.1}`. Each row shows the unique figure name and dedicated Rename/Delete `button` controls. |
| `staged-figure-overlay` | Saved live/dead bitmap at `[ASSUMPTION]` 55% opacity with a white outer outline. `[ASSUMPTION]` Valid placement uses a solid outline; invalid placement uses a dashed outline plus the text `Outside field`. Underlying cells remain visible. |
| `status-message` | Compact `{colors.surface-raised}` rectangle with a white border and direct text. Used for visible success, failure, invalid-placement, and busy feedback; it does not animate or disappear before it can be read. |

## Do's and Don'ts

| Do | Don't |
|---|---|
| Keep dead cells black and live cells white. | Tint cell states or introduce a decorative palette. |
| Keep the Field visibly lined at useful zoom. | Draw dense grid noise when zoomed too far out. |
| Use gray, flat, simple menus and buttons. | Add gradients, shadows, textures, glows, or ornamental icons. |
| Pair gray-state changes with labels, borders, or line styles. | Use shade alone for selection, validity, active mode, or errors. |
| Preserve exact cell alignment in selection and staged figures. | Smooth, interpolate, rotate, or visually distort saved rectangles. |
| Let raygui provide immediate-mode mechanics while these tokens provide the visual delta. | Allow platform or raygui defaults to change the black/white Field or gray control identity. |
| Treat this spine and `EXPERIENCE.md` as authoritative. | Let a later mock, wireframe, or import override the spines without an explicit update. |

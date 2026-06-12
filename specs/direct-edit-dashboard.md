# Direct-Edit Dashboard — Spec

## Objective
Replace the modal "Edit layout" workflow on the Rotorboard dashboard with always-on
direct manipulation. Today the user must click "Edit layout" before moving, resizing,
adding, or deleting widgets, which makes quick layout tweaks (especially moving cards)
inconvenient. After this change, cards are draggable and resizable at all times, each
card is editable via a right-click context menu, and new cards are added from a header
button through a popup followed by a ghost-preview placement. Success: a user can move,
resize, edit, duplicate, delete, and add widgets without ever entering a mode.

## Requirements

### Must-have

1. **Remove edit mode.** The "Edit layout"/"Done" header button is removed, along with
   the `editMode` property on `AppController` and the `editMode` plumbing through
   `DashboardPage.qml`, `DashboardGrid.qml`, and `DashboardWidgetSlot.qml`. No modal
   layout-editing state remains.
2. **Remove the side widget palette.** `WidgetPalette.qml` (the side panel with motor
   SpinBox, metric ComboBox, type buttons, and delete button) is removed from the page;
   the grid always occupies the full content width.
3. **Always-on drag.** Pressing and dragging anywhere on a card's body moves it, at all
   times. The existing snap-to-grid behavior, drag z-ordering, and the swap-with-occupant
   behavior in `DashboardLayoutModel::placeWidget` are preserved. Dragging a card must
   still lock Flickable scrolling for the duration of the drag (the existing
   `dragLockCount` mechanism or equivalent).
4. **Always-on resize.** Each card has a bottom-right resize handle, visible when the
   cursor hovers over the card, that resizes the card with the existing snap/clamp
   behavior. Resizes that would overlap another card or leave the grid are rejected
   (existing `resizeWidget` validation).
5. **Right-click context menu on cards.** Right-clicking a card opens a context menu
   with exactly three actions:
   - **Edit…** — opens the widget popup (req. 7) pre-filled with the card's current
     type, motor, and metric; confirming applies the changes to that card in place.
   - **Duplicate** — creates a copy of the card (same type, motor, metric, spans),
     placed at the first free position that fits (existing `firstFreePlacement`
     fallback in `addWidget`). If no free spot fits, nothing is added.
   - **Delete** — removes the card from the layout.
6. **"+ Add widget" header button.** A button labelled to indicate adding a widget sits
   in the header row (alongside Pause and the source label), styled consistently with
   the existing header buttons.
7. **Widget popup.** Clicking the Add button opens a popup/dialog containing:
   - widget type selection (value tile, chart, motor summary),
   - motor id selection (1–32),
   - metric selection (rpm, voltage, current, temperatureCelsius, pwm) — disabled or
     hidden when type is motor summary (summaries have no single metric),
   - a single confirm action and a cancel action. There is no separate "Place" button.
   The same popup component is reused for the right-click **Edit…** flow (pre-filled,
   confirm applies instead of placing).
8. **Ghost-preview placement.** Confirming the popup in Add mode closes it and attaches
   a semi-transparent ghost preview of the new card (at the type's default span,
   `defaultColSpan`/`defaultRowSpan`) to the cursor, snapped to the grid cell under the
   cursor. Left-clicking while the ghost is over a valid, non-overlapping position
   commits the widget there. Clicking over an invalid/occupied position does not commit
   (the ghost should indicate validity, e.g. by tint).
9. **Ghost cancellation.** While a ghost placement is active, pressing Escape,
   right-clicking, or left-clicking outside the dashboard grid (header bar, page
   margins, any menu) cancels the placement without adding a widget.
10. **Type switch auto-resizes; relocate if blocked.** When the Edit popup changes a
    card's type, the card's span is set to the new type's default span. If that span
    does not fit at the card's current position (grid bounds or overlap with other
    cards), the card relocates to the first free position that fits the full default
    span. If no position fits, the type change is rejected and the card is unchanged
    (the popup may simply close with no effect, but must not corrupt the layout).
11. **Persistence unchanged.** All layout changes (move, resize, add, edit, duplicate,
    delete) persist via the existing `QSettings` JSON mechanism and restore on restart.
12. **Empty/idle text updated.** Grid placeholder text no longer references edit mode or
    the palette (e.g. "Add widgets from the palette" must be replaced or removed).

### Nice-to-have (explicitly deferred)
- Hover fade-in animations for the resize handle.
- Keyboard-driven placement or movement of cards.
- Multi-select or rubber-band selection.
- Undo/redo of layout changes.

## Constraints
- Tech stack stays as-is: Qt 6 / QML frontend, C++ `DashboardLayoutModel`
  (QAbstractListModel) backend, CMake build. No new third-party dependencies.
- `DashboardLayoutModel`'s persisted JSON schema (id/type/motorId/metric/col/row/
  colSpan/rowSpan) must not change; existing saved layouts must load unmodified.
- Existing telemetry behavior (value/chart/summary tiles, stale/warning states,
  Pause/Resume freezing) must be untouched.
- Existing model-level validation (`isValidPlacement`, `overlaps`,
  `firstFreePlacement`, swap-on-place) remains the single source of truth for layout
  legality; QML must not bypass it.
- Auto-seeding of widgets for newly discovered motors (`ensureMotorWidgets`/
  `seedForMotor`) keeps working.
- Windows is the primary dev/build platform (`build.ps1` / `build.bat`).
- Existing tests in `tests/` must keep passing; model changes (duplicate, type-switch
  relocation) get unit coverage there.

## Edge Cases
- **Drag vs. scroll:** a vertical drag on a card moves the card, not the Flickable;
  scrolling via scrollbar/trackpad on empty grid space still works.
- **Drag release out of bounds:** releasing a drag at a position that would leave the
  grid snaps/clamps back to a legal position (existing snap functions).
- **Drop on occupied multi-cell region:** dropping where the existing model logic
  cannot place (occupied by a larger card, no single-cell swap) leaves the card at its
  original position.
- **Duplicate with full grid:** if `firstFreePlacement` finds no room, Duplicate adds
  nothing and the layout is unchanged.
- **Type switch with no room anywhere:** card keeps its old type, span, and position.
- **Edit popup on a motor summary:** metric field is hidden/disabled; switching from
  summary to value/chart requires picking a metric (default to first metric if none).
- **Ghost active + telemetry updates:** live tile updates continue; ghost stays
  attached to the cursor.
- **Ghost over partially out-of-grid cell:** position is clamped so the ghost's full
  span stays inside the grid before validity is evaluated.
- **Right-click during drag or resize:** ignored (no context menu mid-gesture).
- **Two popups:** opening the Add popup while a ghost is active cancels the ghost first.
- **Empty dashboard:** placeholder text shows; Add button flow still works.

## Definition of Done
- [ ] "Edit layout" button and `editMode` are gone from the codebase (no remaining
      references in `qml/` or `src/`).
- [ ] `WidgetPalette.qml` is deleted and no longer referenced; grid spans the full width.
- [ ] A card can be moved by dragging its body with no prior mode toggle, and the new
      position persists across app restart.
- [ ] A card can be resized via the bottom-right handle with no prior mode toggle, and
      the new size persists across restart.
- [ ] Dragging onto a single-cell occupied position swaps the two cards (existing
      behavior verified still working).
- [ ] Right-clicking a card shows Edit…, Duplicate, Delete; each action works as
      specified (Edit pre-fills and applies; Duplicate places a copy at the first free
      spot; Delete removes the card).
- [ ] The Add button opens the popup; confirming starts a ghost preview that follows
      the cursor; left-click on a free cell commits the widget at the type's default
      span.
- [ ] Ghost placement is cancelled by each of: Escape, right-click, and left-click
      outside the grid — with no widget added in each case.
- [ ] Changing a card's type via Edit… resizes it to the new default span, relocating
      it to the first fitting free position when blocked; with no room anywhere the
      card is unchanged.
- [ ] The metric field is hidden/disabled for motor summary in both Add and Edit modes.
- [ ] Existing tests pass, plus new model-level tests covering duplicate placement and
      type-switch relocation (including the no-room case).
- [ ] The project builds cleanly with the standard build script (`build.ps1`).

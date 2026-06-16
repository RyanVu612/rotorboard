# Dashboard Integration Tests — Spec

## Objective
Add integration tests for the configurable widget dashboard so regressions in the
QML dashboard wiring are caught before release. The tests should exercise the real
dashboard components (`DashboardPage`, `DashboardGrid`, `WidgetDialog`, and
`DashboardWidgetSlot`) with the real `DashboardLayoutModel`, while using a minimal
test controller and no live telemetry source.

Recommended approach: add a Qt Quick integration test executable that runs under
`ctest`, loads the dashboard QML with `QQmlApplicationEngine` or `QQuickView`, injects
test doubles for the controller/telemetry surface, and drives interactions through
QML object functions/properties plus Qt input events. This keeps the tests close to
real app behavior without depending on screenshots, network input, serial devices, or
manual GUI inspection.

## Requirements

### Must-have

1. **Create a QML integration test target.** Add a test executable such as
   `rotorboard_dashboard_integration_tests` and register it with CTest. The target
   must link the Qt modules required to load and interact with the QML dashboard
   (`Core`, `Gui`, `Quick`, `QuickControls2`, `Qml`, and `Test`, as needed).
2. **Run offscreen/minimal.** The test target must be able to run with an offscreen or
   minimal Qt platform plugin. It must not require a visible desktop session. If one
   platform is more reliable for the local Qt version, prefer it, but document the
   required environment variable in the test setup.
3. **Use the real dashboard QML.** Tests must load the same QML files used by the app,
   not copied or test-only versions. Object lookup may require adding stable
   `objectName` values to existing QML controls and components.
4. **Use the real layout model.** Tests must inject a real `DashboardLayoutModel` so
   placement, overlap, resize, duplicate, delete, edit, and default-span behavior use
   production C++ logic.
5. **Use a minimal telemetry stub.** Tests should not start telemetry sources or use
   playback data. The injected telemetry object only needs the properties and methods
   QML expects, such as `sampleRevision`, `rowCount()`, `motorIdAt()`,
   `valueForMetric()`, `isMotorStale()`, `warningLevelForMotor()`, and
   `statusForMotor()`.
6. **Use a minimal controller stub.** Tests should provide the controller properties
   and methods consumed by `DashboardPage.qml`, including `telemetryModel`,
   `layoutModel`, `chartsFrozen`, `toggleChartsFrozen()`, `sourceLabel`,
   `linkMonitored`, `linkStateLevel`, `linkStatusText`, and any settings-dialog
   properties required for the page to load.
7. **Assert QML state, not pixels.** Tests must verify behavior through QML object
   properties, model row data, signal observation, and method results. Screenshot or
   pixel assertions are out of scope.
8. **Isolate settings.** Integration tests must use a test-only organization and
   application name, clear `QSettings` before each test, and avoid relying on persisted
   layouts for pass/fail behavior.
9. **Avoid persistence coverage.** Restart/reload persistence should remain covered by
   model/unit tests. These integration tests may clear settings and inspect in-memory
   model state only.
10. **Keep existing tests passing.** Existing model, controller, telemetry, logger, and
    parser tests must continue to run under `ctest`.

### Dashboard behaviors to cover

1. **Page loads with an empty layout.** Loading `DashboardPage.qml` with an empty
   layout model succeeds offscreen, the dashboard grid exists, and the empty placeholder
   is visible.
2. **Add dialog opens from the header.** Activating the `+ Add widget` control opens
   `WidgetDialog` in add mode with default type `value`, motor `1`, and metric `rpm`.
3. **Add confirm starts ghost placement.** Confirming the dialog in add mode closes the
   dialog, sets `DashboardGrid.ghostActive` to true, sets the selected type/motor/metric,
   and computes the correct default span from `DashboardLayoutModel`.
4. **Ghost tracks grid cell state.** Calling or driving `updateGhost()` over a grid
   coordinate updates `ghostCol`, `ghostRow`, `ghostVisible`, and the ghost preview's
   valid/invalid placement state based on `layoutModel.canPlace()`.
5. **Ghost commit adds a widget.** Committing a ghost over a free grid cell adds one
   widget to the layout model with the selected type, motor, metric, position, and
   default span, then clears ghost state.
6. **Ghost does not commit over occupied space.** With an existing widget occupying the
   target region, committing a ghost leaves `rowCount()` unchanged and keeps ghost
   placement active for another attempt.
7. **Ghost cancellation works.** Escape, right-click, and left-click outside the grid
   each cancel ghost placement without adding a widget. At minimum, cover these through
   the QML exposed functions/properties or Qt input events against the overlay.
8. **Add while ghost active resets the flow.** Activating the Add control while a ghost
   is active cancels the old ghost and opens a fresh add dialog.
9. **Card drag updates layout.** Dragging a widget slot body to a new legal grid cell
   updates the layout model through `placeWidget()`, clears the slot's `dragging` state,
   and returns `DashboardGrid.interactive` to true.
10. **Drag locks scrolling.** While a slot drag is active, `dragLockCount` is positive
    and the grid `Flickable.interactive` property is false. Releasing or canceling the
    drag restores both values.
11. **Single-cell swap still works.** Dragging a single-cell widget onto another
    single-cell widget swaps their positions through the real layout model behavior.
12. **Invalid drag leaves layout legal.** Releasing a dragged widget where the model
    rejects placement leaves the widget at a legal position and does not create overlap.
13. **Resize handle updates layout.** Dragging the bottom-right resize handle changes
    `colSpan` and `rowSpan` through `resizeWidget()` when the target span is valid.
14. **Invalid resize is rejected.** Resizing into another widget or outside the grid
    leaves the model's span unchanged.
15. **Right-click menu opens for a card.** Right-clicking a card when not dragging or
    resizing opens the context menu with actions for Edit, Duplicate, and Delete.
16. **Edit action pre-fills dialog.** Choosing Edit opens `WidgetDialog` in edit mode
    with the selected widget id, type, motor id, and metric.
17. **Edit apply updates widget.** Confirming the edit dialog updates the existing
    layout row without adding a new one. Type switches must apply default spans and use
    the existing relocation/rejection behavior from `DashboardLayoutModel`.
18. **Metric control follows type.** The metric field is visible/enabled for value and
    chart widgets, and hidden or disabled for motor summary widgets in both add and
    edit flows.
19. **Duplicate action adds a copy.** Choosing Duplicate creates a second widget with
    matching type, motor, metric, and span at the first free position.
20. **Duplicate full-grid no-ops.** If no placement fits, Duplicate leaves `rowCount()`
    unchanged.
21. **Delete action removes a widget.** Choosing Delete removes the target widget from
    the layout model and updates the repeater.
22. **Pause button toggles chart freeze.** Activating Pause calls the controller stub,
    toggles `chartsFrozen`, updates the header label between Pause and Resume, and
    propagates the value into dashboard widgets.
23. **Telemetry-less widgets stay loadable.** Value, chart, and motor-summary widgets
    render/load with the telemetry stub and no telemetry rows. Missing metric values
    should resolve to stable default display state instead of throwing QML errors.
24. **Auto-seeding remains wired.** When the telemetry stub reports inserted/reset rows
    with motor ids, `DashboardGrid.ensureMotorWidgets()` seeds missing motor-summary
    widgets through the real layout model.
25. **No QML warnings for tested flows.** The integration test harness should fail on
    unexpected QML warnings/errors emitted while loading or exercising the dashboard.

## Constraints

- Do not introduce new third-party test frameworks.
- Do not depend on screenshots, OCR, pixel colors, or manual visual inspection.
- Do not start the production app executable for these tests unless the Qt Quick test
  harness proves insufficient.
- Keep test data small and deterministic.
- Prefer adding stable `objectName` properties over brittle child-index traversal.
- Do not alter the persisted widget JSON schema.
- Do not broaden telemetry, MAVLink, DroneCAN, serial, or CSV playback behavior as part
  of this task.

## Recommended Implementation Plan

1. Add stable `objectName` values to dashboard QML elements the tests must find:
   dashboard page root, add button, pause button, dashboard grid, widget dialog, ghost
   overlay, ghost preview, placeholder text, card menu, menu actions, and widget slots.
2. Add a test support controller class exposing the minimal `DashboardPage.controller`
   contract via `Q_PROPERTY` and `Q_INVOKABLE` methods.
3. Add a test support telemetry class exposing the minimal methods/properties used by
   `DashboardGrid.qml` and emitting row/model change signals.
4. Create a Qt Quick integration test executable that initializes `QGuiApplication`,
   sets a test-only `QSettings` identity, configures `QT_QPA_PLATFORM` for
   offscreen/minimal execution when not already set, loads `DashboardPage.qml`, and
   injects the controller.
5. Add helpers for finding named QML objects, waiting for polish/events, reading layout
   model rows by widget id, and sending mouse/key events to QQuickItems.
6. Implement the dashboard behavior tests listed above, grouping related flows where
   it improves runtime without hiding assertions.
7. Add the target to `CMakeLists.txt`, register it with CTest, and document any local
   run command needed for the platform plugin.

## Edge Cases

- A dashboard with zero widgets must still allow Add and ghost placement.
- A ghost over an occupied or out-of-bounds region must visibly remain active in QML
  state but must not mutate the model.
- Drag and resize cancel paths must restore interaction locks.
- Context menu actions must target the widget that was right-clicked, not whichever
  widget was most recently edited or added.
- Editing a motor summary into a value/chart widget must supply a valid default metric.
- Editing a value/chart into a motor summary must clear the metric stored in the model.
- Auto-seeding should not create duplicate widgets for a motor that already has one.
- Offscreen/minimal platforms may not support every window activation behavior; tests
  should prefer direct item events or exposed QML methods where platform differences
  would make a mouse-only test flaky.

## Definition of Done

- [x] A dashboard integration test target exists and is registered with CTest.
- [x] The target runs with an offscreen or minimal Qt platform plugin.
- [x] Tests load the real dashboard QML and inject a real `DashboardLayoutModel`.
- [x] Tests use a minimal telemetry stub and do not start telemetry sources.
- [x] Tests assert QML/model properties rather than screenshots or pixels.
- [x] All dashboard behaviors listed in this spec have integration coverage.
- [x] Unexpected QML warnings/errors fail the integration test target.
- [x] Persisted layout restart behavior remains covered by model/unit tests, not this
      integration suite.
- [x] Existing test targets continue to pass.

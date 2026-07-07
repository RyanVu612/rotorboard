# Battery Summary Widget — Spec

## Objective

Rotorboard currently only surfaces per-motor ESC telemetry (RPM, voltage, current,
temperature, PWM). It has no visibility into overall battery pack status. Pilots
watching the dashboard during flight need to see pack voltage, current draw, %
remaining, temperature, consumed capacity, and per-cell voltages at a glance,
without leaving the existing dashboard.

This adds a **Battery summary** widget card — a new widget type addable to the
existing dashboard grid alongside Value tile, Chart, and Motor summary — that
displays live telemetry for one battery instance in a compact layout consistent
with the existing `MotorSummaryCard`.

Success: a pilot can add a Battery summary card for each battery on the vehicle
and read pack health (voltage, current, % remaining, temperature, cell balance)
at a glance, with the same color-coded warning behavior already used for motors.

## Requirements

### Telemetry parsing
1. `MavlinkTelemetrySource` parses `MAVLINK_MSG_ID_BATTERY_STATUS` in addition to
   the existing `ESC_STATUS` handling.
2. A new `BatteryTelemetry` struct (parallel to `MotorTelemetry`) carries:
   `batteryId`, `voltage` (V, sum from valid cells or pack voltage), `current` (A),
   `batteryRemaining` (%, -1 if unknown), `temperatureCelsius` (NaN/sentinel if
   unknown), `currentConsumedMah`, `cellVoltages` (list of up to 10 valid cell
   voltages in V, trimmed of `UINT16_MAX` filler entries), `timestampMillis`.
3. `TelemetrySource` gains a `batteryTelemetryReceived(const BatteryTelemetry&)`
   signal, implemented by `MavlinkTelemetrySource` and `FakeTelemetrySource`.
   `DroneCanTelemetrySource` and `CsvPlaybackSource` do not implement it (no
   battery data available from those sources in v1).
4. `FakeTelemetrySource` emits telemetry for exactly one simulated battery
   (`batteryId = 0`): plausible voltage/current/temperature, 6 simulated cell
   voltages, and `batteryRemaining` that slowly decreases over the session
   instead of jumping randomly each tick.
5. `TelemetryManager` and a new `BatteryTelemetryModel` (parallel to
   `MotorTelemetryModel`) track the latest sample and short rolling history per
   `batteryId`, and mark a battery stale using the same
   `kStaleThresholdMillis` (2000ms) convention as motors.

### Multiple batteries
6. Multiple battery instances are supported. Each `BATTERY_STATUS` message's
   `battery_id` field identifies the instance; the model keys samples by
   `batteryId` the same way `MotorTelemetryModel` keys by `motorId`.

### Widget integration
7. `WidgetDialog` gains a `"Battery summary"` entry (`type: "batterySummary"`) in
   `widgetTypes`, following the same UI pattern as `"Motor summary"`: picking
   this type hides the metric combo and shows an instance selector (reusing the
   existing `motorId`-style selector, populated from known battery IDs instead
   of motor IDs).
8. `DashboardLayoutModel` accepts `"batterySummary"` as a valid `addWidget`/
   `editWidget` type, storing the battery instance in the existing generic
   per-entity id field (the same field used as `motorId` today), consistent
   with how `"motorSummary"` widgets are stored. Existing saved layouts (which
   contain only motor-keyed widgets) continue to load unchanged.
9. `DashboardGrid` renders a `BatterySummaryCard` for `"batterySummary"` widget
   entries, sized like other cards in the grid.

### Battery Summary Card content
10. The card is compact and shows, per battery instance:
    - Header: "Battery {batteryId}" + a `StatusBadge` warning indicator.
    - Pack voltage (V), current (A), % remaining, temperature (°C), consumed
      capacity (mAh) — laid out in the same label/value grid style as
      `MotorSummaryCard`'s metric grid.
    - Per-cell voltage breakdown: each valid cell's voltage shown compactly
      (e.g. a small grid of "Cell N: X.XX V" entries), only for cells actually
      reported (no fixed 10-slot display when the pack has fewer cells).
11. Sparkline history on the card is nice-to-have, not required for v1 — a
    static value display satisfies this spec.
12. Any field the source reports as "unknown" (`current_battery == -1`,
    `battery_remaining == -1`, `temperature == INT16_MAX`) displays as
    `"—"` / `"N/A"` rather than a misleading `0` or garbage value.

### Warnings
13. A new `BatteryWarningEvaluator` (parallel to `MotorWarningEvaluator`)
    computes a `WarningLevel` (`Ok` / `Warning` / `Critical` / `Stale`) from
    hardcoded thresholds:
    - `Critical`: `batteryRemaining` (if known) `< 10%`, or any known cell
      voltage `< 3.3V`.
    - `Warning`: `batteryRemaining` (if known) `< 20%`, or any known cell
      voltage `< 3.5V`.
    - `Stale`: no sample within `kStaleThresholdMillis`, same as motors.
    - Unknown fields (`-1`/sentinel) are excluded from threshold checks rather
      than treated as 0.
14. The card's `StatusBadge` and border color reflect this warning level using
    the same visual convention as `MotorSummaryCard`.

### Explicitly deferred (not in v1)
- DroneCAN battery telemetry.
- CSV playback / CSV logging of battery data.
- Configurable warning thresholds (hardcoded only, matching current motor
  warning pattern).
- `voltages_ext` (cells 11–14) — only the standard 10-cell `voltages[]` array
  is parsed.
- Sparkline/history charting on the battery card.
- Any changes to `MAVLINK_MSG_ID_SYS_STATUS` handling.

## Constraints

- Follow the existing architecture: C++/Qt backend models feeding QML via
  Q_PROPERTY/roles, mirroring the `MotorTelemetry` → `MotorTelemetryModel` →
  `MotorSummaryCard` pipeline for `BatteryTelemetry` → `BatteryTelemetryModel`
  → `BatterySummaryCard`.
- No new top-level tab/navigation UI — this stays within the existing single
  dashboard page and its widget grid.
- No new third-party dependencies; `mavlink_msg_battery_status.h` is already
  vendored under `third_party/mavlink_c/common/`.
- Existing saved dashboard layouts (JSON) must continue to load without
  migration steps.
- Match existing QML visual style (colors, spacing, fonts) used by
  `MotorSummaryCard` and `StatusBadge`.

## Edge Cases

- **No battery telemetry received yet**: card shows a stale/no-data state
  (same visual treatment as a stale motor card), not a crash or blank card.
- **Battery instance removed from the add-widget list**: if a saved card
  references a `batteryId` no longer reported by the current source, it
  behaves like a stale/no-data card (mirrors current motor behavior for
  motors that stop reporting).
- **Fields marked "unknown" by the protocol** (`-1` / `INT16_MAX` sentinels):
  rendered as `"—"`, excluded from warning-threshold checks.
- **Fewer than the max cells reported**: only actual reported cells are shown;
  no display of phantom/padding cells.
- **Switching telemetry sources** (e.g. MAVLink → DroneCAN, which has no
  battery data): existing battery cards go stale/no-data rather than showing
  frozen last-known values indefinitely (consistent with the 2s staleness
  convention already used for motors).
- **Multiple `BATTERY_STATUS` messages for different `battery_id`s arriving
  in the same session**: each is tracked and displayed independently; adding
  a card for `battery_id = 1` does not affect the `battery_id = 0` card.

## Definition of Done

- [ ] `MavlinkTelemetrySource` parses `BATTERY_STATUS` and emits
      `batteryTelemetryReceived` with correctly decoded fields (voltage,
      current, %, temperature, consumed mAh, cell voltages), including correct
      handling of the three "unknown" sentinel values.
- [ ] `FakeTelemetrySource` emits a plausible, slowly-draining simulated
      battery on `batteryTelemetryReceived`.
- [ ] `BatteryTelemetryModel` tracks latest sample and staleness per
      `batteryId`, unit-tested the same way `MotorTelemetryModel` is tested.
- [ ] `BatteryWarningEvaluator` unit tests cover Ok/Warning/Critical/Stale
      transitions, including the "unknown value excluded from threshold"
      behavior.
- [ ] `WidgetDialog` offers "Battery summary" as an addable widget type with a
      battery-instance selector, and edit mode round-trips correctly.
- [ ] `DashboardLayoutModel` persists and reloads `"batterySummary"` widgets;
      an existing layout file with only motor widgets still loads unchanged
      (regression check).
- [ ] `BatterySummaryCard` renders pack voltage/current/%/temperature/consumed
      mAh and per-cell voltages in a compact layout, shows "—" for unknown
      fields, and reflects the correct `StatusBadge` warning color.
- [ ] Adding two Battery summary cards for two different battery IDs shows
      independent, correctly-attributed data on each.
- [ ] `rotorboard_dashboard_integration_tests` (or equivalent) covers adding,
      editing, and persisting a Battery summary widget end-to-end.
- [ ] Manual check: running the app against the Fake source shows a working,
      updating Battery summary card with no console warnings/errors.

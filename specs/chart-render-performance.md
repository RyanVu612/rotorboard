# Chart Render Performance — Spec

## Objective

Fix severe UI framerate drops that occur whenever one or more chart (Sparkline)
widgets are visible on the dashboard. The target user is anyone running rotorboard
with live or simulated telemetry at the default 100 ms data rate.

Success: the dashboard stays at a smooth, responsive framerate (≥ 30 fps visually)
regardless of how many chart tiles are on screen, with no perceptible input lag.

---

## Root Cause (established — not a requirement)

Every 100 ms `updateTelemetry` fires → `bumpSampleRevision()` increments
`sampleRevision` → `telemetryRevision` in `DashboardGrid.qml` re-evaluates →
**every** chart tile calls `metricHistory()` → `historyForMetric()` allocates a
fresh `QVariantList` by copying the full ring buffer → `Sparkline.onValuesChanged`
fires → full Canvas repaint. With N chart tiles this is N allocations + N repaints
at 10 Hz, all on the Qt render thread.

---

## Requirements

### Must-have (all options share these)

1. The dashboard UI must remain responsive (no perceptible input lag) at ≥ 10 Hz
   telemetry input with any number of chart tiles visible.
2. Chart tiles must still update visually at a rate that feels live (≥ 5 Hz chart
   repaint rate is acceptable; 10–30 Hz is ideal).
3. Non-chart widgets (ValueTile, MotorSummaryCard) must continue to update at their
   current rate with no regression.
4. The chart-freeze feature must continue to work correctly.
5. No existing public C++ or QML API surface should be removed without a migration
   path (internal implementation changes are fine).

### Option A — Throttled render timer (simplest)

6A. A dedicated QML `Timer` (or C++ equivalent) fires at a fixed chart-refresh
    interval (default 100 ms → target 33 ms / ~30 fps, configurable via a constant).
7A. Chart tiles bind their `history` to a signal/property driven by this timer, not
    by `telemetryRevision` directly.
8A. The ring-buffer copy (`historyToVariantList`) is only triggered by the chart
    timer, not by every `dataChanged` emission.

### Option B — Separate history notification role (medium complexity)

6B. `rolesForSampleUpdate()` is split: scalar roles (rpm, voltage, etc.) and history
    roles (`*HistoryRole`) are emitted on separate `dataChanged` signals.
7B. History `dataChanged` is **throttled** — coalesced so it fires at most once per
    render frame (or at a fixed lower rate, e.g. 30 Hz) regardless of how fast
    telemetry arrives.
8B. `metricHistory()` in `DashboardGrid.qml` depends only on the history-specific
    change signal, not the global `telemetryRevision`.
9B. Scalar bindings (`metricValue`, `motorWarningLevel`, etc.) are unaffected and
    still update at the full telemetry rate.

### Option C — Per-tile direct binding, remove global revision dependency (recommended)

6C. Each `ChartTile` instance connects directly to the C++ model for its specific
    `(motorId, metric)` pair rather than reading through the global `telemetryRevision`
    fan-out.
7C. `DashboardGrid.metricHistory()` is **not** used by `ChartTile`; instead,
    `ChartTile` exposes a C++-backed property or invokable that is notified only
    when that motor's history changes.
8C. A per-motor, per-metric change signal (or a targeted `dataChanged` with only
    history roles for that row) drives repaint — so a 4-motor board with 2 chart
    tiles causes exactly 2 repaints per tick, not N×all-tiles.
9C. The global `telemetryRevision` / `sampleRevision` mechanism is retained for
    scalar consumers (ValueTile, and the numeric/status values on MotorSummaryCard),
    but no widget depends on it for **history** series. (Update 2026-06-12: the
    MotorSummaryCard sparklines were also migrated to the per-motor history pattern
    at the user's request, so they no longer recopy ring buffers via the global
    fan-out either.)
10C. Chart repaints are additionally rate-limited to at most 30 Hz (one
    `requestPaint` call per tile per ~33 ms) to prevent bursts if telemetry arrives
    faster than the display refresh.

---

## Constraints

- **Tech stack**: Qt/QML (C++ backend, QML frontend). No new runtime dependencies.
- **Platform**: Windows (primary), cross-platform Qt.
- **Telemetry rate**: designed to handle 10 Hz today; solution should not degrade
  at up to 100 Hz without further changes.
- **Ring buffer size**: fixed at compile time; history copy cost scales with it.
  The fix must not increase copy frequency relative to render rate.
- **No data loss**: the ring buffer must still receive every sample regardless of
  render throttling — throttling applies only to UI updates, not data storage.
- **Backwards compatibility**: `historyForMetric()`, `valueForMetric()`,
  `sampleRevision`/`sampleRevisionChanged` must remain callable (other consumers
  may exist or be added).

---

## Option Comparison

| | A — Timer | B — Split roles | C — Direct binding (rec.) |
|---|---|---|---|
| Implementation effort | Low | Medium | Medium–High |
| Eliminates N×M fan-out | No (throttles it) | Partially | Yes |
| Scales with more tiles | Poorly | Better | Well |
| Architectural cleanliness | Low | Medium | High |
| Risk of regression | Low | Low–Medium | Medium |

**Chosen: Option C** (decided 2026-06-12). It removes the root cause (global fan-out) rather
than masking it. With Option A, adding 20 chart tiles still causes 20 allocations
and 20 repaints per timer tick. Option C makes cost proportional to the number of
*visible, distinct* (motor, metric) pairs, which is the right scaling behavior.
Option B is a worthwhile intermediate if Option C proves too invasive.

---

## Edge Cases

1. **Motor not yet seen**: `historyForMetric` returns `[]` for an unknown `motorId`.
   Chart tile must display gracefully (blank/empty sparkline) and update once data
   arrives.
2. **Motor goes stale**: chart should continue showing the last-known history (dimmed
   via `isStale`); no crash or blank-out.
3. **Charts frozen**: frozen history snapshot must be taken from ring buffer data, not
   from a throttled update — snapshot must be current at the moment freeze is toggled.
4. **Rapid motor add**: a new motor appearing mid-session must trigger chart tile
   creation and begin receiving history updates without a restart.
5. **Zero or one data point**: `Sparkline` already guards against `series.length < 2`;
   the fix must not bypass this.
6. **Multiple tiles for the same (motor, metric)**: two chart tiles bound to M1/rpm
   must both update; the per-tile direct binding must not assume uniqueness.
7. **Telemetry rate spike**: if telemetry briefly arrives faster than 100 Hz,
   per-tile rate-limiting (req 10C) must absorb the burst without queuing unbounded
   repaints.

---

## Definition of Done

- [ ] With 8 chart tiles visible and fake telemetry running at 100 ms, the Qt frame
      rate (measured via `QQuickWindow::frameSwapped` or equivalent) stays ≥ 30 fps
      on the development machine.
- [ ] With 0 chart tiles visible, behavior is identical to pre-fix (no regression in
      ValueTile / MotorSummaryCard update rate).
- [ ] `historyForMetric()` and `valueForMetric()` remain callable from QML and return
      correct data.
- [ ] `sampleRevision` / `sampleRevisionChanged` still increments on each telemetry
      sample (non-chart consumers unaffected).
- [ ] The chart-freeze feature correctly snapshots history at freeze time and stops
      updating until unfrozen.
- [ ] A motor that goes stale shows a dimmed sparkline with the last-known history
      (no blank or crash).
- [ ] No new motors are missed: a motor that first appears after the dashboard is open
      gets its own chart updates.
- [ ] The ring buffer receives every telemetry sample regardless of render throttle
      (verified by checking ring buffer size grows to capacity, not just to
      render-rate × time).
- [ ] All existing QML widget types (ValueTile, MotorSummaryCard, ChartTile) render
      correctly after the change.

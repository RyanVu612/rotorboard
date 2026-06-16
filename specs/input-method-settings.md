# Input Method Settings Menu

## Goal

Add a settings menu that lets the operator switch telemetry input methods at runtime.

Initial input methods:

- Pixhawk real data
- CSV replay data
- Fake generated data

The app already has telemetry source abstractions for fake data, CSV playback, MAVLink UDP, MAVLink serial, and DroneCAN. This work should reuse the existing source factory and manager instead of adding a parallel input path.

## Current State

Telemetry is configured during startup in `src/main.cpp` using CLI flags. The parsed `SourceConfig` is passed into `AppController`, which builds one concrete `TelemetrySource` through `makeTelemetrySource(config)` and installs it on `TelemetryManager`.

Relevant existing pieces:

- `SourceConfig` and `SourceKind` in `src/telemetry/TelemetrySourceConfig.h`
- `makeTelemetrySource` and `sourceLabelFor` in `src/telemetry/TelemetrySourceFactory.cpp`
- `TelemetryManager::setSource` in `src/telemetry/TelemetryManager.cpp`
- `AppController.sourceLabel` and link status properties exposed to QML
- Dashboard source badge in `qml/DashboardPage.qml`

## Input Method Mapping

User-facing settings should map to existing source kinds:

| UI label | Source kind | Notes |
| --- | --- | --- |
| Pixhawk | `SourceKind::MavlinkSerial` initially | Real Pixhawk data should default to serial MAVLink unless UDP is explicitly needed later. |
| CSV Replay | `SourceKind::Playback` | Uses `CsvPlaybackSource` and a configured CSV file path. |
| Fake Data | `SourceKind::Fake` | Uses `FakeTelemetrySource`; no extra settings required. |

If UDP Pixhawk forwarding remains important, the Pixhawk settings can later expose a serial/UDP mode toggle that maps to `SourceKind::MavlinkSerial` or `SourceKind::Mavlink`.

## Backend Plan

1. Make `AppController` own the active `SourceConfig`.
2. Add QML-facing properties for settings state:
   - `QString inputMethod`
   - `QString playbackPath`
   - `QString mavlinkSerialPort`
   - `int mavlinkSerialBaud`
   - optional future fields for UDP host and port
3. Add QML invokables:
   - `setInputMethod(const QString &method)`
   - `setPlaybackPath(const QString &path)`
   - `setMavlinkSerialPort(const QString &port)`
   - `setMavlinkSerialBaud(int baud)`
   - `applyTelemetrySource()`
4. Rebuild a `SourceConfig` from the settings fields when the user applies changes.
5. Use `makeTelemetrySource(config)` to create the new source.
6. Install it through `TelemetryManager::setSource(...)`.
7. Update `sourceLabel`, `linkMonitored`, `linkStatusText`, and `linkStateLevel` after switching.
8. Reconnect `MavlinkSerialTelemetrySource::linkStatusChanged` only when the active source is MAVLink serial.

## Telemetry Manager Lifecycle

`TelemetryManager` should support clean runtime source switching while the app is already running.

Required behavior:

- Track whether the manager is currently running.
- When `setSource(...)` receives a new source:
  - stop the old source
  - disconnect old source signals
  - install the new source
  - connect `telemetryReceived`
  - start the new source immediately if the manager was already running
- Keep the stale refresh timer behavior unchanged.
- Decide whether to clear current telemetry samples on source switch or let the stale logic age out old values.

Recommended behavior: clear or explicitly stale the current model data on source switch so real, replay, and fake data do not appear blended.

## QML Plan

Add a settings button to the dashboard header near the existing Pause, Add widget, and source badge controls.

Create a compact settings dialog, likely `qml/SettingsDialog.qml`, with:

- Input method segmented control or radio-style selector
- Pixhawk settings section:
  - serial port field
  - baud field
- CSV replay section:
  - CSV path field
  - optional file picker later
- Fake data section:
  - no extra fields initially
- Apply and Cancel actions

The dialog should not switch sources while the user is typing. Apply should be the commit point.

## Persistence

Use `QSettings` to persist:

- selected input method
- CSV playback path
- MAVLink serial port
- MAVLink serial baud
- future UDP host and port if added

Startup precedence:

1. Explicit CLI source flags should win for that launch.
2. If no source flag is provided, load the last saved settings.
3. If no saved settings exist, default to fake generated data.

This preserves CLI workflows while making the settings menu useful for normal app launches.

## Source Badge And Link Status

The dashboard source badge should remain the authoritative display of the active input.

On every source switch:

- Recompute the label with `sourceLabelFor(config)`.
- Emit `sourceLabelChanged()`.
- Set `linkMonitored` true only for MAVLink serial.
- Reset link status when leaving MAVLink serial.
- For MAVLink serial, reconnect link status updates so the badge can show:
  - port open failed
  - waiting for data
  - no valid MAVLink
  - message rate

`linkMonitored` is currently a constant QML property. It will need to become a notifying property if it can change at runtime.

## Testing Plan

Add backend tests for:

- switching fake to playback updates source label
- switching while running starts the new source
- switching away from MAVLink serial resets link monitoring state
- invalid CSV path fails gracefully without crashing
- settings fields produce the expected `SourceConfig`

Manual verification:

- Launch with no flags and confirm fake data runs.
- Open settings, switch to CSV replay, apply a sample file, and confirm replay starts.
- Switch back to fake data and confirm the badge and values update.
- Select Pixhawk and confirm the badge shows MAVLink serial and link status updates.
- Launch with CLI flags and confirm explicit CLI source wins over saved settings.

## Implementation Order

1. Add runtime source switching support to `TelemetryManager`.
2. Move active source config/state ownership into `AppController`.
3. Add QML-facing settings properties and apply method.
4. Add settings persistence with `QSettings`.
5. Build the QML settings dialog and header button.
6. Add backend tests.
7. Run manual UI verification.


# rotorboard

A telemetry dashboard for UAV propulsion systems, targeting HOBBYWING XRotor X11 Plus ESC data over DroneCAN (and MAVLink for development).

## Windows build

Install [Qt 6](https://www.qt.io/download) with MinGW 64-bit and the **SerialPort** module. Add Qt and MinGW to your PATH, then:

```powershell
$env:PATH = "C:\Qt\6.10.2\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;" + $env:PATH
cmake -S . -B build -G Ninja
cmake --build build
```

Initialize third-party headers first (see [third_party/README.md](third_party/README.md)):

```powershell
git clone --depth 1 https://github.com/mavlink/c_library_v2.git third_party/mavlink_c
git clone --depth 1 https://github.com/dronecan/libcanard.git third_party/libcanard
```

## Run

```powershell
.\build\rotorboard_app.exe
.\build\rotorboard_app.exe --log
.\build\rotorboard_app.exe --playback samples\session.csv
.\build\rotorboard_app.exe --mavlink
.\build\rotorboard_app.exe --mavlink 127.0.0.1:14550
.\build\rotorboard_app.exe --mavlink-serial
.\build\rotorboard_app.exe --mavlink-serial /dev/tty.usbmodem14201
.\build\rotorboard_app.exe --mavlink-serial /dev/tty.usbmodem14201 57600
.\build\rotorboard_app.exe --dronecan COM3
.\build\rotorboard_app.exe --dronecan COM3 --log
```

| Flag | Description |
|------|-------------|
| `--log [file]` | Record telemetry CSV under `logs/` (default: timestamped filename) |
| `--playback path` | Replay a logged CSV session |
| `--mavlink [host:port]` | Listen for MAVLink `ESC_STATUS` over UDP (default `0.0.0.0:14550`) |
| `--mavlink-serial [port] [baud]` | Read MAVLink directly from a flight controller over USB serial (default baud `115200`). With no port, auto-detects the first USB serial device; the detected ports are logged either way. A lone numeric argument is treated as the baud rate. |
| `--dronecan [port]` | Read HOBBYWING DroneCAN frames via SLCAN serial (e.g. `COM3` on Windows) |

To connect a Pixhawk directly by USB, plug it in and find its serial device, then pass it to `--mavlink-serial`:

- **macOS:** `ls /dev/tty.usb*` (e.g. `/dev/tty.usbmodem14201`)
- **Linux:** `ls /dev/ttyACM* /dev/ttyUSB*`
- **Windows:** check Device Manager for the COM port (e.g. `COM3`)

The source chip in the dashboard header shows a live link indicator: a coloured dot plus status — *port open failed*, *waiting for data* (port open, no bytes), *no valid MAVLink* (bytes arriving that don't parse), or the message rate (`N msg/s`) once valid MAVLink frames are flowing.

## Running alongside QGroundControl

Use rotorboard as a dedicated motor dashboard on a second monitor while QGC keeps the map full-screen. QGC owns the flight-controller link; rotorboard receives a forwarded MAVLink copy.

| Consumer | UDP port | Role |
|----------|----------|------|
| QGroundControl | `14550` (default link) | Primary GCS (map, mission, params) |
| Rotorboard | `14551` (forward target) | Motor dashboard only |

### Setup

1. Connect the vehicle to QGC as usual (USB, SiK radio, etc.).
2. In QGC: **Application Settings → MAVLink → Ground Station**
   - Enable **MAVLink forwarding**
   - Set target host to `127.0.0.1` (same PC) or the GCS machine's LAN IP if rotorboard runs on another machine
   - Set target port to `14551`
3. Start rotorboard on the secondary port:

   ```powershell
   .\build\rotorboard_app.exe --mavlink 0.0.0.0:14551
   ```

   On Linux/macOS:

   ```bash
   ./build/rotorboard_app --mavlink 0.0.0.0:14551
   ```

4. Place rotorboard on a second monitor; keep QGC map full-screen on the primary display. Close the MAVLink Inspector tab — rotorboard replaces it.

QGC must stay running and connected for forwarding to work. After a link reconnect, verify motor cards repopulate.

### Pre-flight verification

Before field use, confirm the data path end-to-end:

1. **Confirm `ESC_STATUS` is emitted.** In QGC MAVLink Inspector, search for `ESC_STATUS`. Rotorboard parses only this message (MAVLink msg 291). If the flight controller sends other ESC messages but not `ESC_STATUS`, motor cards will stay empty.
2. **Confirm rotorboard receives forwarded packets.** The dashboard header shows source badge **MAVLink**; motor cards populate with RPM, voltage, and current. If cards stay empty, confirm forwarding is enabled, target port is `14551`, and `ESC_STATUS` appears in QGC on the live link.
3. **Confirm stale detection.** If the link drops, cards flip to **STALE** after ~2 s.

Optional session logging:

```powershell
.\build\rotorboard_app.exe --mavlink 0.0.0.0:14551 --log
```

Replay later with `--playback logs\session-*.csv`.

### Limitations

- MAVLink mode shows RPM, voltage, and current from `ESC_STATUS`. Temperature, PWM, and fault status are not populated over MAVLink today.
- Rotorboard is telemetry-only — all vehicle control stays in QGC.

## Tests

```powershell
ctest --test-dir build
```

## DroneCAN hardware path

HOBBYWING X11 Plus / DATALINK box → CAN bus → USB-CAN dongle (SLCAN firmware) → serial port → `rotorboard --dronecan COMx`.

Rotorboard is **telemetry consumer only** — it does not send throttle, arm, or configuration commands.

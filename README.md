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
.\build\rotorboard_app.exe --dronecan COM3
.\build\rotorboard_app.exe --dronecan COM3 --log
```

| Flag | Description |
|------|-------------|
| `--log [file]` | Record telemetry CSV under `logs/` (default: timestamped filename) |
| `--playback path` | Replay a logged CSV session |
| `--mavlink [host:port]` | Listen for MAVLink `ESC_STATUS` over UDP (default `0.0.0.0:14550`) |
| `--dronecan [port]` | Read HOBBYWING DroneCAN frames via SLCAN serial (e.g. `COM3` on Windows) |

## Tests

```powershell
ctest --test-dir build
```

## DroneCAN hardware path

HOBBYWING X11 Plus / DATALINK box → CAN bus → USB-CAN dongle (SLCAN firmware) → serial port → `rotorboard --dronecan COMx`.

Rotorboard is **telemetry consumer only** — it does not send throttle, arm, or configuration commands.

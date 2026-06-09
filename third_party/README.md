# Third-party dependencies

## MAVLink C library (`mavlink_c/`)

Rotorboard uses the generated MAVLink v2 **common** dialect from [mavlink/c_library_v2](https://github.com/mavlink/c_library_v2).

After cloning this repository, initialize the MAVLink headers:

```bash
git clone --depth 1 https://github.com/mavlink/c_library_v2.git third_party/mavlink_c
```

CMake expects headers at `third_party/mavlink_c/common/mavlink.h`.

## DroneCAN / HOBBYWING (`libcanard/`)

Rotorboard decodes HOBBYWING ESC telemetry over DroneCAN using the [DroneCAN libcanard](https://github.com/dronecan/libcanard) fork (UAVCAN v0 / DroneCAN API). Do not use [OpenCyphal/libcanard](https://github.com/OpenCyphal/libcanard) — it is a different library with a different layout and API. Message IDs and field layouts are vendored under `third_party/hobbywing_dsdl/` and decoded in `src/telemetry/dronecan/HobbywingMessages.cpp`.

Clone libcanard before building:

```bash
git clone --depth 1 https://github.com/dronecan/libcanard.git third_party/libcanard
```

CMake compiles `third_party/libcanard/canard.c` and includes `third_party/libcanard/canard.h`. On 64-bit desktop builds (macOS, Linux, Windows), CMake defines `CANARD_64_BIT=1` automatically.

### HOBBYWING messages (v1)

| Message | ID | Fields used |
|---------|-----|-------------|
| `com.hobbywing.esc.StatusMsg1` | 20050 | RPM, PWM, status |
| `com.hobbywing.esc.StatusMsg2` | 20051 | bus voltage (0.1 V), current (0.1 A), temperature |
| `com.hobbywing.esc.StatusMsg3` | 20052 | MOS / cap / motor temperatures |

Full DSDL definitions live in the [DroneCAN DSDL repository](https://github.com/DroneCAN/DSDL) under `com/hobbywing/esc/`. Optional local clones for code generation or reference:

```bash
git clone --depth 1 https://github.com/DroneCAN/DSDL.git third_party/dronecan_dsdl
git clone --depth 1 https://github.com/DroneCAN/dronecan_dsdlc.git third_party/dronecan_dsdlc
```

Rotorboard does not require generated DSDL headers at build time; the manual decoders match the vendored `.uavcan` files in `third_party/hobbywing_dsdl/`.

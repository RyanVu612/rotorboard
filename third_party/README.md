# Third-party dependencies

## MAVLink C library (`mavlink_c/`)

Rotorboard uses the generated MAVLink v2 **common** dialect from [mavlink/c_library_v2](https://github.com/mavlink/c_library_v2).

After cloning this repository, initialize the MAVLink headers:

```bash
git clone --depth 1 https://github.com/mavlink/c_library_v2.git third_party/mavlink_c
```

CMake expects headers at `third_party/mavlink_c/common/mavlink.h`.

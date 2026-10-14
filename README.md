# fletch

A network library: a reliable transport protocol over UDP in C++23, with
congestion control and a deterministic link simulator.

This is a learning project, not production software. No security, no hardening,
no stability promises.

## Build

Requires CMake 3.28+, Ninja, and a C++23 compiler (GCC 14+ or Clang 18+).

```sh
cmake --preset dev && ninja -C build/dev && ctest --preset dev
```

Presets: `dev` (Debug with ASan and UBSan), `debug`, `release`.

# Life Game

Life Game is a C++23 macOS/Linux foundation for a finite Conway Life experiment. The repository
currently contains the reproducible project baseline only; gameplay and presentation features are
introduced by later stories.

## Prerequisites

- CMake 3.28 or newer
- Ninja
- C++23 compiler
- macOS: AppleClang 17 from the Xcode Command Line Tools
- Linux: Clang 18 and the X11, Wayland, OpenGL, and ALSA development packages required by raylib

The build fetches raylib 6.0, raygui 5.0, SQLite 3.53.4, and Catch2 3.15.3 from pinned release
archives. Each archive is verified by its SHA-256 hash during configuration; no system-package
fallback is used.

## Development workflow

```sh
cmake --preset dev-debug
cmake --build --preset dev-debug
ctest --preset dev-debug --output-on-failure
```

Additional committed presets are `dev-debug-tools`, `dev-release`, `ci-linux-debug-asan`,
`ci-linux-release`, `ci-macos-debug`, and `ci-macos-release`. The CI presets select the explicit
Clang 18 Linux or active AppleClang macOS toolchain and enable project warnings as errors.

Generated build trees live under `build/` and runtime state is not stored in the repository.

# PhantomDrive Delivery - 2026-06-11

## What Is Included

- `source/`: cleaned source project, excluding local build caches.
- `windows-release/`: runnable Windows build generated from the Release configuration.

## How To Run

Open:

```text
windows-release/PhantomDriveApp.exe
```

The app generates missing sound files under `assets/sounds/` on startup.

## Main Changes In This Delivery

- Added direct menu entries for `Two-Player Race` and `Adaptive AI Demo`.
- Improved built-in track selection with track name, difficulty, and description.
- Split top-bar button behavior:
  - `Finish Drive` opens the report.
  - `Back` returns to menu without stale reports.
  - `Exit Game` closes the app.
- Fixed Custom Track AI waypoint order and removed duplicate FinishLine waypoint insertion.
- Changed the simulation loop from 50 ms to 33 ms for smoother gameplay.
- Kept game input focus refreshed after mode transitions and race start.
- Replaced gameplay-facing mixed-language prompts in `MainWindow` with English.
- Removed noisy debug logs from mode, AI, and traffic-object code.
- Removed unused `GameTopBar` source/header files.

## Verification

- Clean CMake configure passed.
- Release build passed with Qt 6.8.3 MSVC 2022.
- `windows-release/PhantomDriveApp.exe` desktop launch smoke passed.
- Sound files were generated successfully under `windows-release/assets/sounds/`.
- GUI test executables were built; offscreen runs start their event loop and do not auto-exit, so they were stopped manually.

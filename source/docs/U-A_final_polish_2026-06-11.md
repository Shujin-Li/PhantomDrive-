# U-A Final Polish - 2026-06-11

## Scope

This pass is based on `PhantomDrive0610.rar` and focuses on the U-A items listed near the end of `PhantomDrive_final_optimization_plan_0606.md`.

## Completed

1. Fresh build readiness
   - Verified CMake configure from a clean build directory.
   - Verified `BuiltInTrackFactory.h/.cpp` are included in `CMakeLists.txt`.
   - Kept generated build folders out of the delivery source package.
   - Removed unused `GameTopBar` source/header files because the active UI uses `ArcadeHUD` and the `.ui` top bar.

2. Main menu and demo entry flow
   - Added direct main-menu entries for `Two-Player Race` and `Adaptive AI Demo`.
   - Kept `Arcade Mode`, `Learning Mode`, `Custom Track Mode`, `Driving Report / History`, `Load Custom Track`, and `Guide / Powerups` reachable without code changes.
   - Added built-in track metadata display in the race setup page: track name, difficulty, and description.
   - Adaptive demo now stores and applies the `adaptive` AI difficulty even when launched directly from the main menu.

3. Button semantics
   - `Finish Drive` ends the current session and opens the report.
   - `Back` returns to the main menu and silently stops the current session without opening a stale report.
   - `Exit Game` exits the application instead of sharing the `Finish Drive` behavior.
   - Guarded the old nullable `m_btnFinishDrive` connection to avoid Qt `invalid nullptr parameter` warnings.

4. Custom Track and AI waypoint stability
   - Custom Track AI route now uses `Start -> checkpoints -> first FinishLine`, with duplicate finish waypoints removed.
   - If no FinishLine exists, the route closes back to the start position.
   - AI initialization still supports built-in tracks, custom tracks, and fallback waypoints.

5. Input focus and runtime loop
   - Existing `focusGameViewForDriving()` calls are preserved after entering gameplay and race start.
   - Main simulation timer changed from 50 ms to 33 ms to target about 30 FPS.
   - Physics, AI update, session time, and data sampling now share the same simulation step constant.

6. UI language and console polish
   - Replaced remaining gameplay-facing Chinese HUD/toast text in `MainWindow` with English.
   - Removed noisy debug logs from ArcadeMode, LearningMode, AIOpponentManager, SimpleAIOpponent, TrafficLightObject, and TrafficObjectManager.
   - Adaptive AI feedback is surfaced as a visible toast/status message: `Adaptive adjusted: ...`.

## Verification

- Clean CMake configure passed with Qt 6.8.3 MSVC 2022.
- Release build passed:
  - `PhantomDrive.dll`
  - `PhantomDriveApp.exe`
  - optional test targets
- GUI test executables launch an event loop in offscreen mode and do not auto-exit, so they were stopped manually after confirming the build artifacts were produced.

## Notes

- No large `MainWindow` architecture rewrite was done, to keep final-round risk low.
- The delivery package should include source plus a Windows runtime folder generated from the Release build.

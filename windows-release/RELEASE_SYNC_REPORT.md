# PhantomDrive Release Sync Report

Generated: 2026-06-12 21:34:30 +08:00

## Custom Track Settings Update

- Added Custom Track Editor run settings in the top tool area:
  - Players: Single Player / Two-Player
  - AI: Easy / Normal / Hard / Adaptive
- Defaults are Single Player and Normal AI.
- Play This Track now applies the editor selections before launching the runtime custom track snapshot.
- Two-Player custom tracks reuse the existing P1/P2 runtime and ArcadeHUD two-player overlay.
- P2 spawn now prefers `TrackData::getStartPositions()[1]`; tracks with only one Start use the existing nearby offset fallback so P1/P2 do not spawn exactly overlapped.
- AI route remains `Start -> checkpoints -> Finish`; the custom track JSON format was not changed.
- AI difficulty reuses existing Arcade AI styles/config:
  - Easy: Conservative/Normal with lower speed and acceleration.
  - Normal: existing Normal/Defensive behavior.
  - Hard: Aggressive/Defensive with higher speed and acceleration.
  - Adaptive: existing adaptive demo-style Normal/Aggressive mapping with a mild speed lift.

## Two-Player Finish Gate Update

Updated: 2026-06-12 21:48:00 +08:00

- Two-player races no longer end when the first player reaches the finish.
- P1 and P2 now each set an individual finished flag.
- The session ends and opens the report only after both players have finished.
- The first finisher gets a HUD banner saying the game is waiting for the other player.
- This applies to Custom Track Two-Player and the existing Arcade two-player finish path.

## AI Coach Report Display Fix

Updated: 2026-06-12 22:04:00 +08:00

- Fixed the report panel so a received AI coach markdown response is cached for the current report session.
- Prevented later report refreshes, history selection sync, or local coach-advice updates from immediately overwriting the DeepSeek-generated content.
- Rebuilt `AIAPIClient` prompt/fallback text to remove corrupted Chinese literals that could confuse the model and UI.
- Added `[AIAPIClient]` logs in Qt Creator Application Output for mode, endpoint, model, timeout, success character count, and HTTP/parse failure details.
- Default DeepSeek model is `deepseek-v4-flash`; an explicit `DEEPSEEK_MODEL` environment variable still takes precedence.

## AI Coach Markdown And Mode Context Update

Updated: 2026-06-12 22:37:00 +08:00

- The Coach Advice panel now uses Qt Markdown rendering via `QTextBrowser`.
- P1 and P2 AI coach reports are cached separately by player and report `sessionId`.
- ScoreManager now emits AI report callbacks with `vehicleId` and `sessionId`, preventing P1/P2 report text from being overwritten when switching tabs.
- ScoreReport now carries `drivingMode` and `reportContext`.
- AI prompts are tuned by mode:
  - Learning: traffic-rule practice and compliance.
  - Arcade: race pace, lap consistency, powerups, and AI pressure.
  - Custom Track: checkpoint order, finish discipline, and route layout.
  - Two-Player: player-specific feedback and fair race comparison.
  - Adaptive AI: difficulty adaptation and driver response strategy.

## Build

Commands:

```powershell
$env:Path='C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.8.3\mingw_64\bin;' + $env:Path
cmake -S source -B source\build-codex-debug
cmake --build source\build-codex-debug -j
cmake -S source -B source\build-release-sync -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_MAKE_PROGRAM='C:/Program Files/JetBrains/CLion 2025.2.1/bin/ninja/win/x64/ninja.exe' -DCMAKE_PREFIX_PATH=C:/Qt/6.8.3/mingw_64 -DCMAKE_CXX_COMPILER=C:/Qt/Tools/mingw1310_64/bin/g++.exe
cmake --build source\build-release-sync -j
```

Result: success.

- Debug output: `source\build-codex-debug\bin\PhantomDriveApp.exe`
- Release output: `source\build-release-sync\bin\PhantomDriveApp.exe`
- Release DLL: `source\build-release-sync\bin\libPhantomDrive.dll`

Note: the shell PATH must put `C:\Qt\Tools\mingw1310_64\bin` before older MinGW entries. Without that, CMake try-compile failed by loading the older MinGW runtime from PATH.

## Deployment

Copied/synchronized:

- `source\build-release-sync\bin\PhantomDriveApp.exe` -> `windows-release\PhantomDriveApp.exe`
- `source\build-release-sync\bin\libPhantomDrive.dll` -> `windows-release\libPhantomDrive.dll`
- `source\assets` -> `windows-release\assets`
- `source\docs` -> `windows-release\docs`

Current key release files:

- `windows-release\PhantomDriveApp.exe` size: 51,374 bytes
- `windows-release\libPhantomDrive.dll` size: 3,438,231 bytes
- `windows-release\assets`: 82 files
- `windows-release\docs`: 26 files

## Smoke Test

Launch command:

```powershell
Start-Process -FilePath windows-release\PhantomDriveApp.exe -WorkingDirectory windows-release -PassThru
```

Result: success.

- Main window title: `PhantomDrive`
- UI Automation found and clicked `Custom Track Mode`
- App remained alive after entering the Custom Track entry path

## Notes

- No custom track JSON format changes.
- No TrackValidator rule changes.
- No vehicle physics core changes.
- No scoring algorithm or AI API changes.
- No commit or push was performed.

# PhantomDrive Codex Window Work Summary

Date: 2026-06-12

This window focused on fixing HUD race ranking mismatches. The user reported several cases where the on-screen race order did not match the right-side HUD position display, especially with AI opponents, two-player mode, lap wrapping, Adaptive AI, and Learning mode.

The working tree had existing unrelated edits when this work began. Those were preserved and not reverted.

## Main Problem

The HUD position field was not consistently based on the same race-progress source as the visible race state.

Observed issues included:

- HUD showing a fixed or stale rank such as `3RD / 3` or `1ST / 3`.
- Single-player player-vs-AI ranking lagging behind the actual pass/overtake state.
- Two-player mode ranking P1/P2 correctly relative to each other, but incorrectly relative to AI.
- Adaptive AI mode using AI speed/manager state in ways that did not match visible track progress.
- Learning mode showing AI cars and AI speed rows, but ranking not updating correctly.
- Lap wrap / lapping cases causing progress to jump backward near the start/finish area.

## Key Design Decision

The final direction was to reuse the formal race-ranking logic for HUD ranking wherever the mode has AI race participants.

For Arcade / Custom / Adaptive AI:

- P1/P2 use real lap and next-checkpoint progress.
- AI uses real lap, current waypoint index, and segment progress between waypoints.
- Everyone is converted to a shared `absoluteProgress` value before sorting.

For Learning mode:

- Learning now reuses the same HUD ranking path as race modes.
- Learning updates the same player checkpoint/lap state used by race HUD ranking.
- Learning does not become a formal race-finish mode; it only borrows race progress for HUD ranking.

## Important Code Changes

### `source/src/UI/mainwindow.cpp`

Added helper logic near the anonymous namespace:

- `RaceHudEntry::absoluteProgress`
- `progressOnSegment()`
- `routeHasDuplicateEndpoint()`
- `routeSegmentCountFor()`
- `progressAlongWaypointsNear()`
- `routeSignatureFor()`
- `routeWaypointsFromTrack()`
- `hudRankingWaypoints()`
- `playerRaceAbsoluteProgress()`
- `aiRaceAbsoluteProgress()`

Updated `buildRaceHudEntries()`:

- Builds a unified racer list including P1, optional P2, and active AI.
- Computes `absoluteProgress` for all participants.
- Sorts only by `absoluteProgress`, then stable insertion order for ties.
- Formal race modes use checkpoint/lap-aware progress.
- Non-formal/free fallback still has route-progress support.

Updated `updateRaceHud()`:

- Added `learningRaceHudActive`.
- Added `hudRaceRankingActive`.
- Learning mode now calls `buildRaceHudEntries(..., hudRaceRankingActive, ...)`, so it uses the same ranking logic as race modes.
- AI is included in HUD ranking when active/ranked AI opponents exist.

Updated simulation loop:

- Learning mode now calls `updateArcadeRaceProgress(positionBeforeUpdate)` every tick, just like race modes, so `m_nextCheckpointIndex` and lap state are maintained for HUD ranking.

Updated `updateArcadeRaceProgress()`:

- Allows Learning mode to update player race progress even though `m_arcadeRaceLogicActive` is false.
- Learning mode does not trigger formal race completion/report through this path.
- When Learning reaches `m_totalLaps`, lap HUD progress is reset for continued training.

### `source/include/PhantomDrive/UI/mainwindow.h`

Added lightweight HUD progress state:

- `QHash<QString, qreal> m_hudRaceProgressById`
- `QString m_hudRaceRouteSignature`

These are mainly used by non-formal/free route fallback progress and are cleared on session/race reset.

## Mode Coverage

Single-player Arcade:

- P1 vs AI now compares shared `absoluteProgress`.
- AI no longer updates only at waypoint boundaries; segment progress is included.

Two-player Arcade:

- P1 and P2 remain correctly ranked against each other.
- P1/P2 are also ranked against AI using the same `absoluteProgress` sort.
- Total racers should be P1 + P2 + active AI.

Adaptive AI:

- Treated as Arcade with adaptive AI difficulty.
- Uses the same formal HUD ranking path as normal Arcade.

Custom Track:

- Uses formal race-progress path when `m_customTrackPlaying` is active.
- Falls back to route waypoints from track when needed.

Learning:

- Now reuses race-mode HUD ranking.
- Maintains player checkpoint/lap state only for HUD ranking.
- Does not turn Learning into a formal race completion mode.

## Verification

Commands run:

```powershell
git status -sb
cmake -S source -B source/build-codex-debug
cmake --build source/build-codex-debug -j
```

Result:

- CMake configure passed.
- Build still failed in this environment because MinGW's C++ backend is broken.
- A smoke test compiling a minimal `int main(){return 0;}` also failed.
- Directly running `cc1plus.exe` returned `exit=-1073741515`.

This indicates a local compiler/runtime environment issue rather than a useful project C++ diagnostic.

## Current Working Tree Notes

At the end of this update, `git status -sb` showed:

```text
## master
 M source/src/UI/mainwindow.cpp
?? windows-release_backup_20260612_010651/
```

`CODEX_WINDOW_WORK_SUMMARY.md` is an untracked/working handoff artifact unless the user stages it.

## Recommended Manual Checks

Because the local compiler backend cannot complete a build here, manual runtime checks should be done in an environment where the app builds/runs:

- Learning mode:
  - HUD rank should no longer stay fixed at `1ST / 3`.
  - P1 passing AI should update position.
  - AI passing P1 should update position.
  - Total should include active AI cars.
- Single-player Arcade:
  - P1/AI overtakes update immediately, not only at checkpoints.
- Two-player Arcade:
  - P1/P2 rank correctly against each other and against AI.
- Lap wrap:
  - A racer near start/finish after completing a lap should not be treated as just starting the race.

## Caution For Next Agent

- Do not revert unrelated files unless explicitly asked.
- The current ranking implementation is intentionally centralized in `MainWindow::updateRaceHud()` and local helpers; avoid adding separate HUD ranking logic inside `ArcadeHUD`.
- If Learning mode ranking still misbehaves, inspect whether `updateArcadeRaceProgress()` is actually advancing `m_nextCheckpointIndex` on the active Learning track and whether AI waypoints match the visible route.

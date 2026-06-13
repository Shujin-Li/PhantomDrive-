# PhantomDrive Window Work Summary - 2026-06-13

This document summarizes the main implementation work completed in this Codex window.

## Custom Track Editor Settings

- Added compact `Players` and `AI` selectors to the Custom Track Editor toolbar.
- `Players` supports `Single Player` and `Two-Player`, defaulting to single player.
- `AI` supports `Easy`, `Normal`, `Hard`, and `Adaptive`, defaulting to normal.
- `Play This Track` now reads the current editor selections and applies them when launching a custom track.
- Custom Track JSON format was not changed.

## Custom Track Two-Player Behavior

- Custom Track mode can launch P1 and P2 when `Two-Player` is selected.
- P2 spawn prefers the second start position from `TrackData`.
- If only one start exists, P2 receives a nearby offset spawn so the two players do not fully overlap.
- HUD mode is updated for two-player custom track sessions.

## Two-Player Finish Logic

- Updated two-player race/custom-track finish behavior so the game does not end when only one player reaches the finish.
- Each player is marked finished independently.
- The final end-of-race flow runs only after both Player 1 and Player 2 have finished.
- Checkpoint and finish gate progression remains unchanged.

## AI Coach Report Display

- Fixed two-player report switching so generated AI coach reports no longer disappear when switching between Player 1 and Player 2.
- AI coach reports are cached per player and per session id.
- `ScoreManager` now emits coach report callbacks with vehicle id and session id so the report UI can route results to the correct player.
- Added mode context to score reports, allowing AI advice to be adjusted for Learning, Arcade/Race, Custom Track, Two-Player, and Adaptive AI contexts.
- Replaced plain text AI advice display with Qt Markdown rendering through `QTextBrowser`.

## DeepSeek API Integration

- Hardened `AIAPIClient` environment loading for `PHANTOMDRIVE_AI_MODE`, `PHANTOMDRIVE_AI_TIMEOUT_MS`, `DEEPSEEK_API_KEY`, `DEEPSEEK_BASE_URL`, `DEEPSEEK_MODEL`, and Zhipu equivalents.
- Kept the required DeepSeek model as `deepseek-v4-flash`.
- Normalized DeepSeek endpoint handling so `https://api.deepseek.com` becomes `https://api.deepseek.com/chat/completions` without duplicating the path when a full endpoint is supplied.
- Applied the configured timeout to both `QNetworkRequest::setTransferTimeout()` and the local `QTimer`.
- Fixed timeout handling so the code does not call `readAll()` on an aborted or closed `QNetworkReply`.
- Added effective config and request logs that show mode, key presence, base URL, model, timeout, final URL, response character count, and provider `finish_reason` without printing API keys.
- Increased coach-report `max_tokens` from 1200 to 3000 to reduce incomplete DeepSeek markdown output.

## AI Report History Persistence

- Added `ScoreReport::aiCoachReportMarkdown` and persisted it through `ScoreReport::toJson()` and `SaveLoadManager`.
- Added `SaveLoadManager::updateReportAiCoachReport()` to update the already-saved history record when the asynchronous DeepSeek report arrives.
- Real DeepSeek AI reports are saved into history by matching `sessionId` and `vehicleId`.
- `Local Fallback` reports still display in the current report panel, but are not stored as real AI reports in history.
- Selecting an older history record now prefers the saved AI markdown when it exists.

## Violation Events And Speed Limit Signs

- Fixed report UI state so each loaded report synchronizes its own `violations` list instead of falling back to stale local cache.
- Added display events for overspeed segments detected from driving samples so the violation table can show overspeed cases that were previously only reflected in score penalties.
- Limited those sample-derived overspeed violation events to traffic-rule scoring contexts.
- Updated speed-limit sign rendering to choose the nearest available speed-limit sign asset from 20, 30, 40, 50, 60, 70, 80, 90, 100, and 120.
- Changed the enhanced built-in speed-limit zone from 45 to 50 so the rule value and visible sign match.

## Mode-Specific Scoring

- Split speed-limit scoring by mode.
- Learning and traffic-rule contexts still treat speeding as a rule violation.
- Arcade, Custom Track, and Two-Player race contexts no longer penalize drivers for exceeding road speed limits.
- Race-style modes continue to evaluate collision risk, wrong-way behavior, smoothness, and efficiency without treating high speed itself as a traffic-rule failure.

## Qt Debug Connect Assertion Fix

- Fixed the Qt Debug startup assertion caused by `lambda + Qt::UniqueConnection`.
- The illegal lambda connections were changed to explicit `disconnect(...)` followed by a normal lambda `connect(...)`.
- Existing legal member-function `Qt::UniqueConnection` connections were kept.

## Release Sync

- Built and synchronized the Windows release artifacts during the feature work.
- Synced `source/assets` and `source/docs` into `windows-release`.
- Updated release notes for the Custom Track, finish gate, and AI coach report work.
- Rebuilt and synchronized `windows-release/PhantomDriveApp.exe` and `windows-release/libPhantomDrive.dll` after the DeepSeek, report-history, violation-list, speed-sign, and scoring changes.

## README Refresh And Screenshot Delivery

- Rewrote `source/README.md` so it matches the current final feature set instead of the older tri-mode description.
- Added README sections for Project Overview, Key Features, Screenshots, Build Requirements, Build from Source, Run Windows Release, DeepSeek API Setup, Project Structure, Team / Module Responsibilities, and Demo Checklist.
- Documented the current main entries: Arcade Mode, Learning Mode, Custom Track Mode, Two-Player Race, Adaptive AI Demo, Power-ups, traffic rules, and AI Coach Report / DeepSeek API.
- Documented the safe DeepSeek launcher workflow without exposing any real API key.
- Kept `deepseek-v4-flash` as the documented default DeepSeek model.
- Created `source/docs/images/readme/` and added real README screenshots:
  - `main-menu.png`
  - `arcade-mode.png`
  - `learning-mode.png`
  - `custom-track-editor.png`
  - `custom-track-race.png`
  - `two-player-hud.png`
  - `ai-coach-report.png`
  - `windows-release-launcher.png`
- Replaced the incorrect Learning Mode screenshot with the provided correct Learning Mode screenshot.
- Replaced the previous Windows Release Launcher placeholder with the provided real launcher/package screenshot.
- Replaced the broken/empty AI coach report image with a valid report screenshot.

## Windows Release README Sync

- Copied the refreshed README to `windows-release/README.md`.
- Mirrored `source/docs/images/readme/` to `windows-release/docs/images/readme/`.
- Verified all README image links resolve in both `source` and `windows-release`.
- Confirmed `windows-release/docs/images/readme/` contains the same eight README screenshots as `source/docs/images/readme/`.
- No C++ business logic, UI behavior, or CMake build system files were changed during the README/screenshot update.

## Validation Notes

- Debug builds were run after the main code changes.
- Release smoke launch confirmed `windows-release/PhantomDriveApp.exe` starts and responds.
- The latest Qt connect assertion fix was verified by launching the Debug build without the reported startup assertion.
- Confirmed DeepSeek direct smoke test with `deepseek-v4-flash` returned HTTP 200.
- Verified Debug and Release builds after the final scoring and AI report persistence changes.
- `git diff --check` was run on the touched files; only CRLF conversion warnings were reported.
- Verified the updated README does not contain a real DeepSeek API key; it only contains the placeholder `PASTE_YOUR_REAL_DEEPSEEK_API_KEY_HERE`.
- Verified the README no longer has a screenshot placeholder for Windows Release Launcher.
- Verified the README image references are not broken after syncing to `windows-release`.

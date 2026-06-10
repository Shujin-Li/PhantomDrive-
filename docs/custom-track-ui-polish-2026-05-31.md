# Custom Track UI polish - 2026-05-31

## Scope

This update addresses the UI group feedback from `a-b-custom-track-mode-integration-guide(1).md` and the follow-up issues:

1. Custom Track page did not fill the whole game page.
2. The editor canvas was too dark in dark mode, making Road and Wall hard to read.
3. Light mode had no explicit canvas/panel color handling.
4. Custom Track visual style needed to move closer to the PhantomDrive neon HUD references.

## Changes

- Added `MainWindow::setGameHeaderVisible(bool visible)` to hide the normal driving HUD row while Custom Track Editor is open.
- Collapsed game page margins, spacing, and the old HUD spacer during Custom Track editing, then restored them when returning to driving pages.
- Removed the right-side checklist reserve from the editor canvas layout so the 24 x 18 tile canvas can use the full available width.
- Expanded the Custom Track toolbar/action rows to the full window width with responsive button sizing.
- Added light/dark theme-aware panel, border, text, canvas, and grid colors.
- Reworked tile colors:
  - Road/Asphalt now render as high-contrast dark gray track surfaces.
  - Wall/Barrier now render as gray blocks instead of low-readability red/pink tiles.
  - Grass/background is brighter in both themes.
  - Start/Finish/CP/Item markers keep their original data behavior and remain visually distinct.

## Preserved behavior

- No changes to `TrackData` tile semantics.
- No changes to editor coordinates or world/tile conversion.
- No changes to Save/Load/Export JSON.
- No changes to `TrackValidator`.
- No changes to Custom Track play completion, scoring, report, or AI feedback logic.

## Verification

- Built successfully with CMake/MSVC in `build-codex-speed-ui`.
- Ran `a_b_ddl_event_integration_demo.exe` successfully.
- Ran `a_a_race_integration_test.exe` successfully.

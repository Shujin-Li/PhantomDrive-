# E-B Delivery Notes

Date: 2026-05-28

## What Was Completed

E-B is now wired into the main PhantomDrive demo flow:

- Arcade mode has two collectible powerup boxes: Boost and Shield.
- Boost and Shield use fixed demo drops so acceptance is repeatable.
- Boost temporarily raises the vehicle speed cap for 4 seconds.
- Shield absorbs collision response for 6 seconds.
- Learning mode traffic objects are live: traffic light, speed-limit zone, and pedestrian crossing.
- E-B traffic violations are emitted as `ViolationEvent` and sent to A-B `ScoreManager`.
- U-B feedback/HUD receives visible prompts through the existing `feedbackReady`, `LearningHUD`, sound, and status bar paths.

## Acceptance Steps

1. Build only the main app:
   `cmake --build build-mingw --target PhantomDriveApp -j 8`
2. Run:
   `build-mingw/bin/PhantomDriveApp.exe`
3. Arcade mode:
   - Drive through the yellow powerup boxes.
   - Verify `Boost Collected!` or `Shield Active!` appears.
   - Verify the box disappears, then respawns after a short delay.
4. Learning mode:
   - Drive through the speed-limit zone near the speed sign while fast enough.
   - Drive through the red-light area while the light is red.
   - Drive through the pedestrian crossing while a pedestrian is active.
   - Verify A-B prompts such as `Speeding! -5`, `Red Light Violation! -10`, and pedestrian-zone feedback.
5. Click `Finish Drive`.
   - Verify the report contains the E-B violation types and counts.

## Integration Notes

- `TrafficObjectManager` is the main source of E-B learning-mode violations.
- The main window disables `VehicleSensor` automatic speed-limit violation emission to avoid duplicate scoring.
- Existing A-B duplicate filtering remains in place, and E-B adds local object-level cooldowns to avoid prompt spam.
- No external assets were added; all visuals use `GameViewWidget`, `LearningHUD`, and existing sound generation.

# Custom Track Player and AI Settings

Date: 2026-06-12

Custom Track Editor now exposes two run settings directly in the top tool area:

- Players: Single Player or Two-Player. The default is Single Player.
- AI: Easy, Normal, Hard, or Adaptive. The default is Normal.

When Play This Track is pressed, the editor validates the current track, snapshots it for runtime use, then applies the selected player count and AI difficulty before starting Custom Track Mode. Single Player starts only P1 with the regular HUD. Two-Player starts P1 and P2 with the two-player HUD overlay.

P2 spawn uses the second TrackData start position when one exists. Tracks with only one Start keep the existing JSON format and use a nearby non-overlapping offset around P1.

AI route generation remains unchanged: Start -> checkpoints -> Finish. Difficulty reuses the existing Arcade AI path by mapping Easy, Normal, Hard, and Adaptive to the existing AI styles and speed/acceleration config. Adaptive follows the existing adaptive demo-style mapping rather than introducing a new AI system.

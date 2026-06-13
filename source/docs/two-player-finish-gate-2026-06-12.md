# Two-Player Finish Gate

Date: 2026-06-12

Two-player races now require both players to finish before the session ends.

Behavior:

- P1 reaching the finish sets P1 as finished but keeps the session active.
- P2 reaching the finish sets P2 as finished but keeps the session active.
- The report flow starts only after both P1 and P2 have finished.
- The first finisher gets a HUD banner indicating the game is waiting for the other player.
- This applies to Custom Track Two-Player and the existing Arcade two-player finish path.

No track format, vehicle physics, scoring algorithm, or AI API changes were made for this rule.

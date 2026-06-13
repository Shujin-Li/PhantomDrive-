# AI Coach Report Markdown and Player Cache

Date: 2026-06-12

The driving report panel now renders AI coach output with Qt's Markdown renderer instead of treating the response as plain label text.

Fixes:

- P1 and P2 keep separate AI coach markdown caches.
- ScoreManager emits coach report results with `vehicleId` and `sessionId` so the UI can attach each API response to the correct player.
- Switching between Player 1 and Player 2 no longer clears or overwrites the DeepSeek-generated report.
- Report refresh and history selection no longer replace an available API response with local fallback text for the same session.

Mode context:

- ScoreReport now carries `drivingMode` and `reportContext`.
- AIAPIClient includes this context in the prompt.
- Learning reports emphasize traffic-rule practice.
- Arcade reports emphasize racing pace, lap consistency, powerups, and AI competition.
- Custom Track reports emphasize checkpoints, finish discipline, and route layout.
- Two-Player reports include player-specific context.
- Adaptive AI reports mention AI difficulty adaptation and how the driver can respond.

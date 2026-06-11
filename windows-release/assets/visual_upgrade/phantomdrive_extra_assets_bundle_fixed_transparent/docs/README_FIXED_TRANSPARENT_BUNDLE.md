# PhantomDrive Extra Assets Bundle - FIXED TRANSPARENT VERSION

This version fixes the earlier fake checkerboard background problem.
The marker/custom images are now real RGBA transparent PNGs.

## Contents

### powerups/
- pd_powerup_custom.png    # transparent PNG, 512x512

### track_markers/
- pd_checkpoint.png        # transparent PNG, 512x512
- pd_start.png             # transparent PNG, 768x512
- pd_finish.png            # transparent PNG, 768x512

### traffic/
- pd_speed_limit_20.png ... pd_speed_limit_120.png

## Important Codex integration requirements
1. Replace the previous extra asset bundle with this fixed transparent bundle.
2. Do not use the old generated images if they show checkerboard backgrounds.
3. These images must be loaded as PNG with alpha and drawn with QPixmap/QPainter preserving transparency.
4. Scale images to target rectangles; do not draw them at original pixel size.
5. Suggested in-game sizes:
   - custom powerup icon: same size as other powerup icons, about 32-48 px in world/UI view.
   - checkpoint marker: max about 1.1-1.4 tile width, not a huge screen object.
   - start/finish gate: can span 2-4 tiles depending on design, but should not cover the road or player labels.
6. Keep current item text labels. For powerups, use icon + existing name text.
7. Start/checkpoint/finish replacement is visual only. Do not change start spawn, CP order validation, or finish trigger logic.
8. Speed limit signs can be selected dynamically by speed value, with fallback to pd_speed_limit_60.png or old QPainter drawing.

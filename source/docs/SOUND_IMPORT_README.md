# PhantomDrive Sound Import README

## Recommended countdown choice

Use this package as the active sound pack. It selects **Mixkit - Arcade race game countdown** as the target style because it is short, bright, racing/game-oriented, and avoids the dull low-male-voice feeling.

## Install

Copy/overwrite this package's `source/` directory into the PhantomDrive project root.

The current project should then contain:

```text
source/assets/sounds/countdown_three.wav
source/assets/sounds/countdown_two.wav
source/assets/sounds/countdown_one.wav
source/assets/sounds/countdown_go.wav
source/assets/sounds/countdown_full_321_go.wav
source/assets/sounds/powerup_*.wav
```

## Code expectation

If the code already maps `CountdownThree`, `CountdownTwo`, `CountdownOne`, and `CountdownGo` to these file names, no countdown logic rewrite is needed. The Codex sound agent only needs to ensure the files are copied to the runtime `assets/sounds` directory and that different powerups map to different `powerup_*.wav` files.

## Source shortcut

Open `MIXKIT_ARCADE_RACE_GAME_COUNTDOWN_SELECTED_SOURCE.url` to find the selected external source page.

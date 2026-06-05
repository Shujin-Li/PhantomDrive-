# PhantomDrive

A tri-mode 2D racing game framework built on Qt 6.

## Overview

PhantomDrive is a racing game framework that implements three game modes:
- **Arcade Mode**: High-speed racing with power-ups, AI opponents, lap timing, and race rankings
- **Learning Mode**: Practice driving with traffic rules, AI coaching, and safety scoring
- **Custom Track Mode**: Build a tile-based custom track in the app editor and race it

## Architecture

The framework uses the **Strategy Pattern** for game mode management and a **layered data-driven architecture**:

```
MainWindow
├── GameViewWidget          # 2D tile renderer (QPainter, top-down view)
├── VehiclePhysics          # Data-driven vehicle state (no MCObject inheritance)
├── VehicleSensor           # Samples position/speed/accel at configurable intervals
├── CollisionDetector       # Standalone collision detection (not physics-engine-driven)
├── DrivingDataCollector    # Composes VehicleSensor + CollisionDetector + Storage
│
├── GameMode (Strategy)
│   ├── ArcadeMode          # Power-ups, lap timing, race rankings
│   ├── LearningMode        # Traffic rules, scoring, AI coach
│   └── CustomTrackMode     # Tile editor + play session
│
├── AIOpponentManager       # Manages AI opponents with QLearning difficulty adaptation
│   └── AIOpponent (FSM)    # Conservative / Normal / Aggressive / Defensive styles
│
├── TrafficObjectManager     # Traffic lights, speed limits, pedestrian crossings
├── PowerupWorldRuntime     # 10 power-up types with timed effects
├── ScoreManager            # Event collection, throttled feedback, async AI coach
├── DrivingScoreCalculator   # Weighted scoring (Safety 40% / Rules 30% / Smoothness 20% / Efficiency 10%)
├── ArcadeHUD               # Right-panel: speedometer gauge + lap/time/position/boost
└── LearningHUD             # Right-panel: speed display + traffic state + violations
```

**Key design principle: physics is data-driven.** Vehicle position/speed are computed externally and fed to the engine — `VehiclePhysics` is a state shell, not a physics-driven object. This decouples the simulation loop from the rendering loop.

## Building

### Prerequisites
- Qt 6.8+ (Core, Gui, Widgets, OpenGL, Network, Charts, Xml, Multimedia)
- CMake 3.20+
- C++17 compatible compiler (MSVC 2022, MinGW, GCC)

### Build Steps

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

Build targets:
- `PhantomDriveApp` — main application
- `test_main_window` — main window test
- `test_visualization` — visualization test
- `test_keyboard_input` — keyboard input test
- `ab_report_widget_demo` — A-B report widget demo
- `a_b_ddl_event_integration_demo` — A-B event integration demo
- `a_a_race_integration_test` — A-A race ranking/integration test

### Running

```bash
build/bin/PhantomDriveApp
build/bin/a_b_ddl_event_integration_demo
build/bin/a_a_race_integration_test
```

## Usage Example

```cpp
#include <PhantomDrive/PhantomDrive.h>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    // Get the singleton instance
    GameModeManager* manager = GameModeManager::instance();

    // Create modes
    ArcadeMode* arcadeMode = new ArcadeMode();
    LearningMode* learningMode = new LearningMode();
    CustomTrackMode* customTrackMode = new CustomTrackMode();

    // Register modes
    manager->registerMode(arcadeMode, ModeType::Arcade);
    manager->registerMode(learningMode, ModeType::Learning);
    manager->registerMode(customTrackMode, ModeType::CustomTrack);

    // Switch to Arcade Mode with fade transition
    manager->switchTo(ModeType::Arcade, TransitionType::Fade);

    return app.exec();
}
```

## Module Structure

| Module | Description |
|--------|-------------|
| `GameMode` | Abstract base class for all game modes |
| `GameModeManager` | Singleton managing mode lifecycle and transitions |
| `ModeTransition` | Handles animated transitions between modes |
| `ArcadeMode` | Arcade racing with power-ups and AI |
| `LearningMode` | Learning mode with traffic rules and scoring |
| `CustomTrackMode` | In-app tile editor and custom track racing |
| `AIOpponentManager` | Manages AI opponents with waypoint following and QLearning adaptation |
| `AIOpponent` | Abstract AI opponent base with FSM states (Idle/Racing/Overtaking/Defending/Recovering/Finished) |
| `SimpleAIOpponent` | Waypoint-following AI with 4 configurable styles |
| `DrivingData` | Data structures for driving telemetry |
| `VehiclePhysics` | Data-driven vehicle state shell (throttle/brake/steering input) |
| `VehicleSensor` | Samples vehicle data at configurable intervals, emits `sensorDataReady` |
| `CollisionDetector` | Standalone collision detection with normal-based response |
| `DrivingDataCollector` | Composes VehicleSensor + CollisionDetector + DrivingDataStorage |
| `TrafficObjectManager` | Manages traffic lights, speed limits, pedestrian crossings |
| `TrafficRuleEnforcer` | Deduplicates violation events by position + type (1200ms cooldown per type) |
| `ScoreManager` | Event collection, throttled feedback, async AI coach report generation |
| `DrivingScoreCalculator` | Weighted sub-scores + coach advice generation |
| `TrafficLightObject` | Traffic light with red/yellow/green state and blink on red |
| `SpeedLimitSignObject` | Speed limit sign with configurable limit value |
| `PedestrianCrossingObject` | Pedestrian crossing zone with collision detection |
| `PowerupWorldRuntime` | Manages 10 power-up types (Boost, Shield, EMP, Missile, Oil, Invisibility, Teleport, Magnet, Repair, Random) |
| `PowerupBox` | Collectible power-up box placed on the track |
| `TrackManager` | Track loading, tile lookup, checkpoint management, statistics |
| `TrackData` | Tile grid data structure with metadata |
| `TrackIO` | Load/save tracks to `.pdtrack` / `.json` format |
| `CustomTrackEditorWidget` | 24×18 tile editor with 8 brush types and Y-flip runtime snapshot |
| `ArcadeHUD` | Right-panel HUD with neon speedometer gauge + lap/time/position/boost bars |
| `LearningHUD` | Right-panel HUD with speed display + traffic state + violation signals |
| `DrivingReportWidget` | Post-drive report with live speed chart, sub-score bars, AI coach advice, history trend |
| `InteractiveFeedback` | Floating feedback text overlay with cyberpunk styling |
| `SoundManager` | QMediaPlayer audio with SoundGenerator fallback |

## Scoring System

### Weighted Formula

```
totalScore = 0.4 × safetyScore
           + 0.3 × ruleComplianceScore
           + 0.2 × smoothnessScore
           + 0.1 × efficiencyScore
```

### Sub-scores (0–100 each)
- **Safety** (40%): penalties from collisions, red lights, pedestrian zones
- **Rule Compliance** (30%): penalties from speed violations
- **Smoothness** (20%): penalties from harsh acceleration/braking and sharp steering
- **Efficiency** (10%): penalties from excessive braking, off-throttle time

### Violation Deduplication
`TrafficRuleEnforcer` deduplicates events using a hash of `type:positionX` with 1200ms cooldown per key. Duplicate violations within the window are suppressed.

### Positive Feedback
`ScoreManager` emits "Great! Safe Driving!" (throttled to once per 8000ms) for sustained safe driving.

### AI Coach
`ScoreManager::finishSession()` spawns an async thread to call `AIAPIClient::generateCoachReport()`, emitting `coachReportReady` when complete. A `Mock` fallback is used when no API key is available.

### QLearning Feedback
A normalized `QLearningFeedback` struct is emitted on session end (`qLearningFeedbackReady`), containing `normalizedScore`, per-category penalties, `safetyRisk`, `ruleCompliance`, and `terminalPenalty`. `AIOpponentManager` uses this to adapt AI difficulty: increases speed/aggression on good performance, decreases on poor safety.

## AI Opponent System

### Styles and Default Configs

| Style | Max Speed | Acceleration | Handling | Aggression |
|-------|-----------|--------------|----------|------------|
| Conservative | 155 | 95 | 75 | 0.25 |
| Normal | 195 | 135 | 100 | 0.55 |
| Aggressive | 235 | 175 | 120 | 0.85 |
| Defensive | 175 | 115 | 110 | 0.35 |

### Race Ranking
`AIOpponentManager::updateRaceRankings()` uses `std::stable_sort` on `(lap × 100) + checkpointIndex + progressPercent/100`. Finished racers rank above active ones. Emits `rankingsUpdated` for HUD updates.

### Anti-Shortcut Checkpoint System
The race loop uses gate-based anti-shortcut logic:
1. Player must leave the north sector before checkpoints count
2. `m_blockCheckpointsUntilLeaveNorth` flag prevents lap shortcutting
3. North-sector exit resets the block so checkpoints register normally

### QLearning-Driven Difficulty Adaptation
```cpp
// On good performance → increase difficulty
maxSpeed += 5; aggression += 0.05;
// On poor safety → decrease difficulty
maxSpeed -= 5; aggression -= 0.05; riskTolerance -= 0.03;
```
All values are clamped: speed [80, 300], aggression/risk [0.1, 1.0].

### Simple Avoidance
O(n²) pairwise separation: if distance between any two AI cars < 54px, both are pushed apart by half the overlap.

### Boundary Recovery
If an AI car's position falls outside track bounds, it is clamped back to a safe position and rotated toward the track center.

## Traffic Objects

### Traffic Light
- States: Red / Yellow / Green
- Blink timer activates on Red (500ms interval)
- Emits `trafficLightChanged` signal with remaining time on red

### Speed Limit Sign
- Per-zone speed limits stored in a map by zone ID
- Active limit updated on zone enter/exit
- HUD displays "Limit: XX km/h"

### Pedestrian Crossing
- Activates on player proximity
- Triggers `ViolationEvent::PedestrianCollision`
- Triggers penalty deduction in Learning Mode

## Power-Up System

10 power-up types with timed effects:

| Type | Effect | Duration |
|------|--------|----------|
| Boost | 1.5× speed | 5s |
| Shield | Blue ellipse overlay, no effect | 8s |
| EMP | Triggers EMP animation (stub) | — |
| Repair | Restores health (stub) | — |
| Missile | Triggers missile effect (stub) | — |
| Oil | Sinusoidal steering wobble | until off oil tile |
| Invisibility | 35% opacity, auto-returns to anchor if stuck | 10s |
| Teleport | Snaps to random track position (stub) | — |
| Magnet | Orange dashed circle overlay | 8s |
| Random | Picks random from pool | — |

## Custom Track Editor

### Tile Brushes (8 types)
Road, Grass, Barrier, Start, Finish, Checkpoint, ItemBox, Erase

### Editor Grid
24 rows × 18 columns, tile size computed from available space

### Placement Rules
- Only one Start tile allowed (auto-clears previous on new placement)
- Finish tiles remove any previous FinishLine before placing
- Checkpoints auto-index by placement order, renumber on removal
- ItemBox tiles add to `TrackData::m_itemBoxPositions`

### Coordinate Conversion
`cloneCustomTrackSnapshot()` applies Y-flip conversion (editor rows grow downward, physics world is Y-up) before passing to the runtime engine.

### Validation Hints
Status bar displays real-time validation feedback (e.g., "Need Start tile", "Need Finish tile")

## UI Layout

### Game Page Layout (as of 2026-05-31)
```
┌──────────────────────────────────────────────────────────┐
│ [Speed: 0 km/h | Limit: -- km/h | Light: --]  [ARCADE MODE]  [Back] [Finish Drive] [Exit Game] │
├──────────────────────────────────────────────────────────┤
│                                            │             │
│                                            │  ArcadeHUD  │
│         GameViewWidget (2D top-down)        │  (300px)    │
│                                            │             │
│                                            │  Speed gauge│
│                                            │  Lap / Time  │
│                                            │  Position   │
│                                            │  Boost bar  │
└──────────────────────────────────────────────────────────┘
```

### ArcadeHUD Components
- Mode title (color by mode: pink=Arcade, green=Learning, cyan=CustomTrack)
- Speed / Limit / Light row
- `SpeedometerWidget` — 200×200 custom-painted gauge with neon arc, tick marks, needle, center speed text
- Lap card + Lap Time + Total Time side-by-side
- Position card (1st / 2nd / 3rd / …)
- Boost bar (gradient progress)
- AI Speed card
- Best Lap card
- STOP badge overlay (visible on red light)
- COUNTDOWN overlay (3-2-1-GO centered over gauge)

### Driving Report Widget
- Header: title + "New Drive" + "Back to Menu" buttons
- Summary cards: Average Speed, Max Speed, Total Score, Grade
- Sub-score progress bars (Safety / Rules / Smoothness / Efficiency)
- Live speed chart (QSplineSeries, -60s rolling window)
- Score ratio widget
- AI Coach advice
- History trend chart (5 sessions, 5 series)
- Violation events table

## Integration

This framework is designed to be integrated with:
- **DustRacing2D** physics and rendering engine (MiniCore physics under `src/physics/`)
- **AI Core Group** (AI opponents, FSM, waypoint navigation)
- **Application Group** (UI, menus, HUD)
- **Scoring Group** (AI coach, score reports, QLearning feedback)

## A-B Scoring and AI Coach Module

On branch `v1.0-a-b`, the A-B module has completed the core loop for deterministic driving scoring and AI coach report generation (including signal flow, JSON/Markdown output, collector integration, and DeepSeek → Zhipu/GLM → Mock fallback strategy). This is not a claim that full UI or full game loop is complete.

See:
- `docs/a-b-integration-guide.md`
- `docs/a-b-v2-integration-guide.md`
- `docs/a-b-demo-script.md`

---

## Change Log

### 2026-05-31 — Top HUD Bar Refactoring
- **Removed** `GameTopBar` floating overlay widget (`GameTopBar.h/cpp` removed from CMakeLists.txt and source)
- **Integrated** mode title (`label_ModeTitle`) and Exit Game button (`btn_ExitGame_Top`) directly into the `.ui` `hudLayout` QHBoxLayout
- **New layout**: `[Speed/Limit/Light] [spacer] [ARCADE MODE (200px minW)] [spacer] [Back] [Finish Drive] [Exit Game]`
- **Button styles** applied in `setupUi()`: Back (deep blue border `#0088CC`), Finish Drive (neon red `#FF2255`), Exit Game (dark red `#CC1133`), all 52px height, 8px border-radius
- **Mode title** auto-updates on mode switch: Learning Mode (green `#00FFA0`), Custom Track (cyan `#59F7FF`), Arcade Mode (neon pink `#FF3366`)
- **Right-side HUD** positioned at `y = hudBarHeight(42) + 8` below the unified top bar
- `setGameHeaderVisible()` now controls all top bar widgets

### 2026-05-31 — Custom Track UI Polish
- Added `MainWindow::setGameHeaderVisible(bool)` to hide driving HUD during Custom Track editing
- Collapsed game page margins/spacing during Custom Track editing, restored on return
- Removed right-side checklist reserve from editor canvas for full 24×18 tile canvas width
- Expanded Custom Track toolbar/action rows to full window width
- Added light/dark theme-aware colors for panel, border, text, canvas, grid
- Reworked tile colors: Road/Asphalt (high-contrast dark gray), Wall/Barrier (gray), Grass (bright green)
- `setGameHeaderVisible()` now collapses page margins to 0 and HUD spacer to 0 when entering editor

### 2026-05-30 — Speed Display Unit Fix
- Added `MainWindow::speedToDisplayKmh(qreal)` — unified physics-to-display speed conversion
- `displaySpeedKmh()` now calls the unified function
- AI car speed labels updated to use `speedToDisplayKmh(ai->getSpeed())`
- Vehicle sensor writes km/h units so `DrivingData`, limit comparisons, and report charts share the same unit
- Collision events, Safe Driving ticks, and DrivingReportWidget live chart all now use km/h consistently
- Internal `m_playerSpeed` / `ai->getSpeed()` remain in physics units for movement/collision math

### 2026-05-27 — Single-Person Final Integration Delivery
See `docs/PhantomDrive_single_person_final_delivery.md` for the full acceptance checklist and file-level change summary.

Key additions:
- Race progress tracking and player/AI unified ranking (`AIOpponentManager`)
- AI finish detection and `raceResultsToJson()` export
- AI boundary recovery and simple avoidance
- AI difficulty selection in main menu
- Custom track loading from `.pdtrack` / `.json` files
- History JSON save/load via `SaveLoadManager`
- DrivingReportWidget history trend chart (5 sessions, 5 metrics)
- SoundManager with QMediaPlayer + SoundGenerator fallback
- Async AI coach report generation in `ScoreManager`

### 2026-05-27 — A-B v2 Scoring Module
- Throttled violation feedback: same type deduplicated for ~1200ms
- Safe driving positive feedback: "Great! Safe Driving!" throttled to 8000ms quiet period
- `TrafficRuleEnforcer::filterDuplicates()` — dedupe by `type:positionX` hash
- `ScoreManager::finishSession()` emits `qLearningFeedbackReady` with normalized feedback
- `DrivingScoreCalculator` produces sub-scores: Safety (40%) / RuleCompliance (30%) / Smoothness (20%) / Efficiency (10%)
- Coach advice rule-based generation (collision/speed/brake/steer tips)
- QLearning feedback normalization: `normalizedScore`, per-category penalties, `safetyRisk`, `terminalPenalty`
- Async coach report thread in `ScoreManager::generateCoachReportAsync()`
- DeepSeek → Zhipu/GLM → Mock fallback strategy in `AIAPIClient`

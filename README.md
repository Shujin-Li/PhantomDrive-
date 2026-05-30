# PhantomDrive

A tri-mode 2D racing game framework built on Qt 6.

## Overview

PhantomDrive is a racing game framework that implements three game modes:
- **Arcade Mode**: High-speed racing with power-ups, AI opponents, and lap timing
- **Learning Mode**: Practice driving with traffic rules and AI coaching
- **Custom Track Mode**: Build a tile-based custom track in the main app and race it

## Architecture

The framework uses the **Strategy Pattern** for game mode management:

```
GameMode (Abstract Base Class)
    ├── ArcadeMode
    └── LearningMode

GameModeManager (Singleton)
    └── Manages mode switching with animated transitions
```

## Building

### Prerequisites
- Qt 6.8+ (Core, Gui, Widgets)
- CMake 3.20+
- C++17 compatible compiler

### Build Steps

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
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
| `DrivingData` | Data structures for driving telemetry |
| `IDrivingDataCollector` | Interface for data collection (to be implemented by Engine Group B) |

## Integration

This framework is designed to be integrated with:
- **DustRacing2D** physics and rendering engine
- **AI Core Group** (AI opponents, FSM)
- **Application Group** (UI, menus, HUD)
- **Scoring Group** (AI coach, score reports)

## License

This project is part of a university course project at SCUT.

## A-B Scoring and AI Coach Module

On branch `v1.0-a-b`, the A-B module has completed the core loop for deterministic driving scoring and AI coach report generation (including signal flow, JSON/Markdown output, collector integration, and DeepSeek -> Zhipu/GLM -> Mock fallback strategy). This is not a claim that full UI or full game loop is complete.

See:
- `docs/a-b-integration-guide.md`
- `docs/a-b-demo-script.md`

## 2026-05-27 Integrated Final Status

This package is the single-person integrated final delivery. It uses the U-B HUD/history/audio package as the source base, keeps the A-B v2 scoring demo and integration guide, and adds the missing A-A DDL2 race pieces: player + AI ranking, AI finish result logic, boundary recovery, simple AI avoidance, AI difficulty selection, custom track loading, and `a_a_race_integration_test`.

See `docs/PhantomDrive_single_person_final_delivery.md` for the final acceptance checklist and file-level change summary.

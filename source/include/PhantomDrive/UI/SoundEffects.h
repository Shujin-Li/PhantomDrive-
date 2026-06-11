#ifndef SOUND_EFFECTS_H
#define SOUND_EFFECTS_H

namespace PhantomDrive {

enum class SoundCategory {
    UI,
    Countdown,
    Race,
    Car,
    Item
};

enum class SoundEffect {
    // --- Countdown (voice synthesis) ---
    CountdownThree,
    CountdownTwo,
    CountdownOne,
    CountdownGo,

    // --- UI ---
    ButtonClick,
    ButtonBack,
    RaceStart,
    Pause,
    Resume,
    RaceFinish,
    Victory,
    Fail,

    // --- Car ---
    EngineIdle,
    EngineAccel,
    EngineHighSpeed,
    Brake,
    Crash,
    OffRoad,
    Boost,

    // --- Race ---
    Checkpoint,
    LapComplete,
    FinalLap,

    // --- Items ---
    PowerupCollect,

    // --- Legacy (kept for compat) ---
    CountdownBeep,
    Collision,
    SpeedBoost,
    Violation
};

} // namespace PhantomDrive

#endif // SOUND_EFFECTS_H

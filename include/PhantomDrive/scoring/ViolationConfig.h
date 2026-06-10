#pragma once

#include <QtGlobal>

namespace PhantomDrive {

struct ViolationConfig {
    static constexpr int collisionPenalty = 15;
    static constexpr int speedViolationPenalty = 6;
    static constexpr int redLightPenalty = 12;
    static constexpr int pedestrianPenalty = 20;
    static constexpr int wrongWayPenalty = 10;

    static constexpr int speedCooldownMs = 1000;
    static constexpr int redLightCooldownMs = 1500;
    static constexpr int pedestrianCooldownMs = 1000;
};

}

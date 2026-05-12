#include <PhantomDrive/ai/AggressiveAI.h>

namespace PhantomDrive {

AggressiveAI::AggressiveAI()
{
    m_profile.maxThrottle = 1.0f;

    m_profile.steeringStrength = 0.9f;

    m_profile.brakeSensitivity = 0.2f;
}

}
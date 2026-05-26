#include <PhantomDrive/ai/StrategicAI.h>

namespace PhantomDrive {

StrategicAI::StrategicAI()
{
    m_profile.maxThrottle = 0.7f;

    m_profile.steeringStrength = 0.6f;

    m_profile.brakeSensitivity = 0.5f;
}

}
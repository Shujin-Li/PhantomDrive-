#include <PhantomDrive/ai/ConservativeAI.h>

namespace PhantomDrive {

ConservativeAI::ConservativeAI()
{
    m_profile.maxThrottle = 0.5f;

    m_profile.steeringStrength = 0.3f;

    m_profile.brakeSensitivity = 0.9f;
}

}
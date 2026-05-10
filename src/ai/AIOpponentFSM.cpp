#include <PhantomDrive/ai/AIOpponentFSM.h>

namespace PhantomDrive {

AIOpponentFSM::AIOpponentFSM(QObject* parent)
    : AIOpponent(parent)
{
}

void AIOpponentFSM::update(float deltaTime)
{
    Q_UNUSED(deltaTime)

    m_currentDecision.throttle = 0.8f;
    m_currentDecision.brake = 0.0f;
    m_currentDecision.steering = 0.0f;
}

AIDecision AIOpponentFSM::makeDecision()
{
    return m_currentDecision;
}

}
#include <PhantomDrive/ai/AIOpponentFSM.h>

namespace PhantomDrive {

AIOpponentFSM::AIOpponentFSM(QObject* parent)
    : AIOpponent(parent)
{
}

void AIOpponentFSM::update(float deltaTime)
{
    Q_UNUSED(deltaTime)

    if (m_waypoints.isEmpty())
        return;

    Waypoint target = m_waypoints[currentWaypointIndex];

    fakePositionX += 10.0f;

    if (fakePositionX >= target.x)
    {
        currentWaypointIndex++;

        if (currentWaypointIndex >= m_waypoints.size())
        {
            currentWaypointIndex = 0;
        }
    }

    if (target.x > fakePositionX)
    {
        m_currentDecision.steering = m_profile.steeringStrength;
    }
    else
    {
        m_currentDecision.steering = -m_profile.steeringStrength;
    }

    m_currentDecision.throttle = m_profile.maxThrottle;
    m_currentDecision.brake = 0.0f;
}

AIDecision AIOpponentFSM::makeDecision()
{
    return m_currentDecision;
}

}
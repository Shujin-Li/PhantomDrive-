#include <PhantomDrive/ai/AIOpponent.h>

namespace PhantomDrive {

AIOpponent::AIOpponent(QObject* parent)
    : QObject(parent)
{
}

AIOpponent::~AIOpponent()
{
}

void AIOpponent::setWaypoints(const QList<Waypoint>& waypoints)
{
    m_waypoints = waypoints;
}

}
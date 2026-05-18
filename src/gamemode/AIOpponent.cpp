#include "AIOpponent.h"

#include <cmath>

namespace PhantomDrive {

AIOpponent::AIOpponent(const QString& id, QObject* parent)
    : QObject(parent)
    , m_id(id)
    , m_active(true)
    , m_finished(false)
    , m_playerPosition(0, 0)
{
}

AIOpponent::~AIOpponent()
{
}

void AIOpponent::setPlayerPosition(const QVector2D& position)
{
    m_playerPosition = position;
}

void AIOpponent::setOtherOpponentPositions(const QMap<QString, QVector2D>& positions)
{
    m_otherOpponentPositions = positions;
}

QVector2D AIOpponent::getNearestOpponentPosition() const
{
    QVector2D nearest(0, 0);
    qreal minDist = std::numeric_limits<qreal>::max();

    for (auto it = m_otherOpponentPositions.begin(); it != m_otherOpponentPositions.end(); ++it) {
        qreal dist = (it.value() - getPosition()).length();
        if (dist < minDist) {
            minDist = dist;
            nearest = it.value();
        }
    }
    return nearest;
}

qreal AIOpponent::getDistanceToPlayer() const
{
    return (m_playerPosition - getPosition()).length();
}

bool AIOpponent::isPlayerAhead() const
{
    QVector2D toPlayer = m_playerPosition - getPosition();
    qreal angleToPlayer = std::atan2(toPlayer.y(), toPlayer.x());
    qreal relativeAngle = normalizeAngle(angleToPlayer - getRotation());

    return qAbs(relativeAngle) < M_PI_2;
}

QList<Waypoint> AIOpponent::generateWaypointsFromTrack(const QString& trackDataPath)
{
    QList<Waypoint> waypoints;

    // TODO: 从赛道数据文件加载路径点
    // 实现方式由 TrackManager 提供接口后实现
    Q_UNUSED(trackDataPath);

    return waypoints;
}

qreal AIOpponent::normalizeAngle(qreal angle)
{
    while (angle > M_PI) angle -= 2 * M_PI;
    while (angle < -M_PI) angle += 2 * M_PI;
    return angle;
}

qreal AIOpponent::calculateLateralOffset(const QVector2D& from, const QVector2D& to, qreal heading)
{
    QVector2D toTarget = to - from;
    qreal angleToTarget = std::atan2(toTarget.y(), toTarget.x());
    qreal lateralAngle = normalizeAngle(angleToTarget - heading);

    return toTarget.length() * std::sin(lateralAngle);
}

} // namespace PhantomDrive

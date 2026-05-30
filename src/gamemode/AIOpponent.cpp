#include "AIOpponent.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtMath>
#include <algorithm>
#include <cmath>
#include <limits>

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
    qreal relativeAngle = normalizeAngle(angleToPlayer - qDegreesToRadians(getRotation()));

    return qAbs(relativeAngle) < M_PI_2;
}

QList<Waypoint> AIOpponent::generateWaypointsFromTrack(const QString& trackDataPath)
{
    QList<Waypoint> waypoints;
    QFile file(trackDataPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return waypoints;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return waypoints;
    }

    const QJsonObject root = doc.object();
    const QJsonArray explicitWaypoints = root.value(QStringLiteral("waypoints")).toArray();
    for (int i = 0; i < explicitWaypoints.size(); ++i) {
        const QJsonObject item = explicitWaypoints.at(i).toObject();
        waypoints.append(Waypoint(
            QVector2D(item.value(QStringLiteral("x")).toDouble(),
                      item.value(QStringLiteral("y")).toDouble()),
            item.value(QStringLiteral("preferredSpeed")).toDouble(100.0),
            item.value(QStringLiteral("isCorner")).toBool(false),
            item.value(QStringLiteral("cornerSeverity")).toInt(0),
            item.value(QStringLiteral("index")).toInt(i)));
    }

    if (!waypoints.isEmpty()) {
        return waypoints;
    }

    struct IndexedWaypoint {
        int index;
        Waypoint waypoint;
    };

    QList<IndexedWaypoint> indexed;
    const QJsonArray checkpoints = root.value(QStringLiteral("checkpoints")).toArray();
    for (int i = 0; i < checkpoints.size(); ++i) {
        const QJsonObject item = checkpoints.at(i).toObject();
        const int routeIndex = item.value(QStringLiteral("indexInRoute")).toInt(i);
        indexed.append({
            routeIndex,
            Waypoint(
                QVector2D(item.value(QStringLiteral("positionX")).toDouble(),
                          item.value(QStringLiteral("positionY")).toDouble()),
                100.0,
                false,
                0,
                routeIndex)
        });
    }

    std::sort(indexed.begin(), indexed.end(), [](const IndexedWaypoint& lhs, const IndexedWaypoint& rhs) {
        return lhs.index < rhs.index;
    });

    for (const IndexedWaypoint& item : indexed) {
        waypoints.append(item.waypoint);
    }

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

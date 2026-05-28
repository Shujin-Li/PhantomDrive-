#include "SimpleAIOpponent.h"

#include <QtMath>
#include <QJsonArray>
#include <limits>

namespace PhantomDrive {

SimpleAIOpponent::SimpleAIOpponent(const QString& id, QObject* parent)
    : AIOpponent(id, parent)
    , m_name(id)
    , m_currentWaypointIndex(0)
    , m_position(0, 0)
    , m_rotation(0)
    , m_velocity(0, 0)
    , m_speed(0)
    , m_steeringAngle(0)
    , m_state(AIState::Idle)
    , m_stateDuration(0)
    , m_currentLap(0)
    , m_lapTime(0)
    , m_bestLapTime(0)
    , m_racePosition(0)
    , m_checkpointsPassed(0)
{
}

SimpleAIOpponent::~SimpleAIOpponent()
{
}

Waypoint SimpleAIOpponent::getCurrentWaypoint() const
{
    if (m_currentWaypointIndex >= 0 && m_currentWaypointIndex < m_waypoints.size()) {
        return m_waypoints[m_currentWaypointIndex];
    }
    return Waypoint();
}

Waypoint SimpleAIOpponent::getNextWaypoint() const
{
    if (m_waypoints.isEmpty()) {
        return Waypoint();
    }
    int nextIndex = (m_currentWaypointIndex + 1) % m_waypoints.size();
    return m_waypoints[nextIndex];
}

Waypoint SimpleAIOpponent::getWaypointAt(int index) const
{
    if (index >= 0 && index < m_waypoints.size()) {
        return m_waypoints[index];
    }
    return Waypoint();
}

qreal SimpleAIOpponent::getTotalPathLength() const
{
    qreal total = 0;
    for (int i = 1; i < m_waypoints.size(); ++i) {
        total += (m_waypoints[i].position - m_waypoints[i-1].position).length();
    }
    return total;
}

qreal SimpleAIOpponent::getRemainingPathLength() const
{
    qreal remaining = 0;
    for (int i = m_currentWaypointIndex + 1; i < m_waypoints.size(); ++i) {
        remaining += (m_waypoints[i].position - m_waypoints[i-1].position).length();
    }
    return remaining;
}

qreal SimpleAIOpponent::getProgressPercentage() const
{
    if (m_waypoints.isEmpty()) return 0;
    const qreal waypointProgress = static_cast<qreal>(m_currentWaypointIndex)
        / qMax(1, m_waypoints.size()) * 100.0;
    return qBound(0.0, waypointProgress, 100.0);
}

void SimpleAIOpponent::setState(AIState state)
{
    AIState oldState = m_state;
    if (oldState != state) {
        onStateExit(oldState);
        m_state = state;
        m_stateDuration = 0;
        onStateEnter(state);
        emit stateChanged(oldState, state);
    }
}

AIDecision SimpleAIOpponent::makeDecision()
{
    AIDecision decision;
    decision.throttle = calculateThrottle();
    decision.brake = calculateBraking();
    decision.steering = calculateSteering().x();
    decision.suggestedState = m_state;
    return decision;
}

void SimpleAIOpponent::update(qint64 elapsedMs)
{
    if (m_finished || m_state == AIState::Finished) {
        return;
    }

    m_stateDuration += elapsedMs;
    AIStyle style = m_config.style;

    if (m_speed < 20.0)
    {
        setState(AIState::Recovering);
    }
    else if (getDistanceToPlayer() < 120.0)
    {
        if (style == AIStyle::Aggressive)
        {
            setState(AIState::Overtaking);
        }
        else if (style == AIStyle::Defensive)
        {
            setState(AIState::Defending);
        }
        else
        {
            setState(AIState::Racing);
        }
    }
    else
    {
        setState(AIState::Racing);
    }

    if (!m_waypoints.isEmpty() && m_active && !m_finished) {
        AIDecision decision = makeDecision();

        m_steeringAngle = decision.steering * 30.0;

        m_rotation += m_steeringAngle * (elapsedMs / 1000.0);

        qreal throttle = decision.throttle;
        qreal brake = decision.brake;

        if (m_state == AIState::Overtaking)
        {
            throttle *= 1.2;
        }

        if (m_state == AIState::Recovering)
        {
            throttle *= 0.5;
            brake += 0.3;
        }

        if (m_state == AIState::Defending)
        {
            throttle *= 0.8;
        }

        qreal acceleration = m_config.acceleration * throttle - 50.0 * brake;
        m_speed += acceleration * (elapsedMs / 1000.0);
        m_speed = qMax(0.0, qMin(m_config.maxSpeed, m_speed));

        m_rotation += m_steeringAngle * (elapsedMs / 1000.0) * 2.0;
        qreal rotationRad = m_rotation * M_PI / 180.0;
        m_velocity = QVector2D(qCos(rotationRad) * m_speed, qSin(rotationRad) * m_speed);

        m_position += m_velocity * (elapsedMs / 1000.0);

        Waypoint currentWP = getCurrentWaypoint();
        qreal distToWP = (currentWP.position - m_position).length();
        if (distToWP < 50.0) {
            const int reachedIndex = m_currentWaypointIndex;
            m_checkpointsPassed = qMax(m_checkpointsPassed, reachedIndex + 1);
            m_currentWaypointIndex++;
            emit waypointReached(reachedIndex);

            if (m_currentWaypointIndex >= m_waypoints.size()) {
                m_currentWaypointIndex = 0;
                m_currentLap++;
                m_checkpointsPassed = 0;
                if (m_bestLapTime <= 0 || m_lapTime < m_bestLapTime) {
                    m_bestLapTime = m_lapTime;
                    emit bestLapTimeUpdated(m_bestLapTime);
                }
                emit lapCompleted(m_currentLap, m_lapTime);
                m_lapTime = 0;
            }
        }
    }

    m_lapTime += elapsedMs / 1000.0;
    emit updated(elapsedMs);
}

QVector2D SimpleAIOpponent::calculateSteering()
{
    if (m_waypoints.isEmpty()) return QVector2D(0, 0);

    Waypoint targetWP = getCurrentWaypoint();
    QVector2D toTarget = targetWP.position - m_position;
    if (m_state == AIState::Overtaking)
    {
        const QVector2D nextDirection = (getNextWaypoint().position - targetWP.position).normalized();
        QVector2D lateral(-nextDirection.y(), nextDirection.x());
        if (lateral.isNull()) {
            lateral = QVector2D(1.0, 0.0);
        }
        const qreal side = (m_config.style == AIStyle::Defensive) ? -1.0 : 1.0;
        toTarget += lateral * side * 70.0;
    }

    qreal targetAngle = std::atan2(toTarget.y(), toTarget.x());
    qreal currentAngleRad = m_rotation * M_PI / 180.0;

    qreal angleDiff = targetAngle - currentAngleRad;
    while (angleDiff > M_PI) angleDiff -= 2 * M_PI;
    while (angleDiff < -M_PI) angleDiff += 2 * M_PI;

    qreal steeringValue = angleDiff / M_PI;
    steeringValue = qBound(-1.0, steeringValue, 1.0);

    switch (m_config.style)
    {
    case AIStyle::Aggressive:
        steeringValue *= 1.2;
        break;

    case AIStyle::Conservative:
        steeringValue *= 0.7;
        break;

    case AIStyle::Defensive:
        steeringValue *= 0.5;
        break;

    case AIStyle::Normal:
    default:
        steeringValue *= 1.0;
        break;
    }

    return QVector2D(steeringValue, 0);
}

qreal SimpleAIOpponent::calculateThrottle()
{
    AIStyle style = m_config.style;
    if (m_waypoints.isEmpty()) return 0.5;

    Waypoint currentWP = getCurrentWaypoint();
    qreal distToWP = (currentWP.position - m_position).length();

    if (currentWP.isCorner)
    {
        qreal cornerFactor = 1.0 - (currentWP.cornerSeverity / 3.0);

        switch (style)
        {
        case AIStyle::Aggressive:
            return cornerFactor * 1.0;

        case AIStyle::Conservative:
            return cornerFactor * 0.5;

        case AIStyle::Defensive:
            return cornerFactor * 0.4;

        case AIStyle::Normal:
        default:
            return cornerFactor * 0.7;
        }
    }

    if (distToWP < 100.0) {
        return 0.5;
    }

    switch (style)
    {
    case AIStyle::Aggressive:
        return 1.0;

    case AIStyle::Conservative:
        return 0.7;

    case AIStyle::Defensive:
        return 0.5;

    case AIStyle::Normal:
    default:
        return 0.85;
    }
}

qreal SimpleAIOpponent::calculateBraking()
{
    if (m_waypoints.isEmpty()) return 0;

    Waypoint currentWP = getCurrentWaypoint();
    if (currentWP.isCorner) {
        qreal distToWP = (currentWP.position - m_position).length();
        if (distToWP < 150.0) {
            return currentWP.cornerSeverity / 3.0 * 0.8;
        }
    }

    if (m_speed > currentWP.preferredSpeed && currentWP.preferredSpeed > 0) {
        return (m_speed - currentWP.preferredSpeed) / m_config.maxSpeed;
    }

    switch (m_config.style)
    {
    case AIStyle::Aggressive:
        return 0.1;

    case AIStyle::Conservative:
        return 0.5;

    case AIStyle::Defensive:
        return 0.7;

    case AIStyle::Normal:
    default:
        return 0.3;
    }
}

bool SimpleAIOpponent::usePowerup(int slot)
{
    if (slot >= 0 && slot < m_powerups.size()) {
        int powerupType = m_powerups.takeAt(slot);
        emit powerupUsed(slot, powerupType);
        return true;
    }
    return false;
}

void SimpleAIOpponent::onCollision(const QString& objectId, const QVector2D& point)
{
    Q_UNUSED(objectId);
    Q_UNUSED(point);
    m_speed *= 0.5;
    setState(AIState::Recovering);
    emit collisionOccurred(objectId, point);
}

void SimpleAIOpponent::onNearMiss(const QString& opponentId, qreal distance)
{
    Q_UNUSED(opponentId);
    Q_UNUSED(distance);
}

void SimpleAIOpponent::onOvertaken(const QString& opponentId)
{
    Q_UNUSED(opponentId);
    if (m_state == AIState::Racing) {
        setState(AIState::Defending);
    }
}

void SimpleAIOpponent::onOvertake(const QString& opponentId)
{
    Q_UNUSED(opponentId);
    if (m_state == AIState::Defending) {
        setState(AIState::Racing);
    }
}

QJsonObject SimpleAIOpponent::toJson() const
{
    QJsonObject json;
    json["id"] = m_id;
    json["name"] = m_name;
    json["style"] = static_cast<int>(m_config.style);
    json["position_x"] = m_position.x();
    json["position_y"] = m_position.y();
    json["rotation"] = m_rotation;
    json["speed"] = m_speed;
    json["state"] = static_cast<int>(m_state);
    json["lap"] = m_currentLap;
    json["racePosition"] = m_racePosition;
    return json;
}

void SimpleAIOpponent::fromJson(const QJsonObject& json)
{
    m_name = json["name"].toString();
    m_config.style = static_cast<AIStyle>(json["style"].toInt());
    m_position = QVector2D(json["position_x"].toDouble(), json["position_y"].toDouble());
    m_rotation = json["rotation"].toDouble();
    m_speed = json["speed"].toDouble();
    m_state = static_cast<AIState>(json["state"].toInt());
    m_currentLap = json["lap"].toInt();
    m_racePosition = json["racePosition"].toInt();
}

QVariantMap SimpleAIOpponent::getStateData() const
{
    QVariantMap data;
    data["id"] = m_id;
    data["position"] = m_position;
    data["rotation"] = m_rotation;
    data["speed"] = m_speed;
    data["state"] = static_cast<int>(m_state);
    data["lap"] = m_currentLap;
    return data;
}

void SimpleAIOpponent::loadStateData(const QVariantMap& data)
{
    m_id = data["id"].toString();
    m_position = data["position"].value<QVector2D>();
    m_rotation = data["rotation"].toReal();
    m_speed = data["speed"].toReal();
    m_state = static_cast<AIState>(data["state"].toInt());
    m_currentLap = data["lap"].toInt();
}

void SimpleAIOpponent::onStateEnter(AIState newState)
{
    Q_UNUSED(newState);
}

void SimpleAIOpponent::onStateExit(AIState oldState)
{
    Q_UNUSED(oldState);
}

qreal SimpleAIOpponent::calculateDistanceToWaypoint(const Waypoint& waypoint) const
{
    return (waypoint.position - m_position).length();
}

qreal SimpleAIOpponent::calculateAngleToWaypoint(const Waypoint& waypoint) const
{
    QVector2D toWP = waypoint.position - m_position;
    return std::atan2(toWP.y(), toWP.x());
}

int SimpleAIOpponent::findNearestWaypointIndex() const
{
    int nearestIndex = 0;
    qreal minDist = std::numeric_limits<qreal>::max();

    for (int i = 0; i < m_waypoints.size(); ++i) {
        qreal dist = (m_waypoints[i].position - m_position).length();
        if (dist < minDist) {
            minDist = dist;
            nearestIndex = i;
        }
    }
    return nearestIndex;
}

int SimpleAIOpponent::findNextRelevantWaypoint() const
{
    return m_currentWaypointIndex + 1;
}

} // namespace PhantomDrive

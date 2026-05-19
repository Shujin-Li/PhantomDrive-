#include "SimpleAIOpponent.h"
#include <QDebug>
#include <QtMath>
#include <QJsonArray>

namespace PhantomDrive {
QString stateToString(AIState state)
{
    switch (state)
    {
    case AIState::Idle:
        return "Idle";

    case AIState::Racing:
        return "Racing";

    case AIState::Overtaking:
        return "Overtaking";

    case AIState::Defending:
        return "Defending";

    case AIState::Recovering:
        return "Recovering";

    case AIState::Finished:
        return "Finished";

    default:
        return "Unknown";
    }
}

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
    int nextIndex = m_currentWaypointIndex + 1;
    if (nextIndex < m_waypoints.size()) {
        return m_waypoints[nextIndex];
    }
    return Waypoint();
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
    return static_cast<qreal>(m_currentWaypointIndex) / m_waypoints.size() * 100.0;
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
    m_stateDuration += elapsedMs;

    if (!m_waypoints.isEmpty() && m_active && !m_finished) {
        AIDecision decision = makeDecision();
        Waypoint currentWP = getCurrentWaypoint();

        qreal distToWP = (currentWP.position - m_position).length();

        if (currentWP.isCorner
            && distToWP < 150.0
            && m_speed > currentWP.preferredSpeed)
        {
            setState(AIState::Recovering);
        }
        else if (m_speed < 3.0)
        {
            setState(AIState::Idle);
        }
        else if (m_speed > 180.0)
        {
            setState(AIState::Overtaking);
        }
        else
        {
            setState(AIState::Racing);
        }

        m_steeringAngle = decision.steering * 30.0;
        qreal steeringFactor = 1.0;

        if (m_config.style == AIStyle::Aggressive)
        {
            steeringFactor = 1.3;
        }

        if (m_config.style == AIStyle::Conservative)
        {
            steeringFactor = 1.1;
        }

        m_rotation +=
            m_steeringAngle
            * steeringFactor
            * 2.5
            * (elapsedMs / 1000.0);

        qreal throttle = decision.throttle;
        qreal brake = decision.brake;

        if (m_speed < 5.0)
        {
            throttle = qMax(throttle, 0.8);
        }

        qreal brakeForce = 50.0;

        if (m_config.style == AIStyle::Aggressive)
        {
            brakeForce = 35.0;
        }

        if (m_config.style == AIStyle::Conservative)
        {
            brakeForce = 70.0;
        }

        qreal acceleration =
            m_config.acceleration * throttle
            - brakeForce * brake;
        m_speed += acceleration * (elapsedMs / 1000.0);
        m_speed = qMax(0.0, qMin(m_config.maxSpeed, m_speed));

        qreal rotationRad = m_rotation * M_PI / 180.0;
        m_velocity = QVector2D(qCos(rotationRad) * m_speed, qSin(rotationRad) * m_speed);

        m_position += m_velocity * (elapsedMs / 1000.0);


        if (distToWP < 120.0)
        {
            m_currentWaypointIndex++;

            if (m_currentWaypointIndex >= m_waypoints.size())
            {
                m_currentWaypointIndex = 0;

                m_currentLap++;

                emit lapCompleted(
                    m_currentLap,
                    m_lapTime
                    );

                m_lapTime = 0;
            }

            emit waypointReached(
                m_currentWaypointIndex
                );
        }
        else if (distToWP > 800.0)
        {
            m_currentWaypointIndex =
                findNearestWaypointIndex();

            m_speed *= 0.7;
        }
    }

    m_lapTime += elapsedMs / 1000.0;
    emit updated(elapsedMs);
}

QVector2D SimpleAIOpponent::calculateSteering()
{
    if (m_waypoints.isEmpty()) return QVector2D(0, 0);
    AIStyle style = m_config.style;

    Waypoint targetWP = getCurrentWaypoint();
    Waypoint currentWP = getCurrentWaypoint();

    QVector2D toTarget = targetWP.position - m_position;
    qreal distance = toTarget.length();
    qreal targetAngle = std::atan2(toTarget.y(), toTarget.x());

    qreal currentAngleRad = m_rotation * M_PI / 180.0;

    qreal angleDiff = targetAngle - currentAngleRad;
    while (angleDiff > M_PI) angleDiff -= 2 * M_PI;
    while (angleDiff < -M_PI) angleDiff += 2 * M_PI;

    qreal steeringStrength =
        (angleDiff / M_PI)
        * qMin(distance / 300.0, 1.0);
    if (style == AIStyle::Aggressive)
    {
        steeringStrength *= 1.15;
    }

    if (style == AIStyle::Conservative)
    {
        steeringStrength *= 0.9;
    }

    if (style == AIStyle::Normal)
    {
        steeringStrength *= 1.0;
    }

    if (style == AIStyle::Defensive)
    {
        steeringStrength *= 0.85;
    }

    if (currentWP.isCorner)
    {
        if (style == AIStyle::Aggressive)
        {
            steeringStrength *= 1.05;
        }

        if (style == AIStyle::Conservative)
        {
            steeringStrength *= 1.1;
        }
    }

    steeringStrength =
        qBound(-0.8, steeringStrength, 0.8);

    return QVector2D(steeringStrength, 0);

}

qreal SimpleAIOpponent::calculateThrottle() const
{
    AIStyle style = m_config.style;
    if (m_waypoints.isEmpty()) return 0.5;
    qreal throttle = 1.0;

    Waypoint currentWP = getCurrentWaypoint();
    Waypoint nextWP = getNextWaypoint();

    qreal distToWP = (currentWP.position - m_position).length();

    Waypoint slowWP = currentWP;

    if (nextWP.isCorner)
    {
        slowWP = nextWP;
    }

    if (slowWP.isCorner)
    {
        qreal cornerFactor =
            1.0 - (slowWP.cornerSeverity / 4.0);

        qreal slowDistance =
            (slowWP.position - m_position).length();

        if (slowDistance < 250.0)
        {
            throttle = cornerFactor * 0.55;
        }
        else
        {
            throttle = cornerFactor * 0.8;
        }
    }

    if (distToWP < 100.0) {
        throttle = 0.5;
    }
    if (style == AIStyle::Aggressive)
    {
        throttle *= 1.1;

        if (!currentWP.isCorner)
        {
            throttle += 0.1;
        }
    }

    if (style == AIStyle::Conservative)
    {
        throttle *= 0.65;

        if (distToWP < 260.0)
        {
            throttle *= 0.6;
        }
    }

    if (style == AIStyle::Normal)
    {
        throttle *= 0.9;
    }

    if (style == AIStyle::Defensive)
    {
        throttle *= 0.75;

        if (distToWP < 220.0)
        {
            throttle *= 0.7;
        }
    }


    qreal targetSpeed = 120.0;

    if (currentWP.isCorner)
    {
        targetSpeed = currentWP.preferredSpeed;
    }

    if (m_speed > targetSpeed)
    {
        throttle *= 0.4;
    }

    throttle = qBound(0.0, throttle, 1.0);

    return throttle;
}

qreal SimpleAIOpponent::calculateBraking() const
{
    if (m_waypoints.isEmpty()) return 0;
    AIStyle style = m_config.style;
    qreal brake = 0.0;

    Waypoint currentWP = getCurrentWaypoint();
    qreal distToWP = (currentWP.position - m_position).length();
    if (currentWP.isCorner) {
        qreal distToWP = (currentWP.position - m_position).length();
        if (distToWP < 150.0) {
            brake = currentWP.cornerSeverity / 3.0 * 0.8;
        }
    }

    if (m_speed > currentWP.preferredSpeed && currentWP.preferredSpeed > 0) {
        brake = (m_speed - currentWP.preferredSpeed) / m_config.maxSpeed;
    }

    if (style == AIStyle::Aggressive)
    {
        brake *= 0.3;

        if (!currentWP.isCorner)
        {
            brake *= 0.5;
        }
    }

    if (currentWP.isCorner)
    {
        if (style == AIStyle::Aggressive)
        {
            brake *= 0.5;
        }

        if (style == AIStyle::Conservative)
        {
            brake *= 1.5;
        }
    }

    if (style == AIStyle::Conservative)
    {
        brake *= 1.2;

        if (distToWP < 200.0)
        {
            brake *= 1.1;
        }
    }

    if (style == AIStyle::Normal)
    {
        brake *= 1.0;
    }

    if (style == AIStyle::Defensive)
    {
        brake *= 1.5;

        if (currentWP.isCorner)
        {
            brake *= 1.3;
        }
    }

    brake = qBound(0.0, brake, 1.0);
    return brake;
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
    data["waypoint"] = m_currentWaypointIndex;
    data["steering"] = m_steeringAngle;
    data["velocity"] = m_velocity.length();
    data["style"] = static_cast<int>(m_config.style);
    data["throttle"] = calculateThrottle();
    data["brake"] = calculateBraking();
    return data;
}

QString SimpleAIOpponent::exportStateString() const
{
    QVariantMap data = getStateData();

    QJsonObject jsonObj =
        QJsonObject::fromVariantMap(data);

    QJsonDocument doc(jsonObj);

    return doc.toJson(QJsonDocument::Compact);
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

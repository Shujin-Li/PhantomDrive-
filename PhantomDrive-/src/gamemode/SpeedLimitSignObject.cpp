#include "SpeedLimitSignObject.h"

#include <QDebug>
#include <QDateTime>

namespace PhantomDrive {

SpeedLimitSignObject::SpeedLimitSignObject(const QString& id, QObject* parent)
    : TrafficObject(id, TrafficObjectType::SpeedLimitSign, parent)
    , m_speedLimit(50.0)
    , m_isActive(true)
    , m_zoneId(id)
    , m_detectionRadius(50.0)
    , m_isVehicleInZone(false)
    , m_violationPenalty(5)
    , m_violationCount(0)
    , m_maxSpeedRecorded(0.0)
    , m_lastViolationTime(0)
    , m_minViolationIntervalMs(2000)
{
}

SpeedLimitSignObject::~SpeedLimitSignObject()
{
}

void SpeedLimitSignObject::setSpeedLimit(qreal limit)
{
    if (m_speedLimit != limit && limit >= 0) {
        qreal oldLimit = m_speedLimit;
        m_speedLimit = limit;
        emit speedLimitChanged(oldLimit, m_speedLimit);
    }
}

void SpeedLimitSignObject::setActive(bool active)
{
    m_isActive = active;
}

void SpeedLimitSignObject::setZoneId(const QString& zoneId)
{
    m_zoneId = zoneId;
}

void SpeedLimitSignObject::setDetectionRadius(qreal radius)
{
    if (radius >= 0) {
        m_detectionRadius = radius;
    }
}

bool SpeedLimitSignObject::isVehicleInZone(const QVector2D& vehiclePosition) const
{
    qreal distance = (vehiclePosition - getPosition()).length();
    return distance <= m_detectionRadius;
}

bool SpeedLimitSignObject::checkSpeedViolation(qreal currentSpeed) const
{
    if (currentSpeed > m_speedLimit) {
        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        return (currentTime - m_lastViolationTime) >= m_minViolationIntervalMs;
    }
    return false;
}

qreal SpeedLimitSignObject::getOverspeedPercentage(qreal currentSpeed) const
{
    if (m_speedLimit > 0) {
        return ((currentSpeed - m_speedLimit) / m_speedLimit) * 100.0;
    }
    return 0.0;
}

void SpeedLimitSignObject::incrementViolationCount()
{
    m_violationCount++;
    m_lastViolationTime = QDateTime::currentMSecsSinceEpoch();
}

void SpeedLimitSignObject::updateMaxSpeed(qreal speed)
{
    if (speed > m_maxSpeedRecorded) {
        m_maxSpeedRecorded = speed;
    }
}

void SpeedLimitSignObject::onVehiclePositionChanged(const QVector2D& vehiclePosition)
{
    bool wasInZone = m_isVehicleInZone;
    m_isVehicleInZone = isVehicleInZone(vehiclePosition);

    if (m_isVehicleInZone && !wasInZone) {
        emit zoneEntered(getId(), m_zoneId);
    } else if (!m_isVehicleInZone && wasInZone) {
        emit zoneExited(getId(), m_zoneId);
    }
}

void SpeedLimitSignObject::onVehicleSpeedChanged(qreal speed)
{
    updateMaxSpeed(speed);

    if (m_isVehicleInZone && checkSpeedViolation(speed)) {
        incrementViolationCount();
        emit speedViolation(getId(), speed, m_speedLimit);
    }
}

void SpeedLimitSignObject::reset()
{
    m_isVehicleInZone = false;
    m_violationCount = 0;
    m_maxSpeedRecorded = 0.0;
    m_lastViolationTime = 0;
}

}

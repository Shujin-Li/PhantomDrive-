#include "TrafficObjectManager.h"

#include <QDateTime>

namespace PhantomDrive {

TrafficObjectManager::TrafficObjectManager(QObject* parent)
    : QObject(parent)
    , m_lastVehicleSpeed(0.0)
    , m_currentSpeedLimitZoneId()
{
}

TrafficObjectManager::~TrafficObjectManager()
{
    clear();
}

void TrafficObjectManager::registerTrafficObject(TrafficObject* object)
{
    if (object == nullptr) {
        return;
    }

    QString id = object->getId();
    if (m_trafficObjects.contains(id)) {
        return;
    }

    m_trafficObjects[id] = object;
    emit objectRegistered(id, object->getType());

    switch (object->getType()) {
        case TrafficObjectType::TrafficLight:
            registerTrafficLight(dynamic_cast<TrafficLightObject*>(object));
            break;
        case TrafficObjectType::SpeedLimitSign:
            registerSpeedLimitSign(dynamic_cast<SpeedLimitSignObject*>(object));
            break;
        case TrafficObjectType::PedestrianCrossing:
            registerPedestrianCrossing(dynamic_cast<PedestrianCrossingObject*>(object));
            break;
        default:
            break;
    }
}

void TrafficObjectManager::unregisterTrafficObject(const QString& objectId)
{
    if (!m_trafficObjects.contains(objectId)) {
        return;
    }

    TrafficObject* obj = m_trafficObjects[objectId];
    if (obj) {
        switch (obj->getType()) {
            case TrafficObjectType::TrafficLight:
                m_trafficLights.remove(objectId);
                break;
            case TrafficObjectType::SpeedLimitSign:
                m_speedLimitSigns.remove(objectId);
                break;
            case TrafficObjectType::PedestrianCrossing:
                m_pedestrianCrossings.remove(objectId);
                break;
            default:
                break;
        }

        m_trafficObjects.remove(objectId);
        obj->deleteLater();
        emit objectUnregistered(objectId);
    }
}

TrafficObject* TrafficObjectManager::getTrafficObject(const QString& objectId) const
{
    return m_trafficObjects.value(objectId, nullptr);
}

bool TrafficObjectManager::hasObject(const QString& objectId) const
{
    return m_trafficObjects.contains(objectId);
}

void TrafficObjectManager::registerTrafficLight(TrafficLightObject* light)
{
    if (light) {
        m_trafficLights[light->getId()] = light;
    }
}

void TrafficObjectManager::registerSpeedLimitSign(SpeedLimitSignObject* sign)
{
    if (sign) {
        m_speedLimitSigns[sign->getId()] = sign;
    }
}

void TrafficObjectManager::registerPedestrianCrossing(PedestrianCrossingObject* crossing)
{
    if (crossing) {
        m_pedestrianCrossings[crossing->getId()] = crossing;
    }
}

void TrafficObjectManager::unregisterTrafficLight(const QString& lightId)
{
    m_trafficLights.remove(lightId);
}

void TrafficObjectManager::unregisterSpeedLimitSign(const QString& signId)
{
    m_speedLimitSigns.remove(signId);
}

void TrafficObjectManager::unregisterPedestrianCrossing(const QString& crossingId)
{
    m_pedestrianCrossings.remove(crossingId);
}

QList<TrafficLightObject*> TrafficObjectManager::getTrafficLights() const
{
    return m_trafficLights.values();
}

QList<SpeedLimitSignObject*> TrafficObjectManager::getSpeedLimitSigns() const
{
    return m_speedLimitSigns.values();
}

QList<PedestrianCrossingObject*> TrafficObjectManager::getPedestrianCrossings() const
{
    return m_pedestrianCrossings.values();
}

QList<TrafficObject*> TrafficObjectManager::getAllTrafficObjects() const
{
    return m_trafficObjects.values();
}

QList<TrafficObject*> TrafficObjectManager::getObjectsInRange(const QVector2D& position, qreal radius) const
{
    QList<TrafficObject*> result;
    for (TrafficObject* obj : m_trafficObjects.values()) {
        qreal distance = (obj->getPosition() - position).length();
        if (distance <= radius) {
            result.append(obj);
        }
    }
    return result;
}

QList<TrafficObject*> TrafficObjectManager::getObjectsByType(TrafficObjectType type) const
{
    QList<TrafficObject*> result;
    for (TrafficObject* obj : m_trafficObjects.values()) {
        if (obj->getType() == type) {
            result.append(obj);
        }
    }
    return result;
}

void TrafficObjectManager::update(qint64 elapsedMs)
{
    const qint64 ts = QDateTime::currentMSecsSinceEpoch();
    const qreal speedLimit = getCurrentSpeedLimit(m_lastVehiclePosition);

    for (TrafficLightObject* light : m_trafficLights.values()) {
        light->update(elapsedMs);
        const QString violationKey = QStringLiteral("red:%1").arg(light->getId());
        if (light->checkRedLightViolation(m_lastVehiclePosition)
            && shouldEmitViolation(violationKey, ts, ViolationConfig::redLightCooldownMs)) {
            light->markViolation();
            ViolationEvent event;
            event.timestamp = ts;
            event.type = ViolationType::RedLight;
            event.description = QStringLiteral("Red light violation at %1").arg(light->getId());
            event.position = m_lastVehiclePosition;
            event.speedAtViolation = m_lastVehicleSpeed;
            event.speedLimit = speedLimit;
            event.penaltyPoints = ViolationConfig::redLightPenalty;
            emit violationDetected(event);
        }
    }

    for (PedestrianCrossingObject* crossing : m_pedestrianCrossings.values()) {
        crossing->update(elapsedMs);
    }

    emit allObjectsUpdated();
}

void TrafficObjectManager::startAll()
{
    for (TrafficLightObject* light : m_trafficLights.values()) {
        light->start();
    }
}

void TrafficObjectManager::stopAll()
{
    for (TrafficLightObject* light : m_trafficLights.values()) {
        light->stop();
    }
}

int TrafficObjectManager::getTotalObjectCount() const
{
    return m_trafficObjects.size();
}

int TrafficObjectManager::getActiveObjectCount() const
{
    int count = 0;
    for (TrafficObject* obj : m_trafficObjects.values()) {
        if (obj->isActive()) {
            count++;
        }
    }
    return count;
}

int TrafficObjectManager::getTotalViolationCount() const
{
    return getRedLightViolationCount() + getSpeedViolationCount() + getPedestrianViolationCount();
}

int TrafficObjectManager::getRedLightViolationCount() const
{
    int total = 0;
    for (const TrafficLightObject* light : m_trafficLights.values()) {
        total += light->getRedLightViolationCount();
    }
    return total;
}

int TrafficObjectManager::getSpeedViolationCount() const
{
    int total = 0;
    for (const SpeedLimitSignObject* sign : m_speedLimitSigns.values()) {
        total += sign->getViolationCount();
    }
    return total;
}

int TrafficObjectManager::getPedestrianViolationCount() const
{
    int total = 0;
    for (const PedestrianCrossingObject* crossing : m_pedestrianCrossings.values()) {
        total += crossing->getViolationCount();
    }
    return total;
}

void TrafficObjectManager::resetAllViolationCounts()
{
    for (TrafficLightObject* light : m_trafficLights.values()) {
        light->resetViolationCount();
    }
    for (SpeedLimitSignObject* sign : m_speedLimitSigns.values()) {
        sign->resetViolationCount();
    }
    for (PedestrianCrossingObject* crossing : m_pedestrianCrossings.values()) {
        crossing->resetViolationCount();
    }
}

qreal TrafficObjectManager::getCurrentSpeedLimit(const QVector2D& position) const
{
    qreal lowestLimit = 0.0;

    for (const SpeedLimitSignObject* sign : m_speedLimitSigns.values()) {
        if (sign->isVehicleInZone(position)) {
            qreal limit = sign->getSpeedLimit();
            if (lowestLimit == 0.0 || limit < lowestLimit) {
                lowestLimit = limit;
            }
        }
    }

    return lowestLimit;
}

bool TrafficObjectManager::isRedLightViolation(const QString& lightId, const QVector2D& vehiclePosition) const
{
    TrafficLightObject* light = m_trafficLights.value(lightId, nullptr);
    if (light) {
        return light->checkRedLightViolation(vehiclePosition);
    }
    return false;
}

bool TrafficObjectManager::checkSpeedViolation(const QVector2D& position, qreal speed) const
{
    for (SpeedLimitSignObject* sign : m_speedLimitSigns.values()) {
        if (sign->isVehicleInZone(position)) {
            if (sign->checkSpeedViolation(speed)) {
                return true;
            }
        }
    }
    return false;
}

bool TrafficObjectManager::checkPedestrianViolation(const QVector2D& position) const
{
    for (PedestrianCrossingObject* crossing : m_pedestrianCrossings.values()) {
        if (crossing->checkPedestrianViolation(position)) {
            return true;
        }
    }
    return false;
}

void TrafficObjectManager::onVehiclePositionChanged(const QVector2D& position)
{
    const qint64 ts = QDateTime::currentMSecsSinceEpoch();
    const qreal currentSpeed = m_lastVehicleSpeed;
    const qreal speedLimit = getCurrentSpeedLimit(position);

    for (SpeedLimitSignObject* sign : m_speedLimitSigns.values()) {
        sign->onVehiclePositionChanged(position);
        const QString violationKey = QStringLiteral("speed:%1").arg(sign->getId());
        if (sign->isVehicleInZone(position)
            && sign->checkSpeedViolation(currentSpeed)
            && shouldEmitViolation(violationKey, ts, ViolationConfig::speedCooldownMs)) {
            sign->incrementViolationCount();
            ViolationEvent event;
            event.timestamp = ts;
            event.type = ViolationType::SpeedOverLimit;
            event.description = QStringLiteral("Speed %1 exceeded limit %2").arg(currentSpeed).arg(sign->getSpeedLimit());
            event.position = position;
            event.speedAtViolation = currentSpeed;
            event.speedLimit = sign->getSpeedLimit();
            event.penaltyPoints = ViolationConfig::speedViolationPenalty;
            emit violationDetected(event);
        }
    }

    for (PedestrianCrossingObject* crossing : m_pedestrianCrossings.values()) {
        const QString violationKey = QStringLiteral("pedestrian:%1").arg(crossing->getId());
        if (crossing->checkPedestrianViolation(position)
            && shouldEmitViolation(violationKey, ts, ViolationConfig::pedestrianCooldownMs)) {
            crossing->incrementViolationCount();
            ViolationEvent event;
            event.timestamp = ts;
            event.type = ViolationType::PedestrianCollision;
            event.description = QStringLiteral("Pedestrian crossing violation");
            event.position = position;
            event.speedAtViolation = currentSpeed;
            event.speedLimit = speedLimit;
            event.penaltyPoints = ViolationConfig::pedestrianPenalty;
            emit violationDetected(event);
        }
    }

    checkSpeedLimitZones(position);
    m_lastVehiclePosition = position;
}

void TrafficObjectManager::onVehicleSpeedChanged(qreal speed)
{
    for (SpeedLimitSignObject* sign : m_speedLimitSigns.values()) {
        sign->updateMaxSpeed(speed);
    }
    m_lastVehicleSpeed = speed;
}

void TrafficObjectManager::clear()
{
    qDeleteAll(m_trafficObjects);
    m_trafficObjects.clear();
    m_trafficLights.clear();
    m_speedLimitSigns.clear();
    m_pedestrianCrossings.clear();
    m_lastViolationByKey.clear();
    emit managerCleared();
}

bool TrafficObjectManager::shouldEmitViolation(const QString& key, qint64 timestampMs, int cooldownMs)
{
    const qint64 lastTimestamp = m_lastViolationByKey.value(key, 0);
    if (lastTimestamp > 0 && timestampMs - lastTimestamp < cooldownMs) {
        return false;
    }

    m_lastViolationByKey.insert(key, timestampMs);
    return true;
}

void TrafficObjectManager::checkSpeedLimitZones(const QVector2D& position)
{
    QString currentZone;

    for (SpeedLimitSignObject* sign : m_speedLimitSigns.values()) {
        if (sign->isVehicleInZone(position)) {
            currentZone = sign->getZoneId();
            break;
        }
    }

    if (currentZone != m_currentSpeedLimitZoneId) {
        m_currentSpeedLimitZoneId = currentZone;
    }
}

}

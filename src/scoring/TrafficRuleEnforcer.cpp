#include "PhantomDrive/scoring/TrafficRuleEnforcer.h"

#include "PhantomDrive/gamemode/TrafficLightObject.h"
#include "PhantomDrive/gamemode/TrafficObjectManager.h"

#include <limits>

namespace PhantomDrive {

TrafficRuleEnforcer::TrafficRuleEnforcer(int cooldownMs)
    : m_cooldownMs(cooldownMs)
{
}

bool TrafficRuleEnforcer::shouldEmit(const QString& key, qint64 timestamp, QHash<QString, qint64>& dedupe) const
{
    const qint64 last = dedupe.value(key, std::numeric_limits<qint64>::min());
    if (timestamp - last < m_cooldownMs) {
        return false;
    }
    dedupe.insert(key, timestamp);
    return true;
}

QList<ViolationEvent> TrafficRuleEnforcer::checkFrame(const DrivingData& data,
                                                      const TrafficObjectManager* manager) const
{
    QHash<QString, qint64> dedupe;
    return checkFrameInternal(data, manager, dedupe);
}

QList<ViolationEvent> TrafficRuleEnforcer::checkSequence(const QList<DrivingData>& dataList,
                                                         const TrafficObjectManager* manager) const
{
    QList<ViolationEvent> events;
    QHash<QString, qint64> dedupe;
    for (const DrivingData& data : dataList) {
        events.append(checkFrameInternal(data, manager, dedupe));
    }
    return events;
}

QList<ViolationEvent> TrafficRuleEnforcer::checkFrameInternal(const DrivingData& data,
                                                              const TrafficObjectManager* manager,
                                                              QHash<QString, qint64>& dedupe) const
{
    QList<ViolationEvent> events;
    if (manager == nullptr) {
        return events;
    }

    const qint64 ts = data.timestamp > 0 ? data.timestamp : 0;
    const qreal speedLimit = manager->getCurrentSpeedLimit(data.position);
    if (manager->checkSpeedViolation(data.position, data.speed)) {
        const QString key = QStringLiteral("speed:%1")
                .arg(QString::number(speedLimit, 'f', 2));
        if (shouldEmit(key, ts, dedupe)) {
            ViolationEvent event;
            event.timestamp = ts;
            event.type = ViolationType::SpeedOverLimit;
            event.description = QStringLiteral("Speed %1 exceeded limit %2")
                    .arg(data.speed)
                    .arg(speedLimit);
            event.position = data.position;
            event.speedAtViolation = data.speed;
            event.speedLimit = speedLimit;
            event.penaltyPoints = 6;
            events.append(event);
        }
    }

    if (manager->checkPedestrianViolation(data.position)) {
        const QString key = QStringLiteral("pedestrian");
        if (shouldEmit(key, ts, dedupe)) {
            ViolationEvent event;
            event.timestamp = ts;
            event.type = ViolationType::PedestrianCollision;
            event.description = QStringLiteral("Pedestrian crossing violation");
            event.position = data.position;
            event.speedAtViolation = data.speed;
            event.speedLimit = speedLimit;
            event.penaltyPoints = 20;
            events.append(event);
        }
    }

    for (TrafficLightObject* light : manager->getTrafficLights()) {
        if (light != nullptr && light->checkRedLightViolation(data.position)) {
            const QString key = QStringLiteral("redlight:%1").arg(light->getId());
            if (shouldEmit(key, ts, dedupe)) {
                ViolationEvent event;
                event.timestamp = ts;
                event.type = ViolationType::RedLight;
                event.description = QStringLiteral("Red light violation at %1").arg(light->getId());
                event.position = data.position;
                event.speedAtViolation = data.speed;
                event.speedLimit = speedLimit;
                event.penaltyPoints = 12;
                events.append(event);
            }
        }
    }

    return events;
}

}

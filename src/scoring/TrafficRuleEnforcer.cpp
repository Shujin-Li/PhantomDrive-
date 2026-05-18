#include "PhantomDrive/scoring/TrafficRuleEnforcer.h"

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

QList<ViolationEvent> TrafficRuleEnforcer::filterDuplicates(const QList<ViolationEvent>& events)
{
    QList<ViolationEvent> filtered;
    QHash<QString, qint64> dedupe;

    for (const ViolationEvent& event : events) {
        QString key;
        switch (event.type) {
            case ViolationType::SpeedOverLimit:
                key = QStringLiteral("speed:%1").arg(QString::number(event.speedLimit, 'f', 2));
                break;
            case ViolationType::RedLight:
                key = QStringLiteral("redlight:%1").arg(event.position.x());
                break;
            case ViolationType::PedestrianCollision:
                key = QStringLiteral("pedestrian:%1").arg(event.position.x());
                break;
            default:
                key = QStringLiteral("other:%1:%2").arg(static_cast<int>(event.type)).arg(event.position.x());
                break;
        }

        const qint64 ts = event.timestamp > 0 ? event.timestamp : 0;
        if (shouldEmit(key, ts, dedupe)) {
            filtered.append(event);
        }
    }

    return filtered;
}

}

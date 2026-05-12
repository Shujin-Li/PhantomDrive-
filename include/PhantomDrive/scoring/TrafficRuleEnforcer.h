#pragma once

#include "PhantomDrive/gamemode/DrivingData.h"

#include <QHash>
#include <QString>

namespace PhantomDrive {

class TrafficObjectManager;

class TrafficRuleEnforcer
{
public:
    explicit TrafficRuleEnforcer(int cooldownMs = 1000);

    QList<ViolationEvent> checkFrame(const DrivingData& data,
                                     const TrafficObjectManager* manager) const;
    QList<ViolationEvent> checkSequence(const QList<DrivingData>& dataList,
                                        const TrafficObjectManager* manager) const;

private:
    QList<ViolationEvent> checkFrameInternal(const DrivingData& data,
                                             const TrafficObjectManager* manager,
                                             QHash<QString, qint64>& dedupe) const;
    bool shouldEmit(const QString& key, qint64 timestamp, QHash<QString, qint64>& dedupe) const;

    int m_cooldownMs;
};

}


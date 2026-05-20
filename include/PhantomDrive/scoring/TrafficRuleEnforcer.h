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

    QList<ViolationEvent> filterDuplicates(const QList<ViolationEvent>& events);
};

}

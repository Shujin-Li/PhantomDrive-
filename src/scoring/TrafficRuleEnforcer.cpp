#include "PhantomDrive/scoring/TrafficRuleEnforcer.h"

namespace PhantomDrive {

TrafficRuleEnforcer::TrafficRuleEnforcer(int cooldownMs)
{
    Q_UNUSED(cooldownMs);
}

QList<ViolationEvent> TrafficRuleEnforcer::filterDuplicates(const QList<ViolationEvent>& events)
{
    return events;
}

}

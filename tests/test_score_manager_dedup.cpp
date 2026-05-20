#include "PhantomDrive/scoring/ScoreManager.h"

#include <QCoreApplication>
#include <QDebug>

using namespace PhantomDrive;

namespace {

QList<DrivingData> makeDrivingData()
{
    DrivingData first;
    first.timestamp = 1000;
    first.speed = 35.0;
    first.currentSpeedLimit = 50.0;

    DrivingData second = first;
    second.timestamp = 2000;
    second.speed = 38.0;

    return {first, second};
}

ViolationEvent makeSpeedViolation(qint64 timestamp, const QVector2D& position)
{
    ViolationEvent event;
    event.timestamp = timestamp;
    event.type = ViolationType::SpeedOverLimit;
    event.position = position;
    event.speedLimit = 50.0;
    event.speedAtViolation = 72.0;
    event.penaltyPoints = 5;
    event.description = QStringLiteral("speed over limit");
    return event;
}

}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    ScoreManager manager;
    manager.setVehicleId(QStringLiteral("test-car"));

    const QList<DrivingData> data = makeDrivingData();
    manager.addViolation(makeSpeedViolation(1000, QVector2D(10.0f, 20.0f)));

    const ScoreReport firstReport = manager.evaluate(data, {});
    if (firstReport.metrics.speedViolationCount != 1 || firstReport.violations.size() != 1) {
        qCritical() << "First evaluate should consume exactly one pending violation."
                    << "speedViolationCount=" << firstReport.metrics.speedViolationCount
                    << "violations.size=" << firstReport.violations.size();
        return 1;
    }

    const ScoreReport secondReport = manager.evaluate(data, {});
    if (secondReport.metrics.speedViolationCount != 0 || !secondReport.violations.isEmpty()) {
        qCritical() << "Second evaluate should not reuse previous pending violation."
                    << "speedViolationCount=" << secondReport.metrics.speedViolationCount
                    << "violations.size=" << secondReport.violations.size();
        return 2;
    }

    return 0;
}

#pragma once

#include "PhantomDrive/scoring/ScoreReport.h"
#include "PhantomDrive/scoring/ViolationConfig.h"

namespace PhantomDrive {

struct ScoreCalculatorConfig {
    qreal initialScore = 100.0;
    qreal collisionPenalty = ViolationConfig::collisionPenalty;
    qreal speedViolationPenalty = ViolationConfig::speedViolationPenalty;
    qreal redLightPenalty = ViolationConfig::redLightPenalty;
    qreal pedestrianPenalty = ViolationConfig::pedestrianPenalty;
    qreal wrongWayPenalty = ViolationConfig::wrongWayPenalty;

    qreal harshAccelerationThreshold = 8.0;
    qreal harshBrakingThreshold = -8.0;
    qreal sharpSteeringThreshold = 35.0;
};

class DrivingScoreCalculator
{
public:
    DrivingScoreCalculator();

    void setConfig(const ScoreCalculatorConfig& config);
    ScoreCalculatorConfig config() const;

    ScoreReport evaluate(const QList<DrivingData>& dataList,
                         const QList<ViolationEvent>& violations,
                         const QString& vehicleId = QString(),
                         const QString& drivingMode = QString(),
                         const QString& reportContext = QString()) const;

private:
    ScoreCalculatorConfig m_config;
};

}

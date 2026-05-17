#pragma once

#include "PhantomDrive/scoring/ScoreReport.h"

namespace PhantomDrive {

struct ScoreCalculatorConfig {
    qreal initialScore = 100.0;
    qreal collisionPenalty = 15.0;
    qreal speedViolationPenalty = 6.0;
    qreal redLightPenalty = 12.0;
    qreal pedestrianPenalty = 20.0;
    qreal wrongWayPenalty = 10.0;

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
                         const QString& vehicleId = QString()) const;

private:
    ScoreCalculatorConfig m_config;
};

}


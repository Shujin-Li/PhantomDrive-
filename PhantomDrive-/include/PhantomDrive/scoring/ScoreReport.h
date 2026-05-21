#pragma once

#include "PhantomDrive/gamemode/DrivingData.h"

#include <QDateTime>
#include <QJsonObject>
#include <QList>
#include <QString>

namespace PhantomDrive {

struct ScoreMetrics {
    int dataPointCount = 0;
    qint64 durationMs = 0;
    qreal averageSpeed = 0.0;
    qreal maxSpeed = 0.0;
    int collisionCount = 0;
    int speedViolationCount = 0;
    int redLightViolationCount = 0;
    int pedestrianViolationCount = 0;
    int wrongWayViolationCount = 0;
    int harshAccelerationCount = 0;
    int harshBrakingCount = 0;
    int sharpSteeringCount = 0;
    int overspeedFrameCount = 0;
    qreal overspeedRatio = 0.0;
};

struct ScoreBreakdown {
    qreal safetyScore = 100.0;
    qreal ruleComplianceScore = 100.0;
    qreal smoothnessScore = 100.0;
    qreal efficiencyScore = 100.0;
    qreal totalPenalty = 0.0;
    qreal collisionPenalty = 0.0;
    qreal speedPenalty = 0.0;
    qreal redLightPenalty = 0.0;
    qreal pedestrianPenalty = 0.0;
    qreal smoothnessPenalty = 0.0;
};

struct CoachAdvice {
    QString category;
    QString severity;
    QString message;
    QString evidence;
};

struct QLearningFeedback {
    qreal reward = 0.0;
    qreal normalizedScore = 0.0;
    qreal safetyRisk = 0.0;
    qreal ruleCompliance = 0.0;
    qreal collisionPenalty = 0.0;
    qreal speedPenalty = 0.0;
    qreal smoothnessPenalty = 0.0;
    qreal terminalPenalty = 0.0;
    QString recommendedActionHint;
};

struct ScoreReport {
    QString sessionId;
    QString vehicleId;
    QDateTime generatedAt;
    qreal totalScore = 0.0;
    QString grade;
    QString summary;
    QList<ViolationEvent> violations;
    ScoreMetrics metrics;
    ScoreBreakdown breakdown;
    QList<CoachAdvice> coachAdvices;
    QLearningFeedback qLearningFeedback;

    QJsonObject toJson() const;
    QString toMarkdown() const;

    static QString violationTypeToString(ViolationType type);
    static ViolationType violationTypeFromString(const QString& str);
    static QString gradeFromScore(qreal score);
};

}

Q_DECLARE_METATYPE(PhantomDrive::ScoreReport)


#include "PhantomDrive/scoring/DrivingScoreCalculator.h"

#include <QDateTime>
#include <QtMath>

namespace PhantomDrive {

namespace {
qreal clamp01(qreal v)
{
    return qBound(0.0, v, 1.0);
}

bool hasSpeedViolationEvent(const QList<ViolationEvent>& violations)
{
    for (const ViolationEvent& violation : violations) {
        if (violation.type == ViolationType::SpeedOverLimit) {
            return true;
        }
    }
    return false;
}

bool usesTrafficRuleSpeedScoring(const QString& drivingMode, const QString& reportContext)
{
    const QString mode = drivingMode.trimmed().toLower();
    const QString context = reportContext.trimmed().toLower();

    if (mode.contains(QStringLiteral("learning"))
        || context.contains(QStringLiteral("learning traffic"))
        || context.contains(QStringLiteral("traffic rules"))) {
        return true;
    }

    if (mode.contains(QStringLiteral("arcade"))
        || mode.contains(QStringLiteral("custom"))
        || context.contains(QStringLiteral("two-player"))
        || context.contains(QStringLiteral("custom track"))) {
        return false;
    }

    return true;
}

QList<ViolationEvent> filterModeSpecificViolations(const QList<ViolationEvent>& violations,
                                                   bool useSpeedLimitScoring)
{
    if (useSpeedLimitScoring) {
        return violations;
    }

    QList<ViolationEvent> filtered;
    for (const ViolationEvent& violation : violations) {
        if (violation.type != ViolationType::SpeedOverLimit) {
            filtered.append(violation);
        }
    }
    return filtered;
}

QList<ViolationEvent> buildOverspeedDisplayEvents(const QList<DrivingData>& dataList,
                                                  int penaltyPoints)
{
    QList<ViolationEvent> events;
    bool inSegment = false;
    ViolationEvent current;
    int frameCount = 0;

    auto flushSegment = [&]() {
        if (!inSegment) {
            return;
        }
        if (frameCount >= 3) {
            current.description = QStringLiteral("Speed %1 exceeded limit %2 for %3 samples")
                                      .arg(current.speedAtViolation, 0, 'f', 1)
                                      .arg(current.speedLimit, 0, 'f', 0)
                                      .arg(frameCount);
            events.append(current);
        }
        inSegment = false;
        frameCount = 0;
        current = ViolationEvent();
    };

    for (const DrivingData& data : dataList) {
        const bool overLimit = data.currentSpeedLimit > 0.0 && data.speed > data.currentSpeedLimit;
        if (!overLimit) {
            flushSegment();
            continue;
        }

        if (!inSegment) {
            inSegment = true;
            frameCount = 0;
            current.timestamp = data.timestamp;
            current.type = ViolationType::SpeedOverLimit;
            current.position = data.position;
            current.speedAtViolation = data.speed;
            current.speedLimit = data.currentSpeedLimit;
            current.penaltyPoints = penaltyPoints;
        } else if (data.speed > current.speedAtViolation) {
            current.speedAtViolation = data.speed;
            current.position = data.position;
        }
        ++frameCount;
    }
    flushSegment();
    return events;
}
}

DrivingScoreCalculator::DrivingScoreCalculator()
    : m_config()
{
}

void DrivingScoreCalculator::setConfig(const ScoreCalculatorConfig& config)
{
    m_config = config;
}

ScoreCalculatorConfig DrivingScoreCalculator::config() const
{
    return m_config;
}

ScoreReport DrivingScoreCalculator::evaluate(const QList<DrivingData>& dataList,
                                             const QList<ViolationEvent>& violations,
                                             const QString& vehicleId,
                                             const QString& drivingMode,
                                             const QString& reportContext) const
{
    const bool useSpeedLimitScoring = usesTrafficRuleSpeedScoring(drivingMode, reportContext);
    const QList<ViolationEvent> scoringViolations =
        filterModeSpecificViolations(violations, useSpeedLimitScoring);

    ScoreReport report;
    report.sessionId = QStringLiteral("session_%1").arg(QDateTime::currentMSecsSinceEpoch());
    report.vehicleId = vehicleId;
    report.drivingMode = drivingMode.trimmed();
    report.reportContext = reportContext.trimmed();
    report.generatedAt = QDateTime::currentDateTimeUtc();
    report.violations = scoringViolations;

    report.metrics.dataPointCount = dataList.size();
    if (!dataList.isEmpty()) {
        report.metrics.durationMs = qMax<qint64>(0, dataList.last().timestamp - dataList.first().timestamp);
    }

    qreal speedSum = 0.0;
    for (const DrivingData& data : dataList) {
        speedSum += data.speed;
        report.metrics.maxSpeed = qMax(report.metrics.maxSpeed, data.speed);
        if (useSpeedLimitScoring && data.currentSpeedLimit > 0.0 && data.speed > data.currentSpeedLimit) {
            report.metrics.overspeedFrameCount++;
        }
        if (data.acceleration >= m_config.harshAccelerationThreshold) {
            report.metrics.harshAccelerationCount++;
        }
        if (data.acceleration <= m_config.harshBrakingThreshold) {
            report.metrics.harshBrakingCount++;
        }
        if (qAbs(data.steeringAngle) >= m_config.sharpSteeringThreshold) {
            report.metrics.sharpSteeringCount++;
        }
    }
    if (!dataList.isEmpty()) {
        report.metrics.averageSpeed = speedSum / dataList.size();
        report.metrics.overspeedRatio = static_cast<qreal>(report.metrics.overspeedFrameCount) / dataList.size();
    }

    if (useSpeedLimitScoring
        && !hasSpeedViolationEvent(scoringViolations)
        && report.metrics.overspeedFrameCount > 0) {
        report.violations.append(buildOverspeedDisplayEvents(dataList, m_config.speedViolationPenalty));
    }

    for (const ViolationEvent& v : report.violations) {
        switch (v.type) {
        case ViolationType::Collision:
            report.metrics.collisionCount++;
            break;
        case ViolationType::SpeedOverLimit:
            report.metrics.speedViolationCount++;
            break;
        case ViolationType::RedLight:
            report.metrics.redLightViolationCount++;
            break;
        case ViolationType::PedestrianCollision:
            report.metrics.pedestrianViolationCount++;
            break;
        case ViolationType::WrongWay:
            report.metrics.wrongWayViolationCount++;
            break;
        default:
            break;
        }
    }

    report.breakdown.collisionPenalty = report.metrics.collisionCount * m_config.collisionPenalty;
    report.breakdown.speedPenalty =
            useSpeedLimitScoring
                ? report.metrics.speedViolationCount * m_config.speedViolationPenalty
                      + report.metrics.overspeedRatio * 10.0
                : 0.0;
    report.breakdown.redLightPenalty = report.metrics.redLightViolationCount * m_config.redLightPenalty;
    report.breakdown.pedestrianPenalty = report.metrics.pedestrianViolationCount * m_config.pedestrianPenalty;
    report.breakdown.smoothnessPenalty =
            report.metrics.harshAccelerationCount * 1.8
            + report.metrics.harshBrakingCount * 2.2
            + report.metrics.sharpSteeringCount * 1.5;

    qreal wrongWayPenalty = report.metrics.wrongWayViolationCount * m_config.wrongWayPenalty;

    qreal safetyPenalty =
            report.breakdown.collisionPenalty + report.breakdown.pedestrianPenalty
            + report.metrics.harshBrakingCount * 2.0 + report.metrics.harshAccelerationCount * 1.5;
    qreal rulePenalty =
            report.breakdown.speedPenalty + report.breakdown.redLightPenalty + wrongWayPenalty
            + (useSpeedLimitScoring ? report.metrics.overspeedRatio * 8.0 : 0.0);
    qreal smoothnessPenalty = report.breakdown.smoothnessPenalty;
    qreal efficiencyPenalty = 0.0;
    if (report.metrics.dataPointCount < 20) {
        efficiencyPenalty += 8.0;
    }
    if (report.metrics.averageSpeed < 0.5) {
        efficiencyPenalty += 5.0;
    }

    report.breakdown.safetyScore = qBound(0.0, 100.0 - safetyPenalty, 100.0);
    report.breakdown.ruleComplianceScore = qBound(0.0, 100.0 - rulePenalty, 100.0);
    report.breakdown.smoothnessScore = qBound(0.0, 100.0 - smoothnessPenalty, 100.0);
    report.breakdown.efficiencyScore = qBound(0.0, 100.0 - efficiencyPenalty, 100.0);

    report.breakdown.totalPenalty =
            report.breakdown.collisionPenalty
            + report.breakdown.speedPenalty
            + report.breakdown.redLightPenalty
            + report.breakdown.pedestrianPenalty
            + wrongWayPenalty
            + report.breakdown.smoothnessPenalty
            + efficiencyPenalty;

    qreal weightedScore =
            report.breakdown.safetyScore * 0.4
            + report.breakdown.ruleComplianceScore * 0.3
            + report.breakdown.smoothnessScore * 0.2
            + report.breakdown.efficiencyScore * 0.1;
    report.totalScore = qBound(0.0, weightedScore, 100.0);
    report.grade = ScoreReport::gradeFromScore(report.totalScore);

    if (report.metrics.collisionCount >= 2 || report.metrics.pedestrianViolationCount >= 1) {
        report.coachAdvices.append({"安全", "高", "碰撞风险较高，建议保持安全车距并提前减速。", "碰撞/行人违规次数偏高"});
    }
    if (report.metrics.speedViolationCount >= 2 || report.metrics.overspeedRatio > 0.15) {
        report.coachAdvices.append({"规则", "中", "进入限速区后主动观察限速标志并及时收油。", "超速与限速区违规偏多"});
    }
    if (report.metrics.harshBrakingCount >= 3) {
        report.coachAdvices.append({"平顺", "中", "急刹车较多，建议提前预判交通灯和行人动态。", "急刹次数较高"});
    }
    if (report.metrics.sharpSteeringCount >= 3) {
        report.coachAdvices.append({"操控", "中", "转向输入偏激进，建议提前入弯并平稳修正方向。", "急转向次数较高"});
    }
    if (report.coachAdvices.isEmpty()) {
        report.coachAdvices.append({"综合", "低", "整体驾驶表现优秀，请继续保持稳定驾驶节奏。", "高分且违规较少"});
    }

    report.qLearningFeedback.normalizedScore = report.totalScore / 100.0;
    report.qLearningFeedback.collisionPenalty = clamp01((report.metrics.collisionCount * m_config.collisionPenalty) / 100.0);
    report.qLearningFeedback.speedPenalty = useSpeedLimitScoring
        ? clamp01((report.metrics.speedViolationCount * m_config.speedViolationPenalty
                   + report.metrics.overspeedRatio * 10.0) / 100.0)
        : 0.0;
    report.qLearningFeedback.smoothnessPenalty = clamp01(report.breakdown.smoothnessPenalty / 100.0);
    report.qLearningFeedback.safetyRisk = clamp01(
            report.metrics.collisionCount * 0.25
            + report.metrics.pedestrianViolationCount * 0.35
            + (report.metrics.harshBrakingCount + report.metrics.harshAccelerationCount) * 0.04);
    report.qLearningFeedback.ruleCompliance = clamp01(
            1.0 - (report.metrics.speedViolationCount * 0.08
                   + report.metrics.redLightViolationCount * 0.12
                   + report.metrics.wrongWayViolationCount * 0.1
                   + (useSpeedLimitScoring ? report.metrics.overspeedRatio * 0.5 : 0.0)));
    report.qLearningFeedback.terminalPenalty =
            (report.metrics.collisionCount > 0 ? 0.2 : 0.0)
            + (report.metrics.pedestrianViolationCount > 0 ? 0.35 : 0.0);
    report.qLearningFeedback.reward =
            report.qLearningFeedback.normalizedScore
            - report.qLearningFeedback.collisionPenalty * 0.8
            - report.qLearningFeedback.speedPenalty * 0.5
            - report.qLearningFeedback.smoothnessPenalty * 0.4
            - report.qLearningFeedback.terminalPenalty;
    if (report.qLearningFeedback.safetyRisk > 0.65) {
        report.qLearningFeedback.recommendedActionHint = QStringLiteral("brake_earlier_and_keep_distance");
    } else if (report.qLearningFeedback.ruleCompliance < 0.6) {
        report.qLearningFeedback.recommendedActionHint = QStringLiteral("observe_speed_limit_and_signals");
    } else {
        report.qLearningFeedback.recommendedActionHint = QStringLiteral("keep_smooth_and_stable");
    }

    report.summary = QStringLiteral("本次驾驶总分 %1（%2），违规 %3 次，平均速度 %4。")
            .arg(report.totalScore, 0, 'f', 1)
            .arg(report.grade)
            .arg(report.violations.size())
            .arg(report.metrics.averageSpeed, 0, 'f', 2);

    return report;
}

}

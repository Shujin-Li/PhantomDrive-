#include "PhantomDrive/scoring/ScoreReport.h"

#include <QJsonArray>

namespace PhantomDrive {

QString ScoreReport::violationTypeToString(ViolationType type)
{
    switch (type) {
    case ViolationType::None: return QStringLiteral("None");
    case ViolationType::RedLight: return QStringLiteral("RedLight");
    case ViolationType::SpeedOverLimit: return QStringLiteral("SpeedOverLimit");
    case ViolationType::PedestrianCollision: return QStringLiteral("PedestrianCollision");
    case ViolationType::WrongWay: return QStringLiteral("WrongWay");
    case ViolationType::Collision: return QStringLiteral("Collision");
    }
    return QStringLiteral("Unknown");
}

ViolationType ScoreReport::violationTypeFromString(const QString& str)
{
    if (str == QStringLiteral("RedLight")) return ViolationType::RedLight;
    if (str == QStringLiteral("SpeedOverLimit")) return ViolationType::SpeedOverLimit;
    if (str == QStringLiteral("PedestrianCollision")) return ViolationType::PedestrianCollision;
    if (str == QStringLiteral("WrongWay")) return ViolationType::WrongWay;
    if (str == QStringLiteral("Collision")) return ViolationType::Collision;
    return ViolationType::None;
}

QString ScoreReport::gradeFromScore(qreal score)
{
    if (score >= 90.0) return QStringLiteral("A");
    if (score >= 80.0) return QStringLiteral("B");
    if (score >= 70.0) return QStringLiteral("C");
    if (score >= 60.0) return QStringLiteral("D");
    return QStringLiteral("F");
}

QJsonObject ScoreReport::toJson() const
{
    QJsonObject root;
    root["sessionId"] = sessionId;
    root["vehicleId"] = vehicleId;
    root["drivingMode"] = drivingMode;
    root["reportContext"] = reportContext;
    root["generatedAt"] = generatedAt.toString(Qt::ISODate);
    root["totalScore"] = totalScore;
    root["grade"] = grade;
    root["summary"] = summary;
    root["aiCoachReportMarkdown"] = aiCoachReportMarkdown;

    QJsonObject metricsObj;
    metricsObj["dataPointCount"] = metrics.dataPointCount;
    metricsObj["durationMs"] = static_cast<double>(metrics.durationMs);
    metricsObj["averageSpeed"] = metrics.averageSpeed;
    metricsObj["maxSpeed"] = metrics.maxSpeed;
    metricsObj["collisionCount"] = metrics.collisionCount;
    metricsObj["speedViolationCount"] = metrics.speedViolationCount;
    metricsObj["redLightViolationCount"] = metrics.redLightViolationCount;
    metricsObj["pedestrianViolationCount"] = metrics.pedestrianViolationCount;
    metricsObj["wrongWayViolationCount"] = metrics.wrongWayViolationCount;
    metricsObj["harshAccelerationCount"] = metrics.harshAccelerationCount;
    metricsObj["harshBrakingCount"] = metrics.harshBrakingCount;
    metricsObj["sharpSteeringCount"] = metrics.sharpSteeringCount;
    metricsObj["overspeedFrameCount"] = metrics.overspeedFrameCount;
    metricsObj["overspeedRatio"] = metrics.overspeedRatio;
    root["metrics"] = metricsObj;

    QJsonObject breakdownObj;
    breakdownObj["safetyScore"] = breakdown.safetyScore;
    breakdownObj["ruleComplianceScore"] = breakdown.ruleComplianceScore;
    breakdownObj["smoothnessScore"] = breakdown.smoothnessScore;
    breakdownObj["efficiencyScore"] = breakdown.efficiencyScore;
    breakdownObj["totalPenalty"] = breakdown.totalPenalty;
    breakdownObj["collisionPenalty"] = breakdown.collisionPenalty;
    breakdownObj["speedPenalty"] = breakdown.speedPenalty;
    breakdownObj["redLightPenalty"] = breakdown.redLightPenalty;
    breakdownObj["pedestrianPenalty"] = breakdown.pedestrianPenalty;
    breakdownObj["smoothnessPenalty"] = breakdown.smoothnessPenalty;
    root["breakdown"] = breakdownObj;

    QJsonArray violationsArray;
    for (const ViolationEvent& v : violations) {
        QJsonObject item;
        item["timestamp"] = QString::number(v.timestamp);
        item["type"] = violationTypeToString(v.type);
        item["description"] = v.description;
        item["positionX"] = v.position.x();
        item["positionY"] = v.position.y();
        item["speedAtViolation"] = v.speedAtViolation;
        item["speedLimit"] = v.speedLimit;
        item["penaltyPoints"] = v.penaltyPoints;
        violationsArray.append(item);
    }
    root["violations"] = violationsArray;

    QJsonArray advicesArray;
    for (const CoachAdvice& advice : coachAdvices) {
        QJsonObject item;
        item["category"] = advice.category;
        item["severity"] = advice.severity;
        item["message"] = advice.message;
        item["evidence"] = advice.evidence;
        advicesArray.append(item);
    }
    root["coachAdvices"] = advicesArray;

    QJsonObject qObj;
    qObj["reward"] = qLearningFeedback.reward;
    qObj["normalizedScore"] = qLearningFeedback.normalizedScore;
    qObj["safetyRisk"] = qLearningFeedback.safetyRisk;
    qObj["ruleCompliance"] = qLearningFeedback.ruleCompliance;
    qObj["collisionPenalty"] = qLearningFeedback.collisionPenalty;
    qObj["speedPenalty"] = qLearningFeedback.speedPenalty;
    qObj["smoothnessPenalty"] = qLearningFeedback.smoothnessPenalty;
    qObj["terminalPenalty"] = qLearningFeedback.terminalPenalty;
    qObj["recommendedActionHint"] = qLearningFeedback.recommendedActionHint;
    root["qLearningFeedback"] = qObj;

    return root;
}

QString ScoreReport::toMarkdown() const
{
    QString md;
    md += QStringLiteral("# 驾驶评分报告\n\n");
    md += QStringLiteral("- 会话ID: %1\n").arg(sessionId);
    md += QStringLiteral("- 车辆ID: %1\n").arg(vehicleId);
    md += QStringLiteral("- 生成时间: %1\n").arg(generatedAt.toString(Qt::ISODate));
    md += QStringLiteral("- 总分: %1\n").arg(totalScore, 0, 'f', 1);
    md += QStringLiteral("- 等级: %1\n\n").arg(grade);
    md += QStringLiteral("## 摘要\n%1\n\n").arg(summary);

    md += QStringLiteral("## 维度得分\n");
    md += QStringLiteral("- 安全性: %1\n").arg(breakdown.safetyScore, 0, 'f', 1);
    md += QStringLiteral("- 规则遵守: %1\n").arg(breakdown.ruleComplianceScore, 0, 'f', 1);
    md += QStringLiteral("- 平顺性: %1\n").arg(breakdown.smoothnessScore, 0, 'f', 1);
    md += QStringLiteral("- 效率: %1\n\n").arg(breakdown.efficiencyScore, 0, 'f', 1);

    md += QStringLiteral("## 违规统计\n");
    md += QStringLiteral("- 总数: %1\n").arg(violations.size());
    md += QStringLiteral("- 碰撞: %1\n").arg(metrics.collisionCount);
    md += QStringLiteral("- 超速: %1\n").arg(metrics.speedViolationCount);
    md += QStringLiteral("- 红灯: %1\n").arg(metrics.redLightViolationCount);
    md += QStringLiteral("- 行人: %1\n").arg(metrics.pedestrianViolationCount);
    md += QStringLiteral("- 逆行: %1\n\n").arg(metrics.wrongWayViolationCount);

    md += QStringLiteral("## 教练建议\n");
    if (coachAdvices.isEmpty()) {
        md += QStringLiteral("- 暂无\n");
    } else {
        for (const CoachAdvice& advice : coachAdvices) {
            md += QStringLiteral("- [%1/%2] %3（证据：%4）\n")
                    .arg(advice.category, advice.severity, advice.message, advice.evidence);
        }
    }
    md += QStringLiteral("\n");

    md += QStringLiteral("## Q-Learning Feedback\n");
    md += QStringLiteral("- reward: %1\n").arg(qLearningFeedback.reward, 0, 'f', 4);
    md += QStringLiteral("- normalizedScore: %1\n").arg(qLearningFeedback.normalizedScore, 0, 'f', 4);
    md += QStringLiteral("- safetyRisk: %1\n").arg(qLearningFeedback.safetyRisk, 0, 'f', 4);
    md += QStringLiteral("- ruleCompliance: %1\n").arg(qLearningFeedback.ruleCompliance, 0, 'f', 4);
    md += QStringLiteral("- terminalPenalty: %1\n").arg(qLearningFeedback.terminalPenalty, 0, 'f', 4);
    md += QStringLiteral("- recommendedActionHint: %1\n").arg(qLearningFeedback.recommendedActionHint);
    return md;
}

}

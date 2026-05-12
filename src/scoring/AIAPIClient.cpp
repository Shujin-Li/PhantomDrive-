#include "PhantomDrive/scoring/AIAPIClient.h"

namespace PhantomDrive {

AIAPIClient::AIAPIClient(QObject* parent)
    : QObject(parent)
{
}

QString AIAPIClient::generateShortSummary(const ScoreReport& report) const
{
    return QStringLiteral("评分 %1（%2），违规 %3 次。")
            .arg(report.totalScore, 0, 'f', 1)
            .arg(report.grade)
            .arg(report.violations.size());
}

QString AIAPIClient::generateCoachReport(const ScoreReport& report) const
{
    QString text;
    text += QStringLiteral("# AI 教练报告（Mock）\n\n");
    text += QStringLiteral("> 说明：当前为离线占位实现，未接入真实 LLM API。\n\n");
    text += QStringLiteral("## 总览\n");
    text += QStringLiteral("- 总分：%1\n").arg(report.totalScore, 0, 'f', 1);
    text += QStringLiteral("- 等级：%1\n").arg(report.grade);
    text += QStringLiteral("- 安全风险：%1\n").arg(report.qLearningFeedback.safetyRisk, 0, 'f', 2);
    text += QStringLiteral("- 规则遵守度：%1\n\n").arg(report.qLearningFeedback.ruleCompliance, 0, 'f', 2);

    text += QStringLiteral("## 重点建议\n");
    for (const CoachAdvice& advice : report.coachAdvices) {
        text += QStringLiteral("- [%1/%2] %3（%4）\n")
                .arg(advice.category, advice.severity, advice.message, advice.evidence);
    }
    if (report.coachAdvices.isEmpty()) {
        text += QStringLiteral("- 暂无。\n");
    }
    text += QStringLiteral("\n");

    text += QStringLiteral("## 强化学习反馈\n");
    text += QStringLiteral("- reward: %1\n").arg(report.qLearningFeedback.reward, 0, 'f', 4);
    text += QStringLiteral("- terminalPenalty: %1\n").arg(report.qLearningFeedback.terminalPenalty, 0, 'f', 4);
    text += QStringLiteral("- actionHint: %1\n").arg(report.qLearningFeedback.recommendedActionHint);

    text += QStringLiteral("\n<!-- TODO: 接入真实 LLM API（E-B/U-A/A-A 对接后） -->\n");
    return text;
}

}


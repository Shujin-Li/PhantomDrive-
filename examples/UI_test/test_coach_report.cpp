#include <QApplication>
#include <QDebug>
#include <QDateTime>
#include <QtMath>

#include "PhantomDrive/scoring/ScoreReport.h"
#include "PhantomDrive/scoring/DrivingScoreCalculator.h"
#include "PhantomDrive/scoring/AIAPIClient.h"
#include "PhantomDrive/gamemode/DrivingData.h"

using namespace PhantomDrive;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qDebug() << "=== PhantomDrive AI Coach Report Test ===\n";

    // 1. 创建测试数据
    QList<DrivingData> dataList;
    const qint64 base = QDateTime::currentMSecsSinceEpoch();

    for (int i = 0; i < 100; ++i) {
        DrivingData d;
        d.timestamp = base + i * 100;
        d.position = QVector2D(static_cast<qreal>(i) * 0.5, 0.0);
        d.speed = 10.0 + qSin(i * 0.2) * 3.0 + (i % 15 == 0 ? 8.0 : 0.0);  // 偶尔超速
        d.currentSpeedLimit = 15.0;
        d.acceleration = qSin(i * 0.3) * 3.0;
        d.steeringAngle = qSin(i * 0.15) * 15.0;
        dataList.append(d);
    }

    // 2. 创建违规事件
    QList<ViolationEvent> violations;
    ViolationEvent v1;
    v1.timestamp = base + 5000;
    v1.type = ViolationType::SpeedOverLimit;
    v1.description = "超速行驶";
    v1.penaltyPoints = 6;
    violations.append(v1);

    ViolationEvent v2;
    v2.timestamp = base + 15000;
    v2.type = ViolationType::Collision;
    v2.description = "碰撞障碍物";
    v2.penaltyPoints = 15;
    violations.append(v2);

    ViolationEvent v3;
    v3.timestamp = base + 25000;
    v3.type = ViolationType::RedLight;
    v3.description = "闯红灯";
    v3.penaltyPoints = 12;
    violations.append(v3);

    // 3. 计算评分
    DrivingScoreCalculator calculator;
    ScoreReport report = calculator.evaluate(dataList, violations, "TEST_VEHICLE_001");

    // 4. 输出评分结果
    qDebug() << "=== Score Report ===";
    qDebug() << "Total Score:" << report.totalScore;
    qDebug() << "Grade:" << report.grade;
    qDebug() << "Safety Score:" << report.breakdown.safetyScore;
    qDebug() << "Rule Compliance:" << report.breakdown.ruleComplianceScore;
    qDebug() << "Smoothness:" << report.breakdown.smoothnessScore;
    qDebug() << "Efficiency:" << report.breakdown.efficiencyScore;
    qDebug() << "Violations:" << report.violations.size();

    // 5. 输出教练建议
    qDebug() << "\n=== Coach Advices ===";
    if (report.coachAdvices.isEmpty()) {
        qDebug() << "No advices generated";
    } else {
        for (int i = 0; i < report.coachAdvices.size(); ++i) {
            const CoachAdvice &advice = report.coachAdvices[i];
            qDebug() << QString("[%1] %2 (%3)")
                            .arg(advice.category)
                            .arg(advice.message)
                            .arg(advice.severity);
        }
    }

    // 6. 生成 AI 教练报告
    qDebug() << "\n=== AI Coach Report (Mock) ===";
    AIAPIClient aiClient;
    QString aiReport = aiClient.generateCoachReport(report);
    qDebug().noquote() << aiReport;

    qDebug() << "\n=== Test Complete ===";

    return 0;
}

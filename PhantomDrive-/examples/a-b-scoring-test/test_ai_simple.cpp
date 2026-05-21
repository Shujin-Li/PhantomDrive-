#include <QCoreApplication>
#include <QDebug>

#include "PhantomDrive/scoring/ScoreReport.h"
#include "PhantomDrive/scoring/DrivingScoreCalculator.h"
#include "PhantomDrive/scoring/AIAPIClient.h"
#include "PhantomDrive/gamemode/DrivingData.h"

using namespace PhantomDrive;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qDebug() << "=== PhantomDrive AI Coach Test ===";

    // 创建测试数据
    QList<DrivingData> dataList;
    const qint64 base = QDateTime::currentMSecsSinceEpoch();

    for (int i = 0; i < 50; ++i) {
        DrivingData d;
        d.timestamp = base + i * 100;
        d.speed = 12.0 + (i % 10 == 0 ? 5.0 : 0.0);  // 偶尔超速
        d.currentSpeedLimit = 15.0;
        dataList.append(d);
    }

    // 违规
    QList<ViolationEvent> violations;
    ViolationEvent v;
    v.timestamp = base + 5000;
    v.type = ViolationType::SpeedOverLimit;
    v.description = "超速";
    v.penaltyPoints = 6;
    violations.append(v);

    // 评分
    DrivingScoreCalculator calc;
    ScoreReport report = calc.evaluate(dataList, violations, "TEST");

    qDebug() << "Score:" << report.totalScore << "Grade:" << report.grade;
    qDebug() << "Advices:" << report.coachAdvices.size();

    // AI 报告
    AIAPIClient ai;
    QString coachReport = ai.generateCoachReport(report);
    qDebug().noquote() << "\n" << coachReport;

    return 0;
}

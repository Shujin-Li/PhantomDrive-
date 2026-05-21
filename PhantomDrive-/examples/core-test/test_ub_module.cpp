#include <QCoreApplication>
#include <QDebug>
#include <QDateTime>
#include <QRandomGenerator>

#include <PhantomDrive/core/saveloadmanager.h>
#include <PhantomDrive/scoring/DrivingScoreCalculator.h>
#include <PhantomDrive/scoring/AIAPIClient.h>
#include <PhantomDrive/scoring/TrafficRuleEnforcer.h>
#include <PhantomDrive/gamemode/DrivingData.h>
#include <PhantomDrive/UI/DrivingReportWidget.h>
#include <PhantomDrive/UI/DrivingHistoryChartWidget.h>
#include <PhantomDrive/UI/learninghud.h>
#include <PhantomDrive/UI/ThemeManager.h>

using namespace PhantomDrive;

// 生成随机驾驶报告
ScoreReport generateRandomReport(int index)
{
    ScoreReport report;
    report.sessionId = QString("session_%1").arg(index, 3, 10, QChar('0'));
    report.vehicleId = "test_vehicle_01";
    report.generatedAt = QDateTime::currentDateTime().addSecs(-index * 300);
    report.totalScore = 60.0 + QRandomGenerator::global()->bounded(40.0);
    report.grade = ScoreReport::gradeFromScore(report.totalScore);
    report.summary = QString("Test driving session #%1").arg(index);

    report.metrics.dataPointCount = 100 + QRandomGenerator::global()->bounded(100);
    report.metrics.durationMs = 60000 + QRandomGenerator::global()->bounded(120000);
    report.metrics.averageSpeed = 30.0 + QRandomGenerator::global()->bounded(30.0);
    report.metrics.maxSpeed = 60.0 + QRandomGenerator::global()->bounded(40.0);
    report.metrics.collisionCount = QRandomGenerator::global()->bounded(3);
    report.metrics.speedViolationCount = QRandomGenerator::global()->bounded(5);
    report.metrics.redLightViolationCount = QRandomGenerator::global()->bounded(2);
    report.metrics.pedestrianViolationCount = 0;
    report.metrics.wrongWayViolationCount = 0;

    report.breakdown.safetyScore = 70.0 + QRandomGenerator::global()->bounded(30.0);
    report.breakdown.ruleComplianceScore = 65.0 + QRandomGenerator::global()->bounded(35.0);
    report.breakdown.smoothnessScore = 60.0 + QRandomGenerator::global()->bounded(40.0);
    report.breakdown.efficiencyScore = 55.0 + QRandomGenerator::global()->bounded(45.0);

    int numViolations = QRandomGenerator::global()->bounded(5);
    for (int i = 0; i < numViolations; ++i) {
        ViolationEvent v;
        v.timestamp = report.metrics.durationMs * i / qMax(1, numViolations);
        v.type = static_cast<ViolationType>(QRandomGenerator::global()->bounded(1, 6));
        v.description = QString("Violation #%1 in session %2").arg(i).arg(index);
        v.position.setX(QRandomGenerator::global()->bounded(1000));
        v.position.setY(QRandomGenerator::global()->bounded(1000));
        v.speedAtViolation = 50.0 + QRandomGenerator::global()->bounded(30.0);
        v.speedLimit = 60;
        v.penaltyPoints = 5 + QRandomGenerator::global()->bounded(10);
        report.violations.append(v);
    }

    return report;
}

// 生成驾驶数据用于 AI 评分
QList<DrivingData> generateDrivingData()
{
    QList<DrivingData> dataList;
    const qint64 base = QDateTime::currentMSecsSinceEpoch();

    for (int i = 0; i < 100; ++i) {
        DrivingData d;
        d.timestamp = base + i * 100;
        d.position = QVector2D(static_cast<qreal>(i) * 0.5, 0.0);

        if (i % 15 == 0) {
            d.speed = 18.0;  // 超速
        } else {
            d.speed = 12.0 + QRandomGenerator::global()->bounded(3);
        }

        d.currentSpeedLimit = 15.0;
        d.acceleration = QRandomGenerator::global()->bounded(-5, 6) * 1.0;
        d.steeringAngle = QRandomGenerator::global()->bounded(-20, 21);
        dataList.append(d);
    }
    return dataList;
}

// 测试评分计算器
void testScoreCalculator()
{
    qDebug() << "\n========================================";
    qDebug() << "     评分计算器测试";
    qDebug() << "========================================\n";

    // 1. 生成驾驶数据
    qDebug() << "[1] 生成驾驶数据...";
    QList<DrivingData> dataList = generateDrivingData();
    qDebug() << "    数据点数量:" << dataList.size();

    // 2. 添加违规事件
    qDebug() << "\n[2] 添加违规事件...";
    QList<ViolationEvent> violations;

    ViolationEvent v1;
    v1.timestamp = dataList[30].timestamp;
    v1.type = ViolationType::SpeedOverLimit;
    v1.description = "超速行驶";
    v1.penaltyPoints = 6;
    violations.append(v1);
    qDebug() << "    添加: 超速 (位置 30)";

    ViolationEvent v2;
    v2.timestamp = dataList[60].timestamp;
    v2.type = ViolationType::RedLight;
    v2.description = "闯红灯";
    v2.penaltyPoints = 12;
    violations.append(v2);
    qDebug() << "    添加: 闯红灯 (位置 60)";

    // 3. 计算评分
    qDebug() << "\n[3] 计算驾驶评分...";
    DrivingScoreCalculator calculator;
    ScoreReport report = calculator.evaluate(dataList, violations, "TEST_VEHICLE_001");
    qDebug() << "    总分:" << report.totalScore;
    qDebug() << "    等级:" << report.grade;
    qDebug() << "    违规次数:" << report.violations.size();

    // 4. 显示教练建议
    qDebug() << "\n[4] 教练建议:";
    if (report.coachAdvices.isEmpty()) {
        qDebug() << "    (无)";
    } else {
        for (int i = 0; i < report.coachAdvices.size(); ++i) {
            const CoachAdvice &advice = report.coachAdvices[i];
            qDebug() << QString("    [%1] %2").arg(advice.category).arg(advice.message);
        }
    }

    // 5. 生成 AI 教练报告
    qDebug() << "\n[5] AI 教练报告 (Mock模式):";
    qDebug() << "    (如果没有配置 API Key，将使用本地 Mock 报告)";
    AIAPIClient aiClient;
    QString aiReport = aiClient.generateCoachReport(report);

    qDebug() << "\n--- AI 报告内容 ---";
    qDebug().noquote() << aiReport;
    qDebug() << "--- 报告结束 ---\n";

    // 6. 测试 TrafficRuleEnforcer
    qDebug() << "\n[6] 测试 TrafficRuleEnforcer...";
    TrafficRuleEnforcer enforcer(1000);
    QList<ViolationEvent> filtered = enforcer.filterDuplicates(violations);
    qDebug() << "    原始违规:" << violations.size() << " 去重后:" << filtered.size();

    qDebug() << "\n========================================";
    qDebug() << "     评分计算测试完成";
    qDebug() << "========================================\n";
}

// 测试存档管理器
void testSaveLoadManager()
{
    qDebug() << "\n========================================";
    qDebug() << "     SaveLoadManager 存档模块测试";
    qDebug() << "========================================\n";

    qDebug() << "[1] 生成3条测试报告...";
    QList<ScoreReport> testReports;
    for (int i = 1; i <= 3; ++i) {
        testReports.append(generateRandomReport(i));
    }

    qDebug() << "\n[2] 测试保存报告...";
    for (int i = 0; i < testReports.size(); ++i) {
        if (SaveLoadManager::instance().saveReport(testReports[i])) {
            qDebug() << "  保存报告 #" << (i + 1) << "成功";
        } else {
            qDebug() << "  保存报告 #" << (i + 1) << "失败";
        }
    }

    qDebug() << "\n[3] 测试加载历史...";
    QList<ScoreReport> loadedReports = SaveLoadManager::instance().loadHistory();
    qDebug() << "  加载了" << loadedReports.size() << "条记录";

    for (int i = 0; i < loadedReports.size(); ++i) {
        const auto& r = loadedReports[i];
        qDebug().noquote() << QString("  [%1] %2 | 总分: %3 | 等级: %4")
            .arg(i).arg(r.sessionId).arg(r.totalScore, 0, 'f', 1).arg(r.grade);
        qDebug() << "      违规次数:" << r.violations.size();
        qDebug() << "      教练建议:" << r.coachAdvices.size() << "条";
    }

    qDebug() << "\n========================================";
    qDebug() << "     存档模块测试完成";
    qDebug() << "========================================\n";
}

// 测试图表组件
void testChartWidgets()
{
    qDebug() << "\n========================================";
    qDebug() << "     图表可视化组件测试";
    qDebug() << "========================================\n";

    DrivingHistoryChartWidget historyWidget;
    qDebug() << "[1] 创建历史图表组件成功";

    for (int i = 1; i <= 5; ++i) {
        historyWidget.addReport(generateRandomReport(i));
    }
    qDebug() << "[2] 添加5条历史报告到图表";
    qDebug() << "[3] 验证数据数量:" << historyWidget.getHistory().size();

    qDebug() << "\n========================================";
    qDebug() << "     图表组件测试完成";
    qDebug() << "========================================\n";
}

// 测试 HUD 组件
void testLearningHUD()
{
    qDebug() << "\n========================================";
    qDebug() << "     LearningHUD 组件测试";
    qDebug() << "========================================\n";

    LearningHUD hud;
    qDebug() << "[1] 创建 HUD 组件成功";

    hud.updateCurrentSpeed(45.0);
    qDebug() << "[2] 更新当前速度: 45 km/h";

    hud.updateSpeedLimit(60);
    qDebug() << "[3] 更新限速: 60 km/h";

    hud.updateTrafficLight("green", 10);
    qDebug() << "[4] 更新交通灯: 绿灯";

    hud.updateGameMode("Learning");
    qDebug() << "[5] 更新游戏模式: Learning";

    hud.updateLapInfo(1, 3);
    qDebug() << "[6] 更新圈数: 1/3";

    hud.showPenaltyMessage("超速", 10);
    qDebug() << "[7] 显示违规提示: 超速 -10分";

    qDebug() << "\n========================================";
    qDebug() << "     HUD 组件测试完成";
    qDebug() << "========================================\n";
}

// 测试主题管理器
void testThemeManager()
{
    qDebug() << "\n========================================";
    qDebug() << "     ThemeManager 主题测试";
    qDebug() << "========================================\n";

    QString darkStyle = ThemeManager::getStyleSheet("dark");
    QString lightStyle = ThemeManager::getStyleSheet("light");
    QString racingStyle = ThemeManager::getStyleSheet("racing");

    qDebug() << "[1] 深色主题长度:" << darkStyle.length() << "字符";
    qDebug() << "[2] 浅色主题长度:" << lightStyle.length() << "字符";
    qDebug() << "[3] 赛车主题长度:" << racingStyle.length() << "字符";

    qDebug() << "\n========================================";
    qDebug() << "     主题管理器测试完成";
    qDebug() << "========================================\n";
}

int main(int argc, char* argv[])
{
    qDebug() << "==================================================";
    qDebug() << "     PhantomDrive U-B 模块综合测试";
    qDebug() << "     (AI教练, HUD, 图表, 存档, QSS美化)";
    qDebug() << "==================================================";

    // 测试评分计算器
    testScoreCalculator();

    // 测试存档管理器
    testSaveLoadManager();

    // 测试主题管理器
    testThemeManager();

    qDebug() << "\n所有测试完成！";
    qDebug() << "\n提示: 设置环境变量以使用真实 AI API:";
    qDebug() << "  set DEEPSEEK_API_KEY=your_key";
    qDebug() << "  set ZHIPU_API_KEY=your_key";

    return 0;
}

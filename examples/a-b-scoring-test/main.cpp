#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QJsonDocument>
#include <QRectF>
#include <QtMath>

#include "PhantomDrive/gamemode/SpeedLimitSignObject.h"
#include "PhantomDrive/gamemode/TrafficObjectManager.h"
#include "PhantomDrive/scoring/AIAPIClient.h"
#include "PhantomDrive/scoring/ScoreManager.h"

using namespace PhantomDrive;

static QList<DrivingData> makeGoodData()
{
    QList<DrivingData> list;
    const qint64 base = QDateTime::currentMSecsSinceEpoch();
    for (int i = 0; i < 120; ++i) {
        DrivingData d;
        d.timestamp = base + i * 100;
        d.position = QVector2D(static_cast<qreal>(i) * 0.8, 0.0);
        d.speed = 9.0 + qSin(i * 0.1) * 1.2;
        d.currentSpeedLimit = 15.0;
        d.acceleration = qSin(i * 0.1) * 1.5;
        d.steeringAngle = qSin(i * 0.08) * 8.0;
        list.append(d);
    }
    return list;
}

static QList<DrivingData> makeBadData()
{
    QList<DrivingData> list;
    const qint64 base = QDateTime::currentMSecsSinceEpoch();
    for (int i = 0; i < 120; ++i) {
        DrivingData d;
        d.timestamp = base + i * 100;
        d.position = QVector2D(static_cast<qreal>(i) * 0.6, 1.0);
        d.currentSpeedLimit = 10.0;
        d.speed = (i % 3 == 0) ? 18.0 : 13.0;
        d.acceleration = (i % 7 == 0) ? -9.2 : ((i % 11 == 0) ? 8.6 : 2.5);
        d.steeringAngle = (i % 9 == 0) ? 42.0 : 15.0;
        list.append(d);
    }
    return list;
}

static QList<ViolationEvent> makeBadViolations(const QList<DrivingData>& dataList)
{
    QList<ViolationEvent> list;
    auto addViolation = [&](int idx, ViolationType type, const QString& desc, int points) {
        ViolationEvent v;
        v.timestamp = dataList[idx].timestamp;
        v.type = type;
        v.description = desc;
        v.position = dataList[idx].position;
        v.speedAtViolation = dataList[idx].speed;
        v.speedLimit = dataList[idx].currentSpeedLimit;
        v.penaltyPoints = points;
        list.append(v);
    };

    addViolation(20, ViolationType::SpeedOverLimit, QStringLiteral("超速"), 6);
    addViolation(45, ViolationType::SpeedOverLimit, QStringLiteral("超速"), 6);
    addViolation(70, ViolationType::Collision, QStringLiteral("碰撞障碍物"), 15);
    addViolation(80, ViolationType::RedLight, QStringLiteral("闯红灯"), 12);
    addViolation(95, ViolationType::PedestrianCollision, QStringLiteral("行人区域违规"), 20);
    return list;
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    qRegisterMetaType<PhantomDrive::ScoreReport>("PhantomDrive::ScoreReport");

    ScoreManager manager;
    manager.setVehicleId(QStringLiteral("AB_TEST_VEH_01"));

    TrafficObjectManager trafficManager;
    SpeedLimitSignObject* sign = new SpeedLimitSignObject(QStringLiteral("zone_1"), &trafficManager);
    sign->setPosition(QVector2D(30.0, 0.0));
    sign->setDetectionRadius(100.0);
    sign->setSpeedLimit(12.0);
    trafficManager.registerTrafficObject(sign);

    QObject::connect(&trafficManager, &TrafficObjectManager::violationDetected,
                     &manager, &ScoreManager::onViolationDetected);
    manager.setTrafficObjectManager(&trafficManager);

    trafficManager.onVehicleSpeedChanged(15.0);
    trafficManager.onVehiclePositionChanged(QVector2D(30.0, 0.0));

    bool scoreReadyTriggered = false;
    QObject::connect(&manager, &ScoreManager::scoreReady, [&](const ScoreReport& report) {
        scoreReadyTriggered = true;
        qDebug() << "[scoreReady] totalScore =" << report.totalScore << "grade =" << report.grade;
    });
    QObject::connect(&manager, &ScoreManager::coachReportReady, [&](const QString& markdown) {
        qDebug() << "[coachReportReady] markdown length =" << markdown.size();
    });
    QObject::connect(&manager, &ScoreManager::scoringFailed, [&](const QString& reason) {
        qDebug() << "[scoringFailed]" << reason;
    });

    const QList<DrivingData> goodData = makeGoodData();
    const ScoreReport goodReport = manager.evaluate(goodData, {});
    qDebug() << "\n=== Good Scenario ===";
    qDebug() << "totalScore =" << goodReport.totalScore;
    qDebug() << "grade =" << goodReport.grade;
    qDebug() << "dimension scores ="
             << goodReport.breakdown.safetyScore
             << goodReport.breakdown.ruleComplianceScore
             << goodReport.breakdown.smoothnessScore
             << goodReport.breakdown.efficiencyScore;
    qDebug() << "violations =" << goodReport.violations.size();
    qDebug() << "qLearning reward =" << goodReport.qLearningFeedback.reward
             << "normalized =" << goodReport.qLearningFeedback.normalizedScore
             << "safetyRisk =" << goodReport.qLearningFeedback.safetyRisk;

    const QList<DrivingData> badData = makeBadData();
    const QList<ViolationEvent> badViolations = makeBadViolations(badData);
    const ScoreReport badReport = manager.evaluate(badData, badViolations);
    qDebug() << "\n=== Bad Scenario ===";
    qDebug() << "totalScore =" << badReport.totalScore;
    qDebug() << "grade =" << badReport.grade;
    qDebug() << "dimension scores ="
             << badReport.breakdown.safetyScore
             << badReport.breakdown.ruleComplianceScore
             << badReport.breakdown.smoothnessScore
             << badReport.breakdown.efficiencyScore;
    qDebug() << "violations =" << badReport.violations.size();
    qDebug() << "qLearning reward =" << badReport.qLearningFeedback.reward
             << "ruleCompliance =" << badReport.qLearningFeedback.ruleCompliance
             << "terminalPenalty =" << badReport.qLearningFeedback.terminalPenalty;

    AIAPIClient aiClient;
    const QString coachMd = aiClient.generateCoachReport(badReport);
    qDebug() << "\n=== AI Coach Report (Mock) ===\n" << coachMd;

    const QByteArray jsonBytes = QJsonDocument(badReport.toJson()).toJson(QJsonDocument::Indented);
    const QString markdown = badReport.toMarkdown();
    qDebug() << "\n=== Serialization Check ===";
    qDebug() << "json not empty =" << !jsonBytes.isEmpty() << "size =" << jsonBytes.size();
    qDebug() << "markdown not empty =" << !markdown.isEmpty() << "size =" << markdown.size();

    qDebug() << "\n=== Assertions ===";
    qDebug() << "good grade expected A/B ->" << (goodReport.grade == "A" || goodReport.grade == "B");
    qDebug() << "bad grade expected not A ->" << (badReport.grade != "A");
    qDebug() << "scoreReady signal triggered ->" << scoreReadyTriggered;

    manager.generateCoachReport(badReport);

    return 0;
}

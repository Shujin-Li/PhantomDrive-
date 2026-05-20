#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QJsonDocument>
#include <QtMath>

#include "PhantomDrive/gamemode/SpeedLimitSignObject.h"
#include "PhantomDrive/gamemode/TrafficObjectManager.h"
#include "PhantomDrive/scoring/AIAPIClient.h"
#include "PhantomDrive/scoring/DrivingScoreCalculator.h"
#include "PhantomDrive/scoring/ScoreManager.h"
#include "PhantomDrive/scoring/ViolationConfig.h"

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

static QList<DrivingData> makeMediumData()
{
    QList<DrivingData> list;
    const qint64 base = QDateTime::currentMSecsSinceEpoch();
    for (int i = 0; i < 120; ++i) {
        DrivingData d;
        d.timestamp = base + i * 100;
        d.position = QVector2D(static_cast<qreal>(i) * 0.7, 0.5);
        d.currentSpeedLimit = 12.0;
        d.speed = (i % 10 < 3) ? 13.8 : 11.0;
        d.acceleration = (i % 20 == 0) ? 8.2 : ((i % 17 == 0) ? -8.4 : 1.6);
        d.steeringAngle = (i % 18 == 0) ? 30.0 : 10.0;
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

    addViolation(20, ViolationType::SpeedOverLimit, QStringLiteral("speed"), 6);
    addViolation(45, ViolationType::SpeedOverLimit, QStringLiteral("speed"), 6);
    addViolation(70, ViolationType::Collision, QStringLiteral("collision"), 15);
    addViolation(80, ViolationType::RedLight, QStringLiteral("redlight"), 12);
    addViolation(95, ViolationType::PedestrianCollision, QStringLiteral("pedestrian"), 20);
    return list;
}

static QList<DrivingData> makePenaltyTestData()
{
    QList<DrivingData> list;
    const qint64 base = QDateTime::currentMSecsSinceEpoch();
    for (int i = 0; i < 10; ++i) {
        DrivingData d;
        d.timestamp = base + i * 100;
        d.position = QVector2D(static_cast<qreal>(i), 0.0);
        d.speed = 8.0;
        d.currentSpeedLimit = 12.0;
        list.append(d);
    }
    return list;
}

static ViolationEvent makeSpeedViolation(const QList<DrivingData>& dataList, int penaltyPoints)
{
    ViolationEvent v;
    v.timestamp = dataList[2].timestamp;
    v.type = ViolationType::SpeedOverLimit;
    v.description = QStringLiteral("speed event");
    v.position = dataList[2].position;
    v.speedAtViolation = 14.0;
    v.speedLimit = 12.0;
    v.penaltyPoints = penaltyPoints;
    return v;
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
    manager.setTrafficObjectManager(&trafficManager);

    bool scoreReadyTriggered = false;
    QObject::connect(&manager, &ScoreManager::scoreReady, [&](const ScoreReport&) {
        scoreReadyTriggered = true;
    });

    const ScoreReport goodReport = manager.evaluate(makeGoodData(), {});
    const ScoreReport mediumReport = manager.evaluate(makeMediumData(), {});
    const QList<DrivingData> badData = makeBadData();
    const ScoreReport badReport = manager.evaluate(badData, makeBadViolations(badData));

    AIAPIClient aiClient;
    const QString coachMd = aiClient.generateCoachReport(badReport);
    const bool hasBannedTerms =
            coachMd.contains(QStringLiteral("根据"))
            || coachMd.contains(QStringLiteral("结合"))
            || coachMd.contains(QStringLiteral("你提供的输入"))
            || coachMd.contains(QStringLiteral("评分JSON"))
            || coachMd.contains(QStringLiteral("数据来源"));

    DrivingScoreCalculator calculator;
    const QList<DrivingData> penaltyTestData = makePenaltyTestData();
    const ScoreReport customPenaltyReport = calculator.evaluate(
            penaltyTestData, {makeSpeedViolation(penaltyTestData, 30)}, QStringLiteral("custom-penalty"));
    const ScoreReport fallbackPenaltyReport = calculator.evaluate(
            penaltyTestData, {makeSpeedViolation(penaltyTestData, 0)}, QStringLiteral("fallback-penalty"));

    bool ok = true;
    ok &= (goodReport.grade == "A" || goodReport.grade == "B");
    ok &= (goodReport.totalScore > mediumReport.totalScore + 4.0);
    ok &= (mediumReport.totalScore > badReport.totalScore + 4.0);
    ok &= (badReport.grade != "A");
    ok &= scoreReadyTriggered;
    ok &= coachMd.contains(QStringLiteral("# 驾驶教练报告"));
    ok &= !hasBannedTerms;
    ok &= (customPenaltyReport.breakdown.speedPenalty > fallbackPenaltyReport.breakdown.speedPenalty);
    ok &= (qAbs(fallbackPenaltyReport.breakdown.speedPenalty
                - static_cast<qreal>(ViolationConfig::speedViolationPenalty)) < 0.001);

    if (!ok) {
        qCritical() << "a-b-scoring-test failed.";
        return 1;
    }

    qDebug() << "a-b-scoring-test passed.";
    qDebug() << QString::fromUtf8(QJsonDocument(badReport.toJson()).toJson(QJsonDocument::Indented)).left(300);
    return 0;
}

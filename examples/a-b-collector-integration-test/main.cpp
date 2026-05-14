#include <QCoreApplication>
#include <QDebug>
#include <QJsonDocument>

#include "PhantomDrive/gamemode/DrivingDataCollector.h"
#include "PhantomDrive/scoring/ScoreManager.h"

using namespace PhantomDrive;

namespace {

bool expect(bool condition, const QString& message)
{
    if (condition) {
        qDebug().noquote() << "[PASS]" << message;
        return true;
    }

    qCritical().noquote() << "[FAIL]" << message;
    return false;
}

void injectCollectorData(DrivingDataCollector& collector)
{
    DrivingDataStorage* storage = collector.dataStorage();
    if (storage == nullptr) {
        return;
    }

    storage->clear();

    const qint64 baseTs = 1715601000000;
    for (int i = 0; i < 50; ++i) {
        DrivingData data;
        data.timestamp = baseTs + i * 100;
        data.position = QVector2D(static_cast<qreal>(i), static_cast<qreal>(i % 4) * 0.2);
        data.speed = 9.0 + static_cast<qreal>(i % 5);
        data.currentSpeedLimit = 12.0;
        data.acceleration = (i % 12 == 0) ? -8.4 : 1.1;
        data.steeringAngle = (i % 14 == 0) ? 36.0 : 5.0;
        storage->addData(data);
    }

    ViolationEvent speedViolation;
    speedViolation.timestamp = baseTs + 2200;
    speedViolation.type = ViolationType::SpeedOverLimit;
    speedViolation.description = QStringLiteral("collector speed violation");
    speedViolation.position = QVector2D(22.0, 0.4);
    speedViolation.speedAtViolation = 16.0;
    speedViolation.speedLimit = 12.0;
    speedViolation.penaltyPoints = 6;
    storage->addViolation(speedViolation);

    ViolationEvent collisionViolation;
    collisionViolation.timestamp = baseTs + 3300;
    collisionViolation.type = ViolationType::Collision;
    collisionViolation.description = QStringLiteral("collector collision violation");
    collisionViolation.position = QVector2D(33.0, 0.6);
    collisionViolation.speedAtViolation = 10.0;
    collisionViolation.speedLimit = 12.0;
    collisionViolation.penaltyPoints = 15;
    storage->addViolation(collisionViolation);
}

} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    qRegisterMetaType<PhantomDrive::ScoreReport>("PhantomDrive::ScoreReport");

    bool allPassed = true;

    DrivingDataCollector collector;
    collector.setVehicleId(QStringLiteral("AB_COLLECTOR_TEST"));
    injectCollectorData(collector);

    ScoreManager manager;
    manager.setVehicleId(QStringLiteral("AB_COLLECTOR_TEST"));

    int scoreReadyCount = 0;
    QObject::connect(&manager, &ScoreManager::scoreReady, [&](const ScoreReport& report) {
        ++scoreReadyCount;
        qDebug().noquote() << "[scoreReady] collector totalScore=" << report.totalScore << "grade=" << report.grade;
    });

    const ScoreReport report = manager.evaluateFromCollector(&collector);

    allPassed &= expect(scoreReadyCount == 1, QStringLiteral("evaluateFromCollector emits scoreReady"));
    allPassed &= expect(report.metrics.dataPointCount == collector.getCollectedData().size(),
                        QStringLiteral("report metrics match collector data size"));
    allPassed &= expect(report.violations.size() >= collector.getViolations().size(),
                        QStringLiteral("report contains collector violations"));
    allPassed &= expect(!report.grade.isEmpty(), QStringLiteral("grade is non-empty"));
    allPassed &= expect(!report.qLearningFeedback.recommendedActionHint.trimmed().isEmpty(),
                        QStringLiteral("qLearningFeedback is populated"));

    qDebug().noquote() << "\n=== Collector Score JSON ===";
    qDebug().noquote() << QString::fromUtf8(QJsonDocument(report.toJson()).toJson(QJsonDocument::Indented));

    if (!allPassed) {
        qCritical().noquote() << "A-B collector integration test failed.";
        return 1;
    }

    qDebug().noquote() << "A-B collector integration test passed.";
    return 0;
}

#include <QCoreApplication>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVector2D>

#include "PhantomDrive/scoring/AIAPIClient.h"
#include "PhantomDrive/scoring/ScoreManager.h"

using namespace PhantomDrive;

namespace {

QList<DrivingData> buildDrivingData()
{
    QList<DrivingData> dataList;
    const qint64 baseTs = 1715600000000;

    for (int i = 0; i < 80; ++i) {
        DrivingData data;
        data.timestamp = baseTs + i * 100;
        data.position = QVector2D(static_cast<qreal>(i) * 1.2, static_cast<qreal>(i % 5) * 0.1);
        data.speed = 10.0 + static_cast<qreal>(i % 6);
        data.currentSpeedLimit = 14.0;
        data.acceleration = (i % 10 == 0) ? 8.5 : 1.5;
        data.steeringAngle = (i % 16 == 0) ? 38.0 : 6.0;
        dataList.append(data);
    }

    return dataList;
}

QList<ViolationEvent> buildViolations(const QList<DrivingData>& dataList)
{
    QList<ViolationEvent> violations;

    auto appendViolation = [&](int index, ViolationType type, const QString& desc, int penalty) {
        ViolationEvent event;
        event.timestamp = dataList[index].timestamp;
        event.type = type;
        event.description = desc;
        event.position = dataList[index].position;
        event.speedAtViolation = dataList[index].speed;
        event.speedLimit = dataList[index].currentSpeedLimit;
        event.penaltyPoints = penalty;
        violations.append(event);
    };

    appendViolation(20, ViolationType::SpeedOverLimit, QStringLiteral("speed over limit"), 6);
    appendViolation(52, ViolationType::RedLight, QStringLiteral("red light violation"), 12);

    return violations;
}

bool expect(bool condition, const QString& message)
{
    if (condition) {
        qDebug().noquote() << "[PASS]" << message;
        return true;
    }

    qCritical().noquote() << "[FAIL]" << message;
    return false;
}

void setOrUnsetEnv(const char* name, const QByteArray& value)
{
    if (value.isEmpty()) {
        qunsetenv(name);
    } else {
        qputenv(name, value);
    }
}

} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    qRegisterMetaType<PhantomDrive::ScoreReport>("PhantomDrive::ScoreReport");

    bool allPassed = true;

    ScoreManager manager;
    manager.setVehicleId(QStringLiteral("AB_MAIN_INTEGRATION"));

    int scoreReadyCount = 0;
    int coachReportReadyCount = 0;
    QString lastCoachReport;

    QObject::connect(&manager, &ScoreManager::scoreReady, [&](const ScoreReport& report) {
        ++scoreReadyCount;
        qDebug().noquote() << "[scoreReady] totalScore=" << report.totalScore << "grade=" << report.grade;
    });

    QObject::connect(&manager, &ScoreManager::coachReportReady, [&](const QString& markdown) {
        ++coachReportReadyCount;
        lastCoachReport = markdown;
        qDebug().noquote() << "[coachReportReady] markdown length=" << markdown.size();
    });

    QObject::connect(&manager, &ScoreManager::scoringFailed, [&](const QString& reason) {
        qWarning().noquote() << "[scoringFailed]" << reason;
    });

    const QList<DrivingData> dataList = buildDrivingData();
    const QList<ViolationEvent> violations = buildViolations(dataList);

    const ScoreReport report = manager.evaluate(dataList, violations);
    const QJsonObject reportJson = report.toJson();
    const QString reportMarkdown = report.toMarkdown();

    allPassed &= expect(scoreReadyCount == 1, QStringLiteral("scoreReady signal emitted once"));
    allPassed &= expect(report.totalScore >= 0.0 && report.totalScore <= 100.0,
                        QStringLiteral("totalScore in [0, 100]"));
    allPassed &= expect(!report.grade.isEmpty(), QStringLiteral("grade is non-empty"));
    allPassed &= expect(reportJson.contains(QStringLiteral("metrics")), QStringLiteral("toJson contains metrics"));
    allPassed &= expect(reportJson.contains(QStringLiteral("breakdown")), QStringLiteral("toJson contains breakdown"));
    allPassed &= expect(reportJson.contains(QStringLiteral("violations")), QStringLiteral("toJson contains violations"));
    allPassed &= expect(reportJson.contains(QStringLiteral("coachAdvices")), QStringLiteral("toJson contains coachAdvices"));
    allPassed &= expect(reportJson.contains(QStringLiteral("qLearningFeedback")), QStringLiteral("toJson contains qLearningFeedback"));

    const QJsonObject metricsJson = reportJson.value(QStringLiteral("metrics")).toObject();
    allPassed &= expect(metricsJson.value(QStringLiteral("durationMs")).isDouble(),
                        QStringLiteral("metrics.durationMs is numeric JSON"));
    allPassed &= expect(!reportMarkdown.trimmed().isEmpty(), QStringLiteral("toMarkdown is non-empty"));
    allPassed &= expect(!report.qLearningFeedback.recommendedActionHint.trimmed().isEmpty(),
                        QStringLiteral("qLearningFeedback.recommendedActionHint is non-empty"));

    const QString managerCoachReport = manager.generateCoachReport(report);
    allPassed &= expect(coachReportReadyCount == 1, QStringLiteral("coachReportReady signal emitted once"));
    allPassed &= expect(!managerCoachReport.trimmed().isEmpty(), QStringLiteral("ScoreManager::generateCoachReport non-empty"));

    AIAPIClient aiClient;

    const QByteArray originalMode = qgetenv("PHANTOMDRIVE_AI_MODE");
    const QByteArray originalTimeout = qgetenv("PHANTOMDRIVE_AI_TIMEOUT_MS");
    const QByteArray originalDeepSeekKey = qgetenv("DEEPSEEK_API_KEY");
    const QByteArray originalZhipuKey = qgetenv("ZHIPU_API_KEY");

    qunsetenv("DEEPSEEK_API_KEY");
    qunsetenv("ZHIPU_API_KEY");
    qputenv("PHANTOMDRIVE_AI_MODE", "auto");
    qputenv("PHANTOMDRIVE_AI_TIMEOUT_MS", "300");
    const QString autoReport = aiClient.generateCoachReport(report);
    allPassed &= expect(autoReport.contains(QStringLiteral("Mock"), Qt::CaseInsensitive),
                        QStringLiteral("auto mode without keys falls back to Mock"));

    qputenv("PHANTOMDRIVE_AI_MODE", "mock");
    const QString mockModeReport = aiClient.generateCoachReport(report);
    allPassed &= expect(mockModeReport.contains(QStringLiteral("mode=mock")),
                        QStringLiteral("mock mode forces Mock report"));

    qputenv("PHANTOMDRIVE_AI_MODE", "deepseek");
    qputenv("DEEPSEEK_API_KEY", "invalid-key-for-test");
    qputenv("PHANTOMDRIVE_AI_TIMEOUT_MS", "200");
    const QString deepseekFailureReport = aiClient.generateCoachReport(report);
    allPassed &= expect(deepseekFailureReport.contains(QStringLiteral("Mock"), Qt::CaseInsensitive),
                        QStringLiteral("deepseek mode with invalid key falls back to Mock"));

    setOrUnsetEnv("PHANTOMDRIVE_AI_MODE", originalMode);
    setOrUnsetEnv("PHANTOMDRIVE_AI_TIMEOUT_MS", originalTimeout);
    setOrUnsetEnv("DEEPSEEK_API_KEY", originalDeepSeekKey);
    setOrUnsetEnv("ZHIPU_API_KEY", originalZhipuKey);

    const QByteArray runRealApiTest = qgetenv("PHANTOMDRIVE_RUN_REAL_API_TEST");
    if (runRealApiTest == "1") {
        const QByteArray mode = qgetenv("PHANTOMDRIVE_AI_MODE").trimmed().toLower();
        const bool canRunDeepSeek = (mode == "deepseek" && !qgetenv("DEEPSEEK_API_KEY").isEmpty());
        const bool canRunZhipu = (mode == "zhipu" && !qgetenv("ZHIPU_API_KEY").isEmpty());

        if (canRunDeepSeek || canRunZhipu) {
            const QString realApiReport = aiClient.generateCoachReport(report);
            if (realApiReport.contains(QStringLiteral("Mock"), Qt::CaseInsensitive)) {
                qWarning().noquote() << "[WARN] real api failed and fallback to Mock";
            } else {
                qDebug().noquote() << "[PASS] real api returned non-mock coach report";
            }
        } else {
            qWarning().noquote() << "[WARN] real api test skipped: set PHANTOMDRIVE_AI_MODE=deepseek|zhipu and matching API key";
        }
    }

    qDebug().noquote() << "\n=== Score JSON Preview ===";
    qDebug().noquote() << QString::fromUtf8(QJsonDocument(reportJson).toJson(QJsonDocument::Indented));

    qDebug().noquote() << "\n=== Coach Report Preview ===";
    qDebug().noquote() << lastCoachReport.left(400);

    if (!allPassed) {
        qCritical().noquote() << "A-B main integration smoke test failed.";
        return 1;
    }

    qDebug().noquote() << "A-B main integration smoke test passed.";
    return 0;
}

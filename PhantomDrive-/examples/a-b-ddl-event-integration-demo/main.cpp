#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QJsonObject>
#include <QPointF>
#include <QTimer>
#include <QVector2D>

#include "PhantomDrive/scoring/ScoreManager.h"

using namespace PhantomDrive;

namespace {

void logInfo(const QString& line)
{
    qInfo().noquote() << line;
}

void logWarning(const QString& line)
{
    qWarning().noquote() << line;
}

ViolationEvent makeViolation(qint64 timestamp,
                             ViolationType type,
                             const QString& description,
                             const QVector2D& position,
                             qreal speed,
                             qreal speedLimit,
                             int penaltyPoints)
{
    ViolationEvent event;
    event.timestamp = timestamp;
    event.type = type;
    event.description = description;
    event.position = position;
    event.speedAtViolation = speed;
    event.speedLimit = speedLimit;
    event.penaltyPoints = penaltyPoints;
    return event;
}

QString localViolationTypeToString(ViolationType type)
{
    switch (type) {
    case ViolationType::RedLight: return QStringLiteral("RedLight");
    case ViolationType::SpeedOverLimit: return QStringLiteral("SpeedOverLimit");
    case ViolationType::PedestrianCollision: return QStringLiteral("PedestrianCollision");
    case ViolationType::WrongWay: return QStringLiteral("WrongWay");
    case ViolationType::Collision: return QStringLiteral("Collision");
    default: return QStringLiteral("None");
    }
}

QString violationTypes(const QList<ViolationEvent>& violations)
{
    QStringList types;
    for (const ViolationEvent& violation : violations) {
        types.append(localViolationTypeToString(violation.type));
    }
    return types.join(QStringLiteral(", "));
}

} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    if (qEnvironmentVariableIsEmpty("PHANTOMDRIVE_AI_MODE")) {
        qputenv("PHANTOMDRIVE_AI_MODE", "mock");
    }

    ScoreManager manager;
    manager.startSession(QStringLiteral("AB_DDL_EVENT_DEMO"));

    QObject::connect(&manager, &ScoreManager::feedbackReady,
                     [](const QString& text, int pointsDelta, const QString& severity) {
        logInfo(QStringLiteral("[feedbackReady] %1 pointsDelta=%2 severity=%3")
                .arg(text)
                .arg(pointsDelta)
                .arg(severity));
    });

    QObject::connect(&manager, &ScoreManager::reportReady, [](const ScoreReport& report) {
        logInfo(QStringLiteral("[reportReady] totalScore=%1 grade=%2 violations count=%3 types=%4")
                .arg(report.totalScore, 0, 'f', 2)
                .arg(report.grade)
                .arg(report.violations.size())
                .arg(violationTypes(report.violations)));
    });

    QObject::connect(&manager, &ScoreManager::reportJsonReady, [](const QJsonObject& reportJson) {
        const QJsonObject qLearning = reportJson.value(QStringLiteral("qLearningFeedback")).toObject();
        logInfo(QStringLiteral("[reportJsonReady] qLearningFeedback.reward=%1 qLearningFeedback.recommendedActionHint=%2")
                .arg(qLearning.value(QStringLiteral("reward")).toDouble(), 0, 'f', 4)
                .arg(qLearning.value(QStringLiteral("recommendedActionHint")).toString()));
    });

    QObject::connect(&manager, &ScoreManager::qLearningFeedbackReady,
                     [](const QLearningFeedback& feedback) {
        logInfo(QStringLiteral("[qLearningFeedbackReady] reward=%1 recommendedActionHint=%2")
                .arg(feedback.reward, 0, 'f', 4)
                .arg(feedback.recommendedActionHint));
    });

    QObject::connect(&manager, &ScoreManager::coachReportReady, [&](const QString& markdown) {
        logInfo(QStringLiteral("[coachReportReady] AI coach mock/fallback text:"));
        logInfo(markdown.left(800));
        app.quit();
    });

    QObject::connect(&manager, &ScoreManager::scoringFailed, [](const QString& reason) {
        logWarning(QStringLiteral("[scoringFailed] %1").arg(reason));
    });

    const qint64 base = QDateTime::currentMSecsSinceEpoch();
    manager.recordViolation(makeViolation(base,
                                          ViolationType::SpeedOverLimit,
                                          QStringLiteral("E-B speed over limit"),
                                          QVector2D(12.0f, 2.0f),
                                          18.0,
                                          12.0,
                                          5));
    manager.recordViolation(makeViolation(base + 1600,
                                          ViolationType::RedLight,
                                          QStringLiteral("E-B red light violation"),
                                          QVector2D(25.0f, 4.0f),
                                          10.0,
                                          0.0,
                                          10));
    manager.recordCollision(QPointF(36.0, 6.0), 8.0, QStringLiteral("Wall hit from E-A collision event"));
    manager.recordViolation(makeViolation(base + 4200,
                                          ViolationType::PedestrianCollision,
                                          QStringLiteral("E-B pedestrian zone violation"),
                                          QVector2D(44.0f, 7.0f),
                                          7.0,
                                          0.0,
                                          15));

    manager.recordSafeDrivingTick(base + 9000, 9.0);
    manager.recordSafeDrivingTick(base + 14500, 9.0);

    const ScoreReport report = manager.finishSession();
    logInfo(QStringLiteral("[finishSession] totalScore=%1").arg(report.totalScore, 0, 'f', 2));
    logInfo(QStringLiteral("[finishSession] grade=%1").arg(report.grade));
    logInfo(QStringLiteral("[finishSession] violations count=%1").arg(report.violations.size()));
    logInfo(QStringLiteral("[finishSession] qLearningFeedback.reward=%1")
            .arg(report.qLearningFeedback.reward, 0, 'f', 4));
    logInfo(QStringLiteral("[finishSession] qLearningFeedback.recommendedActionHint=%1")
            .arg(report.qLearningFeedback.recommendedActionHint));

    QTimer::singleShot(8000, &app, [&]() {
        logWarning(QStringLiteral("Timed out waiting for async coach report."));
        app.quit();
    });

    return app.exec();
}

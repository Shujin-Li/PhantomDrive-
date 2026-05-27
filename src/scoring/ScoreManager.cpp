#include "PhantomDrive/scoring/ScoreManager.h"

#include "PhantomDrive/gamemode/IDrivingDataCollector.h"
#include "PhantomDrive/gamemode/TrafficObjectManager.h"
#include "PhantomDrive/scoring/ViolationConfig.h"

#include <QDateTime>
#include <QMetaObject>
#include <QPointer>
#include <QThread>
#include <QVector2D>

namespace PhantomDrive {

namespace {

constexpr qint64 FeedbackThrottleMs = 1200;
constexpr qint64 SafeDrivingQuietMs = 8000;
constexpr qint64 GreatFeedbackThrottleMs = 10000;
constexpr qreal SafeDrivingLowSpeedThreshold = 0.5;

struct FeedbackMessage {
    QString text;
    int pointsDelta = 0;
    QString severity;
};

FeedbackMessage feedbackForViolation(const ViolationEvent& event)
{
    switch (event.type) {
    case ViolationType::SpeedOverLimit:
        return {QStringLiteral("Speeding! -5"), -5, QStringLiteral("warning")};
    case ViolationType::RedLight:
        return {QStringLiteral("Red Light Violation! -10"), -10, QStringLiteral("danger")};
    case ViolationType::PedestrianCollision:
        return {QStringLiteral("Pedestrian Zone Violation! -15"), -15, QStringLiteral("danger")};
    case ViolationType::Collision:
        if (event.description.contains(QStringLiteral("wall"), Qt::CaseInsensitive)
            || event.description.contains(QStringLiteral("boundary"), Qt::CaseInsensitive)) {
            return {QStringLiteral("Wall Hit! -8"), -8, QStringLiteral("danger")};
        }
        return {QStringLiteral("Collision! -8"), -8, QStringLiteral("danger")};
    case ViolationType::WrongWay:
        return {QStringLiteral("Wrong Way! -10"), -10, QStringLiteral("danger")};
    default:
        return {QStringLiteral("Driving Violation!"), 0, QStringLiteral("warning")};
    }
}

int defaultPenaltyForType(ViolationType type)
{
    switch (type) {
    case ViolationType::SpeedOverLimit:
        return 5;
    case ViolationType::RedLight:
        return 10;
    case ViolationType::PedestrianCollision:
        return 15;
    case ViolationType::Collision:
        return 8;
    case ViolationType::WrongWay:
        return ViolationConfig::wrongWayPenalty;
    default:
        return 0;
    }
}

} // namespace

ScoreManager::ScoreManager(QObject* parent)
    : QObject(parent)
    , m_vehicleId()
    , m_trafficManager(nullptr)
    , m_calculator()
    , m_ruleEnforcer()
    , m_aiClient(this)
    , m_lastReport()
    , m_lastViolationTimestampMs(0)
    , m_lastGreatFeedbackTimestampMs(0)
    , m_lastFeedbackByType()
{
    qRegisterMetaType<PhantomDrive::ScoreReport>("PhantomDrive::ScoreReport");
    qRegisterMetaType<PhantomDrive::QLearningFeedback>("PhantomDrive::QLearningFeedback");
    qRegisterMetaType<PhantomDrive::ViolationEvent>("PhantomDrive::ViolationEvent");
    qRegisterMetaType<QJsonObject>("QJsonObject");
}

void ScoreManager::setVehicleId(const QString& vehicleId)
{
    m_vehicleId = vehicleId;
}

void ScoreManager::setTrafficObjectManager(TrafficObjectManager* manager)
{
    if (m_trafficManager != nullptr) {
        disconnect(m_trafficManager, &TrafficObjectManager::violationDetected,
                   this, &ScoreManager::onViolationDetected);
    }
    m_trafficManager = manager;
    if (m_trafficManager != nullptr) {
        connect(m_trafficManager, &TrafficObjectManager::violationDetected,
                this, &ScoreManager::onViolationDetected);
    }
}

void ScoreManager::startSession(const QString& vehicleId)
{
    if (!vehicleId.trimmed().isEmpty()) {
        m_vehicleId = vehicleId;
    }

    m_pendingViolations.clear();
    m_lastReport = ScoreReport();
    m_lastViolationTimestampMs = 0;
    m_lastGreatFeedbackTimestampMs = 0;
    m_lastFeedbackByType.clear();
}

void ScoreManager::recordViolation(const ViolationEvent& event)
{
    ViolationEvent normalized = event;
    if (normalized.timestamp <= 0) {
        normalized.timestamp = QDateTime::currentMSecsSinceEpoch();
    }
    if (normalized.penaltyPoints == 0) {
        normalized.penaltyPoints = defaultPenaltyForType(normalized.type);
    }

    m_pendingViolations.append(normalized);
    m_lastViolationTimestampMs = normalized.timestamp;

    const int typeKey = static_cast<int>(normalized.type);
    const qint64 lastFeedback = m_lastFeedbackByType.value(typeKey, 0);
    if (lastFeedback > 0 && normalized.timestamp - lastFeedback < FeedbackThrottleMs) {
        return;
    }

    const FeedbackMessage feedback = feedbackForViolation(normalized);
    m_lastFeedbackByType.insert(typeKey, normalized.timestamp);
    emit feedbackReady(feedback.text, feedback.pointsDelta, feedback.severity);
}

void ScoreManager::recordCollision(const QPointF& position, qreal speed, const QString& description)
{
    ViolationEvent event;
    event.timestamp = QDateTime::currentMSecsSinceEpoch();
    event.type = ViolationType::Collision;
    event.description = description.trimmed().isEmpty() ? QStringLiteral("Collision") : description;
    event.position = QVector2D(position);
    event.speedAtViolation = speed;
    event.speedLimit = 0.0;
    event.penaltyPoints = defaultPenaltyForType(ViolationType::Collision);

    recordViolation(event);
}

void ScoreManager::recordSafeDrivingTick(qint64 timestampMs, qreal speed)
{
    const qint64 now = timestampMs > 0 ? timestampMs : QDateTime::currentMSecsSinceEpoch();
    if (speed <= SafeDrivingLowSpeedThreshold) {
        return;
    }
    if (m_lastViolationTimestampMs > 0 && now - m_lastViolationTimestampMs < SafeDrivingQuietMs) {
        return;
    }
    if (m_lastGreatFeedbackTimestampMs > 0
        && now - m_lastGreatFeedbackTimestampMs < GreatFeedbackThrottleMs) {
        return;
    }

    m_lastGreatFeedbackTimestampMs = now;
    emit feedbackReady(QStringLiteral("Great! Safe Driving!"), 0, QStringLiteral("positive"));
}

ScoreReport ScoreManager::finishSession(const IDrivingDataCollector* collector)
{
    ScoreReport report;
    if (collector != nullptr) {
        report = evaluateFromCollector(collector);
    } else {
        report = evaluate({}, {});
    }

    m_lastReport = report;
    emit reportReady(report);
    emit reportJsonReady(report.toJson());
    emit qLearningFeedbackReady(report.qLearningFeedback);
    generateCoachReportAsync(report);
    return report;
}

ScoreReport ScoreManager::latestReport() const
{
    return m_lastReport;
}

void ScoreManager::generateCoachReportAsync(const ScoreReport& report)
{
    QPointer<ScoreManager> self(this);
    QThread* worker = QThread::create([self, report]() {
        AIAPIClient client;
        const QString markdown = client.generateCoachReport(report);

        if (self.isNull()) {
            return;
        }

        QMetaObject::invokeMethod(self.data(), [self, markdown]() {
            if (!self.isNull()) {
                emit self->coachReportReady(markdown);
            }
        }, Qt::QueuedConnection);
    });

    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    worker->start();
}

ScoreReport ScoreManager::buildReport(const QList<DrivingData>& data,
                                      const QList<ViolationEvent>& violations,
                                      bool emitFailure)
{
    if (data.isEmpty() && violations.isEmpty() && m_pendingViolations.isEmpty()) {
        if (emitFailure) {
            emit scoringFailed(QStringLiteral("No driving data or violations provided for scoring."));
        }
        ScoreReport emptyReport;
        emptyReport.sessionId = QStringLiteral("session_%1").arg(QDateTime::currentMSecsSinceEpoch());
        emptyReport.vehicleId = m_vehicleId;
        emptyReport.generatedAt = QDateTime::currentDateTimeUtc();
        emptyReport.totalScore = 0.0;
        emptyReport.grade = ScoreReport::gradeFromScore(0.0);
        emptyReport.summary = QStringLiteral("No valid driving data or violations were provided for scoring.");
        return emptyReport;
    }

    QList<ViolationEvent> allViolations = violations;
    allViolations.append(m_pendingViolations);
    allViolations = m_ruleEnforcer.filterDuplicates(allViolations);
    m_pendingViolations.clear();

    return m_calculator.evaluate(data, allViolations, m_vehicleId);
}

ScoreReport ScoreManager::evaluate(const QList<DrivingData>& data,
                                   const QList<ViolationEvent>& violations)
{
    ScoreReport report = buildReport(data, violations, true);
    m_lastReport = report;
    emit scoreReady(report);
    return report;
}

ScoreReport ScoreManager::evaluateFromCollector(const IDrivingDataCollector* collector)
{
    if (collector == nullptr) {
        emit scoringFailed(QStringLiteral("Collector is null."));
        return evaluate({}, {});
    }
    return evaluate(collector->getCollectedData(), collector->getViolations());
}

QString ScoreManager::generateCoachReport(const ScoreReport& report) const
{
    const QString markdown = m_aiClient.generateCoachReport(report);
    emit const_cast<ScoreManager*>(this)->coachReportReady(markdown);
    return markdown;
}

void ScoreManager::addViolation(const ViolationEvent& event)
{
    recordViolation(event);
}

void ScoreManager::onViolationDetected(const ViolationEvent& event)
{
    recordViolation(event);
}

} // namespace PhantomDrive

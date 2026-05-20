#include "PhantomDrive/scoring/ScoreManager.h"

#include "PhantomDrive/gamemode/IDrivingDataCollector.h"
#include "PhantomDrive/gamemode/TrafficObjectManager.h"

#include <QDateTime>

namespace PhantomDrive {

ScoreManager::ScoreManager(QObject* parent)
    : QObject(parent)
    , m_vehicleId()
    , m_trafficManager(nullptr)
    , m_calculator()
    , m_ruleEnforcer()
    , m_aiClient(this)
{
    qRegisterMetaType<PhantomDrive::ScoreReport>("PhantomDrive::ScoreReport");
    qRegisterMetaType<PhantomDrive::ViolationEvent>("PhantomDrive::ViolationEvent");
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

ScoreReport ScoreManager::evaluate(const QList<DrivingData>& data,
                                   const QList<ViolationEvent>& violations)
{
    QList<ViolationEvent> consumedPendingViolations;
    consumedPendingViolations.swap(m_pendingViolations);

    if (data.isEmpty() && violations.isEmpty() && consumedPendingViolations.isEmpty()) {
        emit scoringFailed(QStringLiteral("No driving data or violations provided for scoring."));
        ScoreReport emptyReport;
        emptyReport.sessionId = QStringLiteral("session_%1").arg(QDateTime::currentMSecsSinceEpoch());
        emptyReport.vehicleId = m_vehicleId;
        emptyReport.generatedAt = QDateTime::currentDateTimeUtc();
        emptyReport.totalScore = 0.0;
        emptyReport.grade = ScoreReport::gradeFromScore(0.0);
        emptyReport.summary = QStringLiteral("缺少有效驾驶数据，无法形成可信评分。");
        emit scoreReady(emptyReport);
        return emptyReport;
    }

    QList<ViolationEvent> allViolations;
    if (m_trafficManager != nullptr) {
        // In E-B driven mode, violations come from TrafficObjectManager::violationDetected.
        allViolations = consumedPendingViolations;
    } else {
        allViolations = violations;
        allViolations.append(consumedPendingViolations);
    }
    allViolations = m_ruleEnforcer.filterDuplicates(allViolations);

    ScoreReport report = m_calculator.evaluate(data, allViolations, m_vehicleId);
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
    m_pendingViolations.append(event);
}

void ScoreManager::onViolationDetected(const ViolationEvent& event)
{
    addViolation(event);
}

}

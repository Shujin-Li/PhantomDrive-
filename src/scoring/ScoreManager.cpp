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
}

void ScoreManager::setVehicleId(const QString& vehicleId)
{
    m_vehicleId = vehicleId;
}

void ScoreManager::setTrafficObjectManager(TrafficObjectManager* manager)
{
    m_trafficManager = manager;
}

ScoreReport ScoreManager::evaluate(const QList<DrivingData>& data,
                                   const QList<ViolationEvent>& violations)
{
    if (data.isEmpty()) {
        emit scoringFailed(QStringLiteral("No driving data provided for scoring."));
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

    QList<ViolationEvent> merged = violations;
    if (m_trafficManager != nullptr) {
        merged.append(m_ruleEnforcer.checkSequence(data, m_trafficManager));
    }

    ScoreReport report = m_calculator.evaluate(data, merged, m_vehicleId);
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

}

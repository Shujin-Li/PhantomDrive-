#pragma once

#include "PhantomDrive/scoring/AIAPIClient.h"
#include "PhantomDrive/scoring/DrivingScoreCalculator.h"
#include "PhantomDrive/scoring/TrafficRuleEnforcer.h"
#include "PhantomDrive_global.h"

#include <QObject>

namespace PhantomDrive {

class IDrivingDataCollector;
class TrafficObjectManager;

class PHANTOMDRIVE_EXPORT ScoreManager : public QObject
{
    Q_OBJECT

public:
    explicit ScoreManager(QObject* parent = nullptr);

    void setVehicleId(const QString& vehicleId);
    void setTrafficObjectManager(TrafficObjectManager* manager);

    ScoreReport evaluate(const QList<DrivingData>& data,
                         const QList<ViolationEvent>& violations);
    ScoreReport evaluateFromCollector(const IDrivingDataCollector* collector);
    QString generateCoachReport(const ScoreReport& report) const;

    void addViolation(const ViolationEvent& event);

public slots:
    void onViolationDetected(const ViolationEvent& event);

signals:
    void scoreReady(const PhantomDrive::ScoreReport& report);
    void coachReportReady(const QString& markdown);
    void scoringFailed(const QString& reason);

private:
    QString m_vehicleId;
    TrafficObjectManager* m_trafficManager;
    DrivingScoreCalculator m_calculator;
    TrafficRuleEnforcer m_ruleEnforcer;
    AIAPIClient m_aiClient;
    QList<ViolationEvent> m_pendingViolations;
};

}

#pragma once

#include "PhantomDrive/scoring/AIAPIClient.h"
#include "PhantomDrive/scoring/DrivingScoreCalculator.h"
#include "PhantomDrive/scoring/TrafficRuleEnforcer.h"
#include "PhantomDrive_global.h"

#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QPointF>

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

    void startSession(const QString& vehicleId = QString());
    ScoreReport finishSession(const IDrivingDataCollector* collector = nullptr);
    ScoreReport latestReport() const;

    ScoreReport evaluate(const QList<DrivingData>& data,
                         const QList<ViolationEvent>& violations);
    ScoreReport evaluateFromCollector(const IDrivingDataCollector* collector);
    QString generateCoachReport(const ScoreReport& report) const;

    void addViolation(const ViolationEvent& event);

public slots:
    void recordViolation(const ViolationEvent& event);
    void recordCollision(const QPointF& position, qreal speed, const QString& description = QString());
    void recordSafeDrivingTick(qint64 timestampMs, qreal speed);
    void generateCoachReportAsync(const ScoreReport& report);
    void onViolationDetected(const ViolationEvent& event);

signals:
    void scoreReady(const PhantomDrive::ScoreReport& report);
    void coachReportReady(const QString& markdown);
    void scoringFailed(const QString& reason);
    void feedbackReady(QString text, int pointsDelta, QString severity);
    void reportReady(PhantomDrive::ScoreReport report);
    void reportJsonReady(QJsonObject reportJson);
    void qLearningFeedbackReady(PhantomDrive::QLearningFeedback feedback);

private:
    ScoreReport buildReport(const QList<DrivingData>& data,
                            const QList<ViolationEvent>& violations,
                            bool emitFailure);

    QString m_vehicleId;
    TrafficObjectManager* m_trafficManager;
    DrivingScoreCalculator m_calculator;
    TrafficRuleEnforcer m_ruleEnforcer;
    AIAPIClient m_aiClient;
    QList<ViolationEvent> m_pendingViolations;
    ScoreReport m_lastReport;
    qint64 m_lastViolationTimestampMs;
    qint64 m_lastGreatFeedbackTimestampMs;
    QHash<int, qint64> m_lastFeedbackByType;
};

}

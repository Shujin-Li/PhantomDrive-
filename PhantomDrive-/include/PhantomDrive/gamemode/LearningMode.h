#pragma once

#include "GameMode.h"
#include "DrivingData.h"
#include "PhantomDrive_global.h"

#include <QRectF>
#include <QMap>
#include <QList>
#include <QQueue>

namespace PhantomDrive {

struct TrafficLightState {
    QString id;
    bool isRed;
    qint64 lastChangeTime;
    int cycleDurationMs;

    TrafficLightState() : isRed(true), lastChangeTime(0), cycleDurationMs(5000) {}
};

struct SpeedLimitZone {
    QString id;
    qreal speedLimit;
    QRectF bounds;
    bool isActive;

    SpeedLimitZone() : speedLimit(0.0), isActive(false) {}
};

struct PedestrianCrossing {
    QString id;
    QRectF bounds;
    bool isActive;
    int pedestrianCount;

    PedestrianCrossing() : isActive(false), pedestrianCount(0) {}
};

struct AICoachFeedback {
    qint64 timestamp;
    QString feedback;
    int scoreImpact;
    QString category;

    AICoachFeedback() : timestamp(0), scoreImpact(0) {}
};

class LearningMode : public GameMode
{
    Q_OBJECT

public:
    explicit LearningMode(QObject* parent = nullptr);
    ~LearningMode() override;

    ModeType getType() const override { return ModeType::Learning; }
    QString getName() const override { return QStringLiteral("Learning Mode"); }
    QString getDescription() const override { return QStringLiteral("Practice driving with traffic rules and AI coaching"); }

    void onEnter() override;
    void onExit() override;

    void update(qint64 elapsedMs) override;
    void render() override;

    void registerTrafficLight(const QString& id, const QRectF& position, int cycleDurationMs = 5000);
    void unregisterTrafficLight(const QString& id);
    void setTrafficLightState(const QString& id, bool isRed);
    bool getTrafficLightState(const QString& id) const;
    QList<TrafficLightState> getAllTrafficLights() const;

    void registerSpeedLimitZone(const QString& id, const QRectF& bounds, qreal speedLimit);
    void unregisterSpeedLimitZone(const QString& id);
    void enterSpeedLimitZone(const QString& id);
    void exitSpeedLimitZone(const QString& id);
    qreal getCurrentSpeedLimit() const { return m_currentSpeedLimit; }

    void registerPedestrianCrossing(const QString& id, const QRectF& bounds);
    void triggerPedestrianCollision(const QString& crossingId);
    int getPedestrianViolationCount() const { return m_pedestrianViolations; }

    void recordViolation(ViolationType type, const QString& description, const QVector2D& position);
    QList<ViolationEvent> getViolations() const { return m_violations; }
    int getTotalPenaltyPoints() const;
    int getViolationCount(ViolationType type) const;

    void addAICoachFeedback(const QString& feedback, int scoreImpact, const QString& category);
    QList<AICoachFeedback> getCoachFeedback() const { return m_coachFeedback; }

    qreal getCurrentScore() const { return m_totalScore; }
    void setInitialScore(qreal score);
    void addScore(qreal points);
    void deductScore(qreal points);

    bool isRedLightViolation(const QString& trafficLightId, const QVector2D& vehiclePosition);
    bool isSpeedViolation(qreal currentSpeed);
    bool isPedestrianViolation(const QString& crossingId);

    QJsonObject generateLearningReport() const;

public slots:
    void onTrafficLightChanged(bool isRed) override;
    void onSpeedLimitChanged(qreal newLimit) override;
    void onVehicleCollision(const QString& objectId) override;

signals:
    void trafficLightChanged(const QString& id, bool isRed);
    void speedLimitExceeded(qreal currentSpeed, qreal limit);
    void violationRecorded(const ViolationEvent& violation);
    void pedestrianCollisionDetected(const QString& crossingId);
    void scoreChanged(qreal newScore);
    void redLightViolation(const QString& trafficLightId);
    void learningSessionCompleted(qreal finalScore, int violationCount);
    void aiCoachFeedbackGenerated(const QString& feedback, int scoreImpact);

protected:
    void checkTrafficLightViolations(qint64 elapsedMs);
    void updateSpeedLimit(qint64 elapsedMs);
    void processPedestrianZones();

private:
    QMap<QString, TrafficLightState> m_trafficLights;
    QMap<QString, SpeedLimitZone> m_speedLimitZones;
    QMap<QString, PedestrianCrossing> m_pedestrianCrossings;

    qreal m_currentSpeedLimit;
    QString m_currentSpeedLimitZoneId;
    qint64 m_lastSpeedCheckTime;

    QList<ViolationEvent> m_violations;
    int m_redLightViolations;
    int m_speedViolations;
    int m_pedestrianViolations;

    QList<AICoachFeedback> m_coachFeedback;

    qreal m_totalScore;
    qreal m_initialScore;

    qint64 m_sessionStartTime;
    qint64 m_lastViolationTime;
    int m_minViolationIntervalMs;
};

}

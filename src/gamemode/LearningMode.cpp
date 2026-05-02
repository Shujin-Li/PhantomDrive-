#include "LearningMode.h"

#include <QDebug>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace PhantomDrive {

LearningMode::LearningMode(QObject* parent)
    : GameMode(parent)
    , m_currentSpeedLimit(0.0)
    , m_lastSpeedCheckTime(0)
    , m_redLightViolations(0)
    , m_speedViolations(0)
    , m_pedestrianViolations(0)
    , m_totalScore(100.0)
    , m_initialScore(100.0)
    , m_sessionStartTime(0)
    , m_lastViolationTime(0)
    , m_minViolationIntervalMs(1000)
{
}

LearningMode::~LearningMode()
{
}

void LearningMode::onEnter()
{
    GameMode::onEnter();
    m_sessionStartTime = QDateTime::currentMSecsSinceEpoch();
    m_lastSpeedCheckTime = m_sessionStartTime;
    m_lastViolationTime = 0;
    m_totalScore = m_initialScore;
    qDebug() << "LearningMode: Entered";
}

void LearningMode::onExit()
{
    GameMode::onExit();
    emit learningSessionCompleted(m_totalScore, m_violations.size());
    qDebug() << "LearningMode: Exited with score" << m_totalScore;
}

void LearningMode::update(qint64 elapsedMs)
{
    if (!isActive()) {
        return;
    }

    checkTrafficLightViolations(elapsedMs);
    updateSpeedLimit(elapsedMs);
    processPedestrianZones();

    emit modeUpdated(elapsedMs);
}

void LearningMode::render()
{
}

void LearningMode::registerTrafficLight(const QString& id, const QRectF& position, int cycleDurationMs)
{
    Q_UNUSED(position)
    TrafficLightState state;
    state.id = id;
    state.isRed = true;
    state.lastChangeTime = QDateTime::currentMSecsSinceEpoch();
    state.cycleDurationMs = cycleDurationMs;
    m_trafficLights[id] = state;
}

void LearningMode::unregisterTrafficLight(const QString& id)
{
    m_trafficLights.remove(id);
}

void LearningMode::setTrafficLightState(const QString& id, bool isRed)
{
    if (m_trafficLights.contains(id)) {
        m_trafficLights[id].isRed = isRed;
        m_trafficLights[id].lastChangeTime = QDateTime::currentMSecsSinceEpoch();
        emit trafficLightChanged(id, isRed);
    }
}

bool LearningMode::getTrafficLightState(const QString& id) const
{
    return m_trafficLights.value(id).isRed;
}

QList<TrafficLightState> LearningMode::getAllTrafficLights() const
{
    return m_trafficLights.values();
}

void LearningMode::registerSpeedLimitZone(const QString& id, const QRectF& bounds, qreal speedLimit)
{
    SpeedLimitZone zone;
    zone.id = id;
    zone.bounds = bounds;
    zone.speedLimit = speedLimit;
    zone.isActive = false;
    m_speedLimitZones[id] = zone;
}

void LearningMode::unregisterSpeedLimitZone(const QString& id)
{
    m_speedLimitZones.remove(id);
}

void LearningMode::enterSpeedLimitZone(const QString& id)
{
    if (m_speedLimitZones.contains(id)) {
        SpeedLimitZone& zone = m_speedLimitZones[id];
        zone.isActive = true;
        m_currentSpeedLimit = zone.speedLimit;
        m_currentSpeedLimitZoneId = id;
        emit speedLimitChanged(zone.speedLimit);
    }
}

void LearningMode::exitSpeedLimitZone(const QString& id)
{
    if (m_speedLimitZones.contains(id)) {
        SpeedLimitZone& zone = m_speedLimitZones[id];
        zone.isActive = false;

        if (m_currentSpeedLimitZoneId == id) {
            m_currentSpeedLimit = 0.0;
            m_currentSpeedLimitZoneId.clear();
            emit speedLimitChanged(0.0);
        }
    }
}

void LearningMode::registerPedestrianCrossing(const QString& id, const QRectF& bounds)
{
    PedestrianCrossing crossing;
    crossing.id = id;
    crossing.bounds = bounds;
    crossing.isActive = false;
    crossing.pedestrianCount = 0;
    m_pedestrianCrossings[id] = crossing;
}

void LearningMode::triggerPedestrianCollision(const QString& crossingId)
{
    if (m_pedestrianCrossings.contains(crossingId)) {
        m_pedestrianViolations++;
        recordViolation(ViolationType::PedestrianCollision,
                       QString("Pedestrian collision at %1").arg(crossingId),
                       QVector2D());
        emit pedestrianCollisionDetected(crossingId);
    }
}

int LearningMode::getTotalPenaltyPoints() const
{
    return m_redLightViolations * 10 + m_speedViolations * 5 + m_pedestrianViolations * 15;
}

int LearningMode::getViolationCount(ViolationType type) const
{
    int count = 0;
    for (const auto& v : m_violations) {
        if (v.type == type) {
            count++;
        }
    }
    return count;
}

void LearningMode::addAICoachFeedback(const QString& feedback, int scoreImpact, const QString& category)
{
    AICoachFeedback fb;
    fb.timestamp = QDateTime::currentMSecsSinceEpoch();
    fb.feedback = feedback;
    fb.scoreImpact = scoreImpact;
    fb.category = category;
    m_coachFeedback.append(fb);

    if (scoreImpact > 0) {
        addScore(scoreImpact);
    } else if (scoreImpact < 0) {
        deductScore(-scoreImpact);
    }

    emit aiCoachFeedbackGenerated(feedback, scoreImpact);
}

void LearningMode::setInitialScore(qreal score)
{
    m_initialScore = score;
    m_totalScore = score;
}

void LearningMode::addScore(qreal points)
{
    m_totalScore += points;
    emit scoreChanged(m_totalScore);
}

void LearningMode::deductScore(qreal points)
{
    m_totalScore = qMax(0.0, m_totalScore - points);
    emit scoreChanged(m_totalScore);
}

bool LearningMode::isRedLightViolation(const QString& trafficLightId, const QVector2D& vehiclePosition)
{
    Q_UNUSED(vehiclePosition)

    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    if (currentTime - m_lastViolationTime < m_minViolationIntervalMs) {
        return false;
    }

    if (m_trafficLights.contains(trafficLightId)) {
        const TrafficLightState& light = m_trafficLights[trafficLightId];
        if (light.isRed) {
            recordViolation(ViolationType::RedLight,
                           QString("Ran red light at %1").arg(trafficLightId),
                           vehiclePosition);
            m_redLightViolations++;
            deductScore(10.0);
            emit redLightViolation(trafficLightId);
            m_lastViolationTime = currentTime;
            return true;
        }
    }
    return false;
}

bool LearningMode::isSpeedViolation(qreal currentSpeed)
{
    if (m_currentSpeedLimit > 0.0 && currentSpeed > m_currentSpeedLimit) {
        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        if (currentTime - m_lastViolationTime >= m_minViolationIntervalMs) {
            recordViolation(ViolationType::SpeedOverLimit,
                           QString("Speed %1 exceeded limit %2").arg(currentSpeed).arg(m_currentSpeedLimit),
                           QVector2D());
            m_speedViolations++;
            deductScore(5.0);
            emit speedLimitExceeded(currentSpeed, m_currentSpeedLimit);
            m_lastViolationTime = currentTime;
            return true;
        }
    }
    return false;
}

bool LearningMode::isPedestrianViolation(const QString& crossingId)
{
    if (m_pedestrianCrossings.contains(crossingId)) {
        triggerPedestrianCollision(crossingId);
        return true;
    }
    return false;
}

void LearningMode::recordViolation(ViolationType type, const QString& description, const QVector2D& position)
{
    ViolationEvent event;
    event.timestamp = QDateTime::currentMSecsSinceEpoch();
    event.type = type;
    event.description = description;
    event.position = position;
    event.speedAtViolation = 0.0;
    event.speedLimit = m_currentSpeedLimit;
    event.penaltyPoints = (type == ViolationType::PedestrianCollision) ? 15 :
                          (type == ViolationType::RedLight) ? 10 : 5;

    m_violations.append(event);
    emit violationRecorded(event);
}

QJsonObject LearningMode::generateLearningReport() const
{
    QJsonObject report;

    report["sessionStartTime"] = QString::number(m_sessionStartTime);
    report["sessionEndTime"] = QString::number(QDateTime::currentMSecsSinceEpoch());
    report["totalDuration"] = QString::number(QDateTime::currentMSecsSinceEpoch() - m_sessionStartTime);
    report["initialScore"] = m_initialScore;
    report["finalScore"] = m_totalScore;

    QJsonObject violationsObj;
    violationsObj["total"] = m_violations.size();
    violationsObj["redLight"] = m_redLightViolations;
    violationsObj["speedLimit"] = m_speedViolations;
    violationsObj["pedestrian"] = m_pedestrianViolations;
    violationsObj["totalPenaltyPoints"] = getTotalPenaltyPoints();
    report["violations"] = violationsObj;

    QJsonArray violationsArray;
    for (const auto& v : m_violations) {
        QJsonObject vObj;
        vObj["timestamp"] = QString::number(v.timestamp);
        vObj["type"] = static_cast<int>(v.type);
        vObj["description"] = v.description;
        vObj["positionX"] = v.position.x();
        vObj["positionY"] = v.position.y();
        vObj["penaltyPoints"] = v.penaltyPoints;
        violationsArray.append(vObj);
    }
    report["violationDetails"] = violationsArray;

    QJsonArray feedbackArray;
    for (const auto& fb : m_coachFeedback) {
        QJsonObject fbObj;
        fbObj["timestamp"] = QString::number(fb.timestamp);
        fbObj["feedback"] = fb.feedback;
        fbObj["scoreImpact"] = fb.scoreImpact;
        fbObj["category"] = fb.category;
        feedbackArray.append(fbObj);
    }
    report["coachFeedback"] = feedbackArray;

    return report;
}

void LearningMode::onTrafficLightChanged(bool isRed)
{
    Q_UNUSED(isRed)
}

void LearningMode::onSpeedLimitChanged(qreal newLimit)
{
    m_currentSpeedLimit = newLimit;
}

void LearningMode::onVehicleCollision(const QString& objectId)
{
    recordViolation(ViolationType::Collision,
                   QString("Collision with %1").arg(objectId),
                   QVector2D());
    deductScore(5.0);
}

void LearningMode::checkTrafficLightViolations(qint64 elapsedMs)
{
    Q_UNUSED(elapsedMs)
}

void LearningMode::updateSpeedLimit(qint64 elapsedMs)
{
    Q_UNUSED(elapsedMs)
}

void LearningMode::processPedestrianZones()
{
}

}

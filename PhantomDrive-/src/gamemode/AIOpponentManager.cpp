#include "AIOpponentManager.h"
#include "SimpleAIOpponent.h"

#include <QDebug>

namespace PhantomDrive {

AIOpponentManager::AIOpponentManager(QObject* parent)
    : QObject(parent)
{
}

AIOpponentManager::~AIOpponentManager()
{
    destroyAllOpponents();
}

AIOpponent* AIOpponentManager::createOpponent(const QString& id, AIStyle style)
{
    if (m_opponents.contains(id)) {
        qWarning() << "AIOpponentManager: Opponent with id" << id << "already exists";
        return nullptr;
    }

    SimpleAIOpponent* opponent = new SimpleAIOpponent(id, this);
    m_opponents[id] = opponent;
    m_opponentOrder.append(id);

    AIConfig config;
    config.style = style;
    opponent->setConfig(config);

    opponent->setProperty("id", id);
    connect(opponent, &SimpleAIOpponent::raceFinished,
            this, [this](int position, qreal totalTime) {
                QString id = sender()->property("id").toString();
                emit opponentFinished(id, position);
            });

    emit opponentAdded(id);
    return opponent;
}

void AIOpponentManager::destroyOpponent(const QString& id)
{
    AIOpponent* opponent = m_opponents.take(id);
    if (opponent) {
        m_opponentOrder.removeAll(id);
        delete opponent;
        emit opponentRemoved(id);
    }
}

void AIOpponentManager::destroyAllOpponents()
{
    qDeleteAll(m_opponents);
    m_opponents.clear();
    m_opponentOrder.clear();
}

AIOpponent* AIOpponentManager::getOpponent(const QString& id) const
{
    return m_opponents.value(id, nullptr);
}

QList<AIOpponent*> AIOpponentManager::getAllOpponents() const
{
    return m_opponents.values();
}

int AIOpponentManager::getOpponentCount() const
{
    return m_opponents.size();
}

bool AIOpponentManager::hasOpponent(const QString& id) const
{
    return m_opponents.contains(id);
}

QList<AIOpponent*> AIOpponentManager::getOpponentsByStyle(AIStyle style) const
{
    QList<AIOpponent*> result;
    for (AIOpponent* opponent : m_opponents) {
        if (opponent->getStyle() == style) {
            result.append(opponent);
        }
    }
    return result;
}

AIOpponent* AIOpponentManager::getNearestOpponent(const QVector2D& position) const
{
    AIOpponent* nearest = nullptr;
    qreal minDist = std::numeric_limits<qreal>::max();

    for (AIOpponent* opponent : m_opponents) {
        qreal dist = (opponent->getPosition() - position).length();
        if (dist < minDist) {
            minDist = dist;
            nearest = opponent;
        }
    }
    return nearest;
}

QList<AIOpponent*> AIOpponentManager::getOpponentsInRadius(const QVector2D& center, qreal radius) const
{
    QList<AIOpponent*> result;
    qreal radiusSquared = radius * radius;

    for (AIOpponent* opponent : m_opponents) {
        QVector2D diff = opponent->getPosition() - center;
        if (diff.lengthSquared() <= radiusSquared) {
            result.append(opponent);
        }
    }
    return result;
}

int AIOpponentManager::getOpponentPosition(const QString& opponentId) const
{
    return m_opponentOrder.indexOf(opponentId) + 1;
}

void AIOpponentManager::setWaypointsForAll(const QList<Waypoint>& waypoints)
{
    for (AIOpponent* opponent : m_opponents) {
        opponent->setWaypoints(waypoints);
    }
}

void AIOpponentManager::setTrackBounds(const QRectF& bounds)
{
    m_trackBounds = bounds;
}

void AIOpponentManager::update(qint64 elapsedMs)
{
    for (AIOpponent* opponent : m_opponents) {
        if (opponent->isActive() && !opponent->hasFinished()) {
            opponent->update(elapsedMs);
        }
    }
}

void AIOpponentManager::onPlayerCollision(const QString& opponentId, const QVector2D& point)
{
    AIOpponent* opponent = getOpponent(opponentId);
    if (opponent) {
        opponent->onCollision("player", point);
    }
}

void AIOpponentManager::notifyPowerupCollected(const QString& opponentId, int powerupType)
{
    AIOpponent* opponent = getOpponent(opponentId);
    if (opponent) {
        opponent->addPowerup(powerupType);
    }
}

void AIOpponentManager::notifyPowerupUsed(const QString& opponentId, int slot)
{
    AIOpponent* opponent = getOpponent(opponentId);
    if (opponent) {
        opponent->usePowerup(slot);
    }
}

QJsonObject AIOpponentManager::toJson() const
{
    QJsonObject json;
    QJsonArray opponentsArray;

    for (AIOpponent* opponent : m_opponents) {
        opponentsArray.append(opponent->toJson());
    }
    json["opponents"] = opponentsArray;
    json["count"] = m_opponents.size();

    return json;
}

void AIOpponentManager::fromJson(const QJsonObject& json)
{
    destroyAllOpponents();

    QJsonArray opponentsArray = json["opponents"].toArray();
    for (const QJsonValue& value : opponentsArray) {
        QJsonObject opponentJson = value.toObject();
        QString id = opponentJson["id"].toString();
        AIStyle style = static_cast<AIStyle>(opponentJson["style"].toInt(1));

        AIOpponent* opponent = createOpponent(id, style);
        if (opponent) {
            opponent->fromJson(opponentJson);
        }
    }
}
void AIOpponentManager::onQLearningFeedbackReady(const QLearningFeedback& feedback)
{
    qDebug() << "=== QLearning Feedback Received ===";
    qDebug() << "Reward:" << feedback.reward;
    qDebug() << "Normalized Score:" << feedback.normalizedScore;
    qDebug() << "Safety Risk:" << feedback.safetyRisk;
    qDebug() << "Rule Compliance:" << feedback.ruleCompliance;
    qDebug() << "Recommended Action:" << feedback.recommendedActionHint;

    for (AIOpponent* opponent : m_opponents) {

        AIConfig config = opponent->getConfig();

        if (feedback.ruleCompliance > 0.8) {
            config.maxSpeed += 5.0;
            config.aggressionLevel += 0.05;
        }

        if (feedback.safetyRisk > 0.5) {
            config.maxSpeed -= 5.0;
            config.aggressionLevel -= 0.05;
        }

        config.maxSpeed = qBound(80.0, config.maxSpeed, 300.0);
        config.aggressionLevel = qBound(0.1, config.aggressionLevel, 1.0);

        opponent->setConfig(config);

        qDebug() << opponent->getId()
                 << "updated maxSpeed:"
                 << config.maxSpeed
                 << "aggression:"
                 << config.aggressionLevel;
    }
}
} // namespace PhantomDrive

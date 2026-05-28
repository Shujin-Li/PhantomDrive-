#include "AIOpponentManager.h"
#include "SimpleAIOpponent.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>
#include <QtMath>
#include <QtGlobal>
#include <algorithm>
#include <cmath>
#include <limits>

namespace PhantomDrive {

namespace {

constexpr qreal MinAiSpacing = 54.0;
constexpr qreal BoundaryInset = 12.0;
constexpr int PlayerCheckpointWeight = 100;

AIConfig defaultConfigForStyle(AIStyle style)
{
    AIConfig config;
    config.style = style;

    switch (style) {
    case AIStyle::Conservative:
        config.name = QStringLiteral("Easy");
        config.maxSpeed = 155.0;
        config.acceleration = 95.0;
        config.handling = 75.0;
        config.aggressionLevel = 0.25;
        config.riskTolerance = 0.25;
        config.skillLevel = 3;
        break;
    case AIStyle::Aggressive:
        config.name = QStringLiteral("Hard");
        config.maxSpeed = 235.0;
        config.acceleration = 175.0;
        config.handling = 120.0;
        config.aggressionLevel = 0.85;
        config.riskTolerance = 0.75;
        config.skillLevel = 8;
        break;
    case AIStyle::Defensive:
        config.name = QStringLiteral("Defensive");
        config.maxSpeed = 175.0;
        config.acceleration = 115.0;
        config.handling = 110.0;
        config.aggressionLevel = 0.35;
        config.riskTolerance = 0.35;
        config.skillLevel = 6;
        break;
    case AIStyle::Normal:
    default:
        config.name = QStringLiteral("Medium");
        config.maxSpeed = 195.0;
        config.acceleration = 135.0;
        config.handling = 100.0;
        config.aggressionLevel = 0.55;
        config.riskTolerance = 0.50;
        config.skillLevel = 5;
        break;
    }

    return config;
}

QString styleToString(AIStyle style)
{
    switch (style) {
    case AIStyle::Conservative: return QStringLiteral("Conservative");
    case AIStyle::Aggressive: return QStringLiteral("Aggressive");
    case AIStyle::Defensive: return QStringLiteral("Defensive");
    case AIStyle::Normal: return QStringLiteral("Normal");
    default: return QStringLiteral("Unknown");
    }
}

} // namespace

AIOpponentManager::AIOpponentManager(QObject* parent)
    : QObject(parent)
    , m_playerPosition(0.0, 0.0)
    , m_totalLaps(3)
    , m_allOpponentsFinishedEmitted(false)
{
    m_playerRace.lap = 1;
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
    m_allOpponentsFinishedEmitted = false;

    opponent->setConfig(defaultConfigForStyle(style));
    opponent->setProperty("id", id);

    RaceEntry entry;
    entry.lap = opponent->getCurrentLap();
    m_aiRace.insert(id, entry);

    connect(opponent, &SimpleAIOpponent::stateChanged,
            this, [this, id](AIState oldState, AIState newState) {
                emit opponentStateChanged(id, oldState, newState);
            });

    connect(opponent, &SimpleAIOpponent::raceFinished,
            this, [this, id](int position, qreal) {
                emit opponentFinished(id, position);
            });

    emit opponentAdded(id);
    updateRaceRankings();
    return opponent;
}

void AIOpponentManager::destroyOpponent(const QString& id)
{
    AIOpponent* opponent = m_opponents.take(id);
    if (opponent) {
        m_opponentOrder.removeAll(id);
        m_aiRace.remove(id);
        m_finishOrder.removeAll(id);
        m_raceOrder.removeAll(id);
        delete opponent;
        emit opponentRemoved(id);
        updateRaceRankings();
    }
}

void AIOpponentManager::destroyAllOpponents()
{
    qDeleteAll(m_opponents);
    m_opponents.clear();
    m_opponentOrder.clear();
    m_aiRace.clear();
    m_finishOrder.clear();
    m_raceOrder.clear();
    m_allOpponentsFinishedEmitted = false;
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
    const RaceEntry entry = m_aiRace.value(opponentId);
    if (entry.finalPosition > 0) {
        return entry.finalPosition;
    }
    const int index = m_raceOrder.indexOf(opponentId);
    return index >= 0 ? index + 1 : m_opponentOrder.indexOf(opponentId) + 1;
}

void AIOpponentManager::setWaypointsForAll(const QList<Waypoint>& waypoints)
{
    for (AIOpponent* opponent : m_opponents) {
        opponent->setWaypoints(waypoints);
    }
    refreshRaceProgressFromOpponents();
    updateRaceRankings();
}

void AIOpponentManager::setTrackBounds(const QRectF& bounds)
{
    m_trackBounds = bounds;
}

void AIOpponentManager::setPlayerPosition(const QVector2D& position)
{
    m_playerPosition = position;
}

void AIOpponentManager::setRaceTotalLaps(int laps)
{
    m_totalLaps = qMax(1, laps);
}

int AIOpponentManager::getRaceTotalLaps() const
{
    return m_totalLaps;
}

void AIOpponentManager::setPlayerRaceProgress(int lap,
                                              int checkpointIndex,
                                              qreal progressPercent,
                                              bool finished,
                                              qreal totalTime)
{
    m_playerRace.lap = qMax(0, lap);
    m_playerRace.checkpointIndex = qMax(0, checkpointIndex);
    m_playerRace.progressPercent = qBound(0.0, progressPercent, 100.0);
    m_playerRace.finished = finished;
    m_playerRace.totalTime = totalTime;
    if (finished && m_playerRace.finalPosition <= 0) {
        if (!m_finishOrder.contains(QStringLiteral("player"))) {
            m_finishOrder.append(QStringLiteral("player"));
        }
        m_playerRace.finalPosition = m_finishOrder.indexOf(QStringLiteral("player")) + 1;
    } else if (!finished) {
        m_finishOrder.removeAll(QStringLiteral("player"));
        m_playerRace.finalPosition = 0;
    }
    updateRaceRankings();
}

int AIOpponentManager::getPlayerRacePosition() const
{
    if (m_playerRace.finalPosition > 0) {
        return m_playerRace.finalPosition;
    }
    const int index = m_raceOrder.indexOf(QStringLiteral("player"));
    return index >= 0 ? index + 1 : 1;
}

QStringList AIOpponentManager::getRaceOrder() const
{
    return m_raceOrder;
}

QJsonObject AIOpponentManager::raceResultsToJson() const
{
    QJsonObject root;
    QJsonArray order;
    for (const QString& id : m_raceOrder) {
        QJsonObject item;
        const bool isPlayer = (id == QStringLiteral("player"));
        const RaceEntry entry = isPlayer ? m_playerRace : m_aiRace.value(id);
        item["id"] = id;
        item["kind"] = isPlayer ? QStringLiteral("player") : QStringLiteral("ai");
        item["lap"] = entry.lap;
        item["checkpointIndex"] = entry.checkpointIndex;
        item["progressPercent"] = entry.progressPercent;
        item["finished"] = entry.finished;
        item["finalPosition"] = entry.finalPosition;
        item["totalTime"] = entry.totalTime;
        order.append(item);
    }
    root["totalLaps"] = m_totalLaps;
    root["order"] = order;
    return root;
}

void AIOpponentManager::update(qint64 elapsedMs)
{
    QMap<QString, QVector2D> positions;
    for (auto it = m_opponents.constBegin(); it != m_opponents.constEnd(); ++it) {
        positions.insert(it.key(), it.value()->getPosition());
    }

    for (auto it = m_opponents.begin(); it != m_opponents.end(); ++it) {
        AIOpponent* opponent = it.value();
        if (!opponent) {
            continue;
        }

        QMap<QString, QVector2D> otherPositions = positions;
        otherPositions.remove(it.key());
        opponent->setPlayerPosition(m_playerPosition);
        opponent->setOtherOpponentPositions(otherPositions);

        if (opponent->isActive() && !opponent->hasFinished()) {
            opponent->update(elapsedMs);
            applyBoundaryRecovery(opponent);
            RaceEntry entry = m_aiRace.value(it.key());
            entry.totalTime += elapsedMs / 1000.0;
            m_aiRace.insert(it.key(), entry);
        }
    }

    applySimpleAvoidance();
    refreshRaceProgressFromOpponents();
    updateRaceRankings();
}

void AIOpponentManager::applyBoundaryRecovery(AIOpponent* opponent)
{
    if (!opponent || m_trackBounds.isNull() || m_trackBounds.isEmpty()) {
        return;
    }

    const QPointF current = opponent->getPosition().toPointF();
    if (m_trackBounds.contains(current)) {
        return;
    }

    const QRectF safeBounds = m_trackBounds.adjusted(BoundaryInset,
                                                    BoundaryInset,
                                                    -BoundaryInset,
                                                    -BoundaryInset);
    const qreal x = qBound(safeBounds.left(), current.x(), safeBounds.right());
    const qreal y = qBound(safeBounds.top(), current.y(), safeBounds.bottom());
    const QVector2D clamped(x, y);
    const QVector2D center(m_trackBounds.center());
    const QVector2D toCenter = center - clamped;

    opponent->setPosition(clamped);
    if (!toCenter.isNull()) {
        opponent->setRotation(qRadiansToDegrees(std::atan2(toCenter.y(), toCenter.x())));
    }
    opponent->onCollision(QStringLiteral("track_boundary"), clamped);
}

void AIOpponentManager::applySimpleAvoidance()
{
    const QList<QString> ids = m_opponents.keys();
    for (int i = 0; i < ids.size(); ++i) {
        AIOpponent* a = m_opponents.value(ids[i]);
        if (!a || a->hasFinished()) {
            continue;
        }

        for (int j = i + 1; j < ids.size(); ++j) {
            AIOpponent* b = m_opponents.value(ids[j]);
            if (!b || b->hasFinished()) {
                continue;
            }

            QVector2D delta = b->getPosition() - a->getPosition();
            qreal distance = delta.length();
            if (distance >= MinAiSpacing) {
                continue;
            }

            if (distance < 0.001) {
                delta = QVector2D(1.0, 0.0);
                distance = 1.0;
            }

            const QVector2D direction = delta.normalized();
            const qreal overlap = (MinAiSpacing - distance) * 0.5;
            a->setPosition(a->getPosition() - direction * overlap);
            b->setPosition(b->getPosition() + direction * overlap);
            a->onNearMiss(b->getId(), distance);
            b->onNearMiss(a->getId(), distance);
        }
    }
}

void AIOpponentManager::refreshRaceProgressFromOpponents()
{
    for (auto it = m_opponents.constBegin(); it != m_opponents.constEnd(); ++it) {
        AIOpponent* opponent = it.value();
        if (!opponent) {
            continue;
        }

        RaceEntry entry = m_aiRace.value(it.key());
        entry.lap = opponent->getCurrentLap();
        entry.checkpointIndex = opponent->getCheckpointsPassed();
        entry.progressPercent = opponent->getProgressPercentage();

        if (!entry.finished && m_totalLaps > 0 && opponent->getCurrentLap() >= m_totalLaps) {
            entry.finished = true;
            if (entry.totalTime <= 0.0) {
                entry.totalTime = opponent->getLapTime();
            }
            entry.finalPosition = m_finishOrder.size() + 1;
            m_finishOrder.append(it.key());
            opponent->setFinished(true);
            opponent->setState(AIState::Finished);
            emit opponentFinished(it.key(), entry.finalPosition);
        }

        m_aiRace.insert(it.key(), entry);
    }
}

qreal AIOpponentManager::raceSortValue(const RaceEntry& entry) const
{
    const int totalSegments = qMax(PlayerCheckpointWeight, m_totalLaps * PlayerCheckpointWeight);
    if (entry.finished) {
        return totalSegments + (1000.0 - entry.finalPosition);
    }
    return (entry.lap * PlayerCheckpointWeight)
        + entry.checkpointIndex
        + (entry.progressPercent / 100.0);
}

void AIOpponentManager::updateRaceRankings()
{
    QStringList ordered;
    ordered << QStringLiteral("player");
    for (const QString& id : m_opponentOrder) {
        if (m_opponents.contains(id)) {
            ordered << id;
        }
    }

    std::stable_sort(ordered.begin(), ordered.end(), [this](const QString& lhs, const QString& rhs) {
        const RaceEntry left = (lhs == QStringLiteral("player")) ? m_playerRace : m_aiRace.value(lhs);
        const RaceEntry right = (rhs == QStringLiteral("player")) ? m_playerRace : m_aiRace.value(rhs);
        const qreal leftValue = raceSortValue(left);
        const qreal rightValue = raceSortValue(right);
        if (!qFuzzyCompare(leftValue + 1.0, rightValue + 1.0)) {
            return leftValue > rightValue;
        }
        return lhs < rhs;
    });

    if (ordered != m_raceOrder) {
        m_raceOrder = ordered;
        for (int i = 0; i < m_raceOrder.size(); ++i) {
            const QString id = m_raceOrder.at(i);
            AIOpponent* opponent = m_opponents.value(id, nullptr);
            if (!opponent) {
                continue;
            }
            const int oldPosition = opponent->getRacePosition();
            const int newPosition = i + 1;
            opponent->setRacePosition(newPosition);
            if (oldPosition != newPosition) {
                emit opponent->positionChanged(oldPosition, newPosition);
            }
        }
        emit rankingsUpdated(m_raceOrder);
    }

    if (!m_allOpponentsFinishedEmitted && areAllOpponentsFinished()) {
        m_allOpponentsFinishedEmitted = true;
        emit allOpponentsFinished();
    }
}

bool AIOpponentManager::areAllOpponentsFinished() const
{
    if (m_opponents.isEmpty()) {
        return false;
    }
    for (AIOpponent* opponent : m_opponents) {
        if (opponent && !opponent->hasFinished()) {
            return false;
        }
    }
    return true;
}

void AIOpponentManager::onPlayerCollision(const QString& opponentId, const QVector2D& point)
{
    AIOpponent* opponent = getOpponent(opponentId);
    if (opponent) {
        opponent->onCollision(QStringLiteral("player"), point);
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
    json["race"] = raceResultsToJson();

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
    refreshRaceProgressFromOpponents();
    updateRaceRankings();
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

        if (feedback.ruleCompliance > 0.8 && feedback.safetyRisk < 0.4) {
            config.maxSpeed += 5.0;
            config.aggressionLevel += 0.05;
        }

        if (feedback.safetyRisk > 0.5 || feedback.collisionPenalty > 0.0) {
            config.maxSpeed -= 5.0;
            config.aggressionLevel -= 0.05;
            config.riskTolerance -= 0.03;
        }

        config.maxSpeed = qBound(80.0, config.maxSpeed, 300.0);
        config.aggressionLevel = qBound(0.1, config.aggressionLevel, 1.0);
        config.riskTolerance = qBound(0.1, config.riskTolerance, 1.0);

        opponent->setConfig(config);

        qDebug() << opponent->getId()
                 << styleToString(config.style)
                 << "updated maxSpeed:"
                 << config.maxSpeed
                 << "aggression:"
                 << config.aggressionLevel
                 << "risk:"
                 << config.riskTolerance;
    }
}

} // namespace PhantomDrive

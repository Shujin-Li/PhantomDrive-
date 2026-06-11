#include "PowerupWorldRuntime.h"
#include "AIOpponentManager.h"
#include "SimpleAIOpponent.h"
#include "PowerupBox.h"
#include "core/VehiclePhysics.h"
#include "UI/GameViewWidget.h"

#include <QtMath>
#include <QTimer>
#include <limits>

namespace PhantomDrive {

namespace {
constexpr qreal kMissileSpeed = 420.0;
constexpr qreal kMissileHitRadius = 42.0;
constexpr qint64 kMissileLifetimeMs = 4500;
constexpr qreal kMagnetPullRange = 620.0;
constexpr qreal kMagnetPullSpeed = 140.0;
constexpr qreal kOilContactCooldownMs = 900.0;
}

PowerupWorldRuntime::PowerupWorldRuntime(QObject* parent)
    : QObject(parent)
    , m_nextEffectId(1)
{
}

void PowerupWorldRuntime::clear()
{
    m_missiles.clear();
    m_oilPuddles.clear();
    m_oilContactCooldowns.clear();
    m_nextEffectId = 1;
}

QString PowerupWorldRuntime::findBestMissileTarget(const QVector2D& playerPosition,
                                                   qreal playerRotationDeg,
                                                   AIOpponentManager* aiManager) const
{
    if (!aiManager) {
        return QString();
    }

    const qreal radians = qDegreesToRadians(playerRotationDeg);
    const QVector2D forward(qSin(radians), qCos(radians));

    AIOpponent* bestAhead = nullptr;
    qreal bestAheadDist = std::numeric_limits<qreal>::max();
    AIOpponent* bestAny = nullptr;
    qreal bestAnyDist = std::numeric_limits<qreal>::max();

    for (AIOpponent* opponent : aiManager->getAllOpponents()) {
        if (!opponent || opponent->hasFinished()) {
            continue;
        }

        const QVector2D delta = opponent->getPosition() - playerPosition;
        const qreal dist = delta.length();
        if (dist < 40.0 || dist > 900.0) {
            continue;
        }

        const qreal alignment = QVector2D::dotProduct(delta.normalized(), forward);
        if (alignment > 0.25 && dist < bestAheadDist) {
            bestAheadDist = dist;
            bestAhead = opponent;
        }

        if (dist < bestAnyDist) {
            bestAnyDist = dist;
            bestAny = opponent;
        }
    }

    if (bestAhead) {
        return bestAhead->getId();
    }
    if (bestAny) {
        return bestAny->getId();
    }
    return QString();
}

void PowerupWorldRuntime::spawnMissile(const QVector2D& origin, const QString& targetId)
{
    if (targetId.isEmpty()) {
        return;
    }

    ActiveMissile missile;
    missile.id = QStringLiteral("missile_%1").arg(m_nextEffectId++);
    missile.position = origin;
    missile.targetId = targetId;
    missile.remainingMs = kMissileLifetimeMs;
    m_missiles.append(missile);
}

void PowerupWorldRuntime::spawnOilPuddle(const QVector2D& position, qreal radius, qint64 durationMs)
{
    OilPuddle puddle;
    puddle.id = QStringLiteral("oil_%1").arg(m_nextEffectId++);
    puddle.position = position;
    puddle.radius = radius;
    puddle.remainingMs = durationMs;
    m_oilPuddles.append(puddle);
}

AIOpponent* PowerupWorldRuntime::findOpponentById(AIOpponentManager* aiManager, const QString& id) const
{
    if (!aiManager) {
        return nullptr;
    }

    for (AIOpponent* opponent : aiManager->getAllOpponents()) {
        if (opponent && opponent->getId() == id) {
            return opponent;
        }
    }
    return nullptr;
}

void PowerupWorldRuntime::updateMissiles(qint64 deltaMs, AIOpponentManager* aiManager)
{
    for (int i = m_missiles.size() - 1; i >= 0; --i) {
        ActiveMissile& missile = m_missiles[i];
        missile.remainingMs -= deltaMs;
        if (missile.remainingMs <= 0) {
            m_missiles.removeAt(i);
            continue;
        }

        AIOpponent* target = findOpponentById(aiManager, missile.targetId);
        if (!target || target->hasFinished()) {
            m_missiles.removeAt(i);
            continue;
        }

        const QVector2D targetPos = target->getPosition();
        QVector2D toTarget = targetPos - missile.position;
        const qreal dist = toTarget.length();
        if (dist <= kMissileHitRadius) {
            if (auto* simpleAi = qobject_cast<SimpleAIOpponent*>(target)) {
                simpleAi->applyMissileHit();
            } else {
                target->setMaxSpeed(qMax<qreal>(30.0, target->getMaxSpeed() * 0.3));
            }
            m_missiles.removeAt(i);
            continue;
        }

        if (dist > 0.001) {
            const QVector2D step = toTarget / dist * kMissileSpeed * (deltaMs / 1000.0);
            missile.position += step;
        }
    }
}

void PowerupWorldRuntime::updateOilContacts(qint64 deltaMs,
                                            VehiclePhysics* playerPhysics,
                                            AIOpponentManager* aiManager)
{
    for (int i = m_oilPuddles.size() - 1; i >= 0; --i) {
        m_oilPuddles[i].remainingMs -= deltaMs;
        if (m_oilPuddles[i].remainingMs <= 0) {
            m_oilPuddles.removeAt(i);
        }
    }

    for (const OilPuddle& puddle : m_oilPuddles) {
        if (playerPhysics) {
            const QString playerKey = QStringLiteral("player:%1").arg(puddle.id);
            if (!m_oilContactCooldowns.contains(playerKey)) {
                const qreal dist = (playerPhysics->getPosition() - puddle.position).length();
                if (dist <= puddle.radius) {
                    playerPhysics->activateOilSlip(2800, 0.32, 0.5);
                    m_oilContactCooldowns.insert(playerKey);
                    QTimer::singleShot(static_cast<int>(kOilContactCooldownMs), this, [this, playerKey]() {
                        m_oilContactCooldowns.remove(playerKey);
                    });
                }
            }
        }

        if (!aiManager) {
            continue;
        }

        for (AIOpponent* opponent : aiManager->getAllOpponents()) {
            if (!opponent || opponent->hasFinished()) {
                continue;
            }

            const QString aiKey = QStringLiteral("%1:%2").arg(opponent->getId(), puddle.id);
            if (m_oilContactCooldowns.contains(aiKey)) {
                continue;
            }

            const qreal dist = (opponent->getPosition() - puddle.position).length();
            if (dist <= puddle.radius) {
                if (auto* simpleAi = qobject_cast<SimpleAIOpponent*>(opponent)) {
                    simpleAi->applyOilSlip(2800);
                }
                m_oilContactCooldowns.insert(aiKey);
                QTimer::singleShot(static_cast<int>(kOilContactCooldownMs), this, [this, aiKey]() {
                    m_oilContactCooldowns.remove(aiKey);
                });
            }
        }
    }
}

void PowerupWorldRuntime::updateMagnetPull(qint64 deltaMs,
                                           VehiclePhysics* playerPhysics,
                                           QList<PowerupBox*>& boxes,
                                           GameViewWidget* gameView)
{
    if (!playerPhysics || !playerPhysics->isMagnetActive()) {
        return;
    }

    const QVector2D playerPos = playerPhysics->getPosition();
    const qreal step = kMagnetPullSpeed * (deltaMs / 1000.0);

    for (PowerupBox* box : boxes) {
        if (!box || !box->isActive()) {
            continue;
        }

        const QVector2D delta = playerPos - box->position();
        const qreal dist = delta.length();
        if (dist <= 0.001 || dist > kMagnetPullRange) {
            continue;
        }

        const QVector2D newPos = box->position() + delta / dist * qMin(step, dist - 36.0);
        box->setPosition(newPos);

        if (gameView) {
            gameView->updatePowerupBoxPosition(box->objectName(), newPos);
        }
    }
}

void PowerupWorldRuntime::syncVisuals(GameViewWidget* gameView)
{
    if (!gameView) {
        return;
    }

    QList<QVector2D> oilPositions;
    QList<qreal> oilRadii;
    for (const OilPuddle& puddle : m_oilPuddles) {
        oilPositions.append(puddle.position);
        oilRadii.append(puddle.radius);
    }

    QList<QVector2D> missilePositions;
    for (const ActiveMissile& missile : m_missiles) {
        missilePositions.append(missile.position);
    }

    gameView->setWorldEffects(oilPositions, oilRadii, missilePositions);
}

void PowerupWorldRuntime::update(qint64 deltaMs,
                                 VehiclePhysics* playerPhysics,
                                 AIOpponentManager* aiManager,
                                 QList<PowerupBox*>& boxes,
                                 GameViewWidget* gameView)
{
    updateMissiles(deltaMs, aiManager);
    updateOilContacts(deltaMs, playerPhysics, aiManager);
    updateMagnetPull(deltaMs, playerPhysics, boxes, gameView);
    syncVisuals(gameView);
}

}

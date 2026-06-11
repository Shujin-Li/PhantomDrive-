#pragma once

#include "PhantomDrive_global.h"
#include "Powerup.h"

#include <QObject>
#include <QVector2D>
#include <QString>
#include <QList>
#include <QSet>

namespace PhantomDrive {

class AIOpponent;
class AIOpponentManager;
class GameViewWidget;
class PowerupBox;
class SimpleAIOpponent;
class VehiclePhysics;

struct ActiveMissile {
    QString id;
    QVector2D position;
    QString targetId;
    qint64 remainingMs = 0;
};

struct OilPuddle {
    QString id;
    QVector2D position;
    qreal radius = 0.0;
    qint64 remainingMs = 0;
};

class PHANTOMDRIVE_EXPORT PowerupWorldRuntime : public QObject
{
    Q_OBJECT

public:
    explicit PowerupWorldRuntime(QObject* parent = nullptr);

    void clear();

    QString findBestMissileTarget(const QVector2D& playerPosition,
                                  qreal playerRotationDeg,
                                  AIOpponentManager* aiManager) const;

    void spawnMissile(const QVector2D& origin, const QString& targetId);
    void spawnOilPuddle(const QVector2D& position, qreal radius, qint64 durationMs);

    void update(qint64 deltaMs,
                VehiclePhysics* playerPhysics,
                AIOpponentManager* aiManager,
                QList<PowerupBox*>& boxes,
                GameViewWidget* gameView);

    const QList<ActiveMissile>& missiles() const { return m_missiles; }
    const QList<OilPuddle>& oilPuddles() const { return m_oilPuddles; }

private:
    void updateMissiles(qint64 deltaMs, AIOpponentManager* aiManager);
    void updateOilContacts(qint64 deltaMs,
                           VehiclePhysics* playerPhysics,
                           AIOpponentManager* aiManager);
    void updateMagnetPull(qint64 deltaMs,
                          VehiclePhysics* playerPhysics,
                          QList<PowerupBox*>& boxes,
                          GameViewWidget* gameView);
    void syncVisuals(GameViewWidget* gameView);
    AIOpponent* findOpponentById(AIOpponentManager* aiManager, const QString& id) const;

    QList<ActiveMissile> m_missiles;
    QList<OilPuddle> m_oilPuddles;
    QSet<QString> m_oilContactCooldowns;
    int m_nextEffectId;
};

}

#pragma once

#include "PhantomDrive_global.h"
#include "Powerup.h"

#include <QObject>
#include <QVector2D>
#include <QTimer>
#include <QString>

namespace PhantomDrive {

/**
 * @brief 道具箱（拾取点）
 * 
 * 赛道上的道具箱，玩家经过时随机获得道具
 */
class PowerupBox : public QObject
{
    Q_OBJECT

public:
    explicit PowerupBox(const QVector2D& position, float radius = 2.0f, QObject* parent = nullptr);
    ~PowerupBox() override;

    QVector2D position() const { return m_position; }
    float radius() const { return m_radius; }
    
    bool isActive() const { return m_isActive; }
    void setActive(bool active) { m_isActive = active; }
    
    void setRespawnTime(float seconds) { m_respawnTime = seconds; }
    float respawnTime() const { return m_respawnTime; }
    
    void setMaxRarity(PowerupRarity rarity) { m_maxRarity = rarity; }
    PowerupRarity maxRarity() const { return m_maxRarity; }

    void setFixedPowerupType(PowerupType type);
    void clearFixedPowerupType();
    bool hasFixedPowerupType() const { return m_hasFixedPowerupType; }
    PowerupType fixedPowerupType() const { return m_fixedPowerupType; }

    /**
     * @brief 检测碰撞并给予道具
     * @param playerPosition 玩家位置
     * @param playerId 玩家 ID
     * @return 是否成功给予道具
     */
    bool tryCollect(const QVector2D& playerPosition, const QString& playerId, float collectRadius = -1.0f);

    void setPosition(const QVector2D& position) { m_position = position; }
    
    /**
     * @brief 更新道具箱状态
     * @param deltaTime 时间间隔
     */
    void update(float deltaTime);

signals:
    void collected(const QString& playerId, const PowerupType& type);
    void respawned();
    void emptied();

private:
    void startRespawnTimer();
    void onRespawn();
    
    QVector2D m_position;
    float m_radius;
    
    bool m_isActive;
    float m_respawnTime;
    float m_respawnTimer;
    
    PowerupRarity m_maxRarity;
    bool m_hasFixedPowerupType;
    PowerupType m_fixedPowerupType;
    
    QString m_lastCollectorId;
    qint64 m_lastCollectionTime;
};

}

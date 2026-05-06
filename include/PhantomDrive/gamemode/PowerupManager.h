#pragma once

#include "PhantomDrive_global.h"
#include "Powerup.h"
#include "Powerups.h"

#include <QObject>
#include <QList>
#include <QMap>
#include <QVector2D>
#include <memory>

namespace PhantomDrive {

/**
 * @brief 道具管理器
 * 
 * 管理所有玩家的道具库存和使用
 */
class PHANTOMDRIVE_EXPORT PowerupManager : public QObject
{
    Q_OBJECT

public:
    explicit PowerupManager(QObject* parent = nullptr);
    ~PowerupManager() override;

    // 玩家道具管理
    void addPowerup(const QString& playerId, PowerupPtr powerup);
    PowerupPtr getCurrentPowerup(const QString& playerId) const;
    QList<PowerupPtr> getAllPowerups(const QString& playerId) const;
    
    bool usePowerup(const QString& playerId, const QVector2D& position, float rotation, const QVector2D& velocity);
    void updatePowerups(float deltaTime);
    
    // 道具配置
    void setPowerupProbability(PowerupType type, float probability);
    float getPowerupProbability(PowerupType type) const;
    
    // 道具工厂
    static PowerupPtr createPowerup(PowerupType type);
    static PowerupPtr getRandomPowerup(PowerupRarity maxRarity = PowerupRarity::Legendary);
    
    // 统计
    int getTotalPowerupsUsed() const { return m_totalPowerupsUsed; }
    int getPlayerPowerupCount(const QString& playerId) const;

signals:
    void powerupUsed(const QString& playerId, PowerupType type);
    void powerupExpired(const QString& playerId, PowerupType type);
    void powerupReady(const QString& playerId, PowerupType type);

private slots:
    void onPowerupActivated();
    void onPowerupExpired();
    void onPowerupCooldownReady();

private:
    void initializeDefaultProbabilities();
    
    // 玩家道具库存：playerId -> [powerups]
    QMap<QString, QList<PowerupPtr>> m_playerInventories;
    
    // 道具出现概率
    QMap<PowerupType, float> m_powerupProbabilities;
    
    // 统计
    int m_totalPowerupsUsed;
    QMap<QString, int> m_playerPowerupStats;
};

}

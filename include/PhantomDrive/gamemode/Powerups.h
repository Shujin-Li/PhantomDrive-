#pragma once

#include "PhantomDrive_global.h"
#include "Powerup.h"

namespace PhantomDrive {

/**
 * @brief 瞬间加速道具
 * 
 * 短时间内大幅提升速度
 */
class PHANTOMDRIVE_EXPORT BoostPowerup : public Powerup
{
    Q_OBJECT

public:
    explicit BoostPowerup(QObject* parent = nullptr);
    
    const PowerupConfig& config() const override { return m_config; }

protected:
    void onActivate(const QVector2D& position, float rotation, const QVector2D& velocity) override;
    void onUpdate(float deltaTime) override;
    void onExpire() override;

private:
    PowerupConfig m_config;
    QVector2D m_boostDirection;
    float m_initialSpeed;
};

/**
 * @brief 防护护盾道具
 * 
 * 免疫一次碰撞伤害
 */
class PHANTOMDRIVE_EXPORT ShieldPowerup : public Powerup
{
    Q_OBJECT

public:
    explicit ShieldPowerup(QObject* parent = nullptr);
    
    const PowerupConfig& config() const override { return m_config; }

protected:
    void onActivate(const QVector2D& position, float rotation, const QVector2D& velocity) override;
    void onExpire() override;

private:
    PowerupConfig m_config;
};

/**
 * @brief 追踪导弹道具
 * 
 * 发射一枚追踪前方车辆的导弹
 */
class PHANTOMDRIVE_EXPORT MissilePowerup : public Powerup
{
    Q_OBJECT

public:
    explicit MissilePowerup(QObject* parent = nullptr);
    
    const PowerupConfig& config() const override { return m_config; }

protected:
    void onActivate(const QVector2D& position, float rotation, const QVector2D& velocity) override;

private:
    PowerupConfig m_config;
};

/**
 * @brief 油污陷阱道具
 * 
 * 在地面放置油污，使经过的车辆打滑
 */
class PHANTOMDRIVE_EXPORT OilSlickPowerup : public Powerup
{
    Q_OBJECT

public:
    explicit OilSlickPowerup(QObject* parent = nullptr);
    
    const PowerupConfig& config() const override { return m_config; }

protected:
    void onActivate(const QVector2D& position, float rotation, const QVector2D& velocity) override;

private:
    PowerupConfig m_config;
};

/**
 * @brief 电磁脉冲道具
 * 
 * 范围减速效果
 */
class PHANTOMDRIVE_EXPORT EMPPowerup : public Powerup
{
    Q_OBJECT

public:
    explicit EMPPowerup(QObject* parent = nullptr);
    
    const PowerupConfig& config() const override { return m_config; }

protected:
    void onActivate(const QVector2D& position, float rotation, const QVector2D& velocity) override;
    void onUpdate(float deltaTime) override;

private:
    PowerupConfig m_config;
    float m_effectRadius;
};

/**
 * @brief 隐身道具
 * 
 * 短时间内免疫碰撞
 */
class PHANTOMDRIVE_EXPORT InvisibilityPowerup : public Powerup
{
    Q_OBJECT

public:
    explicit InvisibilityPowerup(QObject* parent = nullptr);
    
    const PowerupConfig& config() const override { return m_config; }

protected:
    void onActivate(const QVector2D& position, float rotation, const QVector2D& velocity) override;
    void onExpire() override;

private:
    PowerupConfig m_config;
};

}

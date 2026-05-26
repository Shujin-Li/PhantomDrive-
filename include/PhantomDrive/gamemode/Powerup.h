#pragma once

#include "PhantomDrive_global.h"

#include <QObject>
#include <QVector2D>
#include <QString>
#include <memory>

namespace PhantomDrive {

/**
 * @brief 道具类型枚举
 */
enum class PowerupType {
    Boost,
    Shield,
    Missile,
    OilSlick,
    EMP,
    Invisibility,
    Repair,
    Teleport,
    Magnet,
    Custom
};

/**
 * @brief 道具稀有度枚举
 */
enum class PowerupRarity {
    Common,
    Rare,
    Legendary
};

/**
 * @brief 道具目标类型枚举
 */
enum class PowerupTarget {
    Self,
    Other,
    Area,
    Projectile,
    Trap
};

/**
 * @brief 道具配置结构
 */
struct PowerupConfig {
    PowerupType type;
    QString name;
    QString description;
    PowerupRarity rarity;
    PowerupTarget target;
    float duration;
    float cooldown;
    float activationDelay;
    float strength;
    float speedBoost;
    float handlingBoost;
    float damage;
    float radius;
    float slowFactor;
    int quantity;
};

class PDPhysicsObject;

/**
 * @brief 道具基类
 * 
 * 所有技能道具的抽象基类
 */
class Powerup : public QObject
{
    Q_OBJECT

public:
    explicit Powerup(PowerupType type, QObject* parent = nullptr);
    ~Powerup() override;

    PowerupType type() const { return m_type; }
    QString name() const { return m_name; }
    QString description() const { return m_description; }
    
    PowerupRarity rarity() const { return m_rarity; }
    PowerupTarget target() const { return m_target; }
    
    float duration() const { return m_duration; }
    float cooldown() const { return m_cooldown; }
    float activationDelay() const { return m_activationDelay; }
    
    bool isActive() const { return m_isActive; }
    bool isUsable() const { return !m_isActive && m_cooldownTimer <= 0; }
    
    void setOwnerId(const QString& id) { m_ownerId = id; }
    QString ownerId() const { return m_ownerId; }
    
    /**
     * @brief 使用道具
     * @param position 使用者位置
     * @param rotation 使用者朝向
     * @param velocity 使用者速度
     */
    virtual void use(const QVector2D& position, float rotation, const QVector2D& velocity);
    
    /**
     * @brief 更新道具状态
     * @param deltaTime 时间间隔（秒）
     */
    virtual void update(float deltaTime);
    
    /**
     * @brief 取消道具效果
     */
    virtual void cancel();
    
    /**
     * @brief 获取道具配置
     */
    virtual const PowerupConfig& config() const = 0;

signals:
    void activated();
    void expired();
    void cooldownReady();
    void effectApplied(const QString& targetId);

protected:
    virtual void onActivate(const QVector2D& position, float rotation, const QVector2D& velocity) = 0;
    virtual void onUpdate(float deltaTime) {}
    virtual void onExpire() {}
    
    void startTimer();
    void stopTimer();
    
    PowerupType m_type;
    QString m_name;
    QString m_description;
    PowerupRarity m_rarity;
    PowerupTarget m_target;
    
    float m_duration;
    float m_cooldown;
    float m_activationDelay;
    
    bool m_isActive;
    float m_activeTimer;
    float m_cooldownTimer;
    
    QString m_ownerId;
};

using PowerupPtr = std::shared_ptr<Powerup>;

}

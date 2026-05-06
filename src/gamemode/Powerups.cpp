#include "Powerups.h"

#include <QtMath>

namespace PhantomDrive {

// ============ BoostPowerup ============
BoostPowerup::BoostPowerup(QObject* parent)
    : Powerup(PowerupType::Boost, parent)
{
    m_name = "瞬间加速";
    m_description = "短时间内大幅提升速度";
    m_rarity = PowerupRarity::Common;
    m_target = PowerupTarget::Self;
    m_duration = 3.0f;
    m_cooldown = 10.0f;
    m_activationDelay = 0.0f;

    m_config.type = PowerupType::Boost;
    m_config.name = m_name;
    m_config.description = m_description;
    m_config.rarity = m_rarity;
    m_config.target = m_target;
    m_config.duration = m_duration;
    m_config.cooldown = m_cooldown;
    m_config.speedBoost = 50.0f;
}

void BoostPowerup::onActivate(const QVector2D& position, float rotation, const QVector2D& velocity)
{
    m_boostDirection = velocity.normalized();
    m_initialSpeed = velocity.length();
    
    // 应用速度加成
    emit effectApplied(m_ownerId);
}

void BoostPowerup::onUpdate(float deltaTime)
{
    // 持续加速效果
    emit effectApplied(m_ownerId);
}

void BoostPowerup::onExpire()
{
    // 效果结束
}

// ============ ShieldPowerup ============
ShieldPowerup::ShieldPowerup(QObject* parent)
    : Powerup(PowerupType::Shield, parent)
{
    m_name = "防护护盾";
    m_description = "免疫一次碰撞伤害";
    m_rarity = PowerupRarity::Rare;
    m_target = PowerupTarget::Self;
    m_duration = 10.0f;
    m_cooldown = 15.0f;

    m_config.type = PowerupType::Shield;
    m_config.name = m_name;
    m_config.description = m_description;
    m_config.rarity = m_rarity;
    m_config.target = m_target;
    m_config.duration = m_duration;
    m_config.cooldown = m_cooldown;
}

void ShieldPowerup::onActivate(const QVector2D& position, float rotation, const QVector2D& velocity)
{
    emit effectApplied(m_ownerId);
}

void ShieldPowerup::onExpire()
{
}

// ============ MissilePowerup ============
MissilePowerup::MissilePowerup(QObject* parent)
    : Powerup(PowerupType::Missile, parent)
{
    m_name = "追踪导弹";
    m_description = "发射一枚追踪前方车辆的导弹";
    m_rarity = PowerupRarity::Rare;
    m_target = PowerupTarget::Projectile;
    m_duration = 0.0f;  // 瞬时
    m_cooldown = 8.0f;

    m_config.type = PowerupType::Missile;
    m_config.name = m_name;
    m_config.description = m_description;
    m_config.rarity = m_rarity;
    m_config.target = m_target;
    m_config.cooldown = m_cooldown;
    m_config.damage = 10.0f;
}

void MissilePowerup::onActivate(const QVector2D& position, float rotation, const QVector2D& velocity)
{
    // 发射导弹
    emit effectApplied(m_ownerId);
}

// ============ OilSlickPowerup ============
OilSlickPowerup::OilSlickPowerup(QObject* parent)
    : Powerup(PowerupType::OilSlick, parent)
{
    m_name = "油污陷阱";
    m_description = "在地面放置油污，使经过的车辆打滑";
    m_rarity = PowerupRarity::Common;
    m_target = PowerupTarget::Trap;
    m_duration = 0.0f;
    m_cooldown = 5.0f;

    m_config.type = PowerupType::OilSlick;
    m_config.name = m_name;
    m_config.description = m_description;
    m_config.rarity = m_rarity;
    m_config.target = m_target;
    m_config.cooldown = m_cooldown;
    m_config.radius = 2.0f;
    m_config.slowFactor = 0.3f;
}

void OilSlickPowerup::onActivate(const QVector2D& position, float rotation, const QVector2D& velocity)
{
    // 放置油污
    emit effectApplied(m_ownerId);
}

// ============ EMPPowerup ============
EMPPowerup::EMPPowerup(QObject* parent)
    : Powerup(PowerupType::EMP, parent)
{
    m_name = "电磁脉冲";
    m_description = "范围减速效果";
    m_rarity = PowerupRarity::Legendary;
    m_target = PowerupTarget::Area;
    m_duration = 5.0f;
    m_cooldown = 20.0f;
    m_effectRadius = 15.0f;

    m_config.type = PowerupType::EMP;
    m_config.name = m_name;
    m_config.description = m_description;
    m_config.rarity = m_rarity;
    m_config.target = m_target;
    m_config.duration = m_duration;
    m_config.cooldown = m_cooldown;
    m_config.radius = m_effectRadius;
    m_config.slowFactor = 0.5f;
}

void EMPPowerup::onActivate(const QVector2D& position, float rotation, const QVector2D& velocity)
{
    // 释放 EMP 冲击波
    emit effectApplied(m_ownerId);
}

void EMPPowerup::onUpdate(float deltaTime)
{
    // 持续影响范围内目标
}

// ============ InvisibilityPowerup ============
InvisibilityPowerup::InvisibilityPowerup(QObject* parent)
    : Powerup(PowerupType::Invisibility, parent)
{
    m_name = "隐身";
    m_description = "短时间内免疫碰撞";
    m_rarity = PowerupRarity::Legendary;
    m_target = PowerupTarget::Self;
    m_duration = 8.0f;
    m_cooldown = 25.0f;

    m_config.type = PowerupType::Invisibility;
    m_config.name = m_name;
    m_config.description = m_description;
    m_config.rarity = m_rarity;
    m_config.target = m_target;
    m_config.duration = m_duration;
    m_config.cooldown = m_cooldown;
}

void InvisibilityPowerup::onActivate(const QVector2D& position, float rotation, const QVector2D& velocity)
{
    emit effectApplied(m_ownerId);
}

void InvisibilityPowerup::onExpire()
{
}

}

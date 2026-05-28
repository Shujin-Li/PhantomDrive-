#include "PowerupManager.h"

#include <QRandomGenerator>
#include <QtMath>

namespace PhantomDrive {

PowerupManager::PowerupManager(QObject* parent)
    : QObject(parent)
    , m_totalPowerupsUsed(0)
{
    initializeDefaultProbabilities();
}

PowerupManager::~PowerupManager()
{
}

void PowerupManager::addPowerup(const QString& playerId, PowerupPtr powerup)
{
    if (!powerup) {
        return;
    }
    
    powerup->setOwnerId(playerId);
    
    // 连接信号
    connect(powerup.get(), &Powerup::activated,
            this, &PowerupManager::onPowerupActivated);
    connect(powerup.get(), &Powerup::expired,
            this, &PowerupManager::onPowerupExpired);
    connect(powerup.get(), &Powerup::cooldownReady,
            this, &PowerupManager::onPowerupCooldownReady);
    
    m_playerInventories[playerId].append(powerup);
}

PowerupPtr PowerupManager::getCurrentPowerup(const QString& playerId) const
{
    auto it = m_playerInventories.find(playerId);
    if (it != m_playerInventories.end() && !it.value().isEmpty()) {
        return it.value().first();
    }
    return nullptr;
}

QList<PowerupPtr> PowerupManager::getAllPowerups(const QString& playerId) const
{
    return m_playerInventories.value(playerId);
}

bool PowerupManager::usePowerup(const QString& playerId, const QVector2D& position, float rotation, const QVector2D& velocity)
{
    PowerupPtr powerup = getCurrentPowerup(playerId);
    if (!powerup || !powerup->isUsable()) {
        return false;
    }
    
    powerup->use(position, rotation, velocity);
    m_totalPowerupsUsed++;
    m_playerPowerupStats[playerId]++;
    
    emit powerupUsed(playerId, powerup->type());
    
    return true;
}

void PowerupManager::updatePowerups(float deltaTime)
{
    for (auto it = m_playerInventories.begin(); it != m_playerInventories.end(); ++it) {
        for (PowerupPtr& powerup : it.value()) {
            if (powerup) {
                powerup->update(deltaTime);
            }
        }
    }
}

void PowerupManager::setPowerupProbability(PowerupType type, float probability)
{
    m_powerupProbabilities[type] = probability;
}

float PowerupManager::getPowerupProbability(PowerupType type) const
{
    return m_powerupProbabilities.value(type, 0.0f);
}

PowerupPtr PowerupManager::createPowerup(PowerupType type)
{
    switch (type) {
        case PowerupType::Boost:
            return std::make_shared<BoostPowerup>();
        case PowerupType::Shield:
            return std::make_shared<ShieldPowerup>();
        case PowerupType::Missile:
            return std::make_shared<MissilePowerup>();
        case PowerupType::OilSlick:
            return std::make_shared<OilSlickPowerup>();
        case PowerupType::EMP:
            return std::make_shared<EMPPowerup>();
        case PowerupType::Invisibility:
            return std::make_shared<InvisibilityPowerup>();
        default:
            return nullptr;
    }
}

PowerupPtr PowerupManager::getRandomPowerup(PowerupRarity maxRarity)
{
    // 使用默认概率（静态函数无法访问非静态成员）
    QMap<PowerupType, float> defaultProbabilities;
    defaultProbabilities[PowerupType::Boost] = 30.0f;
    defaultProbabilities[PowerupType::Shield] = 20.0f;
    defaultProbabilities[PowerupType::Missile] = 20.0f;
    defaultProbabilities[PowerupType::OilSlick] = 15.0f;
    defaultProbabilities[PowerupType::EMP] = 10.0f;
    defaultProbabilities[PowerupType::Invisibility] = 5.0f;
    
    // 根据概率随机选择道具
    float totalProb = 0.0f;
    
    // 计算总概率（只考虑不超过最大稀有度的道具）
    for (auto it = defaultProbabilities.begin(); it != defaultProbabilities.end(); ++it) {
        PowerupPtr powerup = createPowerup(it.key());
        if (powerup && powerup->rarity() <= maxRarity) {
            totalProb += it.value();
        }
    }
    
    if (totalProb <= 0.0f) {
        // 如果没有配置概率，返回一个默认道具
        return createPowerup(PowerupType::Boost);
    }
    
    float randomValue = QRandomGenerator::global()->generateDouble() * totalProb;
    float cumulative = 0.0f;
    
    for (auto it = defaultProbabilities.begin(); it != defaultProbabilities.end(); ++it) {
        PowerupPtr powerup = createPowerup(it.key());
        if (powerup && powerup->rarity() <= maxRarity) {
            cumulative += it.value();
            if (randomValue <= cumulative) {
                return powerup;
            }
        }
    }
    
    // 默认返回
    return createPowerup(PowerupType::Boost);
}

int PowerupManager::getPlayerPowerupCount(const QString& playerId) const
{
    return m_playerInventories.value(playerId).size();
}

void PowerupManager::initializeDefaultProbabilities()
{
    // 设置默认概率
    m_powerupProbabilities[PowerupType::Boost] = 30.0f;
    m_powerupProbabilities[PowerupType::Shield] = 20.0f;
    m_powerupProbabilities[PowerupType::Missile] = 20.0f;
    m_powerupProbabilities[PowerupType::OilSlick] = 15.0f;
    m_powerupProbabilities[PowerupType::EMP] = 10.0f;
    m_powerupProbabilities[PowerupType::Invisibility] = 5.0f;
}

void PowerupManager::onPowerupActivated()
{
    Powerup* powerup = qobject_cast<Powerup*>(sender());
    if (powerup) {
        emit powerupUsed(powerup->ownerId(), powerup->type());
    }
}

void PowerupManager::onPowerupExpired()
{
    Powerup* powerup = qobject_cast<Powerup*>(sender());
    if (powerup) {
        emit powerupExpired(powerup->ownerId(), powerup->type());
    }
}

void PowerupManager::onPowerupCooldownReady()
{
    Powerup* powerup = qobject_cast<Powerup*>(sender());
    if (powerup) {
        emit powerupReady(powerup->ownerId(), powerup->type());
    }
}

}

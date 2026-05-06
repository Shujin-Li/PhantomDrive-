#include "PowerupBox.h"
#include "PowerupManager.h"

#include <QRandomGenerator>
#include <QDateTime>

namespace PhantomDrive {

PowerupBox::PowerupBox(const QVector2D& position, float radius, QObject* parent)
    : QObject(parent)
    , m_position(position)
    , m_radius(radius)
    , m_isActive(true)
    , m_respawnTime(5.0f)
    , m_respawnTimer(0.0f)
    , m_maxRarity(PowerupRarity::Legendary)
    , m_lastCollectionTime(0)
{
}

PowerupBox::~PowerupBox()
{
}

bool PowerupBox::tryCollect(const QVector2D& playerPosition, const QString& playerId)
{
    if (!m_isActive) {
        return false;
    }
    
    // 检测距离
    float distance = QVector2D(playerPosition - m_position).length();
    if (distance > m_radius) {
        return false;
    }
    
    // 防止同一玩家快速重复拾取
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    if (playerId == m_lastCollectorId && (currentTime - m_lastCollectionTime) < 1000) {
        return false;
    }
    
    // 生成随机道具
    PowerupPtr powerup = PowerupManager::getRandomPowerup(m_maxRarity);
    if (!powerup) {
        return false;
    }
    
    // 更新状态
    m_lastCollectorId = playerId;
    m_lastCollectionTime = currentTime;
    m_isActive = false;
    
    emit collected(playerId, powerup->type());
    emit emptied();
    
    // 开始重生计时
    startRespawnTimer();
    
    return true;
}

void PowerupBox::update(float deltaTime)
{
    if (!m_isActive && m_respawnTimer > 0.0f) {
        m_respawnTimer -= deltaTime;
        if (m_respawnTimer <= 0.0f) {
            onRespawn();
        }
    }
}

void PowerupBox::startRespawnTimer()
{
    m_respawnTimer = m_respawnTime;
}

void PowerupBox::onRespawn()
{
    m_isActive = true;
    emit respawned();
}

}

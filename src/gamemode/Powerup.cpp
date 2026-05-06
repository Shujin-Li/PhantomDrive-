#include "Powerup.h"

namespace PhantomDrive {

Powerup::Powerup(PowerupType type, QObject* parent)
    : QObject(parent)
    , m_type(type)
    , m_rarity(PowerupRarity::Common)
    , m_target(PowerupTarget::Self)
    , m_duration(0.0f)
    , m_cooldown(0.0f)
    , m_activationDelay(0.0f)
    , m_isActive(false)
    , m_activeTimer(0.0f)
    , m_cooldownTimer(0.0f)
{
}

Powerup::~Powerup()
{
}

void Powerup::use(const QVector2D& position, float rotation, const QVector2D& velocity)
{
    if (!isUsable()) {
        return;
    }

    m_isActive = true;
    m_activeTimer = m_duration;
    m_cooldownTimer = m_cooldown;

    emit activated();
    onActivate(position, rotation, velocity);

    if (m_duration <= 0.0f) {
        // 瞬时效果
        m_isActive = false;
        emit expired();
    }
}

void Powerup::update(float deltaTime)
{
    if (m_cooldownTimer > 0.0f) {
        m_cooldownTimer -= deltaTime;
        if (m_cooldownTimer <= 0.0f) {
            emit cooldownReady();
        }
    }

    if (m_isActive) {
        m_activeTimer -= deltaTime;
        
        if (m_activeTimer <= 0.0f) {
            m_isActive = false;
            onExpire();
            emit expired();
        } else {
            onUpdate(deltaTime);
        }
    }
}

void Powerup::cancel()
{
    if (m_isActive) {
        m_isActive = false;
        onExpire();
        emit expired();
    }
}

void Powerup::startTimer()
{
}

void Powerup::stopTimer()
{
}

}

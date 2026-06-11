#include "ArcadeMode.h"

#include <QDateTime>
#include <QRandomGenerator>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector2D>
#include <algorithm>

namespace PhantomDrive {

ArcadeMode::ArcadeMode(QObject* parent)
    : GameMode(parent)
    , m_fastestLapTime(0.0)
    , m_currentLapStartTime(0.0)
    , m_currentLap(0)
    , m_totalLaps(3)
    , m_raceFinished(false)
    , m_lastCheckpointIndex(-1)
{
    PowerUpInfo speedBoost;
    speedBoost.id = "speed_boost";
    speedBoost.name = "Speed Boost";
    speedBoost.description = "Temporarily increases max speed by 50%";
    speedBoost.durationMs = 5000;
    speedBoost.speedBoost = 1.5;
    speedBoost.quantity = 3;
    m_availablePowerUps.append(speedBoost);

    PowerUpInfo handlingBoost;
    handlingBoost.id = "handling_boost";
    handlingBoost.name = "Handling Boost";
    handlingBoost.description = "Improves vehicle handling for better cornering";
    handlingBoost.durationMs = 8000;
    handlingBoost.handlingBoost = 1.3;
    handlingBoost.quantity = 3;
    m_availablePowerUps.append(handlingBoost);
}

ArcadeMode::~ArcadeMode()
{
}

void ArcadeMode::onEnter()
{
    GameMode::onEnter();
    m_currentLap = 0;
    m_lastCheckpointIndex = -1;
    m_raceFinished = false;
    m_currentLapStartTime = 0.0;
}

void ArcadeMode::onExit()
{
    GameMode::onExit();
    m_activePowerUpTimers.clear();
}

void ArcadeMode::update(qint64 elapsedMs)
{
    if (!isActive() || m_raceFinished) {
        return;
    }

    if (m_currentLap == 0 && m_lastCheckpointIndex == -1) {
        m_currentLap = 1;
        m_currentLapStartTime = elapsedMs;
    }

    updatePowerUpTimers(elapsedMs);

    emit modeUpdated(elapsedMs);
}

void ArcadeMode::render()
{
}

void ArcadeMode::setTrackId(const QString& trackId)
{
    m_currentTrackId = trackId;
}

void ArcadeMode::addAIOpponent(const QString& opponentId)
{
    if (!m_activeOpponents.contains(opponentId)) {
        m_activeOpponents.append(opponentId);
    }
}

void ArcadeMode::removeAIOpponent(const QString& opponentId)
{
    m_activeOpponents.removeAll(opponentId);
}

int ArcadeMode::getAIOpponentCount() const
{
    return m_activeOpponents.size();
}

void ArcadeMode::activatePowerUp(const QString& powerUpId, const QString& vehicleId)
{
    auto it = std::find_if(m_availablePowerUps.begin(), m_availablePowerUps.end(),
                           [&powerUpId](const PowerUpInfo& info) {
                               return info.id == powerUpId;
                           });

    if (it != m_availablePowerUps.end() && it->quantity > 0) {
        m_activePowerUpTimers[powerUpId + "_" + vehicleId] = it->durationMs;
        it->quantity--;
        emit powerUpActivated(powerUpId, vehicleId);
        emit speedBoostApplied(vehicleId, it->speedBoost);
    }
}

void ArcadeMode::deactivatePowerUp(const QString& powerUpId, const QString& vehicleId)
{
    QString key = powerUpId + "_" + vehicleId;
    if (m_activePowerUpTimers.contains(key)) {
        m_activePowerUpTimers.remove(key);
        emit powerUpDeactivated(powerUpId, vehicleId);
    }
}

QList<PowerUpInfo> ArcadeMode::getAvailablePowerUps() const
{
    return m_availablePowerUps;
}

void ArcadeMode::recordLapTime(qreal lapTime)
{
    LapRecord record;
    record.lapTime = lapTime;
    record.timestamp = QDateTime::currentMSecsSinceEpoch();
    record.trackId = m_currentTrackId;

    m_lapRecords.append(record);

    if (m_fastestLapTime == 0.0 || lapTime < m_fastestLapTime) {
        m_fastestLapTime = lapTime;
    }

    emit lapCompleted(m_currentLap, lapTime);
}

void ArcadeMode::setTotalLaps(qint32 laps)
{
    m_totalLaps = qMax(1, laps);
}

void ArcadeMode::onCheckpointReached(int checkpointId)
{
    Q_UNUSED(checkpointId)

    if (m_lastCheckpointIndex == m_checkpointsOrder.size() - 1) {
        qreal lapTime = static_cast<qreal>(QDateTime::currentMSecsSinceEpoch()) - m_currentLapStartTime;
        recordLapTime(lapTime);

        if (m_currentLap >= m_totalLaps) {
            m_raceFinished = true;
            emit raceFinished(1, lapTime);
        } else {
            m_currentLap++;
            m_lastCheckpointIndex = -1;
            m_currentLapStartTime = QDateTime::currentMSecsSinceEpoch();
        }
    }
}

void ArcadeMode::onVehicleCollision(const QString& objectId)
{
    Q_UNUSED(objectId)
}

void ArcadeMode::spawnRandomPowerUp()
{
    int randomIndex = QRandomGenerator::global()->bounded(static_cast<int>(m_availablePowerUps.size()));
    const PowerUpInfo& powerUp = m_availablePowerUps[randomIndex];

    qreal x = static_cast<qreal>(QRandomGenerator::global()->bounded(-500, 500));
    qreal y = static_cast<qreal>(QRandomGenerator::global()->bounded(-500, 500));

    emit powerUpSpawned(powerUp.id, QVector2D(x, y));
}

void ArcadeMode::updatePowerUpTimers(qint64 elapsedMs)
{
    QStringList keysToRemove;

    for (auto it = m_activePowerUpTimers.begin(); it != m_activePowerUpTimers.end(); ++it) {
        it.value() -= elapsedMs;
        if (it.value() <= 0) {
            QStringList parts = it.key().split("_");
            if (parts.size() >= 2) {
                QString powerUpId = parts[0];
                QString vehicleId = parts.mid(1).join("_");
                emit powerUpDeactivated(powerUpId, vehicleId);
            }
            keysToRemove.append(it.key());
        }
    }

    for (const QString& key : keysToRemove) {
        m_activePowerUpTimers.remove(key);
    }
}

void ArcadeMode::checkRaceCompletion()
{
}

}

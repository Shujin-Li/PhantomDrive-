#pragma once

#include "GameMode.h"
#include "PhantomDrive_global.h"

#include <QMap>
#include <QList>

namespace PhantomDrive {

struct PowerUpInfo {
    QString id;
    QString name;
    QString description;
    int durationMs;
    qreal speedBoost;
    qreal handlingBoost;
    int quantity;

    PowerUpInfo() : durationMs(0), speedBoost(1.0), handlingBoost(1.0), quantity(0) {}
};

struct LapRecord {
    qreal lapTime;
    qint64 timestamp;
    QString trackId;

    LapRecord() : lapTime(0.0), timestamp(0) {}
};

class ArcadeMode : public GameMode
{
    Q_OBJECT

public:
    explicit ArcadeMode(QObject* parent = nullptr);
    ~ArcadeMode() override;

    ModeType getType() const override { return ModeType::Arcade; }
    QString getName() const override { return QStringLiteral("Arcade Mode"); }
    QString getDescription() const override { return QStringLiteral("High-speed racing with power-ups and AI opponents"); }

    void onEnter() override;
    void onExit() override;

    void update(qint64 elapsedMs) override;
    void render() override;

    void setTrackId(const QString& trackId);
    QString getTrackId() const { return m_currentTrackId; }

    void addAIOpponent(const QString& opponentId);
    void removeAIOpponent(const QString& opponentId);
    int getAIOpponentCount() const;

    void activatePowerUp(const QString& powerUpId, const QString& vehicleId);
    void deactivatePowerUp(const QString& powerUpId, const QString& vehicleId);
    QList<PowerUpInfo> getAvailablePowerUps() const;

    void recordLapTime(qreal lapTime);
    qreal getFastestLapTime() const { return m_fastestLapTime; }
    QList<LapRecord> getLapRecords() const { return m_lapRecords; }

    qint32 getCurrentLap() const { return m_currentLap; }
    void setTotalLaps(qint32 laps);
    qint32 getTotalLaps() const { return m_totalLaps; }

    bool isRaceFinished() const { return m_raceFinished; }

public slots:
    void onCheckpointReached(int checkpointId) override;
    void onVehicleCollision(const QString& objectId) override;

signals:
    void powerUpActivated(const QString& powerUpId, const QString& vehicleId);
    void powerUpDeactivated(const QString& powerUpId, const QString& vehicleId);
    void lapCompleted(qint32 lapNumber, qreal lapTime);
    void raceFinished(int position, qreal totalTime);
    void speedBoostApplied(const QString& vehicleId, qreal boostAmount);
    void powerUpSpawned(const QString& powerUpId, const QVector2D& position);

protected:
    void spawnRandomPowerUp();
    void updatePowerUpTimers(qint64 elapsedMs);
    void checkRaceCompletion();

private:
    QString m_currentTrackId;
    QList<QString> m_activeOpponents;
    QList<QString> m_checkpointsOrder;

    QMap<QString, int> m_activePowerUpTimers;
    QList<PowerUpInfo> m_availablePowerUps;

    QList<LapRecord> m_lapRecords;
    qreal m_fastestLapTime;
    qreal m_currentLapStartTime;
    qint32 m_currentLap;
    qint32 m_totalLaps;

    bool m_raceFinished;
    int m_lastCheckpointIndex;
};

}

#pragma once

#include "PhantomDrive_global.h"
#include "track/TrackManager.h"
#include "gamemode/VehicleSensor.h"
#include "gamemode/CollisionDetector.h"
#include "UI/GameViewWidget.h"
#include "UI/GameRenderState.h"
#include "UI/learninghud.h"
#include "UI/DrivingReportWidget.h"
#include "scoring/ScoreReport.h"

#include <QObject>
#include <QVector2D>
#include <QList>
#include <QTimer>
#include <QMap>

namespace PhantomDrive {

struct VehicleState {
    QString id;
    QVector2D position;
    qreal rotation;
    qreal speed;
    QVector2D velocity;
    qreal acceleration;
    bool isAI;
    QString aiStyle;
    QString aiState;

    VehicleState()
        : rotation(0), speed(0), acceleration(0), isAI(false)
    {}
};

struct EngineStatus {
    bool isRunning;
    QString gameMode;
    int currentLap;
    int totalLaps;
    qreal lapTime;
    qreal bestLapTime;
    int playerCheckpointsPassed;
    int totalCheckpoints;
    bool isInSpeedLimitZone;
    qreal currentSpeedLimit;
    qint64 gameTime;

    EngineStatus()
        : isRunning(false), currentLap(0), totalLaps(3), lapTime(0), bestLapTime(0),
          playerCheckpointsPassed(0), totalCheckpoints(0), isInSpeedLimitZone(false),
          currentSpeedLimit(0), gameTime(0)
    {}
};

class PHANTOMDRIVE_EXPORT GameEngine : public QObject
{
    Q_OBJECT

public:
    explicit GameEngine(QObject* parent = nullptr);
    ~GameEngine() override;

    PHANTOMDRIVE_EXPORT static GameEngine* instance(QObject* parent = nullptr);

    void initialize();
    void startGame(const QString& mode = "arcade", const QString& trackId = "");
    void stopGame();
    void pauseGame();
    void resumeGame();
    bool isRunning() const { return m_engineStatus.isRunning; }
    bool isPaused() const { return m_isPaused; }

    GameViewWidget* getGameView() { return m_gameView; }
    TrackManager* getTrackManager() { return m_trackManager; }
    VehicleSensor* getPlayerSensor() { return m_playerSensor; }
    CollisionDetector* getPlayerCollisionDetector() { return m_playerCollisionDetector; }
    const EngineStatus& getEngineStatus() const { return m_engineStatus; }

    void setPlayerVehicleId(const QString& id);
    QString getPlayerVehicleId() const { return m_playerVehicleId; }

    void registerVehicle(const QString& vehicleId, bool isAI = false);
    void unregisterVehicle(const QString& vehicleId);
    VehicleState getVehicleState(const QString& vehicleId) const;
    QList<QString> getRegisteredVehicles() const { return m_vehicleStates.keys(); }

    void updatePlayerVehicle(const QVector2D& position, qreal rotation, qreal speed, qreal acceleration = 0);
    void updateAIVehicle(const QString& aiId, const QVector2D& position, qreal rotation, qreal speed,
                        const QString& aiStyle = "", const QString& aiState = "");

    QList<QVector2D> getTrackWaypoints() const;
    QList<QVector2D> getTrackCheckpoints() const;
    QVector2D getTrackStartPosition() const;
    qreal getTrackStartRotation() const;
    int getTrackCheckpointCount() const;

    QRectF getTrackBounds() const;
    bool isPositionOnTrack(const QVector2D& position) const;
    qreal getTrackFrictionAt(const QVector2D& position) const;
    TileType getTileTypeAt(const QVector2D& position) const;

    void setGameMode(const QString& mode) { m_engineStatus.gameMode = mode; }
    QString getGameMode() const { return m_engineStatus.gameMode; }
    void setTotalLaps(int laps) { m_engineStatus.totalLaps = laps; }
    int getTotalLaps() const { return m_engineStatus.totalLaps; }

    void resetLap();
    void onPlayerCheckpointReached(int checkpointId);

    void registerHUD(LearningHUD* hud);
    void registerReportWidget(DrivingReportWidget* widget);

signals:
    void engineStarted();
    void engineStopped();
    void enginePaused(bool paused);
    void gameModeChanged(const QString& mode);

    void playerVehicleUpdated(const VehicleState& state);
    void aiVehicleUpdated(const QString& aiId, const VehicleState& state);
    void vehicleRegistered(const QString& vehicleId);
    void vehicleUnregistered(const QString& vehicleId);

    void trackLoaded(const QString& trackId);
    void lapCompleted(int lapNumber, qreal lapTime);
    void checkpointReached(int checkpointId, int index);
    void raceFinished();

    void speedDataReady(qreal speed);
    void lapDataReady(int currentLap, int totalLaps);
    void modeDataReady(const QString& mode);
    void speedLimitZoneEntered(qreal limit, const QString& zoneId);
    void speedLimitZoneExited();

    void collisionOccurred(const QString& vehicleId, const QString& objectId);
    void trackBoundsExceeded(const QString& vehicleId);

    void violationDetected(const QString& type, qreal speed, qreal speedLimit, int penaltyPoints);
    void scoreReady(const ScoreReport& report);
    void gameSessionEnded(qint64 durationMs);

public slots:
    void onAIDecision(const QString& aiId, const QVector2D& targetPosition, qreal targetSpeed);
    void onAIUpdateRequest(const QString& aiId);

private slots:
    void onGameLoopTick();
    void onPlayerSensorDataReady(const DrivingData& data);
    void onPlayerCollision(const QString& objectId, const QVector2D& position, qreal impactForce);

private:
    void setupConnections();
    void updateEngineStatus();
    void updateRenderView();
    void checkLapCompletion();
    void checkSpeedLimitZone(const QVector2D& position);

    static GameEngine* s_instance;

    GameViewWidget* m_gameView;
    TrackManager* m_trackManager;
    VehicleSensor* m_playerSensor;
    CollisionDetector* m_playerCollisionDetector;

    QString m_playerVehicleId;
    QMap<QString, VehicleState> m_vehicleStates;
    EngineStatus m_engineStatus;

    bool m_isPaused;
    bool m_isInitialized;

    QTimer* m_gameLoopTimer;
    int m_gameLoopIntervalMs;

    qint64 m_gameStartTime;
    qint64 m_gamePausedTime;
    qreal m_currentLapStartTime;

    QList<int> m_passedCheckpoints;
    int m_currentCheckpointIndex;

    LearningHUD* m_hud;
    DrivingReportWidget* m_reportWidget;
};

}

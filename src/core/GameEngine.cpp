#include "GameEngine.h"
#include "track/TrackData.h"
#include "gamemode/TrafficObjectManager.h"

#include <QDateTime>
#include <QtMath>

namespace PhantomDrive {

GameEngine* GameEngine::s_instance = nullptr;

GameEngine::GameEngine(QObject* parent)
    : QObject(parent)
    , m_gameView(nullptr)
    , m_trackManager(nullptr)
    , m_playerSensor(nullptr)
    , m_playerCollisionDetector(nullptr)
    , m_isPaused(false)
    , m_isInitialized(false)
    , m_gameLoopTimer(nullptr)
    , m_gameLoopIntervalMs(16)
    , m_gameStartTime(0)
    , m_gamePausedTime(0)
    , m_currentLapStartTime(0)
    , m_currentCheckpointIndex(0)
    , m_hud(nullptr)
    , m_reportWidget(nullptr)
{
}

GameEngine::~GameEngine()
{
    stopGame();
    if (m_gameView) {
        delete m_gameView;
        m_gameView = nullptr;
    }
}

GameEngine* GameEngine::instance(QObject* parent)
{
    if (!s_instance) {
        s_instance = new GameEngine(parent);
    }
    return s_instance;
}

void GameEngine::initialize()
{
    if (m_isInitialized) return;

    m_trackManager = TrackManager::instance(this);

    m_gameView = new GameViewWidget();

    m_playerSensor = new VehicleSensor(this);
    m_playerSensor->setVehicleId("player");
    m_playerSensor->setSamplingInterval(50);

    m_playerCollisionDetector = new CollisionDetector(this);
    m_playerCollisionDetector->setVehicleId("player");
    m_playerCollisionDetector->setDetectionRadius(50.0);
    m_playerCollisionDetector->enableCollisionDetection(true);

    m_gameLoopTimer = new QTimer(this);
    m_gameLoopTimer->setInterval(m_gameLoopIntervalMs);

    setupConnections();
    m_isInitialized = true;
}

void GameEngine::startGame(const QString& mode, const QString& trackId)
{
    if (m_engineStatus.isRunning) return;

    m_engineStatus.gameMode = mode;
    m_engineStatus.isRunning = true;
    m_engineStatus.currentLap = 1;
    m_engineStatus.lapTime = 0;
    m_engineStatus.gameTime = 0;
    m_engineStatus.playerCheckpointsPassed = 0;
    m_isPaused = false;

    m_gameStartTime = QDateTime::currentMSecsSinceEpoch();
    m_currentLapStartTime = 0;
    m_passedCheckpoints.clear();
    m_currentCheckpointIndex = 0;

    if (!trackId.isEmpty() && m_trackManager) {
        m_trackManager->loadTrack(trackId);
        emit trackLoaded(trackId);

        if (m_gameView && m_trackManager->hasCurrentTrack()) {
            m_gameView->setTrackData(m_trackManager->getCurrentTrack());
            m_engineStatus.totalCheckpoints = m_trackManager->getCheckpointCount();
        }
    }

    if (!m_playerVehicleId.isEmpty()) {
        registerVehicle(m_playerVehicleId, false);
    }

    m_playerSensor->startSensing();
    m_gameLoopTimer->start();

    emit engineStarted();
    emit gameModeChanged(mode);
}

void GameEngine::stopGame()
{
    if (!m_engineStatus.isRunning) return;

    m_engineStatus.isRunning = false;
    m_isPaused = false;

    if (m_gameLoopTimer && m_gameLoopTimer->isActive()) {
        m_gameLoopTimer->stop();
    }

    if (m_playerSensor) {
        m_playerSensor->stopSensing();
    }

    m_vehicleStates.clear();
    m_passedCheckpoints.clear();

    emit engineStopped();
}

void GameEngine::pauseGame()
{
    if (!m_engineStatus.isRunning || m_isPaused) return;

    m_isPaused = true;
    m_gamePausedTime = QDateTime::currentMSecsSinceEpoch();

    if (m_gameLoopTimer && m_gameLoopTimer->isActive()) {
        m_gameLoopTimer->stop();
    }

    emit enginePaused(true);
}

void GameEngine::resumeGame()
{
    if (!m_engineStatus.isRunning || !m_isPaused) return;

    m_isPaused = false;
    qint64 pauseDuration = QDateTime::currentMSecsSinceEpoch() - m_gamePausedTime;
    m_gameStartTime += pauseDuration;

    if (m_gameLoopTimer) {
        m_gameLoopTimer->start();
    }

    emit enginePaused(false);
}

void GameEngine::setPlayerVehicleId(const QString& id)
{
    m_playerVehicleId = id;
    if (m_playerSensor) {
        m_playerSensor->setVehicleId(id);
    }
    if (m_playerCollisionDetector) {
        m_playerCollisionDetector->setVehicleId(id);
    }
}

void GameEngine::registerVehicle(const QString& vehicleId, bool isAI)
{
    if (m_vehicleStates.contains(vehicleId)) return;

    VehicleState state;
    state.id = vehicleId;
    state.isAI = isAI;

    if (!isAI && m_trackManager && m_trackManager->hasCurrentTrack()) {
        state.position = m_trackManager->getStartPosition();
        state.rotation = m_trackManager->getStartRotation();
    }

    m_vehicleStates[vehicleId] = state;
    emit vehicleRegistered(vehicleId);
}

void GameEngine::unregisterVehicle(const QString& vehicleId)
{
    if (!m_vehicleStates.contains(vehicleId)) return;

    m_vehicleStates.remove(vehicleId);
    emit vehicleUnregistered(vehicleId);
}

VehicleState GameEngine::getVehicleState(const QString& vehicleId) const
{
    if (m_vehicleStates.contains(vehicleId)) {
        return m_vehicleStates[vehicleId];
    }
    return VehicleState();
}

void GameEngine::updatePlayerVehicle(const QVector2D& position, qreal rotation, qreal speed, qreal acceleration)
{
    if (!m_vehicleStates.contains(m_playerVehicleId)) {
        registerVehicle(m_playerVehicleId, false);
    }

    VehicleState& state = m_vehicleStates[m_playerVehicleId];
    state.position = position;
    state.rotation = rotation;
    state.speed = speed;
    state.acceleration = acceleration;
    state.velocity = QVector2D(qCos(rotation * M_PI / 180.0) * speed,
                               qSin(rotation * M_PI / 180.0) * speed);

    if (m_playerSensor) {
        m_playerSensor->updatePosition(position);
        m_playerSensor->updateRotation(rotation);
        m_playerSensor->updateVelocity(state.velocity);
        m_playerSensor->updateAcceleration(acceleration);
    }

    if (m_playerCollisionDetector) {
        m_playerCollisionDetector->updateVehiclePosition(position);
        m_playerCollisionDetector->updateVehicleRotation(rotation);
        m_playerCollisionDetector->checkCollisions();
    }

    if (m_gameView) {
        m_gameView->updatePlayerCar(position, rotation, speed);
        m_gameView->setCameraPosition(position);
    }

    checkSpeedLimitZone(position);
    updateEngineStatus();

    emit playerVehicleUpdated(state);
}

void GameEngine::updateAIVehicle(const QString& aiId, const QVector2D& position, qreal rotation, qreal speed,
                                const QString& aiStyle, const QString& aiState)
{
    if (!m_vehicleStates.contains(aiId)) {
        registerVehicle(aiId, true);
    }

    VehicleState& state = m_vehicleStates[aiId];
    state.position = position;
    state.rotation = rotation;
    state.speed = speed;
    state.aiStyle = aiStyle;
    state.aiState = aiState;
    state.velocity = QVector2D(qCos(rotation * M_PI / 180.0) * speed,
                               qSin(rotation * M_PI / 180.0) * speed);

    if (m_gameView) {
        m_gameView->updateAICar(aiId, position, rotation, speed);
    }

    emit aiVehicleUpdated(aiId, state);
}

QList<QVector2D> GameEngine::getTrackWaypoints() const
{
    if (m_trackManager) {
        return m_trackManager->getWaypoints();
    }
    return QList<QVector2D>();
}

QList<QVector2D> GameEngine::getTrackCheckpoints() const
{
    QList<QVector2D> checkpoints;
    if (m_trackManager && m_trackManager->hasCurrentTrack()) {
        auto cpList = m_trackManager->getAllCheckpoints();
        for (const auto& cp : cpList) {
            checkpoints.append(cp->getPosition());
        }
    }
    return checkpoints;
}

QVector2D GameEngine::getTrackStartPosition() const
{
    if (m_trackManager && m_trackManager->hasCurrentTrack()) {
        return m_trackManager->getStartPosition();
    }
    return QVector2D(0, 0);
}

qreal GameEngine::getTrackStartRotation() const
{
    if (m_trackManager && m_trackManager->hasCurrentTrack()) {
        return m_trackManager->getStartRotation();
    }
    return 0.0;
}

int GameEngine::getTrackCheckpointCount() const
{
    if (m_trackManager && m_trackManager->hasCurrentTrack()) {
        return m_trackManager->getCheckpointCount();
    }
    return 0;
}

QRectF GameEngine::getTrackBounds() const
{
    if (m_trackManager && m_trackManager->hasCurrentTrack()) {
        return m_trackManager->getCurrentTrack()->getBounds();
    }
    return QRectF();
}

bool GameEngine::isPositionOnTrack(const QVector2D& position) const
{
    if (m_trackManager) {
        return m_trackManager->isDrivableTile(position);
    }
    return false;
}

qreal GameEngine::getTrackFrictionAt(const QVector2D& position) const
{
    if (m_trackManager) {
        return m_trackManager->getFrictionAt(position);
    }
    return 1.0;
}

TileType GameEngine::getTileTypeAt(const QVector2D& position) const
{
    if (m_trackManager) {
        return m_trackManager->getTileTypeAt(position);
    }
    return TileType::Unknown;
}

void GameEngine::resetLap()
{
    m_engineStatus.currentLap = 1;
    m_engineStatus.lapTime = 0;
    m_engineStatus.playerCheckpointsPassed = 0;
    m_passedCheckpoints.clear();
    m_currentCheckpointIndex = 0;
    m_currentLapStartTime = 0;

    if (m_trackManager) {
        m_trackManager->resetProgress();
    }
}

void GameEngine::onPlayerCheckpointReached(int checkpointId)
{
    if (!m_trackManager) return;

    int index = m_passedCheckpoints.size();
    m_passedCheckpoints.append(checkpointId);
    m_engineStatus.playerCheckpointsPassed = m_passedCheckpoints.size();
    m_currentCheckpointIndex = index + 1;

    emit checkpointReached(checkpointId, index);

    checkLapCompletion();
}

void GameEngine::onAIDecision(const QString& aiId, const QVector2D& targetPosition, qreal targetSpeed)
{
    if (!m_vehicleStates.contains(aiId)) return;

    const VehicleState& currentState = m_vehicleStates[aiId];
    QVector2D direction = targetPosition - currentState.position;
    qreal distance = direction.length();

    if (distance < 1.0) return;

    qreal targetRotation = qAtan2(direction.y(), direction.x()) * 180.0 / M_PI;
    qreal newSpeed = qMin(currentState.speed + 0.5, targetSpeed);

    QVector2D newPosition = currentState.position + direction.normalized() * newSpeed * 0.016;

    updateAIVehicle(aiId, newPosition, targetRotation, newSpeed,
                   currentState.aiStyle, currentState.aiState);
}

void GameEngine::onAIUpdateRequest(const QString& aiId)
{
    if (!m_vehicleStates.contains(aiId)) return;

    const VehicleState& state = m_vehicleStates[aiId];
    emit aiVehicleUpdated(aiId, state);
}

void GameEngine::onGameLoopTick()
{
    if (!m_engineStatus.isRunning || m_isPaused) return;

    m_engineStatus.gameTime = QDateTime::currentMSecsSinceEpoch() - m_gameStartTime;

    if (m_currentLapStartTime > 0) {
        m_engineStatus.lapTime = (QDateTime::currentMSecsSinceEpoch() - m_gameStartTime) / 1000.0 - m_currentLapStartTime;
    }

    updateRenderView();
}

void GameEngine::onPlayerSensorDataReady(const DrivingData& data)
{
    if (m_playerSensor) {
        qreal speed = m_playerSensor->getCurrentReading().speed;
        emit speedDataReady(speed);
    }

    emit lapDataReady(m_engineStatus.currentLap, m_engineStatus.totalLaps);
    emit modeDataReady(m_engineStatus.gameMode);
}

void GameEngine::onPlayerCollision(const QString& objectId, const QVector2D& position, qreal impactForce)
{
    Q_UNUSED(position);
    Q_UNUSED(impactForce);

    emit collisionOccurred(m_playerVehicleId, objectId);
}

void GameEngine::setupConnections()
{
    if (m_gameLoopTimer) {
        connect(m_gameLoopTimer, &QTimer::timeout, this, &GameEngine::onGameLoopTick);
    }

    if (m_playerSensor) {
        connect(m_playerSensor, &VehicleSensor::sensorDataReady, this, &GameEngine::onPlayerSensorDataReady);
    }

    if (m_playerCollisionDetector) {
        connect(m_playerCollisionDetector, &CollisionDetector::collisionDetected,
                this, &GameEngine::onPlayerCollision);
    }

    if (m_trackManager) {
        connect(m_trackManager, &TrackManager::checkpointReached,
                this, &GameEngine::onPlayerCheckpointReached);
    }
}

void GameEngine::updateEngineStatus()
{
    if (m_playerSensor) {
        DrivingData data = m_playerSensor->getCurrentReading();
        m_engineStatus.isInSpeedLimitZone = data.isInSpeedLimitZone;
        m_engineStatus.currentSpeedLimit = data.currentSpeedLimit;
    }
}

void GameEngine::updateRenderView()
{
    if (m_gameView) {
        m_gameView->refresh();
    }
}

void GameEngine::checkLapCompletion()
{
    if (!m_trackManager || !m_trackManager->hasCurrentTrack()) return;

    int totalCheckpoints = m_trackManager->getCheckpointCount();
    if (m_passedCheckpoints.size() >= totalCheckpoints) {
        qreal lapTime = m_engineStatus.lapTime;

        if (m_engineStatus.bestLapTime == 0 || lapTime < m_engineStatus.bestLapTime) {
            m_engineStatus.bestLapTime = lapTime;
        }

        emit lapCompleted(m_engineStatus.currentLap, lapTime);

        m_engineStatus.currentLap++;
        m_passedCheckpoints.clear();
        m_currentCheckpointIndex = 0;
        m_currentLapStartTime = m_engineStatus.lapTime;

        if (m_engineStatus.currentLap > m_engineStatus.totalLaps) {
            emit raceFinished();
            stopGame();
        }
    }
}

void GameEngine::checkSpeedLimitZone(const QVector2D& position)
{
    if (!m_trackManager || !m_trackManager->hasCurrentTrack()) return;

    qreal currentLimit = m_trackManager->getFrictionAt(position);
    TileType tileType = m_trackManager->getTileTypeAt(position);

    bool isInZone = (tileType == TileType::Road || tileType == TileType::Asphalt);

    if (isInZone && !m_engineStatus.isInSpeedLimitZone) {
        m_engineStatus.isInSpeedLimitZone = true;
        m_engineStatus.currentSpeedLimit = currentLimit * 100;
        emit speedLimitZoneEntered(m_engineStatus.currentSpeedLimit, "");
    } else if (!isInZone && m_engineStatus.isInSpeedLimitZone) {
        m_engineStatus.isInSpeedLimitZone = false;
        m_engineStatus.currentSpeedLimit = 0;
        emit speedLimitZoneExited();
    }
}

void GameEngine::registerHUD(LearningHUD* hud)
{
    if (!hud) return;
    m_hud = hud;

    connect(this, &GameEngine::speedDataReady, hud, &LearningHUD::updateCurrentSpeed);
    connect(this, &GameEngine::speedLimitZoneEntered, [this, hud](qreal limit, const QString&) {
        hud->updateSpeedLimit(static_cast<int>(limit));
    });
    connect(this, &GameEngine::lapDataReady, hud, &LearningHUD::updateLapInfo);
    connect(this, &GameEngine::modeDataReady, hud, &LearningHUD::updateGameMode);
    connect(this, &GameEngine::gameModeChanged, hud, &LearningHUD::updateGameMode);
    connect(this, &GameEngine::collisionOccurred, [this, hud](const QString&, const QString&) {
        hud->showViolationWarning("Collision");
    });
    connect(this, &GameEngine::violationDetected, [hud](const QString& type, qreal, qreal, int points) {
        hud->showViolationWarning(type);
        hud->showPenaltyMessage(QString("-%1 pts").arg(points), points);
    });

}

void GameEngine::registerReportWidget(DrivingReportWidget* widget)
{
    if (!widget) return;
    m_reportWidget = widget;
    widget->setMockDataEnabled(false);

    connect(this, &GameEngine::speedDataReady, [widget](qreal speed) {
        widget->addSpeedData(speed);
    });
    connect(this, &GameEngine::violationDetected, [widget](const QString& type, qreal speed, qreal, int points) {
        PhantomDrive::ViolationEvent v;
        v.timestamp = QDateTime::currentMSecsSinceEpoch();
        v.type = PhantomDrive::ScoreReport::violationTypeFromString(type);
        v.speedAtViolation = speed;
        v.penaltyPoints = points;
        v.description = type;
        widget->addViolationEvent(v);
    });
    connect(this, &GameEngine::scoreReady, widget, &DrivingReportWidget::setCurrentReport);

}

}

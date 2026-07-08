#include "VehiclePhysics.h"
#include "track/TrackManager.h"
#include "track/TrackData.h"
#include "track/TrackTile.h"
#include "track/Checkpoint.h"

#include <QtMath>
#include <QDebug>
#include <QDateTime>

namespace PhantomDrive {

VehiclePhysics::VehiclePhysics(QObject* parent)
    : QObject(parent)
    , m_trackManager(nullptr)
    , m_controlScheme(ControlScheme::Combined)
    , m_position(0, 0)
    , m_previousPosition(0, 0)
    , m_rotation(0)
    , m_speed(0)
    , m_acceleration(0)
    , m_angularVelocity(0)
    , m_maxSpeed(300.0)
    , m_maxReverseSpeed(-100.0)
    , m_accelerationRate(150.0)
    , m_brakeRate(200.0)
    , m_steeringSpeed(180.0)
    , m_maxSteeringAngle(45.0)
    , m_frictionCoefficient(0.95)
    , m_grassFrictionMultiplier(0.9)
    , m_gravity(0)
    , m_speedBoostMultiplier(1.0)
    , m_speedBoostTimerMs(0.0)
    , m_shieldTimerMs(0.0)
    , m_invisibilityTimerMs(0.0)
    , m_invisibilityAnchorPosition(0, 0)
    , m_invisibilityAnchorRotation(0)
    , m_invisibilityAnchorValid(false)
    , m_oilSlipTimerMs(0.0)
    , m_oilSteeringFactor(1.0)
    , m_oilSpeedFactor(1.0)
    , m_magnetTimerMs(0.0)
    , m_isAccelerating(false)
    , m_isBraking(false)
    , m_isSteeringLeft(false)
    , m_isSteeringRight(false)
    , m_isHandbrake(false)
    , m_throttleInput(0.0)
    , m_brakeInput(0.0)
    , m_steeringInput(0.0)
    , m_isColliding(false)
    , m_collisionCooldown(0)
    , m_collisionDuration(500)
    , m_currentLap(0)
    , m_totalCheckpoints(0)
    , m_nextCheckpointIndex(0)
    , m_raceLogicEnabled(false)
    , m_hasLeftNorthSector(false)
    , m_wasOnStartLine(false)
    , m_wasInNorthGate(false)
    , m_blockCheckpointsUntilLeaveNorth(false)
{
}

VehiclePhysics::~VehiclePhysics()
{
}

void VehiclePhysics::initialize(TrackManager* trackManager)
{
    m_trackManager = trackManager;
    reset();
}

void VehiclePhysics::rebindTrack(TrackManager* trackManager)
{
    m_trackManager = trackManager;
}

void VehiclePhysics::reset()
{
    if (m_trackManager && m_trackManager->hasCurrentTrack()) {
        m_position = m_trackManager->getStartPosition();
        m_rotation = m_trackManager->getStartRotation();
        m_totalCheckpoints = m_trackManager->getCheckpointCount();
    } else {
        m_position = QVector2D(320, 320);
        m_rotation = 0;
        m_totalCheckpoints = 0;
    }

    m_speed = 0;
    m_acceleration = 0;
    m_angularVelocity = 0;
    m_currentLap = 1;
    m_nextCheckpointIndex = 0;
    m_previousPosition = m_position;
    m_raceLogicEnabled = false;
    m_hasLeftNorthSector = false;
    m_wasOnStartLine = false;
    m_wasInNorthGate = false;
    m_blockCheckpointsUntilLeaveNorth = false;
    m_isColliding = false;
    m_collisionCooldown = 0;
    m_isAccelerating = false;
    m_isBraking = false;
    m_isSteeringLeft = false;
    m_isSteeringRight = false;
    m_isHandbrake = false;
    m_throttleInput = 0.0;
    m_brakeInput = 0.0;
    m_steeringInput = 0.0;
    m_speedBoostMultiplier = 1.0;
    m_speedBoostTimerMs = 0.0;
    m_shieldTimerMs = 0.0;
    m_invisibilityTimerMs = 0.0;
    m_invisibilityAnchorValid = false;
    m_oilSlipTimerMs = 0.0;
    m_oilSteeringFactor = 1.0;
    m_oilSpeedFactor = 1.0;
    m_magnetTimerMs = 0.0;
}

void VehiclePhysics::resetRaceProgress()
{
    m_currentLap = 1;
    m_nextCheckpointIndex = 0;
    m_hasLeftNorthSector = false;
    m_wasOnStartLine = false;
    m_wasInNorthGate = false;
    m_blockCheckpointsUntilLeaveNorth = false;

    if (m_trackManager && m_trackManager->hasCurrentTrack()) {
        m_totalCheckpoints = m_trackManager->getCheckpointCount();
    } else {
        m_totalCheckpoints = 0;
    }

    m_previousPosition = m_position;
    m_wasOnStartLine = isOnStartFinishTile();
    m_wasInNorthGate = isInNorthGate();
}

bool VehiclePhysics::crossedCheckpointGate(const Checkpoint* cp,
                                           const QVector2D& from,
                                           const QVector2D& to) const
{
    if (!cp) {
        return false;
    }

    const QRectF bounds = cp->getBounds();
    if (bounds.contains(from.toPointF()) || bounds.contains(to.toPointF())) {
        return true;
    }

    const QVector2D mid = (from + to) * 0.5;
    return bounds.contains(mid.toPointF());
}

void VehiclePhysics::update(qreal deltaTimeMs)
{
    const QVector2D positionBeforeUpdate = m_position;

    if (m_speedBoostTimerMs > 0.0) {
        m_speedBoostTimerMs = qMax<qreal>(0.0, m_speedBoostTimerMs - deltaTimeMs);
        if (m_speedBoostTimerMs <= 0.0) {
            m_speedBoostMultiplier = 1.0;
            if (m_speed > m_maxSpeed) {
                m_speed = m_maxSpeed;
            }
        }
    }

    if (m_shieldTimerMs > 0.0) {
        m_shieldTimerMs = qMax<qreal>(0.0, m_shieldTimerMs - deltaTimeMs);
    }

    if (m_invisibilityTimerMs > 0.0) {
        m_invisibilityTimerMs = qMax<qreal>(0.0, m_invisibilityTimerMs - deltaTimeMs);
        if (m_invisibilityTimerMs <= 0.0) {
            resolveInvisibilityExpiry();
        }
    }

    if (m_oilSlipTimerMs > 0.0) {
        m_oilSlipTimerMs = qMax<qreal>(0.0, m_oilSlipTimerMs - deltaTimeMs);
        if (m_oilSlipTimerMs <= 0.0) {
            m_oilSteeringFactor = 1.0;
            m_oilSpeedFactor = 1.0;
        }
    }

    if (m_magnetTimerMs > 0.0) {
        m_magnetTimerMs = qMax<qreal>(0.0, m_magnetTimerMs - deltaTimeMs);
    }

    if (m_collisionCooldown > 0) {
        m_collisionCooldown -= deltaTimeMs;
        if (m_collisionCooldown <= 0) {
            m_isColliding = false;
            m_collisionCooldown = 0;
        }
    }

    qreal dt = deltaTimeMs / 1000.0;

    applyAcceleration(dt);
    applySteering(dt);
    applyFriction(dt);

    applyMovement(dt);

    checkTrackBounds();
    checkCollisions(positionBeforeUpdate);

    m_previousPosition = m_position;
    emit positionUpdated(m_position);
}

void VehiclePhysics::setDriveInput(qreal throttle, qreal brake, qreal steering)
{
    m_throttleInput = qBound<qreal>(0.0, throttle, 1.0);
    m_brakeInput = qBound<qreal>(0.0, brake, 1.0);
    m_steeringInput = qBound<qreal>(-1.0, steering, 1.0);
}

void VehiclePhysics::clearDriveInput()
{
    m_throttleInput = 0.0;
    m_brakeInput = 0.0;
    m_steeringInput = 0.0;
}

bool VehiclePhysics::isPositionFree(const QVector2D& position) const
{
    return canOccupyPosition(position);
}

bool VehiclePhysics::acceptsKey(int key) const
{
    const bool wasdKey = key == Qt::Key_W || key == Qt::Key_S
        || key == Qt::Key_A || key == Qt::Key_D
        || key == Qt::Key_Space;
    const bool arrowKey = key == Qt::Key_Up || key == Qt::Key_Down
        || key == Qt::Key_Left || key == Qt::Key_Right;

    switch (m_controlScheme) {
    case ControlScheme::Wasd:
        return wasdKey;
    case ControlScheme::Arrows:
        return arrowKey;
    case ControlScheme::Combined:
    default:
        return wasdKey || arrowKey;
    }
}

void VehiclePhysics::handleKeyPress(QKeyEvent* event)
{
    int key = event->key();
    if (!acceptsKey(key)) {
        return;
    }
    if (key == Qt::Key_W || key == Qt::Key_Up) {
        m_isAccelerating = true;
        m_isBraking = false;
    } else if (key == Qt::Key_S || key == Qt::Key_Down) {
        m_isBraking = true;
        m_isAccelerating = false;
    } else if (key == Qt::Key_A || key == Qt::Key_Left) {
        m_isSteeringLeft = true;
        m_isSteeringRight = false;
    } else if (key == Qt::Key_D || key == Qt::Key_Right) {
        m_isSteeringRight = true;
        m_isSteeringLeft = false;
    } else if (key == Qt::Key_Space) {
        m_isHandbrake = true;
    }
    event->accept();
}

void VehiclePhysics::handleKeyRelease(QKeyEvent* event)
{
    int key = event->key();
    if (!acceptsKey(key)) {
        return;
    }
    if (key == Qt::Key_W || key == Qt::Key_Up) {
        m_isAccelerating = false;
    } else if (key == Qt::Key_S || key == Qt::Key_Down) {
        m_isBraking = false;
    } else if (key == Qt::Key_A || key == Qt::Key_Left) {
        m_isSteeringLeft = false;
    } else if (key == Qt::Key_D || key == Qt::Key_Right) {
        m_isSteeringRight = false;
    } else if (key == Qt::Key_Space) {
        m_isHandbrake = false;
    }
    event->accept();
}

void VehiclePhysics::activateSpeedBoost(qreal multiplier, qint64 durationMs)
{
    m_speedBoostMultiplier = qMax<qreal>(1.0, multiplier);
    m_speedBoostTimerMs = qMax<qint64>(0, durationMs);
    const qreal launchSpeed = m_maxSpeed * 0.45;
    m_speed = qMin(m_maxSpeed * m_speedBoostMultiplier,
                   qMax(qAbs(m_speed) * m_speedBoostMultiplier, launchSpeed));
}

void VehiclePhysics::activateShield(qint64 durationMs)
{
    m_shieldTimerMs = qMax<qint64>(0, durationMs);
}

void VehiclePhysics::activateRepair()
{
    m_isColliding = false;
    m_collisionCooldown = 0.0;
    if (qAbs(m_speed) < m_maxSpeed * 0.25) {
        m_speed = m_maxSpeed * 0.25;
    }
}

void VehiclePhysics::activateInvisibility(qint64 durationMs)
{
    m_invisibilityAnchorPosition = m_position;
    m_invisibilityAnchorRotation = m_rotation;
    m_invisibilityAnchorValid = true;
    m_invisibilityTimerMs = qMax<qint64>(0, durationMs);
    m_isColliding = false;
    m_collisionCooldown = 0.0;
}

void VehiclePhysics::resolveInvisibilityExpiry()
{
    if (!m_invisibilityAnchorValid) {
        return;
    }

    m_invisibilityAnchorValid = false;

    if (canOccupyPosition(m_position) && isOnDrivableTrack(m_position)) {
        return;
    }

    m_position = m_invisibilityAnchorPosition;
    m_previousPosition = m_invisibilityAnchorPosition;
    m_rotation = m_invisibilityAnchorRotation;
    m_speed = qMin(qAbs(m_speed), m_maxSpeed * 0.35);
    m_isColliding = false;
    m_collisionCooldown = 0.0;

    emit invisibilityRecoveryApplied(m_position);
}

void VehiclePhysics::activateOilSlip(qint64 durationMs, qreal steeringFactor, qreal speedFactor)
{
    m_oilSlipTimerMs = qMax<qint64>(m_oilSlipTimerMs, durationMs);
    m_oilSteeringFactor = qBound<qreal>(0.15, steeringFactor, 1.0);
    m_oilSpeedFactor = qBound<qreal>(0.2, speedFactor, 1.0);
}

void VehiclePhysics::activateMagnet(qint64 durationMs)
{
    m_magnetTimerMs = qMax<qint64>(0, durationMs);
}

bool VehiclePhysics::teleportForward(qreal distance)
{
    if (!m_trackManager || distance <= 0.0) {
        return false;
    }

    const qreal radians = qDegreesToRadians(m_rotation);
    const QVector2D forward(qSin(radians), qCos(radians));
    const qreal distances[] = {distance, distance * 0.75, distance * 0.5, distance * 0.25};

    for (qreal step : distances) {
        const QVector2D candidate = m_position + forward * step;
        if (canOccupyPosition(candidate)) {
            m_position = candidate;
            m_previousPosition = candidate;
            m_speed = qMax(m_speed, m_maxSpeed * 0.35);
            return true;
        }
    }

    return false;
}

void VehiclePhysics::applyAcceleration(qreal deltaTime)
{
    if (m_isColliding) {
        m_speed *= 0.9;
        m_acceleration = 0;
        return;
    }

    const qreal throttle = m_isAccelerating ? 1.0 : m_throttleInput;
    const qreal brake = m_isBraking ? 1.0 : m_brakeInput;

    if (throttle > 0.001) {
        m_speed += m_accelerationRate * throttle * deltaTime;
        const qreal effectiveMaxSpeed = m_maxSpeed * m_speedBoostMultiplier * m_oilSpeedFactor;
        if (m_speed > effectiveMaxSpeed) {
            m_speed = effectiveMaxSpeed;
        }
        m_acceleration = m_accelerationRate * throttle;
    } else if (brake > 0.001) {
        if (m_speed > 0) {
            m_speed -= m_brakeRate * brake * deltaTime;
            if (m_speed < 0) {
                m_speed = 0;
            }
        } else {
            m_speed -= m_accelerationRate * 0.5 * brake * deltaTime;
            if (m_speed < m_maxReverseSpeed) {
                m_speed = m_maxReverseSpeed;
            }
        }
        m_acceleration = -m_brakeRate * brake;
    } else {
        m_acceleration = 0;
    }
}

void VehiclePhysics::applySteering(qreal deltaTime)
{
    if (m_isColliding) {
        return;
    }

    qreal speedFactor = qMin(qAbs(m_speed) / 50.0, 1.0);

    qreal steering = m_steeringInput;
    if (m_isSteeringLeft) {
        steering -= 1.0;
    }
    if (m_isSteeringRight) {
        steering += 1.0;
    }
    steering = qBound<qreal>(-1.0, steering, 1.0);

    if (m_oilSlipTimerMs > 0.0) {
        const qreal wobble = qSin(m_oilSlipTimerMs * 0.014) * (1.0 - m_oilSteeringFactor);
        steering = qBound<qreal>(-1.0, steering + wobble, 1.0);
        steering *= m_oilSteeringFactor;
    }

    if (qAbs(steering) > 0.001) {
        m_rotation += steering * m_steeringSpeed * speedFactor * deltaTime;
    }

    if (m_rotation < 0) {
        m_rotation += 360;
    } else if (m_rotation >= 360) {
        m_rotation -= 360;
    }
}

void VehiclePhysics::applyFriction(qreal deltaTime)
{
    Q_UNUSED(deltaTime);

    if (!m_trackManager || !m_trackManager->hasCurrentTrack()) {
        m_speed *= m_frictionCoefficient;
        return;
    }

    TileType tileType = m_trackManager->getTileTypeAt(m_position);
    qreal friction = m_frictionCoefficient;

    switch (tileType) {
        case TileType::Grass:
        case TileType::Sand:
            friction = m_frictionCoefficient * m_grassFrictionMultiplier;
            break;
        case TileType::Road:
        case TileType::Asphalt:
        case TileType::StartLine:
        case TileType::FinishLine:
            friction = m_frictionCoefficient;
            break;
        case TileType::Wall:
        case TileType::Barrier:
            friction = 0.5;
            break;
        default:
            friction = m_frictionCoefficient;
            break;
    }

    if (m_throttleInput <= 0.001 && m_brakeInput <= 0.001
        && !m_isAccelerating && !m_isBraking) {
        m_speed *= friction;
        if (qAbs(m_speed) < 0.5) {
            m_speed = 0;
        }
    }
}

bool VehiclePhysics::isOnDrivableTrack(const QVector2D& position) const
{
    if (!m_trackManager || !m_trackManager->hasCurrentTrack()) {
        return true;
    }

    switch (m_trackManager->getTileTypeAt(position)) {
    case TileType::Road:
    case TileType::Asphalt:
    case TileType::StartLine:
    case TileType::FinishLine:
        return true;
    default:
        return false;
    }
}

bool VehiclePhysics::isSolidAt(const QVector2D& position) const
{
    if (!m_trackManager || !m_trackManager->hasCurrentTrack()) {
        return false;
    }

    TrackData* track = m_trackManager->getCurrentTrack();
    if (!track->getBounds().contains(position.toPointF())) {
        return true;
    }

    TrackTile* tile = m_trackManager->getTileAtPosition(position);
    if (!tile) {
        return true;
    }

    if (tile->isCollisionTile()) {
        return true;
    }

    const TileType type = tile->getType();
    return type == TileType::Wall || type == TileType::Barrier;
}

bool VehiclePhysics::canOccupyPosition(const QVector2D& position) const
{
    if (!m_trackManager || !m_trackManager->hasCurrentTrack()) {
        return true;
    }

    static const QVector2D sampleOffsets[] = {
        QVector2D(0, 0),
        QVector2D(18, 0),
        QVector2D(-18, 0),
        QVector2D(0, 18),
        QVector2D(0, -18),
        QVector2D(12, 12),
        QVector2D(-12, 12),
        QVector2D(12, -12),
        QVector2D(-12, -12),
    };

    for (const QVector2D& offset : sampleOffsets) {
        if (isSolidAt(position + offset)) {
            return false;
        }
    }

    return true;
}

QVector2D VehiclePhysics::computeBlockingNormal(const QVector2D& from, const QVector2D& to) const
{
    const QVector2D delta = to - from;
    if (delta.lengthSquared() > 0.01) {
        return -delta.normalized();
    }

    const qreal radians = qDegreesToRadians(m_rotation);
    return QVector2D(-qSin(radians), -qCos(radians));
}

void VehiclePhysics::applyMovement(qreal deltaTime)
{
    if (qAbs(m_speed) <= 0.01) {
        return;
    }

    const qreal radians = qDegreesToRadians(m_rotation);
    const QVector2D delta(qSin(radians) * m_speed * deltaTime,
                          qCos(radians) * m_speed * deltaTime);

    if (m_invisibilityTimerMs > 0.0) {
        m_position += delta;
        return;
    }

    if (!m_trackManager || !m_trackManager->hasCurrentTrack()) {
        m_position += delta;
        return;
    }

    const qreal stepSize = 8.0;
    const int steps = qMax(1, static_cast<int>(qCeil(delta.length() / stepSize)));
    const QVector2D stepDelta = delta / static_cast<qreal>(steps);
    QVector2D pos = m_position;

    for (int i = 0; i < steps; ++i) {
        const QVector2D next = pos + stepDelta;

        if (canOccupyPosition(next)) {
            pos = next;
            continue;
        }

        const QVector2D slideX(pos.x() + stepDelta.x(), pos.y());
        const QVector2D slideY(pos.x(), pos.y() + stepDelta.y());
        bool moved = false;

        if (qAbs(stepDelta.x()) > 0.001 && canOccupyPosition(slideX)) {
            pos = slideX;
            moved = true;
        } else if (qAbs(stepDelta.y()) > 0.001 && canOccupyPosition(slideY)) {
            pos = slideY;
            moved = true;
        }

        if (!moved) {
            if (!m_isColliding) {
                handleCollisionResponse(computeBlockingNormal(pos, next), qAbs(m_speed));
            }
            break;
        }
    }

    m_position = pos;
}

void VehiclePhysics::checkTrackBounds()
{
    if (m_invisibilityTimerMs > 0.0) {
        return;
    }

    if (!m_trackManager || !m_trackManager->hasCurrentTrack()) {
        return;
    }

    TrackTile* currentTile = m_trackManager->getTileAtPosition(m_position);
    if (currentTile && currentTile->isCollisionTile()) {
        QVector2D tileCenter = currentTile->getCenter();
        QVector2D toVehicle = m_position - tileCenter;
        qreal distance = toVehicle.length();
        
        if (distance > 0) {
            QVector2D normal = toVehicle.normalized();
            qreal tileHalfSize = 32.0;
            qreal penetration = tileHalfSize - distance;
            
            if (penetration > 0) {
                m_position -= normal * (penetration + 5.0);
                handleCollisionResponse(normal, qAbs(m_speed));
            }
        }
    }

    TileType currentTileType = m_trackManager->getTileTypeAt(m_position);
    if (currentTileType == TileType::Wall || currentTileType == TileType::Barrier) {
        const qreal radians = qDegreesToRadians(m_rotation);
        QVector2D prevPosition = m_position
            - QVector2D(qSin(radians) * m_speed * 0.05, qCos(radians) * m_speed * 0.05);
        TileType prevTileType = m_trackManager->getTileTypeAt(prevPosition);
        
        if (prevTileType != TileType::Wall && prevTileType != TileType::Barrier) {
            const QVector2D hitNormal = computeBlockingNormal(prevPosition, m_position);
            m_position = prevPosition;
            if (!m_isColliding) {
                handleCollisionResponse(hitNormal, qAbs(m_speed));
            }
        } else {
            m_position = prevPosition;
            m_speed *= 0.3;
            handleCollisionResponse(QVector2D(-qCos(qDegreesToRadians(m_rotation)),
                                              -qSin(qDegreesToRadians(m_rotation))), qAbs(m_speed));
        }
    }

    QRectF bounds = m_trackManager->getCurrentTrack()->getBounds();
    qreal margin = 10.0;

    if (m_position.x() < bounds.left() + margin) {
        m_position.setX(bounds.left() + margin);
        handleCollisionResponse(QVector2D(1, 0), qAbs(m_speed));
    } else if (m_position.x() > bounds.right() - margin) {
        m_position.setX(bounds.right() - margin);
        handleCollisionResponse(QVector2D(-1, 0), qAbs(m_speed));
    }

    if (m_position.y() < bounds.top() + margin) {
        m_position.setY(bounds.top() + margin);
        handleCollisionResponse(QVector2D(0, 1), qAbs(m_speed));
    } else if (m_position.y() > bounds.bottom() - margin) {
        m_position.setY(bounds.bottom() - margin);
        handleCollisionResponse(QVector2D(0, -1), qAbs(m_speed));
    }
}

bool VehiclePhysics::isOnStartFinishTile() const
{
    if (!m_trackManager) {
        return false;
    }
    const TileType tileType = m_trackManager->getTileTypeAt(m_position);
    return tileType == TileType::StartLine || tileType == TileType::FinishLine;
}

bool VehiclePhysics::isInNorthGate() const
{
    if (!m_trackManager || !m_trackManager->hasCurrentTrack()) {
        return false;
    }

    const QList<Checkpoint*> checkpoints = m_trackManager->getCurrentTrack()->getCheckpointsInOrder();
    if (checkpoints.isEmpty() || !checkpoints.first()) {
        return false;
    }
    return checkpoints.first()->containsPoint(m_position);
}

void VehiclePhysics::updateRaceProgress(const QVector2D& positionBeforeUpdate)
{
    if (!m_raceLogicEnabled || !m_trackManager || !m_trackManager->hasCurrentTrack()) {
        return;
    }

    TrackData* track = m_trackManager->getCurrentTrack();
    QList<Checkpoint*> checkpoints = track->getCheckpointsInOrder();
    if (m_totalCheckpoints <= 0) {
        m_totalCheckpoints = checkpoints.size();
    }

    const bool onStartLine = isOnStartFinishTile();
    const bool inNorthGate = isInNorthGate();
    const bool inNorthSector = onStartLine || inNorthGate;
    const bool wasInNorthSector = m_wasOnStartLine || m_wasInNorthGate;

    if (!inNorthSector) {
        m_hasLeftNorthSector = true;
        m_blockCheckpointsUntilLeaveNorth = false;
    }

    const bool allCheckpointsCollected =
        m_totalCheckpoints > 0 && m_nextCheckpointIndex >= m_totalCheckpoints;

    if (allCheckpointsCollected && m_hasLeftNorthSector) {
        const bool enteredStartLine = onStartLine && !m_wasOnStartLine;
        const bool enteredNorthGate = inNorthGate && !m_wasInNorthGate;
        if (enteredStartLine || enteredNorthGate) {
            emit lapCompleted();
            m_nextCheckpointIndex = 0;
            m_hasLeftNorthSector = false;
            m_blockCheckpointsUntilLeaveNorth = true;
        }
    }

    if (!m_blockCheckpointsUntilLeaveNorth && m_nextCheckpointIndex < checkpoints.size()) {
        if (m_nextCheckpointIndex == 0) {
            const bool leavingNorth = !inNorthSector && wasInNorthSector;
            if (leavingNorth) {
                emit checkpointReached(0);
                m_nextCheckpointIndex = 1;
            }
        } else {
            Checkpoint* nextCp = checkpoints.at(m_nextCheckpointIndex);
            if (crossedCheckpointGate(nextCp, positionBeforeUpdate, m_position)) {
                emit checkpointReached(m_nextCheckpointIndex);
                ++m_nextCheckpointIndex;
            }
        }
    }

    m_wasOnStartLine = onStartLine;
    m_wasInNorthGate = inNorthGate;
}

void VehiclePhysics::checkCollisions(const QVector2D& positionBeforeUpdate)
{
    Q_UNUSED(positionBeforeUpdate);

    if (!m_trackManager || !m_trackManager->hasCurrentTrack()) {
        return;
    }

    if (m_invisibilityTimerMs > 0.0) {
        return;
    }

    TileType tileType = m_trackManager->getTileTypeAt(m_position);

    if (tileType == TileType::Wall || tileType == TileType::Barrier) {
        if (!m_isColliding) {
            QVector2D normal(0, 1);
            if (m_speed > 0.1) {
                normal = QVector2D(-qCos(qDegreesToRadians(m_rotation)),
                                   -qSin(qDegreesToRadians(m_rotation)));
            }
            handleCollisionResponse(normal, qAbs(m_speed));
        }
    }

    if (tileType == TileType::Grass || tileType == TileType::Sand) {
        qreal grassFriction = m_frictionCoefficient * m_grassFrictionMultiplier;
        m_speed *= grassFriction;
        if (qAbs(m_speed) < 0.5) {
            m_speed = 0;
        }
    }

}

void VehiclePhysics::handleCollision(const QVector2D& normal, qreal impactForce)
{
    handleCollisionResponse(normal, impactForce);
}

bool VehiclePhysics::resolveHazardCollision(const QRectF& hazardBounds,
                                            const QList<QRectF>& blockedAreas,
                                            const QVector2D& positionBeforeUpdate,
                                            const QVector2D& fallbackPosition,
                                            qreal fallbackRotation,
                                            const QSizeF& collisionSize,
                                            qreal impactForce)
{
    const qreal halfWidth = qMax<qreal>(0.0, collisionSize.width() * 0.5);
    const qreal halfHeight = qMax<qreal>(0.0, collisionSize.height() * 0.5);
    auto collisionRectFor = [halfWidth, halfHeight](const QVector2D& position) {
        return QRectF(position.x() - halfWidth,
                      position.y() - halfHeight,
                      halfWidth * 2.0,
                      halfHeight * 2.0);
    };
    auto candidateIsClear = [&](const QVector2D& candidate) {
        const QRectF candidateRect = collisionRectFor(candidate);
        for (const QRectF& blockedBounds : blockedAreas) {
            if (candidateRect.intersects(blockedBounds)) {
                return false;
            }
        }
        return canOccupyPosition(candidate);
    };

    QVector2D normal = m_position - QVector2D(hazardBounds.center());
    if (normal.lengthSquared() < 0.001f) {
        normal = positionBeforeUpdate - QVector2D(hazardBounds.center());
    }
    if (normal.lengthSquared() < 0.001f) {
        normal = QVector2D(0.0f, -1.0f);
    } else {
        normal.normalize();
    }

    QVector2D resolvedPosition = fallbackPosition;
    bool foundSafePosition = false;
    auto tryResolvedCandidate = [&](const QVector2D& candidate) {
        if (!candidateIsClear(candidate)) {
            return false;
        }
        resolvedPosition = candidate;
        foundSafePosition = true;
        return true;
    };

    tryResolvedCandidate(fallbackPosition);
    if (!foundSafePosition) {
        tryResolvedCandidate(positionBeforeUpdate);
    }
    if (!foundSafePosition) {
        tryResolvedCandidate(m_position);
    }

    if (!foundSafePosition) {
        const QRectF expandedBounds = hazardBounds.adjusted(-(halfWidth + 1.0),
                                                            -(halfHeight + 1.0),
                                                            halfWidth + 1.0,
                                                            halfHeight + 1.0);
        QVector2D pushedPosition = m_position;
        const qreal distanceToLeft = qAbs(m_position.x() - expandedBounds.left());
        const qreal distanceToRight = qAbs(expandedBounds.right() - m_position.x());
        const qreal distanceToTop = qAbs(m_position.y() - expandedBounds.top());
        const qreal distanceToBottom = qAbs(expandedBounds.bottom() - m_position.y());
        const qreal smallestPenetration = qMin(qMin(distanceToLeft, distanceToRight),
                                               qMin(distanceToTop, distanceToBottom));

        if (smallestPenetration == distanceToLeft) {
            pushedPosition.setX(expandedBounds.left() - 6.0);
        } else if (smallestPenetration == distanceToRight) {
            pushedPosition.setX(expandedBounds.right() + 6.0);
        } else if (smallestPenetration == distanceToTop) {
            pushedPosition.setY(expandedBounds.top() - 6.0);
        } else {
            pushedPosition.setY(expandedBounds.bottom() + 6.0);
        }

        tryResolvedCandidate(pushedPosition);

        if (!foundSafePosition) {
            const QVector2D tangent(-normal.y(), normal.x());
            const QList<QVector2D> directions = {
                normal,
                tangent,
                -tangent,
                normal + tangent,
                normal - tangent,
                -normal,
                -normal + tangent,
                -normal - tangent
            };
            const QList<QVector2D> seeds = {
                pushedPosition,
                fallbackPosition,
                positionBeforeUpdate
            };

            for (const QVector2D& seed : seeds) {
                for (const QVector2D& direction : directions) {
                    QVector2D unitDirection = direction;
                    if (unitDirection.lengthSquared() <= 0.001f) {
                        continue;
                    }
                    unitDirection.normalize();
                    for (int step = 1; step <= 20; ++step) {
                        const QVector2D candidate = seed + unitDirection * (12.0f * step);
                        if (tryResolvedCandidate(candidate)) {
                            break;
                        }
                    }
                    if (foundSafePosition) {
                        break;
                    }
                }
                if (foundSafePosition) {
                    break;
                }
            }
        }
    }

    if (!foundSafePosition) {
        return false;
    }

    m_position = resolvedPosition;
    m_previousPosition = resolvedPosition;
    m_rotation = fallbackRotation;
    if (!m_isColliding) {
        handleCollisionResponse(normal, impactForce);
    }
    return true;
}

void VehiclePhysics::handleCollisionResponse(const QVector2D& normal, qreal impactForce)
{
    if (m_invisibilityTimerMs > 0.0) {
        return;
    }

    if (m_shieldTimerMs > 0.0) {
        m_speed *= 0.75;
        m_isColliding = true;
        m_collisionCooldown = qMin<qreal>(m_collisionDuration, 250.0);
        return;
    }

    m_speed *= 0.3;

    QVector2D velocity(qCos(qDegreesToRadians(m_rotation)) * m_speed,
                       qSin(qDegreesToRadians(m_rotation)) * m_speed);

    qreal dotProduct = QVector2D::dotProduct(velocity, normal);

    if (dotProduct < 0) {
        velocity = velocity - normal * (1.5f * dotProduct);
        m_speed = velocity.length();

        qreal newAngle = qAtan2(velocity.y(), velocity.x());
        m_rotation = qRadiansToDegrees(newAngle);
    }

    emit collisionOccurred("boundary", m_position, impactForce);

    m_isColliding = true;
    m_collisionCooldown = m_collisionDuration;
}

}

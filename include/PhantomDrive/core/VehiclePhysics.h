#pragma once

#include "PhantomDrive_global.h"
#include "track/TrackData.h"
#include "track/TrackManager.h"
#include "track/Checkpoint.h"

#include <QObject>
#include <QVector2D>
#include <QKeyEvent>

namespace PhantomDrive {

class PHANTOMDRIVE_EXPORT VehiclePhysics : public QObject
{
    Q_OBJECT

public:
    explicit VehiclePhysics(QObject* parent = nullptr);
    ~VehiclePhysics() override;

    void initialize(TrackManager* trackManager);

    void update(qreal deltaTimeMs);

    void handleKeyPress(QKeyEvent* event);
    void handleKeyRelease(QKeyEvent* event);
    void handleCollision(const QVector2D& normal, qreal impactForce);
    void activateSpeedBoost(qreal multiplier, qint64 durationMs);
    void activateShield(qint64 durationMs);
    void activateRepair();

    QVector2D getPosition() const { return m_position; }
    qreal getRotation() const { return m_rotation; }
    qreal getSpeed() const { return m_speed; }
    qreal getAcceleration() const { return m_acceleration; }
    bool isSpeedBoostActive() const { return m_speedBoostTimerMs > 0.0; }
    bool isShieldActive() const { return m_shieldTimerMs > 0.0; }

    void setPosition(const QVector2D& pos) { m_position = pos; }
    void setRotation(qreal rot) { m_rotation = rot; }
    void reset();
    void resetRaceProgress();
    void setRaceLogicEnabled(bool enabled) { m_raceLogicEnabled = enabled; }

    bool isColliding() const { return m_isColliding; }
    qreal getCollisionCooldown() const { return m_collisionCooldown; }

signals:
    void positionUpdated(const QVector2D& position);
    void collisionOccurred(const QString& objectType, const QVector2D& position, qreal impactForce);
    void lapCompleted();
    void checkpointReached(int checkpointId);

private:
    void applyAcceleration(qreal deltaTime);
    void applySteering(qreal deltaTime);
    void applyFriction(qreal deltaTime);
    void applyGravity(qreal deltaTime);
    void applyMovement(qreal deltaTime);
    bool canOccupyPosition(const QVector2D& position) const;
    bool isSolidAt(const QVector2D& position) const;
    QVector2D computeBlockingNormal(const QVector2D& from, const QVector2D& to) const;
    void checkTrackBounds();
    void checkCollisions(const QVector2D& positionBeforeUpdate);
    void handleCollisionResponse(const QVector2D& normal, qreal impactForce);
    void updateRaceProgress(const QVector2D& positionBeforeUpdate);
    bool isOnStartFinishTile() const;
    bool isInNorthGate() const;
    bool crossedCheckpointGate(const Checkpoint* cp, const QVector2D& from, const QVector2D& to) const;

    TrackManager* m_trackManager;

    QVector2D m_position;
    QVector2D m_previousPosition;
    qreal m_rotation;
    qreal m_speed;
    qreal m_acceleration;
    qreal m_angularVelocity;

    qreal m_maxSpeed;
    qreal m_maxReverseSpeed;
    qreal m_accelerationRate;
    qreal m_brakeRate;
    qreal m_steeringSpeed;
    qreal m_maxSteeringAngle;
    qreal m_frictionCoefficient;
    qreal m_grassFrictionMultiplier;
    qreal m_gravity;
    qreal m_speedBoostMultiplier;
    qreal m_speedBoostTimerMs;
    qreal m_shieldTimerMs;

    bool m_isAccelerating;
    bool m_isBraking;
    bool m_isSteeringLeft;
    bool m_isSteeringRight;
    bool m_isHandbrake;

    bool m_isColliding;
    qreal m_collisionCooldown;
    qreal m_collisionDuration;

    int m_currentLap;
    int m_totalCheckpoints;
    int m_nextCheckpointIndex;
    bool m_raceLogicEnabled;
    bool m_hasLeftNorthSector;
    bool m_wasOnStartLine;
    bool m_wasInNorthGate;
    bool m_blockCheckpointsUntilLeaveNorth;
};

}

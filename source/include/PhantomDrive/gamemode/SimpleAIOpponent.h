#pragma once

#include "AIOpponent.h"

namespace PhantomDrive {

class TrackManager;
class VehiclePhysics;

class SimpleAIOpponent : public AIOpponent
{
    Q_OBJECT

public:
    explicit SimpleAIOpponent(const QString& id, QObject* parent = nullptr);
    ~SimpleAIOpponent() override;

    void initializePhysics(TrackManager* trackManager);
    bool usesVehiclePhysics() const { return m_physics != nullptr; }
    bool isPhysicsColliding() const;
    bool isPositionFree(const QVector2D& position) const;
    void applyExternalCollision(const QVector2D& normal, qreal impactForce);
    void applyMissileHit();
    void applyOilSlip(qint64 durationMs);

    QString getName() const override { return m_name; }
    AIStyle getStyle() const override { return m_config.style; }
    AIConfig getConfig() const override { return m_config; }
    void setConfig(const AIConfig& config) override { m_config = config; }

    void setWaypoints(const QList<Waypoint>& waypoints) override;
    QList<Waypoint> getWaypoints() const override { return m_waypoints; }
    void addWaypoint(const Waypoint& waypoint) override { m_waypoints.append(waypoint); }
    void clearWaypoints() override { m_waypoints.clear(); }

    int getCurrentWaypointIndex() const override { return m_currentWaypointIndex; }
    void setCurrentWaypointIndex(int index) override;
    Waypoint getCurrentWaypoint() const override;
    Waypoint getNextWaypoint() const override;
    Waypoint getWaypointAt(int index) const override;

    qreal getTotalPathLength() const override;
    qreal getRemainingPathLength() const override;
    qreal getProgressPercentage() const override;

    QVector2D getPosition() const override { return m_position; }
    void setPosition(const QVector2D& position) override;

    qreal getRotation() const override { return m_rotation; }
    void setRotation(qreal rotation) override;

    QVector2D getVelocity() const override { return m_velocity; }
    void setVelocity(const QVector2D& velocity) override { m_velocity = velocity; }

    qreal getSpeed() const override { return m_speed; }
    qreal getMaxSpeed() const override { return m_config.maxSpeed; }
    void setMaxSpeed(qreal speed) override;

    qreal getSteeringAngle() const override { return m_steeringAngle; }
    void setSteeringAngle(qreal angle) override { m_steeringAngle = angle; }

    AIState getState() const override { return m_state; }
    void setState(AIState state) override;
    void setFinished(bool finished) override;

    qreal getStateDuration() const override { return m_stateDuration; }
    void resetStateDuration() override { m_stateDuration = 0; }

    int getCurrentLap() const override { return m_currentLap; }
    void setCurrentLap(int lap) override { m_currentLap = lap; }

    qreal getLapTime() const override { return m_lapTime; }
    void setLapTime(qreal time) override { m_lapTime = time; }

    qreal getBestLapTime() const override { return m_bestLapTime; }
    void setBestLapTime(qreal time) override { m_bestLapTime = time; }

    int getRacePosition() const override { return m_racePosition; }
    void setRacePosition(int pos) override { m_racePosition = pos; }

    int getCheckpointsPassed() const override { return m_checkpointsPassed; }
    void setCheckpointsPassed(int count) override { m_checkpointsPassed = count; }

    AIDecision makeDecision() override;
    void update(qint64 elapsedMs) override;

    QVector2D calculateSteering() override;
    qreal calculateThrottle() override;
    qreal calculateBraking() override;

    bool hasPowerup() const override { return !m_powerups.isEmpty(); }
    int getPowerupCount() const override { return m_powerups.size(); }
    void addPowerup(int powerupType) override { m_powerups.append(powerupType); }
    bool usePowerup(int slot) override;
    void clearPowerups() override { m_powerups.clear(); }

    void onCollision(const QString& objectId, const QVector2D& point) override;
    void onNearMiss(const QString& opponentId, qreal distance) override;
    void onOvertaken(const QString& opponentId) override;
    void onOvertake(const QString& opponentId) override;

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& json) override;
    QVariantMap getStateData() const override;
    void loadStateData(const QVariantMap& data) override;

    bool recoverToRoute(bool forceReposition = false);

protected:
    void onStateEnter(AIState newState) override;
    void onStateExit(AIState oldState) override;

    qreal calculateDistanceToWaypoint(const Waypoint& waypoint) const override;
    qreal calculateAngleToWaypoint(const Waypoint& waypoint) const override;
    int findNearestWaypointIndex() const override;
    int findNextRelevantWaypoint() const override;

    void syncFromPhysics();
    qreal waypointReachRadius() const;
    bool hasPassedCurrentWaypoint(const Waypoint& waypoint) const;
    int findForwardWaypointIndex(int startIndex) const;
    bool isFarFromCurrentRoute() const;
    void resetProgressTracking();

    VehiclePhysics* m_physics;
    QString m_name;
    AIConfig m_config;
    QList<Waypoint> m_waypoints;
    int m_currentWaypointIndex;

    QVector2D m_position;
    qreal m_rotation;
    QVector2D m_velocity;
    qreal m_speed;
    qreal m_steeringAngle;

    AIState m_state;
    qreal m_stateDuration;
    qint64 m_stuckTimerMs;
    qint64 m_noProgressTimerMs;
    qint64 m_recoveryCooldownMs;
    QVector2D m_lastProgressPosition;
    qreal m_lastWaypointDistance;

    int m_currentLap;
    qreal m_lapTime;
    qreal m_bestLapTime;
    int m_racePosition;
    int m_checkpointsPassed;

    QList<int> m_powerups;
};

} // namespace PhantomDrive

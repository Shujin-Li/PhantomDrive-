#pragma once

#include <QtGlobal>
#include <QVector2D>
#include <QString>

namespace PhantomDrive {

struct DrivingData {
    qint64 timestamp;

    QVector2D position;
    qreal rotation;
    QVector2D velocity;

    qreal speed;
    qreal steeringAngle;
    qreal acceleration;

    bool isBraking;
    bool isAccelerating;
    bool isHonking;

    qreal currentSpeedLimit;
    bool isInSpeedLimitZone;
    QString currentZoneId;

    bool hasCollided;
    QString collisionObjectId;

    qreal lapTime;
    qint32 currentLap;
    qint32 checkpointsPassed;

    DrivingData()
        : timestamp(0)
        , rotation(0.0)
        , speed(0.0)
        , steeringAngle(0.0)
        , acceleration(0.0)
        , isBraking(false)
        , isAccelerating(false)
        , isHonking(false)
        , currentSpeedLimit(0.0)
        , isInSpeedLimitZone(false)
        , hasCollided(false)
        , lapTime(0.0)
        , currentLap(0)
        , checkpointsPassed(0)
    {}
};

enum class ViolationType {
    None,
    RedLight,
    SpeedOverLimit,
    PedestrianCollision,
    WrongWay,
    Collision
};

struct ViolationEvent {
    qint64 timestamp;
    ViolationType type;
    QString description;
    QVector2D position;
    qreal speedAtViolation;
    qreal speedLimit;
    int penaltyPoints;

    ViolationEvent()
        : timestamp(0)
        , type(ViolationType::None)
        , penaltyPoints(0)
    {}
};

}

#include "CollisionDetector.h"

#include <QDateTime>
#include <QtMath>
#include <QDebug>

namespace PhantomDrive {

CollisionDetector::CollisionDetector(QObject* parent)
    : QObject(parent)
    , m_detectionRadius(1.5)
    , m_isEnabled(true)
    , m_currentVehicleRotation(0.0)
    , m_lastCollisionTime(0)
    , m_collisionDebounceMs(500)
{
}

CollisionDetector::~CollisionDetector()
{
}

void CollisionDetector::setVehicleId(const QString& vehicleId)
{
    m_vehicleId = vehicleId;
}

void CollisionDetector::setDetectionRadius(qreal radius)
{
    if (radius <= 0.0) {
        emit errorOccurred("Detection radius must be positive");
        return;
    }
    m_detectionRadius = radius;
}

void CollisionDetector::enableCollisionDetection(bool enable)
{
    m_isEnabled = enable;
    emit detectorEnabled(enable);
}

void CollisionDetector::registerCollidableObject(const QString& objectId, const QVector2D& position, qreal radius)
{
    if (objectId.isEmpty()) {
        emit errorOccurred("Object ID cannot be empty");
        return;
    }

    CollidableObject obj(objectId, position, radius);
    m_collidableObjects[objectId] = obj;
}

void CollisionDetector::updateCollidableObject(const QString& objectId, const QVector2D& position)
{
    if (!m_collidableObjects.contains(objectId)) {
        emit errorOccurred(QString("Object %1 not found").arg(objectId));
        return;
    }

    m_collidableObjects[objectId].position = position;

    if (m_isEnabled) {
        checkCollisions();
    }
}

void CollisionDetector::removeCollidableObject(const QString& objectId)
{
    if (m_collidableObjects.contains(objectId)) {
        m_collidableObjects.remove(objectId);

        if (m_currentCollisions.contains(objectId)) {
            m_currentCollisions.removeAll(objectId);
            emit collisionEnded(objectId);
        }
    }
}

void CollisionDetector::clearCollidableObjects()
{
    m_collidableObjects.clear();
    m_currentCollisions.clear();
}

QList<QString> CollisionDetector::getCollidableObjects() const
{
    return m_collidableObjects.keys();
}

QVector2D CollisionDetector::getCollidableObjectPosition(const QString& objectId) const
{
    if (m_collidableObjects.contains(objectId)) {
        return m_collidableObjects[objectId].position;
    }
    return QVector2D();
}

void CollisionDetector::updateVehiclePosition(const QVector2D& position)
{
    m_currentVehiclePosition = position;

    if (m_isEnabled) {
        checkCollisions();
    }
}

void CollisionDetector::updateVehicleRotation(qreal rotation)
{
    m_currentVehicleRotation = rotation;
}

void CollisionDetector::checkCollisions()
{
    if (!m_isEnabled) {
        return;
    }

    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

    QMapIterator<QString, CollidableObject> it(m_collidableObjects);
    while (it.hasNext()) {
        it.next();
        CollidableObject& obj = const_cast<CollidableObject&>(it.value());
        qreal distance = calculateDistance(m_currentVehiclePosition, obj.position);
        qreal combinedRadius = m_detectionRadius + obj.radius;

        bool wasColliding = obj.isColliding;
        obj.isColliding = (distance <= combinedRadius);

        if (obj.isColliding && !wasColliding) {
            if (!m_currentCollisions.contains(obj.id)) {
                m_currentCollisions.append(obj.id);

                if (currentTime - m_lastCollisionTime >= m_collisionDebounceMs) {
                    qreal impactForce = calculateImpactForce(m_currentVehicleVelocity.length());
                    emit collisionDetected(obj.id, obj.position, impactForce);

                    DrivingData collisionData = createCollisionData(obj.id, obj.position);
                    m_collisionHistory.append(collisionData);
                    emit collisionDataRecorded(collisionData);

                    m_lastCollisionTime = currentTime;
                }
            }
        } else if (!obj.isColliding && wasColliding) {
            if (m_currentCollisions.contains(obj.id)) {
                m_currentCollisions.removeAll(obj.id);
                emit collisionEnded(obj.id);
            }
        }

        if (!obj.isColliding && distance <= combinedRadius * 1.5) {
            emit nearMissDetected(obj.id, distance);
        }
    }
}

void CollisionDetector::clearCollisionHistory()
{
    m_collisionHistory.clear();
}

qreal CollisionDetector::calculateDistance(const QVector2D& pos1, const QVector2D& pos2) const
{
    return QVector2D(pos2 - pos1).length();
}

qreal CollisionDetector::calculateImpactForce(qreal relativeSpeed, qreal vehicleMass) const
{
    return 0.5 * vehicleMass * relativeSpeed * relativeSpeed;
}

DrivingData CollisionDetector::createCollisionData(const QString& objectId, const QVector2D& position) const
{
    DrivingData data;
    data.timestamp = QDateTime::currentMSecsSinceEpoch();
    data.position = m_currentVehiclePosition;
    data.velocity = m_currentVehicleVelocity;
    data.rotation = m_currentVehicleRotation;
    data.hasCollided = true;
    data.collisionObjectId = objectId;
    data.speed = m_currentVehicleVelocity.length();

    return data;
}

}

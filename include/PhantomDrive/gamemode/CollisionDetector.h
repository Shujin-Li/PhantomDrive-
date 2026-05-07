#pragma once

#include "PhantomDrive_global.h"
#include "DrivingData.h"

#include <QObject>
#include <QVector2D>
#include <QList>
#include <QSet>
#include <QMap>

namespace PhantomDrive {

class CollisionDetector : public QObject
{
    Q_OBJECT

public:
    explicit CollisionDetector(QObject* parent = nullptr);
    ~CollisionDetector() override;

    void setVehicleId(const QString& vehicleId);
    QString vehicleId() const { return m_vehicleId; }

    void setDetectionRadius(qreal radius);
    qreal detectionRadius() const { return m_detectionRadius; }

    void enableCollisionDetection(bool enable);
    bool isCollisionDetectionEnabled() const { return m_isEnabled; }

    void registerCollidableObject(const QString& objectId, const QVector2D& position, qreal radius);
    void updateCollidableObject(const QString& objectId, const QVector2D& position);
    void removeCollidableObject(const QString& objectId);
    void clearCollidableObjects();

    QList<QString> getCollidableObjects() const;
    QVector2D getCollidableObjectPosition(const QString& objectId) const;

    QList<QString> getCurrentCollisions() const { return m_currentCollisions; }
    QList<DrivingData> getCollisionHistory() const { return m_collisionHistory; }

    void clearCollisionHistory();

public slots:
    void updateVehiclePosition(const QVector2D& position);
    void updateVehicleRotation(qreal rotation);
    void checkCollisions();

signals:
    void collisionDetected(const QString& objectId, const QVector2D& position, qreal impactForce);
    void collisionEnded(const QString& objectId);
    void nearMissDetected(const QString& objectId, qreal distance);
    void collisionDataRecorded(const DrivingData& data);
    void detectorEnabled(bool enabled);
    void errorOccurred(const QString& error);

private:
    struct CollidableObject {
        QString id;
        QVector2D position;
        qreal radius;
        bool isColliding;

        CollidableObject() : radius(1.0), isColliding(false) {}
        CollidableObject(const QString& id, const QVector2D& pos, qreal r)
            : id(id), position(pos), radius(r), isColliding(false) {}
    };

    qreal calculateDistance(const QVector2D& pos1, const QVector2D& pos2) const;
    qreal calculateImpactForce(qreal relativeSpeed, qreal vehicleMass = 1000.0) const;
    DrivingData createCollisionData(const QString& objectId, const QVector2D& position) const;

    QString m_vehicleId;
    qreal m_detectionRadius;
    bool m_isEnabled;

    QVector2D m_currentVehiclePosition;
    qreal m_currentVehicleRotation;
    QVector2D m_currentVehicleVelocity;

    QMap<QString, CollidableObject> m_collidableObjects;
    QList<QString> m_currentCollisions;
    QList<DrivingData> m_collisionHistory;

    qint64 m_lastCollisionTime;
    int m_collisionDebounceMs;
};

}

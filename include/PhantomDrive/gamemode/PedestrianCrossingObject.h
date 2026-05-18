#pragma once

#include "TrafficObject.h"

#include <QString>
#include <QRectF>
#include <QList>

namespace PhantomDrive {

class PedestrianCrossingObject : public TrafficObject
{
    Q_OBJECT

public:
    struct Pedestrian {
        QString id;
        QVector2D position;
        qreal progress;
        bool isCrossing;
        qreal speed;

        Pedestrian() : progress(0.0), isCrossing(false), speed(1.0) {}
    };

    explicit PedestrianCrossingObject(const QString& id, QObject* parent = nullptr);
    ~PedestrianCrossingObject() override;

    QRectF getBounds() const { return m_bounds; }
    void setBounds(const QRectF& bounds);

    bool isActive() const { return m_isActive; }
    void setActive(bool active);

    int getPedestrianCount() const { return m_pedestrians.size(); }
    const QList<Pedestrian>& getPedestrians() const { return m_pedestrians; }

    void addPedestrian(const QString& pedestrianId);
    void removePedestrian(const QString& pedestrianId);
    void clearPedestrians();

    bool isVehicleInZone(const QVector2D& vehiclePosition) const;
    bool checkPedestrianViolation(const QVector2D& vehiclePosition) const;
    bool isPedestrianInDangerZone() const;

    int getViolationPenalty() const { return m_violationPenalty; }
    void setViolationPenalty(int penalty) { m_violationPenalty = penalty; }

    int getViolationCount() const { return m_violationCount; }
    void incrementViolationCount();
    void resetViolationCount() override { m_violationCount = 0; }

    qint64 getLastPedestrianTime() const { return m_lastPedestrianTime; }

    void spawnPedestrian();
    void update(qint64 elapsedMs) override;

signals:
    void pedestrianSpawned(const QString& crossingId, const QString& pedestrianId);
    void pedestrianRemoved(const QString& crossingId, const QString& pedestrianId);
    void pedestrianCrossed(const QString& crossingId, const QString& pedestrianId);
    void pedestrianViolation(const QString& crossingId);
    void zoneEntered(const QString& crossingId);
    void zoneExited(const QString& crossingId);

public slots:
    void onVehicleApproaching(const QVector2D& vehiclePosition);
    void onVehicleInZone(const QVector2D& vehiclePosition);
    void reset() override;

private:
    void updatePedestrians(qint64 elapsedMs);
    Pedestrian* findPedestrian(const QString& pedestrianId);

    QRectF m_bounds;
    bool m_isActive;
    QList<Pedestrian> m_pedestrians;

    int m_violationPenalty;
    int m_violationCount;
    qint64 m_lastPedestrianTime;
    qint64 m_lastUpdateTime;

    qreal m_pedestrianSpawnInterval;
    qreal m_pedestrianSpeed;
    int m_maxPedestrians;

    qint64 m_sessionStartTime;
};

}

#pragma once

#include "TrafficObject.h"
#include "TrafficLightObject.h"
#include "SpeedLimitSignObject.h"
#include "PedestrianCrossingObject.h"
#include "PhantomDrive/scoring/ViolationConfig.h"

#include <QObject>
#include <QString>
#include <QMap>
#include <QList>
#include <QVector2D>

namespace PhantomDrive {

class TrafficObjectManager : public QObject
{
    Q_OBJECT

public:
    explicit TrafficObjectManager(QObject* parent = nullptr);
    ~TrafficObjectManager();
    TrafficObjectManager(const TrafficObjectManager&) = delete;
    TrafficObjectManager& operator=(const TrafficObjectManager&) = delete;

    void registerTrafficObject(TrafficObject* object);
    void unregisterTrafficObject(const QString& objectId);
    TrafficObject* getTrafficObject(const QString& objectId) const;
    bool hasObject(const QString& objectId) const;

    void registerTrafficLight(TrafficLightObject* light);
    void registerSpeedLimitSign(SpeedLimitSignObject* sign);
    void registerPedestrianCrossing(PedestrianCrossingObject* crossing);

    void unregisterTrafficLight(const QString& lightId);
    void unregisterSpeedLimitSign(const QString& signId);
    void unregisterPedestrianCrossing(const QString& crossingId);

    QList<TrafficLightObject*> getTrafficLights() const;
    QList<SpeedLimitSignObject*> getSpeedLimitSigns() const;
    QList<PedestrianCrossingObject*> getPedestrianCrossings() const;

    QList<TrafficObject*> getAllTrafficObjects() const;
    QList<TrafficObject*> getObjectsInRange(const QVector2D& position, qreal radius) const;
    QList<TrafficObject*> getObjectsByType(TrafficObjectType type) const;

    void update(qint64 elapsedMs);
    void startAll();
    void stopAll();

    int getTotalObjectCount() const;
    int getActiveObjectCount() const;

    int getTotalViolationCount() const;
    int getRedLightViolationCount() const;
    int getSpeedViolationCount() const;
    int getPedestrianViolationCount() const;

    void resetAllViolationCounts();

    qreal getCurrentSpeedLimit(const QVector2D& position) const;
    bool isRedLightViolation(const QString& lightId, const QVector2D& vehiclePosition) const;
    bool checkSpeedViolation(const QVector2D& position, qreal speed) const;
    bool checkPedestrianViolation(const QVector2D& position) const;

    void onVehiclePositionChanged(const QVector2D& position);
    void onVehicleSpeedChanged(qreal speed);

signals:
    void objectRegistered(const QString& objectId, TrafficObjectType type);
    void objectUnregistered(const QString& objectId);
    void trafficLightViolated(const QString& lightId);
    void speedLimitViolated(const QString& signId, qreal speed, qreal limit);
    void pedestrianViolation(const QString& crossingId);
    void allObjectsUpdated();
    void managerCleared();
    void violationDetected(const ViolationEvent& event);

public slots:
    void clear();

private:
    void checkSpeedLimitZones(const QVector2D& position);

    QMap<QString, TrafficObject*> m_trafficObjects;
    QMap<QString, TrafficLightObject*> m_trafficLights;
    QMap<QString, SpeedLimitSignObject*> m_speedLimitSigns;
    QMap<QString, PedestrianCrossingObject*> m_pedestrianCrossings;

    QVector2D m_lastVehiclePosition;
    qreal m_lastVehicleSpeed;

    QString m_currentSpeedLimitZoneId;
};

}

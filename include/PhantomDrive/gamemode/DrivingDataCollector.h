#pragma once

#include "PhantomDrive_global.h"
#include "IDrivingDataCollector.h"
#include "VehicleSensor.h"
#include "CollisionDetector.h"
#include "DrivingDataStorage.h"

#include <QObject>
#include <QTimer>
#include <QFile>

namespace PhantomDrive {

class PHANTOMDRIVE_EXPORT DrivingDataCollector : public IDrivingDataCollector
{
    Q_OBJECT

public:
    explicit DrivingDataCollector(QObject* parent = nullptr);
    ~DrivingDataCollector() override;

    void setVehicleId(const QString& vehicleId);
    QString vehicleId() const { return m_vehicleId; }

    void setSamplingInterval(int intervalMs);
    int samplingInterval() const;

    void setStorageEnabled(bool enabled);
    bool isStorageEnabled() const { return m_isStorageEnabled; }

    void setCollisionDetectionEnabled(bool enabled);
    bool isCollisionDetectionEnabled() const;

    void startCollection() override;
    void stopCollection() override;
    bool isCollecting() const override { return m_isCollecting; }

    DrivingData getCurrentData() const override;
    QList<DrivingData> getCollectedData() const override;
    QList<ViolationEvent> getViolations() const override;

    void clearData() override;

    VehicleSensor* vehicleSensor() const { return m_vehicleSensor; }
    CollisionDetector* collisionDetector() const { return m_collisionDetector; }
    DrivingDataStorage* dataStorage() const { return m_dataStorage; }

    bool exportData(const QString& filePath) const;
    bool importData(const QString& filePath);

    qint64 getCollectionDuration() const;
    int getTotalDataPoints() const;
    int getTotalViolations() const;

public slots:
    void registerCollidableObject(const QString& objectId, const QVector2D& position, qreal radius = 1.0);
    void updateCollidableObject(const QString& objectId, const QVector2D& position);
    void removeCollidableObject(const QString& objectId);

    void setCurrentSpeedLimit(qreal limit, const QString& zoneId = QString());
    void updateCheckpointProgress(int checkpointId, qreal lapTime, qint32 currentLap);

signals:
    void collectionStarted(int samplingInterval);
    void collectionStopped(int totalDataPoints);
    void storageBufferFull();
    void criticalCollisionDetected(const QString& objectId, qreal impactForce);
    void dataExportCompleted(const QString& filePath);
    void dataImportCompleted(const QString& filePath);

protected slots:
    void onSensorDataReady(const DrivingData& data);
    void onCollisionDetected(const QString& objectId, const QVector2D& position, qreal impactForce);
    void onSpeedLimitExceeded(qreal currentSpeed, qreal limit);
    void onStorageBufferFull();

private:
    void initializeComponents();
    void cleanupComponents();
    ViolationEvent createViolationEvent(ViolationType type, const QString& description, qreal speed = 0.0) const;

    QString m_vehicleId;
    int m_samplingInterval;
    bool m_isCollecting;
    bool m_isStorageEnabled;

    qint64 m_collectionStartTime;
    qint64 m_collectionEndTime;

    VehicleSensor* m_vehicleSensor;
    CollisionDetector* m_collisionDetector;
    DrivingDataStorage* m_dataStorage;

    int m_currentCheckpoint;
    qreal m_currentLapTime;
    qint32 m_currentLapNumber;
};

}

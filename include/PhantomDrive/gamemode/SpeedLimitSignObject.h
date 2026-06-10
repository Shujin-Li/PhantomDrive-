#pragma once

#include "TrafficObject.h"

#include <QString>
#include <QVector2D>
#include <QRectF>

namespace PhantomDrive {

class SpeedLimitSignObject : public TrafficObject
{
    Q_OBJECT

public:
    explicit SpeedLimitSignObject(const QString& id, QObject* parent = nullptr);
    ~SpeedLimitSignObject() override;

    qreal getSpeedLimit() const { return m_speedLimit; }
    void setSpeedLimit(qreal limit);

    bool isActive() const { return m_isActive; }
    void setActive(bool active);

    QString getZoneId() const { return m_zoneId; }
    void setZoneId(const QString& zoneId);

    qreal getDetectionRadius() const { return m_detectionRadius; }
    void setDetectionRadius(qreal radius);

    bool isVehicleInZone(const QVector2D& vehiclePosition) const;
    bool checkSpeedViolation(qreal currentSpeed) const;
    qreal getOverspeedPercentage(qreal currentSpeed) const;

    int getViolationPenalty() const { return m_violationPenalty; }
    void setViolationPenalty(int penalty) { m_violationPenalty = penalty; }

    int getViolationCount() const { return m_violationCount; }
    void incrementViolationCount();
    void resetViolationCount() override { m_violationCount = 0; }

    qreal getMaxSpeedRecorded() const { return m_maxSpeedRecorded; }
    void updateMaxSpeed(qreal speed);

signals:
    void speedLimitChanged(qreal oldLimit, qreal newLimit);
    void zoneEntered(const QString& signId, const QString& zoneId);
    void zoneExited(const QString& signId, const QString& zoneId);
    void speedViolation(const QString& signId, qreal currentSpeed, qreal limit);

public slots:
    void onVehiclePositionChanged(const QVector2D& vehiclePosition);
    void onVehicleSpeedChanged(qreal speed);
    void reset() override;

private:
    qreal m_speedLimit;
    bool m_isActive;
    QString m_zoneId;
    qreal m_detectionRadius;
    bool m_isVehicleInZone;

    int m_violationPenalty;
    int m_violationCount;
    qreal m_maxSpeedRecorded;
    qint64 m_lastViolationTime;
    int m_minViolationIntervalMs;
};

}

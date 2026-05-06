#pragma once

#include "PhantomDrive_global.h"
#include "DrivingData.h"

#include <QObject>
#include <QList>
#include <QJsonArray>
#include <QJsonObject>

namespace PhantomDrive {

class PHANTOMDRIVE_EXPORT DrivingDataStorage : public QObject
{
    Q_OBJECT

public:
    explicit DrivingDataStorage(QObject* parent = nullptr);
    ~DrivingDataStorage() override;

    void setMaxStorageSize(int maxSize);
    int maxStorageSize() const { return m_maxStorageSize; }

    int currentSize() const { return m_dataBuffer.size(); }
    void clear();

    void addData(const DrivingData& data);
    void addDataBatch(const QList<DrivingData>& dataBatch);

    QList<DrivingData> getData() const { return m_dataBuffer; }
    QList<DrivingData> getData(qint64 startTime, qint64 endTime) const;
    QList<DrivingData> getData(int startIndex, int endIndex) const;

    QList<ViolationEvent> getViolations() const { return m_violations; }
    void addViolation(const ViolationEvent& violation);
    void clearViolations();

    DrivingData getLatestData() const;
    DrivingData getDataAt(qint64 timestamp) const;

    QJsonArray toJsonArray() const;
    QString toJsonString() const;

    bool exportToFile(const QString& filePath) const;
    bool importFromFile(const QString& filePath);

    QList<DrivingData> filterBySpeed(qreal minSpeed, qreal maxSpeed) const;
    QList<DrivingData> filterByCollision(bool hasCollision) const;
    QList<DrivingData> filterByTimeRange(qint64 startMs, qint64 endMs) const;

    qint64 getRecordingDuration() const;
    qreal getAverageSpeed() const;
    qreal getMaxSpeed() const;
    int getTotalCollisions() const;

signals:
    void dataAdded(const DrivingData& data);
    void dataBatchAdded(int count);
    void bufferFull();
    void bufferCleared();
    void violationAdded(const ViolationEvent& violation);
    void exportCompleted(const QString& filePath);
    void importCompleted(const QString& filePath);
    void errorOccurred(const QString& error);

private:
    void enforceMaxSize();
    bool validateData(const DrivingData& data) const;

    QList<DrivingData> m_dataBuffer;
    QList<ViolationEvent> m_violations;
    int m_maxStorageSize;
    bool m_isCircularBuffer;

    qint64 m_firstDataTime;
    qint64 m_lastDataTime;
};

}

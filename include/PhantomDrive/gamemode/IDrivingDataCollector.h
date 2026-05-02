#pragma once

#include "PhantomDrive_global.h"
#include "DrivingData.h"

#include <QObject>
#include <QVariant>

namespace PhantomDrive {

class PHANTOMDRIVE_EXPORT IDrivingDataCollector : public QObject
{
    Q_OBJECT

public:
    explicit IDrivingDataCollector(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IDrivingDataCollector() = default;

    virtual void startCollection() = 0;
    virtual void stopCollection() = 0;
    virtual bool isCollecting() const = 0;

    virtual DrivingData getCurrentData() const = 0;
    virtual QList<DrivingData> getCollectedData() const = 0;
    virtual QList<ViolationEvent> getViolations() const = 0;

    virtual void clearData() = 0;

signals:
    void dataCollected(const DrivingData& data);
    void violationDetected(const ViolationEvent& violation);
    void collectionStarted();
    void collectionStopped();
    void dataBatchReady(const QList<DrivingData>& batch);

protected:
    void emitDataCollected(const DrivingData& data) {
        emit dataCollected(data);
    }

    void emitViolationDetected(const ViolationEvent& violation) {
        emit violationDetected(violation);
    }

    void emitDataBatchReady(const QList<DrivingData>& batch) {
        emit dataBatchReady(batch);
    }
};

}

#pragma once

#include "PhantomDrive_global.h"
#include "collectible/BlockerVehicle.h"

#include <QList>
#include <QRectF>

namespace PhantomDrive {

class TrackData;

class PHANTOMDRIVE_EXPORT BlockerVehicleManager
{
public:
    BlockerVehicleManager() = default;

    void setTrack(const TrackData* track);
    void resetForCoinChallenge();
    void clear();
    void update(qreal deltaSeconds, int runCoins, int remainingSeconds, qint64 elapsedMs);

    QList<BlockerVehicle> activeVehicles() const;
    QList<QRectF> activeVehicleBounds() const;
    QList<QRectF> allVehicleBounds() const;
    int activeStage() const { return m_activeStage; }

private:
    bool buildCounterClockwiseLoop();
    QVector2D sampleLoopPosition(qreal distanceAlongLoop, QVector2D* tangentOut = nullptr, int* segmentIndexOut = nullptr) const;
    void updateVehiclePose(BlockerVehicle& vehicle) const;
    void buildVehicleLayout();

    const TrackData* m_track = nullptr;
    QList<QVector2D> m_loopWaypoints;
    QList<qreal> m_loopSegmentLengths;
    qreal m_loopLength = 0.0;
    QList<BlockerVehicle> m_vehicles;
    int m_activeStage = 0;
};

} // namespace PhantomDrive

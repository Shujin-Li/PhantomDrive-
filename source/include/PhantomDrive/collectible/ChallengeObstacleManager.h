#pragma once

#include "PhantomDrive_global.h"
#include "collectible/ChallengeObstacle.h"

#include <QList>
#include <QRectF>

namespace PhantomDrive {

class TrackData;

class PHANTOMDRIVE_EXPORT ChallengeObstacleManager
{
public:
    ChallengeObstacleManager() = default;

    void setTrack(const TrackData* track);
    void resetForCoinChallenge();
    void clear();
    void updateProgress(int runCoins, int remainingSeconds, qint64 elapsedMs);

    QList<ChallengeObstacle> activeObstacles() const;
    QList<QRectF> activeObstacleBounds() const;
    QList<QRectF> allObstacleBounds() const;
    int activeStage() const { return m_activeStage; }

private:
    void buildObstacleLayout();

    const TrackData* m_track = nullptr;
    QList<ChallengeObstacle> m_obstacles;
    int m_activeStage = 0;
};

} // namespace PhantomDrive

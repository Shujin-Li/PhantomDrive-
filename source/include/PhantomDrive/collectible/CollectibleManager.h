#pragma once

#include "PhantomDrive_global.h"
#include "collectible/CoinItem.h"

#include <QObject>
#include <QList>
#include <QRectF>

namespace PhantomDrive {

class TrackData;
class CurrencyManager;

class PHANTOMDRIVE_EXPORT CollectibleManager : public QObject
{
    Q_OBJECT

public:
    explicit CollectibleManager(QObject* parent = nullptr);

    void setTrack(const TrackData* track);
    void setBlockedAreas(const QList<QRectF>& blockedAreas);
    void resetForCoinChallenge();
    void clear();
    bool update(const QRectF& playerCollectRect,
                const QPointF& playerPosition,
                qreal deltaSeconds,
                CurrencyManager& currency,
                bool magnetActive = false,
                qreal magnetRadius = 0.0,
                qreal magnetPullSpeed = 0.0);

    QList<CoinItem> coins() const { return m_coins; }
    QList<CoinItem> collectedCoinsThisFrame() const { return m_collectedCoinsThisFrame; }
    QList<QPointF> routeGuidePoints() const { return m_routeGuidePoints; }
    int runCoins() const { return m_runCoins; }
    int activeCoinCount() const;
    bool hasActiveCoins() const;
    void addRunCoins(int amount, CurrencyManager& currency);
    void setTargetActiveCoinCount(int count);

private:
    QList<QPointF> buildRouteGuidePoints() const;
    QList<QPointF> buildCandidatePoints() const;
    QPointF snapToNearestDrivableTile(const QPointF& point) const;
    bool tryRespawnCoin(CoinItem& coin, const QPointF& playerPosition);
    int findNearestRoutePointIndex(const QPointF& point, qreal maxDistance) const;
    void reserveRouteSegmentAround(int routePointIndex, qreal cooldownSeconds);
    bool isBlockedByHazard(const QPointF& point, qreal radius) const;
    qreal randomRespawnDelaySeconds() const;

    const TrackData* m_track;
    QList<CoinItem> m_coins;
    QList<CoinItem> m_collectedCoinsThisFrame;
    QList<QPointF> m_routeGuidePoints;
    QList<qreal> m_routePointCooldowns;
    QList<QPointF> m_candidatePoints;
    QList<QPointF> m_recentCollectedPositions;
    QList<QRectF> m_blockedAreas;
    int m_runCoins;
    int m_targetActiveCoinCount;
    int m_routeSpawnCursor = 0;
    int m_randomSpawnCursor = 0;
};

} // namespace PhantomDrive

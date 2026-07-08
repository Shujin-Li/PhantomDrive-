#include "collectible/CollectibleManager.h"

#include "economy/CurrencyManager.h"
#include "track/Checkpoint.h"
#include "track/TrackData.h"
#include "track/TrackTile.h"

#include <QLineF>
#include <QRandomGenerator>
#include <QtMath>
#include <limits>

namespace PhantomDrive {

namespace {

constexpr qreal kCoinRadius = 12.0;
constexpr int kMinimumActiveCoinCount = 10;
constexpr int kInitialActiveCoinCount = 12;
constexpr int kMaximumActiveCoinCount = 16;
constexpr int kTargetCandidatePointCount = 72;
constexpr qreal kCandidateSpacing = 58.0;
constexpr qreal kRouteGuideSpacing = 104.0;
constexpr qreal kMinimumDistanceFromPlayer = 260.0;
constexpr qreal kMinimumDistanceBetweenCoins = 82.0;
constexpr qreal kRetryRespawnDelaySeconds = 0.35;
constexpr qreal kRespawnDelayMinSeconds = 3.0;
constexpr qreal kRespawnDelayMaxSeconds = 6.0;
constexpr qreal kRouteSegmentCooldownSeconds = 11.5;
constexpr qreal kRouteNeighborCooldownSeconds = 6.5;
constexpr qreal kRouteCoinRespawnDelayMinSeconds = 5.8;
constexpr qreal kRouteCoinRespawnDelayMaxSeconds = 8.8;
constexpr qreal kMinimumDistanceFromRecentCollected = 220.0;
constexpr int kRecentCollectedWindow = 8;
constexpr qreal kRouteCoinSpawnRatio = 0.82;

bool isDrivableTileType(TileType type)
{
    switch (type) {
    case TileType::Road:
    case TileType::Asphalt:
    case TileType::StartLine:
    case TileType::FinishLine:
        return true;
    default:
        return false;
    }
}

bool circleIntersectsRect(const QPointF& center, qreal radius, const QRectF& rect)
{
    const qreal closestX = qBound(rect.left(), center.x(), rect.right());
    const qreal closestY = qBound(rect.top(), center.y(), rect.bottom());
    const qreal dx = center.x() - closestX;
    const qreal dy = center.y() - closestY;
    return (dx * dx + dy * dy) <= radius * radius;
}

bool containsNearbyPoint(const QList<QPointF>& points, const QPointF& candidate, qreal minDistance)
{
    for (const QPointF& point : points) {
        if (QLineF(point, candidate).length() < minDistance) {
            return true;
        }
    }
    return false;
}

void shufflePoints(QList<QPointF>& points)
{
    if (points.size() < 2) {
        return;
    }

    for (int i = points.size() - 1; i > 0; --i) {
        const int j = QRandomGenerator::global()->bounded(i + 1);
        points.swapItemsAt(i, j);
    }
}

} // namespace

CollectibleManager::CollectibleManager(QObject* parent)
    : QObject(parent)
    , m_track(nullptr)
    , m_runCoins(0)
    , m_targetActiveCoinCount(kInitialActiveCoinCount)
{
}

void CollectibleManager::setTrack(const TrackData* track)
{
    m_track = track;
}

void CollectibleManager::setBlockedAreas(const QList<QRectF>& blockedAreas)
{
    m_blockedAreas = blockedAreas;
}

void CollectibleManager::resetForCoinChallenge()
{
    m_coins.clear();
    m_collectedCoinsThisFrame.clear();
    m_routeGuidePoints = buildRouteGuidePoints();
    m_routePointCooldowns.fill(0.0, m_routeGuidePoints.size());
    m_candidatePoints = buildCandidatePoints();
    m_recentCollectedPositions.clear();
    m_runCoins = 0;
    m_routeSpawnCursor = 0;
    m_randomSpawnCursor = 0;

    if (!m_track || m_candidatePoints.isEmpty()) {
        return;
    }

    const int coinPoolCount = qMin(kMaximumActiveCoinCount, m_candidatePoints.size());
    const int activeCoinCount = qMin(m_targetActiveCoinCount, coinPoolCount);
    const QPointF startPoint = m_track->getStartPosition().toPointF();

    for (int i = 0; i < coinPoolCount; ++i) {
        CoinItem coin;
        coin.radius = kCoinRadius;
        coin.value = 1;
        coin.phaseSeed = QRandomGenerator::global()->bounded(3600);
        coin.active = false;
        coin.collected = true;
        coin.respawnTimer = 0.0;

        if (i < activeCoinCount) {
            if (!tryRespawnCoin(coin, startPoint)) {
                const QPointF fallback = m_candidatePoints.at(i % m_candidatePoints.size());
                coin.position = fallback;
                coin.active = true;
                coin.collected = false;
                coin.respawnTimer = 0.0;
            }
        }

        m_coins.append(coin);
    }
}

void CollectibleManager::clear()
{
    m_coins.clear();
    m_collectedCoinsThisFrame.clear();
    m_routeGuidePoints.clear();
    m_routePointCooldowns.clear();
    m_candidatePoints.clear();
    m_recentCollectedPositions.clear();
    m_blockedAreas.clear();
    m_runCoins = 0;
    m_targetActiveCoinCount = kInitialActiveCoinCount;
    m_track = nullptr;
    m_routeSpawnCursor = 0;
    m_randomSpawnCursor = 0;
}

bool CollectibleManager::update(const QRectF& playerCollectRect,
                                const QPointF& playerPosition,
                                qreal deltaSeconds,
                                CurrencyManager& currency,
                                bool magnetActive,
                                qreal magnetRadius,
                                qreal magnetPullSpeed)
{
    bool changed = false;
    m_collectedCoinsThisFrame.clear();

    if (deltaSeconds > 0.0 && !m_routePointCooldowns.isEmpty()) {
        for (qreal& cooldown : m_routePointCooldowns) {
            cooldown = qMax<qreal>(0.0, cooldown - deltaSeconds);
        }
    }

    for (CoinItem& coin : m_coins) {
        if (!coin.active || coin.collected) {
            continue;
        }

        if (isBlockedByHazard(coin.position, coin.radius + 4.0)) {
            coin.active = false;
            coin.collected = true;
            coin.respawnTimer = kRetryRespawnDelaySeconds;
            coin.phaseSeed = QRandomGenerator::global()->bounded(3600);
            changed = true;
            continue;
        }

        if (magnetActive && deltaSeconds > 0.0 && magnetRadius > 0.0 && magnetPullSpeed > 0.0) {
            const QLineF magnetLine(coin.position, playerPosition);
            const qreal distanceToPlayer = magnetLine.length();
            if (distanceToPlayer > 0.001 && distanceToPlayer <= magnetRadius) {
                const qreal pullRatio = 1.0 - qBound<qreal>(0.0, distanceToPlayer / magnetRadius, 1.0);
                const qreal pullStep = qMin(distanceToPlayer,
                                            magnetPullSpeed * deltaSeconds * (0.72 + pullRatio * 0.78));
                const QPointF direction((playerPosition.x() - coin.position.x()) / distanceToPlayer,
                                        (playerPosition.y() - coin.position.y()) / distanceToPlayer);
                coin.position += direction * pullStep;
                changed = true;
            }
        }

        if (!circleIntersectsRect(coin.position, coin.radius, playerCollectRect)) {
            continue;
        }

        CoinItem collectedCoin = coin;
        const int routePointIndex = findNearestRoutePointIndex(coin.position, kRouteGuideSpacing * 0.72);
        coin.active = false;
        coin.collected = true;
        coin.respawnTimer = randomRespawnDelaySeconds();
        if (routePointIndex >= 0) {
            reserveRouteSegmentAround(routePointIndex, kRouteSegmentCooldownSeconds);
            coin.respawnTimer = qMax(coin.respawnTimer,
                                     kRouteCoinRespawnDelayMinSeconds
                                         + QRandomGenerator::global()->generateDouble()
                                               * (kRouteCoinRespawnDelayMaxSeconds
                                                  - kRouteCoinRespawnDelayMinSeconds));
        }
        coin.phaseSeed = QRandomGenerator::global()->bounded(3600);
        m_recentCollectedPositions.append(coin.position);
        m_collectedCoinsThisFrame.append(collectedCoin);
        while (m_recentCollectedPositions.size() > kRecentCollectedWindow) {
            m_recentCollectedPositions.removeFirst();
        }

        const int value = qMax(1, coin.value);
        m_runCoins += value;
        currency.addCoins(value);
        changed = true;
    }

    if (deltaSeconds <= 0.0) {
        return changed;
    }

    const int desiredActiveCoinCount = qMin(m_targetActiveCoinCount, m_coins.size());
    int activeCoinCount = this->activeCoinCount();

    for (CoinItem& coin : m_coins) {
        if (coin.active) {
            continue;
        }

        if (activeCoinCount >= desiredActiveCoinCount) {
            continue;
        }

        coin.respawnTimer -= deltaSeconds;
        if (coin.respawnTimer > 0.0) {
            continue;
        }

        if (tryRespawnCoin(coin, playerPosition)) {
            changed = true;
            ++activeCoinCount;
        } else {
            coin.respawnTimer = kRetryRespawnDelaySeconds;
        }
    }

    return changed;
}

int CollectibleManager::activeCoinCount() const
{
    int count = 0;
    for (const CoinItem& coin : m_coins) {
        if (coin.active && !coin.collected) {
            ++count;
        }
    }
    return count;
}

bool CollectibleManager::hasActiveCoins() const
{
    return activeCoinCount() > 0;
}

void CollectibleManager::addRunCoins(int amount, CurrencyManager& currency)
{
    const int coinAmount = qMax(0, amount);
    if (coinAmount <= 0) {
        return;
    }

    m_runCoins += coinAmount;
    currency.addCoins(coinAmount);
}

void CollectibleManager::setTargetActiveCoinCount(int count)
{
    const int maxActiveCoinCount = m_candidatePoints.isEmpty()
        ? kMaximumActiveCoinCount
        : qMin(kMaximumActiveCoinCount, m_candidatePoints.size());
    const int minActiveCoinCount = qMin(kMinimumActiveCoinCount, maxActiveCoinCount);
    m_targetActiveCoinCount = qBound(minActiveCoinCount, count, maxActiveCoinCount);
}

QList<QPointF> CollectibleManager::buildRouteGuidePoints() const
{
    QList<QPointF> routePoints;
    if (!m_track) {
        return routePoints;
    }

    QList<QPointF> anchors;
    anchors.append(m_track->getStartPosition().toPointF());

    const QList<Checkpoint*> checkpoints = m_track->getCheckpointsInOrder();
    for (const Checkpoint* checkpoint : checkpoints) {
        if (checkpoint) {
            anchors.append(checkpoint->getPosition().toPointF());
        }
    }

    if (anchors.size() < 2) {
        for (const QVector2D& itemBox : m_track->getItemBoxPositions()) {
            anchors.append(itemBox.toPointF());
            if (anchors.size() >= 3) {
                break;
            }
        }
    }

    if (anchors.size() < 2) {
        return routePoints;
    }

    auto appendGuidePoint = [&routePoints, this](const QPointF& point) {
        const QPointF snapped = snapToNearestDrivableTile(point);
        if (containsNearbyPoint(routePoints, snapped, kRouteGuideSpacing * 0.55)) {
            return;
        }
        routePoints.append(snapped);
    };

    for (int i = 0; i < anchors.size(); ++i) {
        const QPointF from = anchors.at(i);
        const QPointF to = anchors.at((i + 1) % anchors.size());
        appendGuidePoint(from);

        const QLineF segment(from, to);
        if (segment.length() < 1.0) {
            continue;
        }

        const int sampleCount = qMax(1, qFloor(segment.length() / kRouteGuideSpacing));
        for (int sample = 1; sample < sampleCount; ++sample) {
            const qreal t = static_cast<qreal>(sample) / static_cast<qreal>(sampleCount);
            appendGuidePoint(QPointF(from.x() + (to.x() - from.x()) * t,
                                     from.y() + (to.y() - from.y()) * t));
        }
    }

    if (routePoints.size() >= 2
        && QLineF(routePoints.constFirst(), routePoints.constLast()).length() > kRouteGuideSpacing * 0.55) {
        routePoints.append(routePoints.constFirst());
    }

    return routePoints;
}

QList<QPointF> CollectibleManager::buildCandidatePoints() const
{
    QList<QPointF> candidates;
    if (!m_track) {
        return candidates;
    }

    auto appendCandidate = [&candidates, this](const QPointF& point) {
        const QPointF snapped = snapToNearestDrivableTile(point);
        if (containsNearbyPoint(candidates, snapped, kCandidateSpacing)) {
            return;
        }
        candidates.append(snapped);
    };

    for (const QPointF& routePoint : m_routeGuidePoints) {
        appendCandidate(routePoint);
        if (candidates.size() >= kTargetCandidatePointCount) {
            return candidates;
        }
    }

    const QList<Checkpoint*> checkpoints = m_track->getCheckpointsInOrder();
    for (const Checkpoint* checkpoint : checkpoints) {
        if (checkpoint) {
            appendCandidate(checkpoint->getPosition().toPointF());
        }
    }

    for (const QVector2D& itemBox : m_track->getItemBoxPositions()) {
        appendCandidate(itemBox.toPointF());
    }

    QList<QPointF> roadCenters;
    const QList<QList<TrackTile*>> tiles = m_track->getTiles();
    for (int row = 0; row < tiles.size(); ++row) {
        const QList<TrackTile*>& rowTiles = tiles.at(row);
        for (int col = 0; col < rowTiles.size(); ++col) {
            const TrackTile* tile = rowTiles.at(col);
            if (!tile || !isDrivableTileType(tile->getType())) {
                continue;
            }
            roadCenters.append(TrackData::tileToWorldCenter(row, col).toPointF());
        }
    }

    shufflePoints(roadCenters);
    for (const QPointF& center : roadCenters) {
        appendCandidate(center);
        if (candidates.size() >= kTargetCandidatePointCount) {
            break;
        }
    }

    return candidates;
}

QPointF CollectibleManager::snapToNearestDrivableTile(const QPointF& point) const
{
    if (!m_track) {
        return point;
    }

    QPointF bestPoint = point;
    qreal bestDistanceSquared = std::numeric_limits<qreal>::max();

    const QList<QList<TrackTile*>> tiles = m_track->getTiles();
    for (int row = 0; row < tiles.size(); ++row) {
        const QList<TrackTile*>& rowTiles = tiles.at(row);
        for (int col = 0; col < rowTiles.size(); ++col) {
            const TrackTile* tile = rowTiles.at(col);
            if (!tile || !isDrivableTileType(tile->getType())) {
                continue;
            }

            const QPointF center = TrackData::tileToWorldCenter(row, col).toPointF();
            const qreal dx = center.x() - point.x();
            const qreal dy = center.y() - point.y();
            const qreal distanceSquared = dx * dx + dy * dy;
            if (distanceSquared < bestDistanceSquared) {
                bestDistanceSquared = distanceSquared;
                bestPoint = center;
            }
        }
    }

    return bestPoint;
}

bool CollectibleManager::tryRespawnCoin(CoinItem& coin, const QPointF& playerPosition)
{
    if (m_candidatePoints.isEmpty()) {
        return false;
    }

    auto tryFromPoints = [&](const QList<QPointF>& points, int& cursor, bool sequential) {
        if (points.isEmpty()) {
            return false;
        }

        const int startIndex = sequential
            ? qBound(0, cursor, points.size() - 1)
            : QRandomGenerator::global()->bounded(points.size());
        for (int offset = 0; offset < points.size(); ++offset) {
            const int pointIndex = (startIndex + offset) % points.size();
            const QPointF candidate = points.at(pointIndex);
            const int routePointIndex = findNearestRoutePointIndex(candidate, kRouteGuideSpacing * 0.45);

            if (routePointIndex >= 0
                && routePointIndex < m_routePointCooldowns.size()
                && m_routePointCooldowns.at(routePointIndex) > 0.0) {
                continue;
            }

            if (QLineF(candidate, playerPosition).length() < kMinimumDistanceFromPlayer) {
                continue;
            }

            if (containsNearbyPoint(m_recentCollectedPositions, candidate, kMinimumDistanceFromRecentCollected)) {
                continue;
            }

            if (isBlockedByHazard(candidate, coin.radius + 6.0)) {
                continue;
            }

            bool overlapsOtherCoin = false;
            for (const CoinItem& other : m_coins) {
                if (&other == &coin || !other.active || other.collected) {
                    continue;
                }
                if (QLineF(candidate, other.position).length() < kMinimumDistanceBetweenCoins) {
                    overlapsOtherCoin = true;
                    break;
                }
            }

            if (overlapsOtherCoin) {
                continue;
            }

            coin.position = candidate;
            coin.active = true;
            coin.collected = false;
            coin.respawnTimer = 0.0;
            if (coin.phaseSeed <= 0) {
                coin.phaseSeed = QRandomGenerator::global()->bounded(3600);
            }

            cursor = (pointIndex + 1) % points.size();
            return true;
        }

        return false;
    };

    const bool preferRoute = !m_routeGuidePoints.isEmpty()
        && QRandomGenerator::global()->generateDouble() < kRouteCoinSpawnRatio;

    if (preferRoute && tryFromPoints(m_routeGuidePoints, m_routeSpawnCursor, true)) {
        return true;
    }
    if (tryFromPoints(m_candidatePoints, m_randomSpawnCursor, false)) {
        return true;
    }
    if (!preferRoute && tryFromPoints(m_routeGuidePoints, m_routeSpawnCursor, true)) {
        return true;
    }

    return false;
}

int CollectibleManager::findNearestRoutePointIndex(const QPointF& point, qreal maxDistance) const
{
    if (m_routeGuidePoints.isEmpty()) {
        return -1;
    }

    const qreal maxDistanceSquared = maxDistance * maxDistance;
    qreal bestDistanceSquared = maxDistanceSquared;
    int bestIndex = -1;
    for (int i = 0; i < m_routeGuidePoints.size(); ++i) {
        const QPointF& routePoint = m_routeGuidePoints.at(i);
        const qreal dx = routePoint.x() - point.x();
        const qreal dy = routePoint.y() - point.y();
        const qreal distanceSquared = dx * dx + dy * dy;
        if (distanceSquared <= bestDistanceSquared) {
            bestDistanceSquared = distanceSquared;
            bestIndex = i;
        }
    }

    return bestIndex;
}

void CollectibleManager::reserveRouteSegmentAround(int routePointIndex, qreal cooldownSeconds)
{
    if (routePointIndex < 0 || routePointIndex >= m_routePointCooldowns.size()) {
        return;
    }

    const int routePointCount = m_routePointCooldowns.size();
    for (int offset = -1; offset <= 3; ++offset) {
        const int index = (routePointIndex + offset + routePointCount) % routePointCount;
        const qreal cooldown = offset == 0 || offset == 1
            ? cooldownSeconds
            : kRouteNeighborCooldownSeconds;
        m_routePointCooldowns[index] = qMax(m_routePointCooldowns.at(index), cooldown);
    }
}

bool CollectibleManager::isBlockedByHazard(const QPointF& point, qreal radius) const
{
    for (const QRectF& rect : m_blockedAreas) {
        if (circleIntersectsRect(point, radius, rect)) {
            return true;
        }
    }
    return false;
}

qreal CollectibleManager::randomRespawnDelaySeconds() const
{
    return kRespawnDelayMinSeconds
        + QRandomGenerator::global()->generateDouble()
              * (kRespawnDelayMaxSeconds - kRespawnDelayMinSeconds);
}

} // namespace PhantomDrive

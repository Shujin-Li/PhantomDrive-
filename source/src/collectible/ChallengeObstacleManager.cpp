#include "collectible/ChallengeObstacleManager.h"

#include "track/Checkpoint.h"
#include "track/TrackData.h"
#include "track/TrackTile.h"

#include <QLineF>
#include <QRandomGenerator>
#include <QtMath>

namespace PhantomDrive {

namespace {

struct ObstacleCandidate {
    QVector2D position;
    QVector2D edgeOffset;
};

bool isDrivableObstacleTile(TileType type)
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

QVector2D normalizedOrZero(const QVector2D& value)
{
    if (value.lengthSquared() > 0.001f) {
        return value.normalized();
    }
    return QVector2D();
}

ObstacleCandidate makeNaturalCandidate(const TrackData* track, int row, int col)
{
    ObstacleCandidate candidate;
    candidate.position = TrackData::tileToWorldCenter(row, col);
    if (!track) {
        return candidate;
    }

    auto roadAt = [track](int r, int c) {
        const TrackTile* tile = track->getTileAt(r, c);
        return tile && isDrivableObstacleTile(tile->getType());
    };

    QVector2D edge;
    if (!roadAt(row, col - 1)) {
        edge += QVector2D(-1.0f, 0.0f);
    }
    if (!roadAt(row, col + 1)) {
        edge += QVector2D(1.0f, 0.0f);
    }
    if (!roadAt(row - 1, col)) {
        edge += QVector2D(0.0f, -1.0f);
    }
    if (!roadAt(row + 1, col)) {
        edge += QVector2D(0.0f, 1.0f);
    }

    if (edge.lengthSquared() <= 0.001f) {
        const bool vertical = roadAt(row - 1, col) || roadAt(row + 1, col);
        edge = vertical ? QVector2D((QRandomGenerator::global()->bounded(2) == 0) ? -1.0f : 1.0f, 0.0f)
                        : QVector2D(0.0f, (QRandomGenerator::global()->bounded(2) == 0) ? -1.0f : 1.0f);
    }

    candidate.edgeOffset = normalizedOrZero(edge);
    return candidate;
}

bool intersectsAnyRect(const QRectF& rect, const QList<QRectF>& bounds, qreal padding)
{
    for (const QRectF& existing : bounds) {
        if (rect.adjusted(-padding, -padding, padding, padding).intersects(existing)) {
            return true;
        }
    }
    return false;
}

} // namespace

void ChallengeObstacleManager::setTrack(const TrackData* track)
{
    m_track = track;
}

void ChallengeObstacleManager::resetForCoinChallenge()
{
    m_activeStage = 0;
    buildObstacleLayout();
}

void ChallengeObstacleManager::clear()
{
    m_track = nullptr;
    m_obstacles.clear();
    m_activeStage = 0;
}

void ChallengeObstacleManager::updateProgress(int runCoins, int remainingSeconds, qint64 elapsedMs)
{
    int nextStage = 0;
    if (elapsedMs >= 16000 || runCoins >= 18) {
        nextStage = 1;
    }
    if (elapsedMs >= 36000 || runCoins >= 38 || remainingSeconds <= 24) {
        nextStage = 2;
    }
    m_activeStage = qMax(m_activeStage, nextStage);
}

QList<ChallengeObstacle> ChallengeObstacleManager::activeObstacles() const
{
    QList<ChallengeObstacle> result;
    for (const ChallengeObstacle& obstacle : m_obstacles) {
        if (obstacle.stageRequired <= m_activeStage) {
            result.append(obstacle);
        }
    }
    return result;
}

QList<QRectF> ChallengeObstacleManager::activeObstacleBounds() const
{
    QList<QRectF> bounds;
    for (const ChallengeObstacle& obstacle : activeObstacles()) {
        bounds.append(obstacle.bounds());
    }
    return bounds;
}

QList<QRectF> ChallengeObstacleManager::allObstacleBounds() const
{
    QList<QRectF> bounds;
    for (const ChallengeObstacle& obstacle : m_obstacles) {
        bounds.append(obstacle.bounds());
    }
    return bounds;
}

void ChallengeObstacleManager::buildObstacleLayout()
{
    m_obstacles.clear();
    if (!m_track) {
        return;
    }

    QList<ObstacleCandidate> candidates;
    const QVector2D start = m_track->getStartPosition();

    auto appendCandidate = [&](const ObstacleCandidate& candidate) {
        if ((candidate.position - start).length() < 300.0f) {
            return;
        }
        for (const ObstacleCandidate& existing : candidates) {
            if ((existing.position - candidate.position).length() < 170.0f) {
                return;
            }
        }
        candidates.append(candidate);
    };

    for (const Checkpoint* checkpoint : m_track->getCheckpointsInOrder()) {
        if (checkpoint) {
            const QPoint tile = TrackData::worldToTile(checkpoint->getPosition());
            appendCandidate(makeNaturalCandidate(m_track, tile.y(), tile.x()));
        }
    }

    for (const QVector2D& itemBox : m_track->getItemBoxPositions()) {
        const QPoint tile = TrackData::worldToTile(itemBox);
        appendCandidate(makeNaturalCandidate(m_track, tile.y(), tile.x()));
    }

    const QList<QList<TrackTile*>> tiles = m_track->getTiles();
    for (int row = 0; row < tiles.size(); ++row) {
        const QList<TrackTile*>& rowTiles = tiles.at(row);
        for (int col = 0; col < rowTiles.size(); ++col) {
            const TrackTile* tile = rowTiles.at(col);
            if (!tile || !isDrivableObstacleTile(tile->getType())) {
                continue;
            }
            if (((row + col) % 7) != 0) {
                continue;
            }
            appendCandidate(makeNaturalCandidate(m_track, row, col));
        }
    }

    if (candidates.isEmpty()) {
        appendCandidate({start + QVector2D(260.0f, 200.0f), QVector2D(-1.0f, 0.0f)});
        appendCandidate({start + QVector2D(-220.0f, 420.0f), QVector2D(1.0f, 0.0f)});
        appendCandidate({start + QVector2D(120.0f, 680.0f), QVector2D(0.0f, -1.0f)});
        appendCandidate({start + QVector2D(460.0f, 580.0f), QVector2D(-1.0f, 0.0f)});
    }

    for (int i = candidates.size() - 1; i > 0; --i) {
        const int j = QRandomGenerator::global()->bounded(i + 1);
        candidates.swapItemsAt(i, j);
    }

    QList<QRectF> occupiedBounds;
    const QList<ChallengeObstacleType> types = {
        ChallengeObstacleType::Tree,
        ChallengeObstacleType::ConstructionWall,
        ChallengeObstacleType::ConstructionBarrel,
        ChallengeObstacleType::Rock,
        ChallengeObstacleType::ConstructionWall
    };

    for (int i = 0; i < qMin(types.size(), candidates.size()); ++i) {
        ChallengeObstacle obstacle;
        obstacle.id = QStringLiteral("challenge_obstacle_%1").arg(i + 1);
        obstacle.type = types.at(i);
        const ObstacleCandidate candidate = candidates.at(i);
        obstacle.stageRequired = 0;
        obstacle.rotation = (i % 2 == 0) ? 0.0 : 90.0;
        QVector2D edgeOffset = candidate.edgeOffset;
        if (edgeOffset.lengthSquared() <= 0.001f) {
            edgeOffset = QVector2D((QRandomGenerator::global()->bounded(2) == 0) ? -1.0f : 1.0f, 0.0f);
        }

        switch (obstacle.type) {
        case ChallengeObstacleType::ConstructionWall:
            obstacle.size = QSizeF(56.0, 18.0);
            obstacle.collisionSize = QSizeF(40.0, 12.0);
            obstacle.circular = false;
            obstacle.position = candidate.position + edgeOffset * 12.0f;
            if (qAbs(edgeOffset.x()) > qAbs(edgeOffset.y())) {
                obstacle.rotation = 90.0;
            }
            break;
        case ChallengeObstacleType::Tree:
            obstacle.circular = true;
            obstacle.radius = 19.0 + QRandomGenerator::global()->bounded(3.5);
            obstacle.collisionRadius = 13.0 + QRandomGenerator::global()->bounded(2.5);
            obstacle.size = QSizeF(obstacle.radius * 2.1, obstacle.radius * 2.0);
            obstacle.position = candidate.position + edgeOffset * (10.0f + QRandomGenerator::global()->bounded(4.0f));
            break;
        case ChallengeObstacleType::Rock:
            obstacle.circular = true;
            obstacle.radius = 16.0;
            obstacle.collisionRadius = 12.5;
            obstacle.size = QSizeF(32.0, 28.0);
            obstacle.position = candidate.position + edgeOffset * 18.0f;
            break;
        case ChallengeObstacleType::ConstructionBarrel:
        case ChallengeObstacleType::ConeBarrier:
            obstacle.circular = true;
            obstacle.radius = 14.0;
            obstacle.collisionRadius = 11.0;
            obstacle.size = QSizeF(28.0, 28.0);
            obstacle.position = candidate.position + edgeOffset * 10.0f;
            break;
        }

        const QRectF bounds = obstacle.bounds();
        if (intersectsAnyRect(bounds, occupiedBounds, 26.0)) {
            continue;
        }

        occupiedBounds.append(bounds);
        m_obstacles.append(obstacle);
    }
}

} // namespace PhantomDrive

#include "collectible/BlockerVehicleManager.h"

#include "track/Checkpoint.h"
#include "track/TrackData.h"
#include "track/TrackTile.h"

#include <QPoint>
#include <QQueue>
#include <QSet>
#include <QVector>
#include <QtMath>
#include <cmath>
#include <limits>

namespace PhantomDrive {

namespace {

bool isDriveableBlockerTile(TileType type)
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

bool isValidTrackTileCoord(const TrackData* track, const QPoint& tileCoord)
{
    return track
        && tileCoord.y() >= 0
        && tileCoord.y() < track->getRowCount()
        && tileCoord.x() >= 0
        && tileCoord.x() < track->getColCount();
}

int blockerTileKey(const TrackData* track, const QPoint& tileCoord)
{
    return tileCoord.y() * track->getColCount() + tileCoord.x();
}

QVector2D normalizedOrFallback(const QVector2D& vector, const QVector2D& fallback = QVector2D(0.0f, -1.0f))
{
    if (vector.lengthSquared() > 0.001f) {
        return vector.normalized();
    }
    return fallback.normalized();
}

QPoint findNearestDriveableBlockerTile(const TrackData* track, const QVector2D& worldPos)
{
    if (!track) {
        return QPoint(-1, -1);
    }

    const QPoint tileCoord = TrackData::worldToTile(worldPos);
    if (isValidTrackTileCoord(track, tileCoord)) {
        const TrackTile* tile = track->getTileAt(tileCoord.y(), tileCoord.x());
        if (tile && isDriveableBlockerTile(tile->getType())) {
            return tileCoord;
        }
    }

    QPoint bestTile(-1, -1);
    qreal bestDistanceSq = std::numeric_limits<qreal>::max();
    for (int row = 0; row < track->getRowCount(); ++row) {
        for (int col = 0; col < track->getColCount(); ++col) {
            const TrackTile* tile = track->getTileAt(row, col);
            if (!tile || !isDriveableBlockerTile(tile->getType())) {
                continue;
            }

            const qreal distanceSq = (TrackData::tileToWorldCenter(row, col) - worldPos).lengthSquared();
            if (distanceSq < bestDistanceSq) {
                bestDistanceSq = distanceSq;
                bestTile = QPoint(col, row);
            }
        }
    }

    return bestTile;
}

QList<QPoint> blockerCheckpointGoalTiles(const TrackData* track, const Checkpoint* checkpoint)
{
    QList<QPoint> goals;
    if (!track || !checkpoint) {
        return goals;
    }

    const QRectF checkpointBounds = checkpoint->getBounds();
    for (int row = 0; row < track->getRowCount(); ++row) {
        for (int col = 0; col < track->getColCount(); ++col) {
            const TrackTile* tile = track->getTileAt(row, col);
            if (!tile
                || !isDriveableBlockerTile(tile->getType())
                || !tile->getBounds().intersects(checkpointBounds)) {
                continue;
            }
            goals.append(QPoint(col, row));
        }
    }

    if (goals.isEmpty()) {
        const QPoint fallbackTile = findNearestDriveableBlockerTile(track, checkpoint->getPosition());
        if (fallbackTile.x() >= 0) {
            goals.append(fallbackTile);
        }
    }

    return goals;
}

QList<QPoint> blockerFinishGoalTiles(const TrackData* track)
{
    QList<QPoint> goals;
    if (!track) {
        return goals;
    }

    for (int row = 0; row < track->getRowCount(); ++row) {
        for (int col = 0; col < track->getColCount(); ++col) {
            const TrackTile* tile = track->getTileAt(row, col);
            if (tile && tile->getType() == TileType::FinishLine) {
                goals.append(QPoint(col, row));
            }
        }
    }

    if (goals.isEmpty()) {
        const QPoint fallbackTile = findNearestDriveableBlockerTile(track, track->getStartPosition());
        if (fallbackTile.x() >= 0) {
            goals.append(fallbackTile);
        }
    }

    return goals;
}

QList<QPoint> findBlockerTilePath(const TrackData* track,
                                  const QPoint& start,
                                  const QList<QPoint>& goals,
                                  QPoint* reachedGoal = nullptr)
{
    QList<QPoint> path;
    if (reachedGoal) {
        *reachedGoal = QPoint(-1, -1);
    }

    if (!track
        || !isValidTrackTileCoord(track, start)
        || !track->getTileAt(start.y(), start.x())
        || !isDriveableBlockerTile(track->getTileAt(start.y(), start.x())->getType())) {
        return path;
    }

    QSet<int> goalKeys;
    for (const QPoint& goal : goals) {
        if (!isValidTrackTileCoord(track, goal)) {
            continue;
        }
        const TrackTile* tile = track->getTileAt(goal.y(), goal.x());
        if (tile && isDriveableBlockerTile(tile->getType())) {
            goalKeys.insert(blockerTileKey(track, goal));
        }
    }
    if (goalKeys.isEmpty()) {
        return path;
    }

    if (goalKeys.contains(blockerTileKey(track, start))) {
        if (reachedGoal) {
            *reachedGoal = start;
        }
        path.append(start);
        return path;
    }

    QVector<QVector<bool>> visited(track->getRowCount(), QVector<bool>(track->getColCount(), false));
    QVector<QVector<QPoint>> parents(track->getRowCount(),
                                     QVector<QPoint>(track->getColCount(), QPoint(-1, -1)));
    QQueue<QPoint> frontier;
    frontier.enqueue(start);
    visited[start.y()][start.x()] = true;

    const QPoint directions[] = {
        QPoint(1, 0),
        QPoint(-1, 0),
        QPoint(0, 1),
        QPoint(0, -1)
    };

    QPoint goalPoint(-1, -1);
    while (!frontier.isEmpty()) {
        const QPoint current = frontier.dequeue();
        for (const QPoint& direction : directions) {
            const QPoint next = current + direction;
            if (!isValidTrackTileCoord(track, next)
                || visited[next.y()][next.x()]) {
                continue;
            }

            const TrackTile* tile = track->getTileAt(next.y(), next.x());
            if (!tile || !isDriveableBlockerTile(tile->getType())) {
                continue;
            }

            visited[next.y()][next.x()] = true;
            parents[next.y()][next.x()] = current;

            if (goalKeys.contains(blockerTileKey(track, next))) {
                goalPoint = next;
                frontier.clear();
                break;
            }

            frontier.enqueue(next);
        }
    }

    if (goalPoint.x() < 0) {
        return path;
    }

    for (QPoint current = goalPoint; current.x() >= 0; current = parents[current.y()][current.x()]) {
        path.prepend(current);
        if (current == start) {
            break;
        }
    }

    if (path.isEmpty() || path.first() != start) {
        path.clear();
        return path;
    }

    if (reachedGoal) {
        *reachedGoal = goalPoint;
    }

    return path;
}

QRectF routeBounds(const BlockerVehicle& vehicle)
{
    const qreal halfW = vehicle.size.width() * 0.5;
    const qreal halfH = vehicle.size.height() * 0.5;
    const qreal minX = qMin(vehicle.pointA.x(), vehicle.pointB.x()) - halfW;
    const qreal maxX = qMax(vehicle.pointA.x(), vehicle.pointB.x()) + halfW;
    const qreal minY = qMin(vehicle.pointA.y(), vehicle.pointB.y()) - halfH;
    const qreal maxY = qMax(vehicle.pointA.y(), vehicle.pointB.y()) + halfH;
    return QRectF(QPointF(minX, minY), QPointF(maxX, maxY));
}

bool intersectsAnyRect(const QRectF& bounds, const QList<QRectF>& existing, qreal padding)
{
    for (const QRectF& rect : existing) {
        if (bounds.adjusted(-padding, -padding, padding, padding).intersects(rect)) {
            return true;
        }
    }
    return false;
}

} // namespace

bool BlockerVehicleManager::buildCounterClockwiseLoop()
{
    m_loopWaypoints.clear();
    m_loopSegmentLengths.clear();
    m_loopLength = 0.0;

    if (!m_track) {
        return false;
    }

    const QPoint startTile = findNearestDriveableBlockerTile(m_track, m_track->getStartPosition());
    if (startTile.x() < 0) {
        return false;
    }

    QList<QPoint> loopTiles;
    auto appendTile = [&loopTiles](const QPoint& tile) {
        if (!loopTiles.isEmpty() && loopTiles.constLast() == tile) {
            return;
        }
        loopTiles.append(tile);
    };
    appendTile(startTile);

    QPoint currentTile = startTile;
    QList<QList<QPoint>> segmentGoals;
    const QList<Checkpoint*> checkpoints = m_track->getCheckpointsInOrder();
    segmentGoals.reserve(checkpoints.size() + 1);
    for (const Checkpoint* checkpoint : checkpoints) {
        const QList<QPoint> goals = blockerCheckpointGoalTiles(m_track, checkpoint);
        if (!goals.isEmpty()) {
            segmentGoals.append(goals);
        }
    }

    const QList<QPoint> finishGoals = blockerFinishGoalTiles(m_track);
    if (!finishGoals.isEmpty()) {
        segmentGoals.append(finishGoals);
    }

    for (const QList<QPoint>& goals : segmentGoals) {
        QPoint reachedGoal(-1, -1);
        const QList<QPoint> tilePath = findBlockerTilePath(m_track, currentTile, goals, &reachedGoal);
        if (tilePath.isEmpty()) {
            continue;
        }

        for (int i = 1; i < tilePath.size(); ++i) {
            appendTile(tilePath.at(i));
        }
        currentTile = reachedGoal;
    }

    if (currentTile != startTile) {
        QPoint reachedGoal(-1, -1);
        const QList<QPoint> returnPath = findBlockerTilePath(m_track, currentTile, QList<QPoint>{startTile}, &reachedGoal);
        for (int i = 1; i < returnPath.size(); ++i) {
            appendTile(returnPath.at(i));
        }
    }

    if (loopTiles.size() < 6) {
        return false;
    }

    QList<QVector2D> loopPoints;
    loopPoints.reserve(loopTiles.size());
    for (const QPoint& tile : loopTiles) {
        const QVector2D point = TrackData::tileToWorldCenter(tile.y(), tile.x());
        if (!loopPoints.isEmpty() && (loopPoints.constLast() - point).length() < 8.0f) {
            continue;
        }
        loopPoints.append(point);
    }

    for (int index = 0; index < loopPoints.size(); ++index) {
        const QVector2D from = loopPoints.at(index);
        const QVector2D to = loopPoints.at((index + 1) % loopPoints.size());
        const qreal segmentLength = (to - from).length();
        if (segmentLength < 10.0) {
            continue;
        }
        m_loopWaypoints.append(from);
        m_loopSegmentLengths.append(segmentLength);
        m_loopLength += segmentLength;
    }

    return m_loopWaypoints.size() >= 3 && m_loopLength > 1.0;
}

QVector2D BlockerVehicleManager::sampleLoopPosition(qreal distanceAlongLoop,
                                                    QVector2D* tangentOut,
                                                    int* segmentIndexOut) const
{
    if (m_loopWaypoints.isEmpty() || m_loopSegmentLengths.isEmpty() || m_loopLength <= 0.001) {
        if (tangentOut) {
            *tangentOut = QVector2D(0.0f, -1.0f);
        }
        if (segmentIndexOut) {
            *segmentIndexOut = 0;
        }
        return QVector2D();
    }

    qreal wrappedDistance = std::fmod(distanceAlongLoop, m_loopLength);
    if (wrappedDistance < 0.0) {
        wrappedDistance += m_loopLength;
    }

    for (int segment = 0; segment < m_loopWaypoints.size(); ++segment) {
        const qreal segmentLength = m_loopSegmentLengths.at(segment);
        const QVector2D from = m_loopWaypoints.at(segment);
        const QVector2D to = m_loopWaypoints.at((segment + 1) % m_loopWaypoints.size());
        if (wrappedDistance <= segmentLength || segment == m_loopWaypoints.size() - 1) {
            const qreal t = segmentLength > 0.001 ? wrappedDistance / segmentLength : 0.0;
            if (tangentOut) {
                *tangentOut = normalizedOrFallback(to - from);
            }
            if (segmentIndexOut) {
                *segmentIndexOut = segment;
            }
            return from + (to - from) * static_cast<float>(t);
        }
        wrappedDistance -= segmentLength;
    }

    if (tangentOut) {
        *tangentOut = normalizedOrFallback(m_loopWaypoints.constFirst() - m_loopWaypoints.constLast());
    }
    if (segmentIndexOut) {
        *segmentIndexOut = 0;
    }
    return m_loopWaypoints.constFirst();
}

void BlockerVehicleManager::updateVehiclePose(BlockerVehicle& vehicle) const
{
    if (m_loopWaypoints.isEmpty() || m_loopLength <= 0.001) {
        return;
    }

    const qreal distanceAlongLoop = qBound<qreal>(0.0, vehicle.progress, 1.0) * m_loopLength;
    QVector2D tangent;
    int segmentIndex = 0;
    vehicle.position = sampleLoopPosition(distanceAlongLoop, &tangent, &segmentIndex);
    const QVector2D forward = normalizedOrFallback(tangent);
    const qreal halfLength = vehicle.size.height() * 0.46;
    vehicle.pointA = vehicle.position - forward * halfLength;
    vehicle.pointB = vehicle.position + forward * halfLength;
    vehicle.rotation = qRadiansToDegrees(qAtan2(forward.y(), forward.x())) + 90.0;
}

void BlockerVehicleManager::setTrack(const TrackData* track)
{
    m_track = track;
}

void BlockerVehicleManager::resetForCoinChallenge()
{
    m_activeStage = 0;
    buildVehicleLayout();
}

void BlockerVehicleManager::clear()
{
    m_track = nullptr;
    m_loopWaypoints.clear();
    m_loopSegmentLengths.clear();
    m_loopLength = 0.0;
    m_vehicles.clear();
    m_activeStage = 0;
}

void BlockerVehicleManager::update(qreal deltaSeconds, int runCoins, int remainingSeconds, qint64 elapsedMs)
{
    int nextStage = 0;
    if (elapsedMs >= 20000 || runCoins >= 22) {
        nextStage = 1;
    }
    if (elapsedMs >= 42000 || runCoins >= 44 || remainingSeconds <= 20) {
        nextStage = 2;
    }
    m_activeStage = qMax(m_activeStage, nextStage);

    if (deltaSeconds <= 0.0) {
        return;
    }

    if (m_loopLength <= 0.001) {
        return;
    }

    for (BlockerVehicle& vehicle : m_vehicles) {
        vehicle.progress += (vehicle.speed * deltaSeconds) / m_loopLength;
        vehicle.progress = std::fmod(vehicle.progress, 1.0);
        if (vehicle.progress < 0.0) {
            vehicle.progress += 1.0;
        }
        updateVehiclePose(vehicle);
    }
}

QList<BlockerVehicle> BlockerVehicleManager::activeVehicles() const
{
    QList<BlockerVehicle> result;
    for (const BlockerVehicle& vehicle : m_vehicles) {
        if (vehicle.stageRequired <= m_activeStage) {
            result.append(vehicle);
        }
    }
    return result;
}

QList<QRectF> BlockerVehicleManager::activeVehicleBounds() const
{
    QList<QRectF> bounds;
    for (const BlockerVehicle& vehicle : activeVehicles()) {
        bounds.append(vehicle.bounds());
    }
    return bounds;
}

QList<QRectF> BlockerVehicleManager::allVehicleBounds() const
{
    QList<QRectF> bounds;
    for (const BlockerVehicle& vehicle : m_vehicles) {
        bounds.append(vehicle.bounds());
    }
    return bounds;
}

void BlockerVehicleManager::buildVehicleLayout()
{
    m_vehicles.clear();
    m_loopWaypoints.clear();
    m_loopSegmentLengths.clear();
    m_loopLength = 0.0;
    if (!m_track || !buildCounterClockwiseLoop()) {
        return;
    }

    QList<QRectF> occupiedBounds;
    const QList<qreal> loopOffsets = {0.18, 0.56};
    const QList<qreal> trafficSpeeds = {46.0, 54.0};
    const int vehicleCount = loopOffsets.size();
    for (int i = 0; i < vehicleCount; ++i) {
        BlockerVehicle vehicle;
        vehicle.id = QStringLiteral("coin_challenge_blocker_%1").arg(i + 1);
        vehicle.size = QSizeF(24.0, 40.0);
        vehicle.collisionSize = QSizeF(18.0, 30.0);
        vehicle.stageRequired = i == 0 ? 0 : 1;
        vehicle.speed = trafficSpeeds.at(i);
        vehicle.progress = loopOffsets.at(i);
        vehicle.forward = true;
        updateVehiclePose(vehicle);

        const QRectF bounds = routeBounds(vehicle);
        if (intersectsAnyRect(bounds, occupiedBounds, 64.0)) {
            continue;
        }

        occupiedBounds.append(bounds);
        m_vehicles.append(vehicle);
    }
}

} // namespace PhantomDrive

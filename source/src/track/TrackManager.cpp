#include "TrackManager.h"

#include <QDebug>
#include <QPoint>
#include <QQueue>
#include <QSet>
#include <QVector>
#include <QtMath>
#include <limits>

namespace PhantomDrive {

namespace {

bool isAiWaypointTile(const TrackTile* tile)
{
    if (!tile) {
        return false;
    }

    switch (tile->getType()) {
    case TileType::Road:
    case TileType::Asphalt:
    case TileType::StartLine:
    case TileType::FinishLine:
        return true;
    default:
        return false;
    }
}

bool isValidTileCoord(TrackData* track, const QPoint& tileCoord)
{
    return track
        && tileCoord.y() >= 0
        && tileCoord.y() < track->getRowCount()
        && tileCoord.x() >= 0
        && tileCoord.x() < track->getColCount();
}

int tileKey(TrackData* track, const QPoint& tileCoord)
{
    return tileCoord.y() * track->getColCount() + tileCoord.x();
}

QPoint findNearestAiTile(TrackData* track, const QVector2D& worldPos)
{
    if (!track) {
        return QPoint(-1, -1);
    }

    const QPoint tileCoord = TrackData::worldToTile(worldPos);
    if (isValidTileCoord(track, tileCoord) && isAiWaypointTile(track->getTileAt(tileCoord.y(), tileCoord.x()))) {
        return tileCoord;
    }

    QPoint bestTile(-1, -1);
    qreal bestDistanceSq = std::numeric_limits<qreal>::max();
    for (int row = 0; row < track->getRowCount(); ++row) {
        for (int col = 0; col < track->getColCount(); ++col) {
            TrackTile* tile = track->getTileAt(row, col);
            if (!isAiWaypointTile(tile)) {
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

QList<QPoint> checkpointGoalTiles(TrackData* track, const Checkpoint* checkpoint)
{
    QList<QPoint> goals;
    if (!track || !checkpoint) {
        return goals;
    }

    const QRectF checkpointBounds = checkpoint->getBounds();
    for (int row = 0; row < track->getRowCount(); ++row) {
        for (int col = 0; col < track->getColCount(); ++col) {
            TrackTile* tile = track->getTileAt(row, col);
            if (!isAiWaypointTile(tile) || !tile->getBounds().intersects(checkpointBounds)) {
                continue;
            }
            goals.append(QPoint(col, row));
        }
    }

    if (goals.isEmpty()) {
        const QPoint fallbackTile = findNearestAiTile(track, checkpoint->getPosition());
        if (fallbackTile.x() >= 0) {
            goals.append(fallbackTile);
        }
    }

    return goals;
}

QList<QPoint> finishGoalTiles(TrackData* track)
{
    QList<QPoint> goals;
    if (!track) {
        return goals;
    }

    for (int row = 0; row < track->getRowCount(); ++row) {
        for (int col = 0; col < track->getColCount(); ++col) {
            TrackTile* tile = track->getTileAt(row, col);
            if (tile && tile->getType() == TileType::FinishLine) {
                goals.append(QPoint(col, row));
            }
        }
    }

    if (goals.isEmpty()) {
        const QPoint fallbackTile = findNearestAiTile(track, track->getStartPosition());
        if (fallbackTile.x() >= 0) {
            goals.append(fallbackTile);
        }
    }

    return goals;
}

QList<QPoint> findTilePath(TrackData* track,
                           const QPoint& start,
                           const QList<QPoint>& goals,
                           QPoint* reachedGoal)
{
    QList<QPoint> path;
    if (reachedGoal) {
        *reachedGoal = QPoint(-1, -1);
    }

    if (!track || !isValidTileCoord(track, start) || !isAiWaypointTile(track->getTileAt(start.y(), start.x()))) {
        return path;
    }

    QSet<int> goalKeys;
    for (const QPoint& goal : goals) {
        if (isValidTileCoord(track, goal) && isAiWaypointTile(track->getTileAt(goal.y(), goal.x()))) {
            goalKeys.insert(tileKey(track, goal));
        }
    }
    if (goalKeys.isEmpty()) {
        return path;
    }

    if (goalKeys.contains(tileKey(track, start))) {
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
            if (!isValidTileCoord(track, next)
                || visited[next.y()][next.x()]
                || !isAiWaypointTile(track->getTileAt(next.y(), next.x()))) {
                continue;
            }

            visited[next.y()][next.x()] = true;
            parents[next.y()][next.x()] = current;

            if (goalKeys.contains(tileKey(track, next))) {
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

} // namespace

TrackManager* TrackManager::s_instance = nullptr;

TrackManager::TrackManager(QObject* parent)
    : QObject(parent)
    , m_currentTrack(nullptr)
    , m_database(new TrackDatabase(this))
    , m_trackIO(new TrackIO(this))
    , m_currentCheckpoint(0)
    , m_lastPassedCheckpoint(-1)
{
}

TrackManager::~TrackManager()
{
}

TrackManager* TrackManager::instance(QObject* parent)
{
    if (!s_instance) {
        s_instance = new TrackManager(parent ? parent : nullptr);
    }
    return s_instance;
}

bool TrackManager::loadTrack(const QString& trackId)
{
    TrackData* track = m_database->getTrack(trackId);
    if (!track) {
        qWarning() << "Failed to load track:" << trackId;
        return false;
    }

    m_currentTrack = track;
    m_waypoints = generateAiWaypoints(track);
    m_currentCheckpoint = 0;
    m_lastPassedCheckpoint = -1;

    emit trackLoaded(track);
    return true;
}

bool TrackManager::loadTrackFromFile(const QString& filePath)
{
    TrackData* track = m_trackIO->loadTrack(filePath);
    if (!track) {
        qWarning() << "Failed to load track from file:" << filePath;
        return false;
    }

    m_currentTrack = track;
    m_waypoints = generateAiWaypoints(track);
    m_currentCheckpoint = 0;
    m_lastPassedCheckpoint = -1;

    emit trackLoaded(track);
    return true;
}

bool TrackManager::saveCurrentTrack(const QString& filePath) const
{
    if (!m_currentTrack) {
        return false;
    }
    return m_trackIO->saveTrack(m_currentTrack, filePath);
}

void TrackManager::unloadTrack()
{
    if (m_currentTrack) {
        m_currentTrack = nullptr;
        m_waypoints.clear();
        m_currentCheckpoint = 0;
        m_lastPassedCheckpoint = -1;
        emit trackUnloaded();
    }
}

void TrackManager::setCurrentTrack(TrackData* track)
{
    if (!track) {
        unloadTrack();
        return;
    }

    if (track->parent() == nullptr) {
        track->setParent(this);
    }

    m_currentTrack = track;
    m_waypoints = generateAiWaypoints(track);
    m_currentCheckpoint = 0;
    m_lastPassedCheckpoint = -1;

    emit trackLoaded(track);
}

TrackTile* TrackManager::getTileAt(int row, int col) const
{
    if (m_currentTrack) {
        return m_currentTrack->getTileAt(row, col);
    }
    return nullptr;
}

TrackTile* TrackManager::getTileAtPosition(const QVector2D& worldPos) const
{
    if (!m_currentTrack) {
        return nullptr;
    }

    int row = static_cast<int>(worldPos.y() / 64.0);
    int col = static_cast<int>(worldPos.x() / 64.0);

    return m_currentTrack->getTileAt(row, col);
}

QList<TrackTile*> TrackManager::getTilesInRect(const QRectF& rect) const
{
    QList<TrackTile*> result;
    if (!m_currentTrack) {
        return result;
    }

    int startRow = static_cast<int>(rect.top() / 64.0);
    int endRow = static_cast<int>(rect.bottom() / 64.0);
    int startCol = static_cast<int>(rect.left() / 64.0);
    int endCol = static_cast<int>(rect.right() / 64.0);

    for (int r = startRow; r <= endRow; ++r) {
        for (int c = startCol; c <= endCol; ++c) {
            TrackTile* tile = m_currentTrack->getTileAt(r, c);
            if (tile) {
                result.append(tile);
            }
        }
    }

    return result;
}

Checkpoint* TrackManager::getCheckpoint(int id) const
{
    if (m_currentTrack) {
        return m_currentTrack->getCheckpoint(id);
    }
    return nullptr;
}

Checkpoint* TrackManager::getNextCheckpoint(int currentCheckpointId) const
{
    if (!m_currentTrack) {
        return nullptr;
    }

    QList<Checkpoint*> checkpoints = m_currentTrack->getCheckpointsInOrder();
    for (int i = 0; i < checkpoints.size(); ++i) {
        if (checkpoints[i]->getId() == currentCheckpointId) {
            int nextIndex = (i + 1) % checkpoints.size();
            return checkpoints[nextIndex];
        }
    }

    return checkpoints.isEmpty() ? nullptr : checkpoints.first();
}

int TrackManager::getCheckpointCount() const
{
    if (m_currentTrack) {
        return m_currentTrack->getCheckpointsInOrder().size();
    }
    return 0;
}

QList<Checkpoint*> TrackManager::getAllCheckpoints() const
{
    if (m_currentTrack) {
        return m_currentTrack->getAllCheckpoints();
    }
    return QList<Checkpoint*>();
}

QVector2D TrackManager::getStartPosition(int playerIndex) const
{
    if (!m_currentTrack) {
        return QVector2D(0, 0);
    }

    QList<QVector2D> positions = m_currentTrack->getStartPositions();
    if (playerIndex < positions.size()) {
        return positions[playerIndex];
    }

    return m_currentTrack->getStartPosition();
}

qreal TrackManager::getStartRotation() const
{
    if (m_currentTrack) {
        return m_currentTrack->getStartRotation();
    }
    return 0.0;
}

int TrackManager::getMaxPlayers() const
{
    if (m_currentTrack) {
        return m_currentTrack->getStartPositions().size();
    }
    return 1;
}

bool TrackManager::isValidPosition(const QVector2D& position) const
{
    if (!m_currentTrack) {
        return false;
    }

    QRectF bounds = m_currentTrack->getBounds();
    return bounds.contains(position.toPointF());
}

bool TrackManager::isDrivableTile(const QVector2D& position) const
{
    TrackTile* tile = getTileAtPosition(position);
    return tile && tile->isDrivable();
}

TileType TrackManager::getTileTypeAt(const QVector2D& position) const
{
    TrackTile* tile = getTileAtPosition(position);
    return tile ? tile->getType() : TileType::Unknown;
}

qreal TrackManager::getFrictionAt(const QVector2D& position) const
{
    TrackTile* tile = getTileAtPosition(position);
    return tile ? tile->getFriction() : 0.8;
}

QList<QVector2D> TrackManager::getWaypoints() const
{
    return m_waypoints;
}

void TrackManager::setWaypoints(const QList<QVector2D>& waypoints)
{
    m_waypoints = waypoints;
}

QList<QVector2D> TrackManager::generateAiWaypoints(TrackData* track) const
{
    QList<QVector2D> waypoints;
    if (!track) {
        return waypoints;
    }

    const QPoint startTile = findNearestAiTile(track, track->getStartPosition());
    if (startTile.x() < 0) {
        return waypoints;
    }

    auto appendTileCenter = [&waypoints](const QPoint& tileCoord) {
        const QVector2D point = TrackData::tileToWorldCenter(tileCoord.y(), tileCoord.x());
        if (waypoints.isEmpty() || (waypoints.last() - point).lengthSquared() > 1.0f) {
            waypoints.append(point);
        }
    };

    QList<QList<QPoint>> segmentGoals;
    const QList<Checkpoint*> checkpoints = track->getCheckpointsInOrder();
    segmentGoals.reserve(checkpoints.size() + 1);

    for (Checkpoint* checkpoint : checkpoints) {
        const QList<QPoint> goals = checkpointGoalTiles(track, checkpoint);
        if (!goals.isEmpty()) {
            segmentGoals.append(goals);
        }
    }

    const QList<QPoint> finishGoals = finishGoalTiles(track);
    if (!finishGoals.isEmpty()) {
        segmentGoals.append(finishGoals);
    }

    if (segmentGoals.isEmpty()) {
        appendTileCenter(startTile);
        return waypoints;
    }

    QPoint currentTile = startTile;
    for (const QList<QPoint>& goals : segmentGoals) {
        QPoint reachedGoal(-1, -1);
        const QList<QPoint> tilePath = findTilePath(track, currentTile, goals, &reachedGoal);
        if (tilePath.isEmpty()) {
            qWarning() << "TrackManager: failed to build AI waypoint path for segment from"
                       << currentTile << "to goal set of size" << goals.size();
            continue;
        }

        for (const QPoint& tileCoord : tilePath) {
            appendTileCenter(tileCoord);
        }

        currentTile = reachedGoal;
    }

    if (waypoints.isEmpty()) {
        appendTileCenter(startTile);
    }

    return waypoints;
}

QList<QVector2D> TrackManager::rebuildWaypoints()
{
    m_waypoints = generateAiWaypoints(m_currentTrack);
    return m_waypoints;
}

QList<TrackInfo> TrackManager::getAvailableTracks() const
{
    return m_database->getAllTracks();
}

QList<TrackInfo> TrackManager::searchTracks(const QString& query) const
{
    return m_database->searchTracks(query);
}

qreal TrackManager::calculateDistanceToNextCheckpoint(const QVector2D& position, int currentCheckpoint) const
{
    if (!m_currentTrack) {
        return 0.0;
    }

    Checkpoint* next = getNextCheckpoint(currentCheckpoint);
    if (!next) {
        return 0.0;
    }

    QVector2D diff = next->getPosition() - position;
    return diff.length();
}

int TrackManager::getNearestCheckpointIndex(const QVector2D& position) const
{
    if (!m_currentTrack) {
        return -1;
    }

    QList<Checkpoint*> checkpoints = m_currentTrack->getCheckpointsInOrder();
    int nearestIndex = -1;
    qreal nearestDistance = std::numeric_limits<qreal>::max();

    for (int i = 0; i < checkpoints.size(); ++i) {
        QVector2D diff = checkpoints[i]->getPosition() - position;
        qreal distance = diff.length();
        if (distance < nearestDistance) {
            nearestDistance = distance;
            nearestIndex = i;
        }
    }

    return nearestIndex;
}

QVariantMap TrackManager::getTrackStatistics() const
{
    QVariantMap stats;

    if (!m_currentTrack) {
        return stats;
    }

    stats["name"] = m_currentTrack->getName();
    stats["rows"] = m_currentTrack->getRowCount();
    stats["cols"] = m_currentTrack->getColCount();
    stats["checkpointCount"] = getCheckpointCount();
    stats["estimatedLapTime"] = m_currentTrack->getEstimatedLapTime();
    stats["trackLength"] = m_currentTrack->getTrackLength();
    stats["difficulty"] = m_currentTrack->getDifficulty();
    stats["maxLaps"] = m_currentTrack->getMaxLaps();

    int roadCount = 0;
    int grassCount = 0;
    for (int r = 0; r < m_currentTrack->getRowCount(); ++r) {
        for (int c = 0; c < m_currentTrack->getColCount(); ++c) {
            TrackTile* tile = m_currentTrack->getTileAt(r, c);
            if (tile) {
                if (tile->getType() == TileType::Road || tile->getType() == TileType::Asphalt) {
                    roadCount++;
                } else if (tile->getType() == TileType::Grass) {
                    grassCount++;
                }
            }
        }
    }

    stats["roadTileCount"] = roadCount;
    stats["grassTileCount"] = grassCount;
    stats["totalTiles"] = m_currentTrack->getRowCount() * m_currentTrack->getColCount();

    return stats;
}

void TrackManager::onCheckpointReached(int checkpointId)
{
    if (!m_currentTrack) {
        return;
    }

    QList<Checkpoint*> checkpoints = m_currentTrack->getCheckpointsInOrder();
    for (int i = 0; i < checkpoints.size(); ++i) {
        if (checkpoints[i]->getId() == checkpointId) {
            if (i != m_lastPassedCheckpoint) {
                m_lastPassedCheckpoint = i;
                emit checkpointReached(checkpointId, i);
            }
            break;
        }
    }
}

void TrackManager::resetProgress()
{
    m_currentCheckpoint = 0;
    m_lastPassedCheckpoint = -1;
}

}

#include "TrackManager.h"

#include <QDebug>
#include <QtMath>
#include <limits>

namespace PhantomDrive {

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
        return m_currentTrack->getAllCheckpoints().size();
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

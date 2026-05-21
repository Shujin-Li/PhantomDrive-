#pragma once

#include "PhantomDrive_global.h"
#include "TrackData.h"
#include "TrackTile.h"
#include "Checkpoint.h"
#include "TrackDatabase.h"
#include "TrackIO.h"

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QVector2D>
#include <QRectF>

namespace PhantomDrive {

class TrackManager : public QObject
{
    Q_OBJECT

public:
    static TrackManager* instance(QObject* parent = nullptr);

    bool loadTrack(const QString& trackId);
    bool loadTrackFromFile(const QString& filePath);
    bool saveCurrentTrack(const QString& filePath) const;

    TrackData* getCurrentTrack() const { return m_currentTrack; }
    bool hasCurrentTrack() const { return m_currentTrack != nullptr; }

    void unloadTrack();

    TrackTile* getTileAt(int row, int col) const;
    TrackTile* getTileAtPosition(const QVector2D& worldPos) const;
    QList<TrackTile*> getTilesInRect(const QRectF& rect) const;

    Checkpoint* getCheckpoint(int id) const;
    Checkpoint* getNextCheckpoint(int currentCheckpointId) const;
    int getCheckpointCount() const;
    QList<Checkpoint*> getAllCheckpoints() const;

    QVector2D getStartPosition(int playerIndex = 0) const;
    qreal getStartRotation() const;
    int getMaxPlayers() const;

    bool isValidPosition(const QVector2D& position) const;
    bool isDrivableTile(const QVector2D& position) const;
    TileType getTileTypeAt(const QVector2D& position) const;
    qreal getFrictionAt(const QVector2D& position) const;

    QList<QVector2D> getWaypoints() const;
    void setWaypoints(const QList<QVector2D>& waypoints);

    TrackDatabase* getDatabase() const { return m_database; }
    QList<TrackInfo> getAvailableTracks() const;
    QList<TrackInfo> searchTracks(const QString& query) const;

    qreal calculateDistanceToNextCheckpoint(const QVector2D& position, int currentCheckpoint) const;
    int getNearestCheckpointIndex(const QVector2D& position) const;

    QVariantMap getTrackStatistics() const;

signals:
    void trackLoaded(TrackData* track);
    void trackUnloaded();
    void trackChanged();
    void checkpointReached(int checkpointId, int index);
    void tileChanged(int row, int col);

public slots:
    void onCheckpointReached(int checkpointId);
    void resetProgress();

protected:
    explicit TrackManager(QObject* parent = nullptr);
    ~TrackManager();
    TrackManager(const TrackManager&) = delete;
    TrackManager& operator=(const TrackManager&) = delete;

private:
    static TrackManager* s_instance;

    TrackData* m_currentTrack;
    TrackDatabase* m_database;
    TrackIO* m_trackIO;

    QList<QVector2D> m_waypoints;
    int m_currentCheckpoint;
    int m_lastPassedCheckpoint;

    QMap<QString, QString> m_customWaypoints;
};

}

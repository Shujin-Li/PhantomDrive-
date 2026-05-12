#pragma once

#include "PhantomDrive_global.h"

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QJsonObject>

namespace PhantomDrive {

class TrackData;

struct TrackInfo {
    QString id;
    QString name;
    QString author;
    QString description;
    QString difficulty;
    QString filePath;
    qreal estimatedTime;
    qreal bestTime;
    qint64 lastPlayed;
    bool isBuiltIn;

    TrackInfo() : estimatedTime(0.0), bestTime(0.0), lastPlayed(0), isBuiltIn(false) {}
};

class PHANTOMDRIVE_EXPORT TrackDatabase : public QObject
{
    Q_OBJECT

public:
    explicit TrackDatabase(QObject* parent = nullptr);
    ~TrackDatabase() override;

    static TrackDatabase* instance(QObject* parent = nullptr);

    bool loadTrackList(const QString& directory);
    bool saveTrackList(const QString& filePath) const;

    TrackData* getTrack(const QString& trackId) const;
    bool addTrack(TrackData* track, const QString& filePath, bool isBuiltIn = false);
    bool removeTrack(const QString& trackId);
    bool updateTrackInfo(const TrackInfo& info);

    QList<TrackInfo> getAllTracks() const;
    QList<TrackInfo> getBuiltInTracks() const;
    QList<TrackInfo> getCustomTracks() const;
    QList<TrackInfo> searchTracks(const QString& query) const;

    TrackInfo getTrackInfo(const QString& trackId) const;
    bool updateBestTime(const QString& trackId, qreal time);

    bool importTrack(const QString& sourcePath, const QString& destDirectory);
    bool exportTrack(const QString& trackId, const QString& destPath) const;

    QString getTracksDirectory() const { return m_tracksDirectory; }
    void setTracksDirectory(const QString& dir);

    QString getLastError() const { return m_lastError; }

signals:
    void trackListLoaded(int count);
    void trackAdded(const QString& trackId);
    void trackRemoved(const QString& trackId);
    void trackUpdated(const QString& trackId);
    void bestTimeUpdated(const QString& trackId, qreal newBestTime);
    void errorOccurred(const QString& error);

protected:
    TrackData* loadTrackFromFile(const QString& filePath) const;

private:
    static TrackDatabase* s_instance;

    QString m_tracksDirectory;
    QString m_metadataFile;
    QMap<QString, TrackInfo> m_trackInfoMap;
    QMap<QString, TrackData*> m_loadedTracks;
    QString m_lastError;
};

}

#include "TrackDatabase.h"

#include "TrackData.h"
#include "TrackIO.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUuid>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

namespace PhantomDrive {

TrackDatabase* TrackDatabase::s_instance = nullptr;

TrackDatabase::TrackDatabase(QObject* parent)
    : QObject(parent)
    , m_tracksDirectory("tracks")
    , m_metadataFile("track_database.json")
{
}

TrackDatabase::~TrackDatabase()
{
}

TrackDatabase* TrackDatabase::instance(QObject* parent)
{
    if (!s_instance) {
        s_instance = new TrackDatabase(parent ? parent : nullptr);
    }
    return s_instance;
}

bool TrackDatabase::loadTrackList(const QString& directory)
{
    QDir dir(directory);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            m_lastError = QString("Cannot create tracks directory: %1").arg(directory);
            emit errorOccurred(m_lastError);
            return false;
        }
    }

    m_tracksDirectory = directory;

    QStringList filters;
    filters << "*.pdtrack" << "*.json";
    QFileInfoList fileInfos = dir.entryInfoList(filters, QDir::Files);

    int loadedCount = 0;
    for (const QFileInfo& fileInfo : fileInfos) {
        TrackIO io;
        TrackData* track = io.loadTrack(fileInfo.absoluteFilePath());
        if (track) {
            TrackInfo info;
            info.id = track->getId().isEmpty() ? QUuid::createUuid().toString() : track->getId();
            info.name = track->getName();
            info.author = track->getAuthor();
            info.description = track->getDescription();
            info.difficulty = track->getDifficulty();
            info.filePath = fileInfo.absoluteFilePath();
            info.estimatedTime = track->getEstimatedLapTime();
            info.isBuiltIn = false;

            m_trackInfoMap[info.id] = info;
            m_loadedTracks[info.id] = track;

            loadedCount++;
        } else {
            qWarning() << "Failed to load track:" << fileInfo.absoluteFilePath()
                       << "Error:" << io.getLastError();
        }
    }

    emit trackListLoaded(loadedCount);
    return true;
}

bool TrackDatabase::saveTrackList(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        const_cast<TrackDatabase*>(this)->m_lastError = "Cannot open file for writing";
        emit const_cast<TrackDatabase*>(this)->errorOccurred(m_lastError);
        return false;
    }

    QJsonObject rootObj;
    QJsonArray tracksArray;

    for (auto it = m_trackInfoMap.constBegin(); it != m_trackInfoMap.constEnd(); ++it) {
        const TrackInfo& info = it.value();
        QJsonObject trackObj;
        trackObj["id"] = info.id;
        trackObj["name"] = info.name;
        trackObj["author"] = info.author;
        trackObj["description"] = info.description;
        trackObj["difficulty"] = info.difficulty;
        trackObj["filePath"] = info.filePath;
        trackObj["estimatedTime"] = info.estimatedTime;
        trackObj["bestTime"] = info.bestTime;
        trackObj["lastPlayed"] = QString::number(info.lastPlayed);
        trackObj["isBuiltIn"] = info.isBuiltIn;
        tracksArray.append(trackObj);
    }

    rootObj["tracks"] = tracksArray;
    rootObj["version"] = "1.0";

    QJsonDocument doc(rootObj);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    return true;
}

TrackData* TrackDatabase::getTrack(const QString& trackId) const
{
    if (m_loadedTracks.contains(trackId)) {
        return m_loadedTracks[trackId];
    }

    const TrackInfo* info = &m_trackInfoMap[trackId];
    if (info && !info->filePath.isEmpty()) {
        TrackIO io;
        TrackData* track = io.loadTrack(info->filePath);
        if (track) {
            const_cast<TrackDatabase*>(this)->m_loadedTracks[trackId] = track;
            return track;
        }
    }

    return nullptr;
}

bool TrackDatabase::addTrack(TrackData* track, const QString& filePath, bool isBuiltIn)
{
    if (!track) {
        m_lastError = "Track is null";
        emit errorOccurred(m_lastError);
        return false;
    }

    QString trackId = track->getId();
    if (trackId.isEmpty()) {
        trackId = QUuid::createUuid().toString();
        track->setId(trackId);
    }

    TrackIO io;
    if (!io.saveTrack(track, filePath)) {
        m_lastError = io.getLastError();
        emit errorOccurred(m_lastError);
        return false;
    }

    TrackInfo info;
    info.id = trackId;
    info.name = track->getName();
    info.author = track->getAuthor();
    info.description = track->getDescription();
    info.difficulty = track->getDifficulty();
    info.filePath = filePath;
    info.estimatedTime = track->getEstimatedLapTime();
    info.isBuiltIn = isBuiltIn;

    m_trackInfoMap[trackId] = info;
    m_loadedTracks[trackId] = track;

    emit trackAdded(trackId);
    return true;
}

bool TrackDatabase::removeTrack(const QString& trackId)
{
    if (!m_trackInfoMap.contains(trackId)) {
        m_lastError = "Track not found";
        emit errorOccurred(m_lastError);
        return false;
    }

    TrackInfo info = m_trackInfoMap[trackId];
    if (!info.isBuiltIn) {
        QFile file(info.filePath);
        if (file.exists()) {
            file.remove();
        }
    }

    if (m_loadedTracks.contains(trackId)) {
        delete m_loadedTracks[trackId];
        m_loadedTracks.remove(trackId);
    }

    m_trackInfoMap.remove(trackId);

    emit trackRemoved(trackId);
    return true;
}

bool TrackDatabase::updateTrackInfo(const TrackInfo& info)
{
    if (!m_trackInfoMap.contains(info.id)) {
        m_lastError = "Track not found";
        emit errorOccurred(m_lastError);
        return false;
    }

    m_trackInfoMap[info.id] = info;
    emit trackUpdated(info.id);
    return true;
}

QList<TrackInfo> TrackDatabase::getAllTracks() const
{
    return m_trackInfoMap.values();
}

QList<TrackInfo> TrackDatabase::getBuiltInTracks() const
{
    QList<TrackInfo> result;
    for (const TrackInfo& info : m_trackInfoMap.values()) {
        if (info.isBuiltIn) {
            result.append(info);
        }
    }
    return result;
}

QList<TrackInfo> TrackDatabase::getCustomTracks() const
{
    QList<TrackInfo> result;
    for (const TrackInfo& info : m_trackInfoMap.values()) {
        if (!info.isBuiltIn) {
            result.append(info);
        }
    }
    return result;
}

QList<TrackInfo> TrackDatabase::searchTracks(const QString& query) const
{
    QList<TrackInfo> result;
    QString lowerQuery = query.toLower();

    for (const TrackInfo& info : m_trackInfoMap.values()) {
        if (info.name.toLower().contains(lowerQuery) ||
            info.author.toLower().contains(lowerQuery) ||
            info.description.toLower().contains(lowerQuery) ||
            info.difficulty.toLower().contains(lowerQuery)) {
            result.append(info);
        }
    }

    return result;
}

TrackInfo TrackDatabase::getTrackInfo(const QString& trackId) const
{
    return m_trackInfoMap.value(trackId);
}

bool TrackDatabase::updateBestTime(const QString& trackId, qreal time)
{
    if (!m_trackInfoMap.contains(trackId)) {
        return false;
    }

    TrackInfo& info = m_trackInfoMap[trackId];
    if (info.bestTime == 0.0 || time < info.bestTime) {
        info.bestTime = time;
        emit bestTimeUpdated(trackId, time);
        return true;
    }

    return false;
}

bool TrackDatabase::importTrack(const QString& sourcePath, const QString& destDirectory)
{
    QFileInfo sourceInfo(sourcePath);
    if (!sourceInfo.exists()) {
        m_lastError = "Source file does not exist";
        emit errorOccurred(m_lastError);
        return false;
    }

    QDir destDir(destDirectory);
    if (!destDir.exists()) {
        destDir.mkpath(".");
    }

    QString fileName = sourceInfo.fileName();
    QString destPath = destDir.absoluteFilePath(fileName);

    if (QFile::copy(sourcePath, destPath)) {
        TrackIO io;
        TrackData* track = io.loadTrack(destPath);
        if (track) {
            QString trackId = track->getId().isEmpty() ? QUuid::createUuid().toString() : track->getId();
            track->setId(trackId);

            TrackInfo info;
            info.id = trackId;
            info.name = track->getName();
            info.author = track->getAuthor();
            info.description = track->getDescription();
            info.difficulty = track->getDifficulty();
            info.filePath = destPath;
            info.estimatedTime = track->getEstimatedLapTime();
            info.isBuiltIn = false;

            m_trackInfoMap[trackId] = info;
            m_loadedTracks[trackId] = track;

            emit trackAdded(trackId);
            return true;
        }
    }

    m_lastError = "Failed to import track";
    emit errorOccurred(m_lastError);
    return false;
}

bool TrackDatabase::exportTrack(const QString& trackId, const QString& destPath) const
{
    const TrackInfo* info = &m_trackInfoMap[trackId];
    if (!info || info->filePath.isEmpty()) {
        const_cast<TrackDatabase*>(this)->m_lastError = "Track not found";
        emit const_cast<TrackDatabase*>(this)->errorOccurred(m_lastError);
        return false;
    }

    if (QFile::copy(info->filePath, destPath)) {
        return true;
    }

    const_cast<TrackDatabase*>(this)->m_lastError = "Failed to export track";
    emit const_cast<TrackDatabase*>(this)->errorOccurred(m_lastError);
    return false;
}

void TrackDatabase::setTracksDirectory(const QString& dir)
{
    m_tracksDirectory = dir;
    loadTrackList(dir);
}

TrackData* TrackDatabase::loadTrackFromFile(const QString& filePath) const
{
    TrackIO io;
    return io.loadTrack(filePath);
}

}

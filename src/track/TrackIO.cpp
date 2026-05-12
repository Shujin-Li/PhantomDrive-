#include "TrackIO.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>

namespace PhantomDrive {

const QStringList TrackIO::SUPPORTED_EXTENSIONS = {"pdtrack", "json"};

TrackIO::TrackIO(QObject* parent)
    : QObject(parent)
{
}

TrackIO::~TrackIO()
{
}

bool TrackIO::saveTrack(TrackData* track, const QString& filePath) const
{
    if (!track) {
        const_cast<TrackIO*>(this)->setError("Track is null");
        return false;
    }

    emit const_cast<TrackIO*>(this)->saveStarted(filePath);

    QJsonObject jsonObj;
    if (!saveTrackToJson(track, jsonObj)) {
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        const_cast<TrackIO*>(this)->setError(QString("Cannot open file for writing: %1").arg(filePath));
        emit const_cast<TrackIO*>(this)->errorOccurred(m_lastError);
        return false;
    }

    QJsonDocument doc(jsonObj);
    QTextStream out(&file);
    out << doc.toJson(QJsonDocument::Indented);
    file.close();

    emit const_cast<TrackIO*>(this)->saveCompleted(filePath);
    return true;
}

TrackData* TrackIO::loadTrack(const QString& filePath) const
{
    emit const_cast<TrackIO*>(this)->loadStarted(filePath);

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const_cast<TrackIO*>(this)->setError(QString("Cannot open file for reading: %1").arg(filePath));
        emit const_cast<TrackIO*>(this)->errorOccurred(m_lastError);
        return nullptr;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        const_cast<TrackIO*>(this)->setError(QString("JSON parse error: %1").arg(parseError.errorString()));
        emit const_cast<TrackIO*>(this)->errorOccurred(m_lastError);
        return nullptr;
    }

    if (!doc.isObject()) {
        const_cast<TrackIO*>(this)->setError("Invalid track file format: root is not an object");
        emit const_cast<TrackIO*>(this)->errorOccurred(m_lastError);
        return nullptr;
    }

    TrackData* track = new TrackData();
    if (!loadTrackFromJson(track, doc.object())) {
        delete track;
        return nullptr;
    }

    emit const_cast<TrackIO*>(this)->loadCompleted(filePath);
    return track;
}

bool TrackIO::saveTrackToJson(TrackData* track, QJsonObject& json) const
{
    if (!track) {
        const_cast<TrackIO*>(this)->setError("Track is null");
        return false;
    }

    json["version"] = "1.0";
    json["type"] = "PhantomDriveTrack";
    json["name"] = track->getName();
    json["id"] = track->getId();
    json["author"] = track->getAuthor();
    json["description"] = track->getDescription();
    json["rowCount"] = track->getRowCount();
    json["colCount"] = track->getColCount();
    json["startPositionX"] = track->getStartPosition().x();
    json["startPositionY"] = track->getStartPosition().y();
    json["startRotation"] = track->getStartRotation();
    json["estimatedLapTime"] = track->getEstimatedLapTime();
    json["trackLength"] = track->getTrackLength();
    json["difficulty"] = track->getDifficulty();
    json["maxLaps"] = track->getMaxLaps();

    QJsonArray startPositions;
    for (const QVector2D& pos : track->getStartPositions()) {
        QJsonObject posObj;
        posObj["x"] = pos.x();
        posObj["y"] = pos.y();
        startPositions.append(posObj);
    }
    json["startPositions"] = startPositions;

    QJsonArray tilesArray;
    for (int r = 0; r < track->getRowCount(); ++r) {
        for (int c = 0; c < track->getColCount(); ++c) {
            TrackTile* tile = track->getTileAt(r, c);
            if (tile) {
                QVariantMap tileMap = tile->toVariantMap();
                QJsonObject tileObj;
                for (auto it = tileMap.begin(); it != tileMap.end(); ++it) {
                    tileObj[it.key()] = QJsonValue::fromVariant(it.value());
                }
                tilesArray.append(tileObj);
            }
        }
    }
    json["tiles"] = tilesArray;

    QJsonArray checkpointsArray;
    for (const Checkpoint* cp : track->getCheckpointsInOrder()) {
        QVariantMap cpMap = cp->toVariantMap();
        QJsonObject cpObj;
        for (auto it = cpMap.begin(); it != cpMap.end(); ++it) {
            cpObj[it.key()] = QJsonValue::fromVariant(it.value());
        }
        checkpointsArray.append(cpObj);
    }
    json["checkpoints"] = checkpointsArray;

    return true;
}

bool TrackIO::loadTrackFromJson(TrackData* track, const QJsonObject& jsonObject) const
{
    if (!track) {
        const_cast<TrackIO*>(this)->setError("Track is null");
        return false;
    }

    if (jsonObject["type"].toString() != "PhantomDriveTrack") {
        const_cast<TrackIO*>(this)->setError("Invalid track file format");
        return false;
    }

    track->setName(jsonObject["name"].toString());
    track->setId(jsonObject["id"].toString());
    track->setAuthor(jsonObject["author"].toString());
    track->setDescription(jsonObject["description"].toString());

    int rowCount = jsonObject["rowCount"].toInt();
    int colCount = jsonObject["colCount"].toInt();
    track->setSize(rowCount, colCount);

    QVector2D startPos(
        jsonObject["startPositionX"].toDouble(),
        jsonObject["startPositionY"].toDouble()
    );
    track->setStartPosition(startPos);
    track->setStartRotation(jsonObject["startRotation"].toDouble());
    track->setEstimatedLapTime(jsonObject["estimatedLapTime"].toDouble());
    track->setTrackLength(jsonObject["trackLength"].toDouble());
    track->setDifficulty(jsonObject["difficulty"].toString());
    track->setMaxLaps(jsonObject["maxLaps"].toInt());

    QJsonArray startPosArray = jsonObject["startPositions"].toArray();
    for (const QJsonValue& val : startPosArray) {
        QJsonObject posObj = val.toObject();
        track->addStartPosition(QVector2D(posObj["x"].toDouble(), posObj["y"].toDouble()));
    }

    QJsonArray tilesArray = jsonObject["tiles"].toArray();
    for (const QJsonValue& val : tilesArray) {
        QVariantMap tileMap = val.toVariant().toMap();
        int row = tileMap["row"].toInt();
        int col = tileMap["col"].toInt();
        if (row >= 0 && row < rowCount && col >= 0 && col < colCount) {
            TrackTile* tile = track->tile(row, col);
            if (tile) {
                tile->fromVariantMap(tileMap);
            }
        }
    }

    QJsonArray checkpointsArray = jsonObject["checkpoints"].toArray();
    for (const QJsonValue& val : checkpointsArray) {
        QVariantMap cpMap = val.toVariant().toMap();
        Checkpoint* cp = new Checkpoint();
        cp->fromVariantMap(cpMap);
        track->addCheckpoint(cp);
    }

    return true;
}

QStringList TrackIO::getSupportedExtensions()
{
    return SUPPORTED_EXTENSIONS;
}

bool TrackIO::isValidTrackFile(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    return SUPPORTED_EXTENSIONS.contains(fileInfo.suffix().toLower());
}

void TrackIO::setError(const QString& error)
{
    m_lastError = error;
}

}

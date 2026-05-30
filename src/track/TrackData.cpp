#include "TrackData.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QtMath>

namespace PhantomDrive {

TrackData::TrackData(QObject* parent)
    : QObject(parent)
    , m_rowCount(0)
    , m_colCount(0)
    , m_startRotation(0.0)
    , m_estimatedLapTime(0.0)
    , m_trackLength(0.0)
    , m_maxLaps(3)
{
}

TrackData::TrackData(const QString& name, QObject* parent)
    : QObject(parent)
    , m_name(name)
    , m_rowCount(0)
    , m_colCount(0)
    , m_startRotation(0.0)
    , m_estimatedLapTime(0.0)
    , m_trackLength(0.0)
    , m_maxLaps(3)
{
}

TrackData::~TrackData()
{
    clear();
}

void TrackData::setSize(int rows, int cols)
{
    clearTiles();
    m_rowCount = rows;
    m_colCount = cols;

    m_tiles.resize(rows);
    for (int r = 0; r < rows; ++r) {
        m_tiles[r].resize(cols);
        for (int c = 0; c < cols; ++c) {
            m_tiles[r][c] = new TrackTile(r, c, TileType::Grass, this);
        }
    }

    emit trackChanged();
}

QVector2D TrackData::tileToWorldCenter(int row, int col, qreal tileSize)
{
    return QVector2D(col * tileSize + tileSize / 2.0,
                     row * tileSize + tileSize / 2.0);
}

QPoint TrackData::worldToTile(const QVector2D& worldPos, qreal tileSize)
{
    return QPoint(static_cast<int>(qFloor(worldPos.x() / tileSize)),
                  static_cast<int>(qFloor(worldPos.y() / tileSize)));
}

TrackTile* TrackData::getTileAt(int row, int col) const
{
    if (row >= 0 && row < m_rowCount && col >= 0 && col < m_colCount) {
        return m_tiles[row][col];
    }
    return nullptr;
}

void TrackData::setTileAt(int row, int col, TrackTile* tile)
{
    if (row >= 0 && row < m_rowCount && col >= 0 && col < m_colCount) {
        if (m_tiles[row][col]) {
            m_tiles[row][col]->deleteLater();
        }
        tile->setParent(this);
        tile->setPosition(row, col);
        m_tiles[row][col] = tile;
        emit tileChanged(row, col);
        emit trackChanged();
    }
}

TrackTile* TrackData::tile(int row, int col) const
{
    return getTileAt(row, col);
}

void TrackData::addCheckpoint(Checkpoint* checkpoint)
{
    if (!checkpoint) return;

    checkpoint->setParent(this);
    m_checkpoints[checkpoint->getId()] = checkpoint;
    m_checkpointsOrdered.append(checkpoint);

    std::sort(m_checkpointsOrdered.begin(), m_checkpointsOrdered.end(),
              [](const Checkpoint* a, const Checkpoint* b) {
                  return a->getIndexInRoute() < b->getIndexInRoute();
              });

    emit checkpointsChanged();
}

void TrackData::removeCheckpoint(int checkpointId)
{
    if (m_checkpoints.contains(checkpointId)) {
        Checkpoint* cp = m_checkpoints[checkpointId];
        m_checkpoints.remove(checkpointId);
        m_checkpointsOrdered.removeAll(cp);
        cp->deleteLater();
        emit checkpointsChanged();
    }
}

Checkpoint* TrackData::getCheckpoint(int id) const
{
    return m_checkpoints.value(id, nullptr);
}

QList<Checkpoint*> TrackData::getAllCheckpoints() const
{
    return m_checkpoints.values();
}

QList<Checkpoint*> TrackData::getCheckpointsInOrder() const
{
    return m_checkpointsOrdered;
}

void TrackData::clearCheckpoints()
{
    qDeleteAll(m_checkpoints.values());
    m_checkpoints.clear();
    m_checkpointsOrdered.clear();
    emit checkpointsChanged();
}

void TrackData::addItemBoxPosition(const QVector2D& pos)
{
    m_itemBoxPositions.append(pos);
    emit trackChanged();
}

bool TrackData::removeItemBoxAt(const QVector2D& pos, qreal radius)
{
    for (int i = 0; i < m_itemBoxPositions.size(); ++i) {
        if ((m_itemBoxPositions.at(i) - pos).length() <= radius) {
            m_itemBoxPositions.removeAt(i);
            emit trackChanged();
            return true;
        }
    }

    return false;
}

void TrackData::clearItemBoxPositions()
{
    if (m_itemBoxPositions.isEmpty()) {
        return;
    }

    m_itemBoxPositions.clear();
    emit trackChanged();
}

bool TrackData::isValid() const
{
    if (m_rowCount <= 0 || m_colCount <= 0) {
        return false;
    }

    bool hasRoad = false;
    for (int r = 0; r < m_rowCount; ++r) {
        for (int c = 0; c < m_colCount; ++c) {
            if (m_tiles[r][c]->getType() == TileType::Road ||
                m_tiles[r][c]->getType() == TileType::Asphalt) {
                hasRoad = true;
                break;
            }
        }
        if (hasRoad) break;
    }

    return hasRoad && !m_startPositions.isEmpty();
}

bool TrackData::validateCheckpoints() const
{
    if (m_checkpointsOrdered.isEmpty()) {
        return false;
    }

    for (const Checkpoint* cp : m_checkpointsOrdered) {
        if (cp->isMandatory() && !containsPoint(cp->getPosition())) {
            return false;
        }
    }

    return true;
}

QRectF TrackData::getBounds() const
{
    return QRectF(0, 0, m_colCount * 64.0, m_rowCount * 64.0);
}

bool TrackData::containsPoint(const QVector2D& point) const
{
    QRectF bounds = getBounds();
    return bounds.contains(point.toPointF());
}

void TrackData::clear()
{
    clearTiles();
    clearCheckpoints();
    m_startPositions.clear();
    m_itemBoxPositions.clear();
    emit trackChanged();
}

void TrackData::clearTiles()
{
    for (int r = 0; r < m_tiles.size(); ++r) {
        for (int c = 0; c < m_tiles[r].size(); ++c) {
            if (m_tiles[r][c]) {
                m_tiles[r][c]->deleteLater();
                m_tiles[r][c] = nullptr;
            }
        }
    }
    m_tiles.clear();
}

QJsonObject TrackData::toJsonObject() const
{
    QJsonObject map;
    map["name"] = m_name;
    map["id"] = m_id;
    map["author"] = m_author;
    map["description"] = m_description;
    map["rowCount"] = m_rowCount;
    map["colCount"] = m_colCount;
    map["startPositionX"] = m_startPosition.x();
    map["startPositionY"] = m_startPosition.y();
    map["startRotation"] = m_startRotation;
    map["estimatedLapTime"] = m_estimatedLapTime;
    map["trackLength"] = m_trackLength;
    map["difficulty"] = m_difficulty;
    map["maxLaps"] = m_maxLaps;

    QJsonArray startPositions;
    for (const QVector2D& pos : m_startPositions) {
        QJsonObject posObj;
        posObj["x"] = pos.x();
        posObj["y"] = pos.y();
        startPositions.append(posObj);
    }
    map["startPositions"] = startPositions;

    QJsonArray itemBoxes;
    for (const QVector2D& pos : m_itemBoxPositions) {
        QJsonObject itemObj;
        itemObj["x"] = pos.x();
        itemObj["y"] = pos.y();
        itemBoxes.append(itemObj);
    }
    map["itemBoxes"] = itemBoxes;

    QJsonArray tilesArray;
    for (int r = 0; r < m_rowCount; ++r) {
        for (int c = 0; c < m_colCount; ++c) {
            if (m_tiles[r][c]) {
                tilesArray.append(QJsonValue::fromVariant(m_tiles[r][c]->toVariantMap()));
            }
        }
    }
    map["tiles"] = tilesArray;

    QJsonArray checkpointsArray;
    for (const Checkpoint* cp : m_checkpointsOrdered) {
        checkpointsArray.append(QJsonValue::fromVariant(cp->toVariantMap()));
    }
    map["checkpoints"] = checkpointsArray;

    return map;
}

QVariantMap TrackData::toVariantMap() const
{
    QJsonObject obj = toJsonObject();
    QJsonDocument doc(obj);
    return doc.toVariant().toMap();
}

void TrackData::fromJsonObject(const QJsonObject& obj)
{
    clear();

    m_name = obj["name"].toString();
    m_id = obj["id"].toString();
    m_author = obj["author"].toString();
    m_description = obj["description"].toString();
    m_rowCount = obj["rowCount"].toInt();
    m_colCount = obj["colCount"].toInt();
    m_startPosition.setX(obj["startPositionX"].toDouble());
    m_startPosition.setY(obj["startPositionY"].toDouble());
    m_startRotation = obj["startRotation"].toDouble();
    m_estimatedLapTime = obj["estimatedLapTime"].toDouble();
    m_trackLength = obj["trackLength"].toDouble();
    m_difficulty = obj["difficulty"].toString();
    m_maxLaps = obj["maxLaps"].toInt();

    QJsonArray startPosArray = obj["startPositions"].toArray();
    m_startPositions.clear();
    for (const QJsonValue& val : startPosArray) {
        QJsonObject posObj = val.toObject();
        m_startPositions.append(QVector2D(posObj["x"].toDouble(), posObj["y"].toDouble()));
    }

    m_itemBoxPositions.clear();
    const QJsonArray itemBoxArray = obj["itemBoxes"].toArray();
    for (const QJsonValue& val : itemBoxArray) {
        const QJsonObject itemObj = val.toObject();
        m_itemBoxPositions.append(QVector2D(itemObj["x"].toDouble(), itemObj["y"].toDouble()));
    }

    if (m_rowCount > 0 && m_colCount > 0) {
        setSize(m_rowCount, m_colCount);

        QJsonArray tilesArray = obj["tiles"].toArray();
        for (const QJsonValue& val : tilesArray) {
            QVariantMap tileMap = val.toVariant().toMap();
            int row = tileMap["row"].toInt();
            int col = tileMap["col"].toInt();
            if (row >= 0 && row < m_rowCount && col >= 0 && col < m_colCount) {
                TrackTile* tile = new TrackTile(this);
                tile->fromVariantMap(tileMap);
                m_tiles[row][col] = tile;
            }
        }
    }

    QJsonArray checkpointsArray = obj["checkpoints"].toArray();
    for (const QJsonValue& val : checkpointsArray) {
        QVariantMap cpMap = val.toVariant().toMap();
        Checkpoint* cp = new Checkpoint(this);
        cp->fromVariantMap(cpMap);
        m_checkpoints[cp->getId()] = cp;
        m_checkpointsOrdered.append(cp);
    }

    emit trackChanged();
}

void TrackData::fromVariantMap(const QVariantMap& data)
{
    QJsonDocument doc(QJsonObject::fromVariantMap(data));
    fromJsonObject(doc.object());
}

}

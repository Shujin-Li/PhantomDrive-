#include "TrackTile.h"

#include <QDebug>
#include <QVariantMap>

namespace PhantomDrive {

TrackTile::TrackTile(QObject* parent)
    : QObject(parent)
    , m_row(0)
    , m_col(0)
    , m_type(TileType::Road)
    , m_direction(TileDirection::North)
    , m_friction(0.8)
    , m_isDrivable(true)
    , m_isCollisionTile(false)
{
}

TrackTile::TrackTile(int row, int col, TileType type, QObject* parent)
    : QObject(parent)
    , m_row(row)
    , m_col(col)
    , m_type(type)
    , m_direction(TileDirection::North)
    , m_friction(0.8)
    , m_isDrivable(true)
    , m_isCollisionTile(false)
{
    applyTileDefaults(type);
}

void TrackTile::applyTileDefaults(TileType type) {
    switch (type) {
        case TileType::Road:
        case TileType::Asphalt:
        case TileType::StartLine:
        case TileType::FinishLine:
            m_friction = 0.9;
            m_isDrivable = true;
            m_isCollisionTile = false;
            break;
        case TileType::Grass:
            m_friction = 0.5;
            m_isDrivable = true;
            m_isCollisionTile = false;
            break;
        case TileType::Sand:
            m_friction = 0.3;
            m_isDrivable = true;
            m_isCollisionTile = false;
            break;
        case TileType::Wall:
        case TileType::Barrier:
            m_friction = 1.0;
            m_isDrivable = false;
            m_isCollisionTile = true;
            break;
        default:
            m_friction = 0.8;
            m_isDrivable = true;
            m_isCollisionTile = false;
            break;
    }
}

TrackTile::~TrackTile()
{
}

void TrackTile::setPosition(int row, int col)
{
    if (m_row != row || m_col != col) {
        m_row = row;
        m_col = col;
        emit positionChanged(row, col);
        emit tileChanged();
    }
}

QRectF TrackTile::getBounds() const
{
    qreal x = m_col * DEFAULT_TILE_SIZE;
    qreal y = m_row * DEFAULT_TILE_SIZE;
    return QRectF(x, y, DEFAULT_TILE_SIZE, DEFAULT_TILE_SIZE);
}

QVector2D TrackTile::getCenter() const
{
    qreal x = m_col * DEFAULT_TILE_SIZE + DEFAULT_TILE_SIZE / 2.0;
    qreal y = m_row * DEFAULT_TILE_SIZE + DEFAULT_TILE_SIZE / 2.0;
    return QVector2D(x, y);
}

QVariantMap TrackTile::toVariantMap() const
{
    QVariantMap map;
    map["row"] = m_row;
    map["col"] = m_col;
    map["type"] = static_cast<int>(m_type);
    map["direction"] = static_cast<int>(m_direction);
    map["friction"] = m_friction;
    map["drivable"] = m_isDrivable;
    map["collisionTile"] = m_isCollisionTile;
    map["assetId"] = m_assetId;
    return map;
}

void TrackTile::fromVariantMap(const QVariantMap& data)
{
    m_row = data.value("row").toInt();
    m_col = data.value("col").toInt();
    m_type = static_cast<TileType>(data.value("type").toInt());
    m_direction = static_cast<TileDirection>(data.value("direction").toInt());
    m_friction = data.value("friction").toDouble();
    m_isDrivable = data.value("drivable").toBool();
    m_isCollisionTile = data.value("collisionTile").toBool();
    m_assetId = data.value("assetId").toString();
}

}

#pragma once

#include "PhantomDrive_global.h"

#include <QObject>
#include <QString>
#include <QRectF>
#include <QVector2D>

namespace PhantomDrive {

enum class TileType {
    Road,
    Grass,
    Sand,
    Asphalt,
    StartLine,
    FinishLine,
    Wall,
    Barrier,
    Unknown
};

enum class TileDirection {
    North = 0,
    East = 90,
    South = 180,
    West = 270,
    NorthEast = 45,
    SouthEast = 135,
    SouthWest = 225,
    NorthWest = 315
};

class PHANTOMDRIVE_EXPORT TrackTile : public QObject
{
    Q_OBJECT

public:
    explicit TrackTile(QObject* parent = nullptr);
    TrackTile(int row, int col, TileType type, QObject* parent = nullptr);
    ~TrackTile() override;

    int getRow() const { return m_row; }
    int getCol() const { return m_col; }
    void setPosition(int row, int col);

    TileType getType() const { return m_type; }
    void setType(TileType type) { m_type = type; }

    TileDirection getDirection() const { return m_direction; }
    void setDirection(TileDirection dir) { m_direction = dir; }

    qreal getFriction() const { return m_friction; }
    void setFriction(qreal friction) { m_friction = qMax(0.0, qMin(1.0, friction)); }

    bool isDrivable() const { return m_isDrivable; }
    void setDrivable(bool drivable) { m_isDrivable = drivable; }

    bool isCollisionTile() const { return m_isCollisionTile; }
    void setCollisionTile(bool collision) { m_isCollisionTile = collision; }

    QRectF getBounds() const;
    QVector2D getCenter() const;

    QString getAssetId() const { return m_assetId; }
    void setAssetId(const QString& id) { m_assetId = id; }

    QVariantMap toVariantMap() const;
    void fromVariantMap(const QVariantMap& data);

private:
    void applyTileDefaults(TileType type);

signals:
    void tileChanged();
    void positionChanged(int row, int col);
    void typeChanged(TileType type);

private:
    int m_row;
    int m_col;
    TileType m_type;
    TileDirection m_direction;
    qreal m_friction;
    bool m_isDrivable;
    bool m_isCollisionTile;
    QString m_assetId;

    static constexpr qreal DEFAULT_TILE_SIZE = 64.0;
};

}

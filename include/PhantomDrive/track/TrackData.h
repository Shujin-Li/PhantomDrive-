#pragma once

#include "PhantomDrive_global.h"
#include "TrackTile.h"
#include "Checkpoint.h"

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QVector2D>
#include <QRectF>
#include <QJsonArray>

namespace PhantomDrive {

class TrackData : public QObject
{
    Q_OBJECT

public:
    explicit TrackData(QObject* parent = nullptr);
    explicit TrackData(const QString& name, QObject* parent = nullptr);
    ~TrackData() override;

    QString getName() const { return m_name; }
    void setName(const QString& name) { m_name = name; }

    QString getId() const { return m_id; }
    void setId(const QString& id) { m_id = id; }

    QString getAuthor() const { return m_author; }
    void setAuthor(const QString& author) { m_author = author; }

    QString getDescription() const { return m_description; }
    void setDescription(const QString& desc) { m_description = desc; }

    int getRowCount() const { return m_rowCount; }
    int getColCount() const { return m_colCount; }
    void setSize(int rows, int cols);

    QList<QList<TrackTile*>> getTiles() const { return m_tiles; }
    TrackTile* getTileAt(int row, int col) const;
    void setTileAt(int row, int col, TrackTile* tile);
    TrackTile* tile(int row, int col) const;

    void addCheckpoint(Checkpoint* checkpoint);
    void removeCheckpoint(int checkpointId);
    Checkpoint* getCheckpoint(int id) const;
    QList<Checkpoint*> getAllCheckpoints() const;
    QList<Checkpoint*> getCheckpointsInOrder() const;
    void clearCheckpoints();

    QVector2D getStartPosition() const { return m_startPosition; }
    void setStartPosition(const QVector2D& pos) { m_startPosition = pos; }

    qreal getStartRotation() const { return m_startRotation; }
    void setStartRotation(qreal rotation) { m_startRotation = rotation; }

    QList<QVector2D> getStartPositions() const { return m_startPositions; }
    void addStartPosition(const QVector2D& pos) { m_startPositions.append(pos); }
    void clearStartPositions() { m_startPositions.clear(); }

    qreal getEstimatedLapTime() const { return m_estimatedLapTime; }
    void setEstimatedLapTime(qreal time) { m_estimatedLapTime = time; }

    qreal getTrackLength() const { return m_trackLength; }
    void setTrackLength(qreal length) { m_trackLength = length; }

    QString getDifficulty() const { return m_difficulty; }
    void setDifficulty(const QString& difficulty) { m_difficulty = difficulty; }

    int getMaxLaps() const { return m_maxLaps; }
    void setMaxLaps(int laps) { m_maxLaps = laps; }

    bool isValid() const;
    bool validateCheckpoints() const;

    QRectF getBounds() const;
    bool containsPoint(const QVector2D& point) const;

    void clear();
    void clearTiles();

    QVariantMap toVariantMap() const;
    void fromVariantMap(const QVariantMap& data);
    QJsonObject toJsonObject() const;
    void fromJsonObject(const QJsonObject& obj);

signals:
    void trackChanged();
    void tileChanged(int row, int col);
    void checkpointsChanged();
    void startPositionChanged(const QVector2D& position);

private:
    QString m_name;
    QString m_id;
    QString m_author;
    QString m_description;
    int m_rowCount;
    int m_colCount;

    QList<QList<TrackTile*>> m_tiles;
    QMap<int, Checkpoint*> m_checkpoints;
    QList<Checkpoint*> m_checkpointsOrdered;

    QVector2D m_startPosition;
    qreal m_startRotation;
    QList<QVector2D> m_startPositions;

    qreal m_estimatedLapTime;
    qreal m_trackLength;
    QString m_difficulty;
    int m_maxLaps;
};

}

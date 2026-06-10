#pragma once

#include "PhantomDrive_global.h"
#include "track/TrackData.h"

#include <QButtonGroup>
#include <QColor>
#include <QMap>
#include <QPushButton>
#include <QRectF>
#include <QWidget>

namespace PhantomDrive {

enum class CustomTrackBrush {
    Road,
    Grass,
    Barrier,
    Start,
    Finish,
    Checkpoint,
    ItemBox,
    Erase
};

class PHANTOMDRIVE_EXPORT CustomTrackEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CustomTrackEditorWidget(QWidget* parent = nullptr);
    ~CustomTrackEditorWidget() override;

    TrackData* trackData() const { return m_track; }
    void setTrackData(TrackData* track);
    void resetTrack();

signals:
    void trackChanged(PhantomDrive::TrackData* track);
    void playRequested(PhantomDrive::TrackData* track);
    void saveRequested(PhantomDrive::TrackData* track);
    void loadRequested();
    void exportJsonRequested(PhantomDrive::TrackData* track);
    void backRequested();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    void setupButtons();
    void layoutButtons();
    QRectF gridRect() const;
    bool cellAt(const QPointF& pos, int& row, int& col) const;
    QVector2D cellCenter(int row, int col) const;
    void applyBrushAt(int row, int col);
    void setTileType(int row, int col, TileType type);
    void placeStart(int row, int col);
    void placeFinish(int row, int col);
    void placeCheckpoint(int row, int col);
    void placeItemBox(int row, int col);
    void eraseCell(int row, int col);
    void renumberCheckpoints();
    void removeCheckpointAt(int row, int col);
    void removeFinishTiles();
    bool hasItemBoxAt(int row, int col) const;
    bool hasStartAt(int row, int col) const;
    bool isFinishAt(int row, int col) const;
    Checkpoint* checkpointAt(int row, int col) const;
    QColor tileColor(TileType type) const;

    TrackData* m_track;
    QButtonGroup* m_brushGroup;
    QMap<CustomTrackBrush, QPushButton*> m_brushButtons;
    QPushButton* m_playButton;
    QPushButton* m_saveButton;
    QPushButton* m_loadButton;
    QPushButton* m_exportButton;
    QPushButton* m_backButton;
    CustomTrackBrush m_currentBrush;
    qreal m_tileSize;

    static constexpr int EditorRows = 18;
    static constexpr int EditorCols = 24;
};

} // namespace PhantomDrive

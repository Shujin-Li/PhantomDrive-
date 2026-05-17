#pragma once

#include "PhantomDrive_global.h"
#include "GameRenderState.h"
#include "track/TrackData.h"
#include "track/TrackManager.h"

#include <QWidget>
#include <QVector2D>
#include <QList>
#include <QColor>

namespace PhantomDrive {

class GameViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GameViewWidget(QWidget* parent = nullptr);
    ~GameViewWidget() override;

    void setTrackData(TrackData* track);
    void updatePlayerCar(const QVector2D& position, qreal rotation, qreal speed = 0);
    void updateAICar(const QString& carId, const QVector2D& position, qreal rotation, qreal speed = 0);
    void removeAICar(const QString& carId);
    void addPowerupBox(const QString& boxId, const QVector2D& position, const QString& powerupType = "");
    void removePowerupBox(const QString& boxId);
    void addTrafficLight(const QString& lightId, const QVector2D& position, const QString& state = "green");
    void updateTrafficLight(const QString& lightId, const QString& state);
    void addSpeedLimitSign(const QString& signId, const QVector2D& position, int limit);
    void updateSpeedLimitSign(const QString& signId, int limit);
    void addPedestrianCrossing(const QString& crossingId, const QVector2D& position, const QSizeF& size);
    void setRenderState(const RenderState& state);
    const RenderState& getRenderState() const { return m_renderState; }
    void setCameraPosition(const QVector2D& pos);
    void setCameraZoom(qreal zoom);
    void resetCamera();

public slots:
    void clearAll();
    void refresh();

signals:
    void renderStateChanged(const RenderState& state);
    void objectClicked(const GameRenderObject& obj);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    void drawTrack(QPainter& painter);
    void drawPlayerCar(QPainter& painter, const GameRenderObject& car);
    void drawAICar(QPainter& painter, const GameRenderObject& car);
    void drawPowerupBox(QPainter& painter, const GameRenderObject& box);
    void drawTrafficLight(QPainter& painter, const GameRenderObject& light);
    void drawSpeedLimitSign(QPainter& painter, const GameRenderObject& sign);
    void drawPedestrianCrossing(QPainter& painter, const GameRenderObject& crossing);
    QPointF worldToScreen(const QVector2D& worldPos) const;
    QVector2D screenToWorld(const QPointF& screenPos) const;
    QRectF getViewportWorldBounds() const;
    void updateRenderState();

    RenderState m_renderState;
    TrackData* m_currentTrack;
    qreal m_tileSize;
    QColor m_backgroundColor;
    bool m_showGrid;
};

}

#pragma once

#include "PhantomDrive_global.h"
#include "GameRenderState.h"
#include "track/TrackData.h"
#include "track/TrackManager.h"

#include <QWidget>
#include <QVector2D>
#include <QList>
#include <QColor>

class QTimer;

namespace PhantomDrive {

class PHANTOMDRIVE_EXPORT GameViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GameViewWidget(QWidget* parent = nullptr);
    ~GameViewWidget() override;

    void setTrackData(TrackData* track);
    TrackData* trackData() const { return m_currentTrack; }
    void updatePlayerCar(const QVector2D& position, qreal rotation, qreal speed = 0);
    void updatePlayerCar(const QString& playerId,
                         const QVector2D& position,
                         qreal rotation,
                         qreal speed = 0,
                         const QColor& color = QColor(),
                         bool boostActive = false,
                         bool shieldActive = false,
                         bool invisibleActive = false,
                         bool magnetActive = false);
    void updateAICar(const QString& carId, const QVector2D& position, qreal rotation, qreal speed = 0);
    void removeAICar(const QString& carId);
    void clearAllAICars();
    void addPowerupBox(const QString& boxId, const QVector2D& position, const QString& powerupType = "");
    void removePowerupBox(const QString& boxId);
    void addTrafficLight(const QString& lightId, const QVector2D& position, const QString& state = "green");
    void updateTrafficLight(const QString& lightId, const QString& state);
    void addSpeedLimitSign(const QString& signId, const QVector2D& position, int limit);
    void updateSpeedLimitSign(const QString& signId, int limit);
    void addPedestrianCrossing(const QString& crossingId, const QVector2D& position, const QSizeF& size);
    void clearScenarioObjects();
    void setPlayerEffectState(bool boostActive, bool shieldActive,
                              bool invisibleActive = false, bool magnetActive = false);
    void setWorldEffects(const QList<QVector2D>& oilPositions,
                         const QList<qreal>& oilRadii,
                         const QList<QVector2D>& missilePositions);
    void updatePowerupBoxPosition(const QString& boxId, const QVector2D& position);
    void setRenderState(const RenderState& state);
    const RenderState& getRenderState() const { return m_renderState; }
    void setCameraPosition(const QVector2D& pos);
    void setCameraZoom(qreal zoom);
    void resetCamera();
    void showCollisionImpact(const QVector2D& worldPosition, qreal intensity = 1.0);
    void setPaused(bool paused);
    bool isPaused() const { return m_paused; }

public slots:
    void clearAll();
    void refresh();

signals:
    void renderStateChanged(const RenderState& state);
    void objectClicked(const GameRenderObject& obj);
    void keyInputReceived(QKeyEvent* event);
    void keyReleased(QKeyEvent* event);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private:
    void drawOuterBackdrop(QPainter& painter);
    void drawTrack(QPainter& painter);
    void drawCyberGrid(QPainter& painter, const QRectF& worldBounds);
    void drawPlayerCar(QPainter& painter, const GameRenderObject& car);
    void drawAICar(QPainter& painter, const GameRenderObject& car);
    void drawRaceCar(QPainter& painter, const GameRenderObject& car, bool playerCar);
    void drawPowerupBox(QPainter& painter, const GameRenderObject& box);
    void drawTrafficLight(QPainter& painter, const GameRenderObject& light);
    void drawSpeedLimitSign(QPainter& painter, const GameRenderObject& sign);
    void drawPedestrianCrossing(QPainter& painter, const GameRenderObject& crossing);
    void drawWorldEffects(QPainter& painter);
    void drawCollisionImpacts(QPainter& painter);
    void drawPausedOverlay(QPainter& painter);
    void drawCheckpoints(QPainter& painter);
    void drawStartFinishMarkers(QPainter& painter);
    QPointF worldToScreen(const QVector2D& worldPos) const;
    QVector2D screenToWorld(const QPointF& screenPos) const;
    QRectF getViewportWorldBounds() const;
    void updateRenderState();

    RenderState m_renderState;
    TrackData* m_currentTrack;
    qreal m_tileSize;
    QColor m_backgroundColor;
    bool m_showGrid;
    bool m_playerBoostActive;
    bool m_playerShieldActive;
    bool m_playerInvisibleActive;
    bool m_playerMagnetActive;
    bool m_paused;

    struct CollisionImpactEffect {
        QVector2D worldPosition;
        qint64 startMs = 0;
        int durationMs = 320;
        qreal intensity = 1.0;
    };

    QList<CollisionImpactEffect> m_collisionImpactEffects;
    QTimer* m_collisionImpactTimer;
    qint64 m_lastCollisionImpactMs;

    QList<QVector2D> m_oilPuddlePositions;
    QList<qreal> m_oilPuddleRadii;
    QList<QVector2D> m_missilePositions;
};

}

#pragma once

#include "PhantomDrive_global.h"
#include "GameRenderState.h"
#include "collectible/BlockerVehicle.h"
#include "collectible/ChallengeObstacle.h"
#include "collectible/CoinItem.h"
#include "track/TrackData.h"
#include "track/TrackManager.h"

#include <QObject>
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
                         bool magnetActive = false,
                         const QString& skinId = QString());
    void updateAICar(const QString& carId, const QVector2D& position, qreal rotation, qreal speed = 0);
    void removeAICar(const QString& carId);
    void clearAllAICars();
    void updateCoins(const QList<CoinItem>& coins);
    void clearCoins();
    void updateChallengeObstacles(const QList<ChallengeObstacle>& obstacles);
    void clearChallengeObstacles();
    void updateBlockerVehicles(const QList<BlockerVehicle>& vehicles);
    void clearBlockerVehicles();
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
    void setCoinChallengeOverlayState(const CoinChallengeOverlayState& state);
    void setBalloonRushSceneState(const BalloonRushSceneState& state);
    void clearCoinChallengeOverlayState();
    void clearBalloonRushSceneState();
    const RenderState& getRenderState() const { return m_renderState; }
    void setCameraPosition(const QVector2D& pos);
    void setCameraZoom(qreal zoom);
    void resetCamera();
    void showCollisionImpact(const QVector2D& worldPosition, qreal intensity = 1.0);
    void triggerCoinGoalAnimation(const QPointF& worldPosition, int value = 1);
    void clearCoinGoalAnimations();
    void triggerMilestoneCelebration(const QString& headline,
                                     const QString& detail,
                                     const QColor& accent = QColor());
    void clearMilestoneCelebrations();
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
    void drawCoin(QPainter& painter, const GameRenderObject& coin);
    void drawChallengeObstacle(QPainter& painter, const GameRenderObject& obstacle);
    void drawBlockerVehicle(QPainter& painter, const GameRenderObject& vehicle);
    void drawPowerupBox(QPainter& painter, const GameRenderObject& box);
    void drawTrafficLight(QPainter& painter, const GameRenderObject& light);
    void drawSpeedLimitSign(QPainter& painter, const GameRenderObject& sign);
    void drawPedestrianCrossing(QPainter& painter, const GameRenderObject& crossing);
    void drawWorldEffects(QPainter& painter);
    void drawCollisionImpacts(QPainter& painter);
    void drawCoinChallengeOverlay(QPainter& painter);
    void drawCoinChallengeCountdownOverlay(QPainter& painter, const CoinChallengeOverlayState& overlay);
    void drawBalloonRushSceneOverlay(QPainter& painter);
    void drawCoinGoalAnimations(QPainter& painter);
    void drawMilestoneCelebrations(QPainter& painter);
    void drawPausedOverlay(QPainter& painter);
    void drawCheckpoints(QPainter& painter);
    void drawStartFinishMarkers(QPainter& painter);
    void advanceCoinGoalAnimations();
    void advanceMilestoneCelebrations();
    QRectF coinGoalOverlayRect() const;
    QRectF coinGoalTrackRect() const;
    QPointF coinGoalAnimationTarget(int value = 1) const;
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

    struct CoinFlyAnim {
        QPointF start;
        QPointF control;
        QPointF end;
        qreal t = 0.0;
        qreal duration = 0.72;
        qreal rotation = 0.0;
        qreal rotationSpeed = 720.0;
        qreal startScale = 1.22;
        qreal endScale = 0.38;
        int value = 1;
    };

    struct MilestoneCelebration {
        QString headline;
        QString detail;
        QString channelLabel;
        QColor accent;
        qreal progress = 0.0;
        qreal duration = 2.5;
        bool compact = false;
    };

    QList<CollisionImpactEffect> m_collisionImpactEffects;
    QTimer* m_collisionImpactTimer;
    qint64 m_lastCollisionImpactMs;
    QList<CoinFlyAnim> m_coinFlyAnimations;
    QTimer* m_coinFlyAnimTimer;
    QList<MilestoneCelebration> m_milestoneCelebrations;
    QTimer* m_milestoneAnimTimer;
    int m_coinChallengeDisplayedRunCoins;
    int m_coinChallengeTargetRunCoins;
    qreal m_coinGoalFlashProgress;
    bool m_coinGoalDisplayInitialized;

    QList<QVector2D> m_oilPuddlePositions;
    QList<qreal> m_oilPuddleRadii;
    QList<QVector2D> m_missilePositions;
};

}

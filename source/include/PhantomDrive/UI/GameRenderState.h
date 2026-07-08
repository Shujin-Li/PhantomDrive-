#pragma once

#include "PhantomDrive_global.h"

#include <QObject>
#include <QVector2D>
#include <QString>
#include <QList>
#include <QColor>
#include <QPointF>
#include <QVariantMap>
#include <QRectF>

namespace PhantomDrive {

enum class RenderObjectType {
    PlayerCar,
    AICar,
    CollectibleCoin,
    ChallengeObstacle,
    BlockerVehicle,
    PowerupBox,
    TrafficLight,
    SpeedLimitSign,
    PedestrianCrossing,
    TrackTile
};

struct GameRenderObject {
    RenderObjectType type;
    QVector2D position;
    qreal rotation;
    QSizeF size;
    QString label;
    QColor color;
    QVariantMap extraData;

    GameRenderObject()
        : type(RenderObjectType::TrackTile)
        , position(0, 0)
        , rotation(0)
        , size(32, 32)
        , color(Qt::white)
    {}
};

struct CoinChallengeOverlayState {
    bool visible = false;
    bool countdownActive = false;
    bool tenSecondWarningVisible = false;
    bool goalComplete = false;
    bool criticalIntegrity = false;
    bool destroyed = false;
    bool forcedEndActive = false;
    int remainingSeconds = 0;
    int countdownSeconds = 0;
    int countdownStage = -1;
    int totalCoins = 0;
    int runCoins = 0;
    int activeCoins = 0;
    int goalCoins = 70;
    int speedKmh = 0;
    int averageSpeedKmh = 0;
    int maxSpeedKmh = 0;
    int efficiencyCoinsPerMinute = 0;
    int powerupRemainingSeconds = 0;
    int integrityPercent = 100;
    int integrityStage = 0;
    int recentDamageAmount = 0;
    qreal goalProgress = 0.0;
    qreal timeProgress = 0.0;
    qreal countdownStageProgress = 0.0;
    qreal tenSecondWarningProgress = 0.0;
    qreal damageFlashProgress = 0.0;
    qreal lowIntegrityPulseProgress = 0.0;
    qreal forcedEndProgress = 0.0;
    bool powerupActive = false;
    int loopsCompleted = 0;
    QString powerupLabel;
    QString routeHintText;
    QString integrityStatusText;
    QString damagePopupText;
    QString damagePopupDetail;
    QString forcedEndHeadline;
    QString forcedEndDetail;
    QList<QVector2D> routeGuidePoints;
};

struct BalloonRushSceneState {
    bool active = false;
    bool triggerVisible = false;
    bool bonusSceneVisible = false;
    bool returnTransitionVisible = false;
    bool introCountdownVisible = false;
    bool milestonePopupVisible = false;
    int remainingSeconds = 0;
    int introCountdownValue = 0;
    int collectedCoins = 0;
    int runCoins = 0;
    int goalCoins = 70;
    int recentGainValue = 0;
    int laneIndex = 1;
    qreal laneVisual = 1.0;
    qreal triggerProgress = 0.0;
    qreal badgeOpacity = 0.0;
    qreal badgeScale = 1.0;
    qreal badgeYOffset = 0.0;
    qreal goalProgress = 0.0;
    qreal gainFlashProgress = 0.0;
    qreal introCountdownProgress = 0.0;
    qreal milestonePopupProgress = 0.0;
    qreal returnTransitionProgress = 0.0;
    qreal roadScroll = 0.0;
    QString headline;
    QString detail;
    QString introCountdownLabel;
    QString milestoneHeadline;
    QString milestoneDetail;
    QColor milestoneAccent;
    QList<QPointF> coinLayout;
};

struct RenderState {
    QList<GameRenderObject> trackTiles;
    QList<GameRenderObject> playerCars;
    QList<GameRenderObject> aiCars;
    QList<GameRenderObject> coins;
    QList<GameRenderObject> challengeObstacles;
    QList<GameRenderObject> blockerVehicles;
    QList<GameRenderObject> powerupBoxes;
    QList<GameRenderObject> trafficLights;
    QList<GameRenderObject> speedLimitSigns;
    QList<GameRenderObject> pedestrianCrossings;

    QVector2D cameraPosition;
    qreal cameraZoom;
    QRectF viewportBounds;
    CoinChallengeOverlayState coinChallengeOverlay;
    BalloonRushSceneState balloonRushScene;

    RenderState()
        : cameraPosition(0, 0)
        , cameraZoom(1.0)
    {}

    void clear() {
        trackTiles.clear();
        playerCars.clear();
        aiCars.clear();
        coins.clear();
        challengeObstacles.clear();
        blockerVehicles.clear();
        powerupBoxes.clear();
        trafficLights.clear();
        speedLimitSigns.clear();
        pedestrianCrossings.clear();
        coinChallengeOverlay = CoinChallengeOverlayState();
        balloonRushScene = BalloonRushSceneState();
    }
};

}

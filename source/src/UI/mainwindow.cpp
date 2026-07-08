#include "UI/mainwindow.h"
#include "ui_mainwindow.h"

#include "UI/DrivingReportWidget.h"
#include "UI/GarageWidget.h"
#include "UI/MainMenuWidget.h"
#include "UI/CustomTrackEditorWidget.h"
#include "UI/ThemeManager.h"
#include "collectible/BlockerVehicleManager.h"
#include "collectible/ChallengeObstacleManager.h"
#include "collectible/CollectibleManager.h"
#include "gamemode/CustomTrackMode.h"
#include "gamemode/VehicleSensor.h"
#include "gamemode/PowerupBox.h"
#include "gamemode/PowerupManager.h"
#include "gamemode/PowerupWorldRuntime.h"
#include "gamemode/TrafficLightObject.h"
#include "gamemode/TrafficObjectManager.h"
#include "gamemode/SpeedLimitSignObject.h"
#include "gamemode/PedestrianCrossingObject.h"
#include "gamemode/AIOpponent.h"
#include "gamemode/SimpleAIOpponent.h"
#include "track/TrackData.h"
#include "track/TrackManager.h"
#include "track/TrackTile.h"
#include "track/Checkpoint.h"
#include "track/TrackIO.h"
#include "track/TrackValidator.h"
#include "track/BuiltInTrackFactory.h"

#include <QRectF>
#include "scoring/ScoreReport.h"
#include "core/saveloadmanager.h"
#include "core/VehiclePhysics.h"
#include "economy/CurrencyManager.h"
#include "profile/PlayerProfileStore.h"
#include "shop/SkinManager.h"

#include <QApplication>
#include <QComboBox>
#include <QDateTime>
#include <QDebug>
#include <QDialog>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QHash>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLayoutItem>
#include <QMessageBox>
#include <QPointer>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QScreen>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QStatusBar>
#include <QStringList>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QVariant>
#include <QDir>
#include <QFileInfo>
#include <QRandomGenerator>
#include <QtMath>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

#include <algorithm>
#include <limits>

using namespace PhantomDrive;

namespace {

constexpr qreal kVehicleSeparationDistance = 48.0;
constexpr qreal kVehicleImpactDistance = 34.0;
constexpr qreal kVehicleContactReleaseDistance = 52.0;
constexpr qint64 kFinishedAiContactCooldownMs = 1000;
constexpr qint64 kVehicleImpactVisualCooldownMs = 350;
constexpr qint64 kRaceStartCollisionImpactGuardMs = 1000;
constexpr int kCoinChallengeDefaultDurationMs = 60 * 1000;
constexpr int kCoinChallengeGoalCoins = 70;
constexpr int kCoinChallengeCountdownGoWindowMs = 350;
constexpr int kCoinChallengeMagnetDurationMs = 4000;
constexpr int kCoinChallengeBalloonRushTriggerDurationMs = 1300;
constexpr int kCoinChallengeBalloonRushSceneDurationMs = 7000;
constexpr int kCoinChallengeBalloonRushReturnTransitionDurationMs = 1050;
constexpr int kCoinChallengeBalloonRushIntroCountdownTotalMs = 3000;
constexpr int kCoinChallengeIntegrityMax = 100;
constexpr int kCoinChallengeDamageCooldownMs = 480;
constexpr int kCoinChallengeDamageFlashDurationMs = 560;
constexpr int kCoinChallengeDamagePopupDurationMs = 1150;
constexpr int kCoinChallengeLowIntegrityWarningDurationMs = 380;
constexpr int kCoinChallengeForcedEndDurationMs = 1650;
constexpr int kCoinChallengeBlockerCollisionDamage = 18;
constexpr qreal kCoinChallengeMagnetRadius = 210.0;
constexpr qreal kCoinChallengeMagnetPullSpeed = 460.0;
constexpr float kCoinChallengePowerupRespawnSeconds = 999.0f;
constexpr int kCoinChallengeBalloonRushMilestone = 30;
constexpr qreal kCoinChallengeBalloonRushCoinSpeedPerSecond = 1.05;
constexpr qreal kCoinChallengeBalloonRushCoinSpawnIntervalMs = 255.0;
constexpr int kCoinChallengeCriticalIntegrityThreshold = 25;

int coinChallengeObstacleDamage(ChallengeObstacleType type)
{
    switch (type) {
    case ChallengeObstacleType::Rock:
        return 5;
    case ChallengeObstacleType::ConeBarrier:
    case ChallengeObstacleType::ConstructionBarrel:
        return 8;
    case ChallengeObstacleType::Tree:
        return 12;
    case ChallengeObstacleType::ConstructionWall:
        return 15;
    default:
        return 10;
    }
}

QString coinChallengeObstacleDamageDetail(ChallengeObstacleType type)
{
    switch (type) {
    case ChallengeObstacleType::Rock:
        return QStringLiteral("Rock impact");
    case ChallengeObstacleType::ConeBarrier:
        return QStringLiteral("Barrier hit");
    case ChallengeObstacleType::ConstructionBarrel:
        return QStringLiteral("Barrel hit");
    case ChallengeObstacleType::Tree:
        return QStringLiteral("Tree impact");
    case ChallengeObstacleType::ConstructionWall:
        return QStringLiteral("Heavy wall hit");
    default:
        return QStringLiteral("Vehicle damaged");
    }
}

QString coinChallengeIntegrityStatusLabel(int integrity)
{
    if (integrity <= 0) {
        return QStringLiteral("DESTROYED");
    }
    if (integrity <= 20) {
        return QStringLiteral("CRITICAL");
    }
    if (integrity <= 40) {
        return QStringLiteral("HEAVY DAMAGE");
    }
    if (integrity <= 70) {
        return QStringLiteral("DAMAGED");
    }
    return QStringLiteral("GOOD");
}

QString coinChallengeEndReasonLabel(CoinChallengeEndReason reason, bool goalComplete)
{
    switch (reason) {
    case CoinChallengeEndReason::GoalComplete:
        return QStringLiteral("Goal Complete");
    case CoinChallengeEndReason::ForcedFinish:
        return QStringLiteral("Forced Finish");
    case CoinChallengeEndReason::VehicleDestroyed:
        return QStringLiteral("Vehicle Destroyed");
    case CoinChallengeEndReason::FatalCrash:
        return QStringLiteral("Fatal Crash");
    case CoinChallengeEndReason::Timeout:
        return goalComplete ? QStringLiteral("Goal Complete") : QStringLiteral("Time Up");
    case CoinChallengeEndReason::None:
    default:
        return goalComplete ? QStringLiteral("Goal Complete") : QStringLiteral("Challenge Complete");
    }
}

SoundEffect coinChallengeCountdownSoundForStage(int stage)
{
    switch (stage) {
    case 3:
        return SoundEffect::CountdownThree;
    case 2:
        return SoundEffect::CountdownTwo;
    case 1:
        return SoundEffect::CountdownOne;
    default:
        return SoundEffect::CountdownGo;
    }
}

int balloonRushIntroCountdownStage(int remainingMs)
{
    if (remainingMs <= 0 || remainingMs > kCoinChallengeBalloonRushIntroCountdownTotalMs) {
        return -1;
    }
    if (remainingMs > 2000) {
        return 3;
    }
    if (remainingMs > 1000) {
        return 2;
    }
    return 1;
}

QString balloonRushIntroCountdownLabel(int remainingMs)
{
    switch (balloonRushIntroCountdownStage(remainingMs)) {
    case 3:
        return QStringLiteral("3");
    case 2:
        return QStringLiteral("2");
    case 1:
        return QStringLiteral("1");
    default:
        return QString();
    }
}

qreal balloonRushIntroCountdownStageProgress(int remainingMs)
{
    const int stage = balloonRushIntroCountdownStage(remainingMs);
    switch (stage) {
    case 3:
        return qBound<qreal>(0.0,
                             static_cast<qreal>(kCoinChallengeBalloonRushIntroCountdownTotalMs - remainingMs) / 1000.0,
                             1.0);
    case 2:
        return qBound<qreal>(0.0,
                             static_cast<qreal>(2000 - remainingMs) / 1000.0,
                             1.0);
    case 1:
        return qBound<qreal>(0.0,
                             static_cast<qreal>(1000 - remainingMs) / 1000.0,
                             1.0);
    default:
        return 0.0;
    }
}

constexpr qreal kCoinChallengeSpawnTileSize = TrackData::DefaultTileSize;

bool isCoinChallengeDrivableSurface(TileType type)
{
    switch (type) {
    case TileType::Road:
    case TileType::Asphalt:
    case TileType::StartLine:
    case TileType::FinishLine:
        return true;
    default:
        return false;
    }
}

bool snapToNearestCoinChallengeRoadPoint(TrackData* track, const QVector2D& source, QVector2D& snapped, int maxTileRadius = 3)
{
    if (!track) {
        return false;
    }

    const QPoint tileCoord = TrackData::worldToTile(source, kCoinChallengeSpawnTileSize);
    qreal bestDistanceSquared = std::numeric_limits<qreal>::max();
    bool found = false;

    for (int radius = 0; radius <= maxTileRadius; ++radius) {
        for (int row = tileCoord.y() - radius; row <= tileCoord.y() + radius; ++row) {
            for (int col = tileCoord.x() - radius; col <= tileCoord.x() + radius; ++col) {
                TrackTile* tile = track->getTileAt(row, col);
                if (!tile || !isCoinChallengeDrivableSurface(tile->getType())) {
                    continue;
                }

                const QVector2D center = TrackData::tileToWorldCenter(row, col, kCoinChallengeSpawnTileSize);
                const qreal distanceSquared = (center - source).lengthSquared();
                if (!found || distanceSquared < bestDistanceSquared) {
                    snapped = center;
                    bestDistanceSquared = distanceSquared;
                    found = true;
                }
            }
        }
        if (found) {
            return true;
        }
    }

    return false;
}

QPointF quadraticPoint(const QPointF& start, const QPointF& control, const QPointF& end, qreal t)
{
    const qreal invT = 1.0 - t;
    return start * (invT * invT) + control * (2.0 * invT * t) + end * (t * t);
}

qreal easeOutCubic(qreal t)
{
    const qreal clamped = qBound<qreal>(0.0, t, 1.0);
    const qreal inv = 1.0 - clamped;
    return 1.0 - inv * inv * inv;
}

void drawTinyRewardCar(QPainter& painter, const QRectF& bounds, const QColor& accent)
{
    painter.save();
    painter.setBrush(QColor(10, 18, 36, 190));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(bounds.adjusted(0.0, bounds.height() * 0.30, 0.0, 0.0), 6.0, 6.0);

    QLinearGradient bodyFill(bounds.topLeft(), bounds.bottomRight());
    bodyFill.setColorAt(0.0, accent.lighter(155));
    bodyFill.setColorAt(0.65, accent);
    bodyFill.setColorAt(1.0, QColor(34, 16, 78));
    painter.setBrush(bodyFill);
    painter.setPen(QPen(QColor(214, 248, 255, 180), 1.4));
    painter.drawRoundedRect(bounds, 7.0, 7.0);

    painter.setBrush(QColor(12, 28, 52, 210));
    painter.setPen(QPen(QColor(86, 224, 255, 180), 1.0));
    painter.drawRoundedRect(QRectF(bounds.left() + bounds.width() * 0.18,
                                   bounds.top() + bounds.height() * 0.18,
                                   bounds.width() * 0.64,
                                   bounds.height() * 0.28),
                            4.0,
                            4.0);

    painter.setBrush(QColor(24, 24, 28));
    painter.setPen(QPen(QColor(118, 118, 130), 0.9));
    painter.drawEllipse(QPointF(bounds.left() + bounds.width() * 0.24, bounds.bottom()), bounds.width() * 0.12, bounds.width() * 0.12);
    painter.drawEllipse(QPointF(bounds.right() - bounds.width() * 0.24, bounds.bottom()), bounds.width() * 0.12, bounds.width() * 0.12);
    painter.restore();
}

QString resolveCoinChallengeRewardArtPath()
{
    const QStringList assetRelatives = {
        QStringLiteral("ui/coinspic.png"),
        QStringLiteral("coinspic.png")
    };
    const QStringList roots = {
        QCoreApplication::applicationDirPath() + QStringLiteral("/assets"),
        QCoreApplication::applicationDirPath() + QStringLiteral("/../assets"),
        QCoreApplication::applicationDirPath() + QStringLiteral("/../../assets"),
        QCoreApplication::applicationDirPath() + QStringLiteral("/../../../assets"),
        QDir::currentPath() + QStringLiteral("/assets"),
        QDir::currentPath() + QStringLiteral("/source/assets")
    };

    for (const QString& root : roots) {
        for (const QString& assetRelative : assetRelatives) {
            const QString candidate = QDir(root).filePath(assetRelative);
            const QFileInfo info(candidate);
            if (info.exists() && info.isFile()) {
                return info.absoluteFilePath();
            }
        }
    }

    return QString();
}

QPixmap loadCoinChallengeRewardArt()
{
    static QString cachedPath;
    static qint64 cachedStampMs = -1;
    static QPixmap cachedPixmap;

    const QString path = resolveCoinChallengeRewardArtPath();
    if (path.isEmpty()) {
        cachedPath.clear();
        cachedStampMs = -1;
        cachedPixmap = QPixmap();
        return cachedPixmap;
    }

    const QFileInfo info(path);
    const qint64 stampMs = info.lastModified().toMSecsSinceEpoch();
    if (path != cachedPath || stampMs != cachedStampMs || cachedPixmap.isNull()) {
        cachedPath = path;
        cachedStampMs = stampMs;
        cachedPixmap.load(path);
    }

    return cachedPixmap;
}

qreal rewardPixmapDevicePixelRatio()
{
    if (QScreen* screen = QApplication::primaryScreen()) {
        return qMax<qreal>(1.0, screen->devicePixelRatio());
    }
    return 1.0;
}

QPixmap makeCoinChallengeChestVisual(const QSize& size, int runCoins, bool goalComplete, qreal animationProgress)
{
    const qreal dpr = rewardPixmapDevicePixelRatio();
    QPixmap pixmap(qRound(size.width() * dpr), qRound(size.height() * dpr));
    pixmap.setDevicePixelRatio(dpr);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const QRectF bounds(QPointF(0.0, 0.0), QSizeF(size));
    const QRectF safeBounds = bounds.adjusted(10.0, 16.0, -10.0, -10.0);
    const QRectF base(safeBounds.left() + 18.0, safeBounds.top() + 70.0, safeBounds.width() * 0.60, safeBounds.height() * 0.42);
    const QRectF lid(base.left() + 18.0, base.top() - base.height() * 0.40, base.width() * 0.84, base.height() * 0.44);
    const qreal anim = easeOutCubic(animationProgress);
    const QColor frameGold = goalComplete ? QColor(255, 225, 118) : QColor(255, 200, 66);
    const QColor deepGold = goalComplete ? QColor(231, 162, 22) : QColor(218, 142, 14);
    const QColor purpleA(88, 34, 118);
    const QColor purpleB(44, 22, 92);

    QRadialGradient glow(safeBounds.center() + QPointF(-12.0, 6.0), safeBounds.width() * 0.48);
    glow.setColorAt(0.0, QColor(255, 224, 108, goalComplete ? 130 + qRound(anim * 48.0) : 92 + qRound(anim * 36.0)));
    glow.setColorAt(0.68, QColor(255, 180, 60, 36));
    glow.setColorAt(1.0, QColor(255, 180, 60, 0));
    painter.setBrush(glow);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(safeBounds.adjusted(-8.0, -6.0, 12.0, 10.0));

    painter.setBrush(QColor(12, 16, 24, 88));
    painter.drawEllipse(QRectF(base.left() + 8.0, base.bottom() - 12.0, base.width() * 1.18, 26.0));

    QLinearGradient baseFrame(base.topLeft(), base.bottomRight());
    baseFrame.setColorAt(0.0, frameGold.lighter(135));
    baseFrame.setColorAt(0.55, frameGold);
    baseFrame.setColorAt(1.0, deepGold.darker(130));
    painter.setBrush(baseFrame);
    painter.setPen(QPen(QColor(255, 245, 180, 220), 3.0));
    painter.drawRoundedRect(base, 18.0, 18.0);

    QRectF frontPanel = base.adjusted(18.0, 16.0, -18.0, -18.0);
    QLinearGradient frontFill(frontPanel.topLeft(), frontPanel.bottomRight());
    frontFill.setColorAt(0.0, purpleA);
    frontFill.setColorAt(1.0, purpleB);
    painter.setBrush(frontFill);
    painter.setPen(QPen(QColor(255, 215, 104, 220), 2.2));
    painter.drawRoundedRect(frontPanel, 12.0, 12.0);

    painter.setPen(QPen(QColor(255, 228, 132, 220), 6.0));
    painter.drawLine(QPointF(frontPanel.left() + 8.0, frontPanel.top() + 12.0),
                     QPointF(frontPanel.right() - 8.0, frontPanel.top() + 12.0));
    painter.drawLine(QPointF(frontPanel.left() + frontPanel.width() * 0.14, frontPanel.bottom() - 8.0),
                     QPointF(frontPanel.left() + frontPanel.width() * 0.86, frontPanel.bottom() - 8.0));

    QRectF lockRect(frontPanel.center().x() - 15.0, frontPanel.top() + 14.0, 30.0, 36.0);
    painter.setBrush(QColor(248, 190, 48));
    painter.setPen(QPen(QColor(126, 68, 8), 2.0));
    painter.drawRoundedRect(lockRect, 8.0, 8.0);
    painter.drawArc(QRectF(lockRect.left() + 6.0, lockRect.top() - 12.0, 18.0, 18.0), 0, 180 * 16);
    painter.drawEllipse(QRectF(lockRect.center().x() - 3.5, lockRect.center().y() - 3.0, 7.0, 7.0));

    painter.save();
    painter.translate(lid.center());
    painter.rotate(-24.0);
    QLinearGradient lidFrame(QPointF(-lid.width() * 0.5, -lid.height() * 0.5),
                             QPointF(lid.width() * 0.5, lid.height() * 0.5));
    lidFrame.setColorAt(0.0, frameGold.lighter(140));
    lidFrame.setColorAt(0.55, frameGold);
    lidFrame.setColorAt(1.0, deepGold);
    painter.setBrush(lidFrame);
    painter.setPen(QPen(QColor(255, 242, 188, 220), 3.0));
    painter.drawRoundedRect(QRectF(-lid.width() * 0.5, -lid.height() * 0.5, lid.width(), lid.height()), 18.0, 18.0);
    painter.setBrush(QColor(76, 28, 112));
    painter.setPen(QPen(QColor(255, 220, 118), 2.0));
    painter.drawRoundedRect(QRectF(-lid.width() * 0.32, -lid.height() * 0.28, lid.width() * 0.64, lid.height() * 0.56), 12.0, 12.0);
    painter.restore();

    const int displayCoins = qMax(0, runCoins);
    const int coinCount = qBound(4, 5 + displayCoins / 14, 10);
    for (int i = 0; i < coinCount; ++i) {
        const qreal angle = -24.0 + i * 13.0;
        const qreal x = base.left() + base.width() * (0.18 + (i % 4) * 0.15);
        const qreal y = base.top() - 6.0 - (i / 4) * 16.0 - (i % 2) * 6.0;
        painter.save();
        painter.translate(x, y);
        painter.rotate(angle);
        painter.setBrush(QColor(255, 214, 92));
        painter.setPen(QPen(QColor(255, 245, 184), 2.0));
        painter.drawEllipse(QRectF(-18.0, -18.0, 36.0, 36.0));
        painter.setPen(QPen(QColor(163, 98, 8), 1.8));
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 16, QFont::Black));
        painter.drawText(QRectF(-13.0, -15.0, 26.0, 30.0), Qt::AlignCenter, QStringLiteral("$"));
        painter.restore();
    }

    const QPointF chestTarget(base.left() + base.width() * 0.52, base.top() - 10.0);
    const int flyCoins = qBound(4, 4 + displayCoins / 16, 7);
    for (int i = 0; i < flyCoins; ++i) {
        const qreal localT = qBound<qreal>(0.0, anim * 1.28 - i * 0.13, 1.0);
        if (localT <= 0.0) {
            continue;
        }

        const QPointF start(safeBounds.right() - 26.0 - i * 18.0,
                            safeBounds.top() + 38.0 + (i % 3) * 26.0);
        const QPointF control(safeBounds.center().x() + 40.0 - i * 10.0,
                              safeBounds.top() - 12.0 - i * 6.0);
        const QPointF point = quadraticPoint(start, control, chestTarget, easeOutCubic(localT));
        const qreal scale = 1.10 - localT * 0.58;
        const qreal alpha = 255.0 * (1.0 - qMax<qreal>(0.0, localT - 0.86) / 0.14);

        painter.save();
        painter.translate(point);
        painter.rotate(22.0 + i * 12.0 + localT * 280.0);
        painter.setBrush(QColor(255, 214, 92, qRound(alpha)));
        painter.setPen(QPen(QColor(255, 245, 184, qRound(alpha * 0.88)), 1.8));
        painter.drawEllipse(QRectF(-13.0 * scale, -13.0 * scale, 26.0 * scale, 26.0 * scale));
        painter.setPen(QPen(QColor(163, 98, 8, qRound(alpha * 0.72)), 1.2));
        painter.setFont(QFont(QStringLiteral("Segoe UI"), qMax(8, qRound(11 * scale)), QFont::Black));
        painter.drawText(QRectF(-10.0 * scale, -10.0 * scale, 20.0 * scale, 20.0 * scale),
                         Qt::AlignCenter,
                         QStringLiteral("$"));
        painter.restore();
    }

    drawTinyRewardCar(painter,
                      QRectF(bounds.left() + 26.0, bounds.bottom() - 54.0, 62.0, 24.0),
                      goalComplete ? QColor(88, 255, 182) : QColor(68, 220, 255));

    painter.setPen(goalComplete ? QColor(125, 255, 184) : QColor(255, 236, 184));
    painter.setFont(QFont(QStringLiteral("Segoe UI"), 14, QFont::Black));
    painter.drawText(QRectF(safeBounds.left(), safeBounds.bottom() - 24.0, safeBounds.width(), 18.0),
                     Qt::AlignCenter,
                     goalComplete ? QStringLiteral("TREASURE SECURED") : QStringLiteral("CHEST FILLED"));
    return pixmap;
}

QPixmap makeCoinChallengeBagVisual(const QSize& size, int totalCoins, int runCoins, qreal animationProgress)
{
    const qreal dpr = rewardPixmapDevicePixelRatio();
    QPixmap pixmap(qRound(size.width() * dpr), qRound(size.height() * dpr));
    pixmap.setDevicePixelRatio(dpr);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const qreal anim = easeOutCubic(animationProgress);
    const QRectF bounds(QPointF(0.0, 0.0), QSizeF(size));
    const QRectF safeBounds = bounds.adjusted(12.0, 16.0, -12.0, -10.0);
    const QPixmap rewardArt = loadCoinChallengeRewardArt();
    if (!rewardArt.isNull()) {
        const QRectF artArea = safeBounds.adjusted(0.0, -2.0, 0.0, -28.0);
        const QSize scaledSize = rewardArt.size().scaled(artArea.size().toSize(),
                                                         Qt::KeepAspectRatio);
        const QRectF artRect(artArea.left() + (artArea.width() - scaledSize.width()) * 0.5,
                             artArea.top() + (artArea.height() - scaledSize.height()) * 0.44,
                             scaledSize.width(),
                             scaledSize.height());

        QRadialGradient artGlow(artRect.center() + QPointF(artRect.width() * 0.06, artRect.height() * 0.02),
                                artRect.width() * 0.56);
        artGlow.setColorAt(0.0, QColor(255, 222, 104, 116 + qRound(anim * 52.0)));
        artGlow.setColorAt(0.52, QColor(255, 180, 58, 38));
        artGlow.setColorAt(1.0, QColor(255, 180, 58, 0));
        painter.setBrush(artGlow);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(artRect.adjusted(-28.0, -14.0, 24.0, 28.0));

        painter.setBrush(QColor(8, 12, 22, 96));
        painter.drawEllipse(QRectF(artRect.left() + artRect.width() * 0.10,
                                   artRect.bottom() - 14.0,
                                   artRect.width() * 0.78,
                                   26.0));

        painter.drawPixmap(artRect.toAlignedRect(),
                           rewardArt.scaled(scaledSize,
                                            Qt::KeepAspectRatio,
                                            Qt::SmoothTransformation));

        const QPointF bagTarget(artRect.left() + artRect.width() * 0.70,
                                artRect.top() + artRect.height() * 0.56);
        const int flyCoins = qBound(4, 4 + runCoins / 15, 8);
        for (int i = 0; i < flyCoins; ++i) {
            const qreal localT = qBound<qreal>(0.0, anim * 1.35 - i * 0.11, 1.0);
            if (localT <= 0.0) {
                continue;
            }

            const QPointF start(safeBounds.left() + 18.0 + (i % 3) * 28.0,
                                safeBounds.top() + 28.0 + i * 18.0);
            const QPointF control(safeBounds.center().x() - 12.0 + i * 9.0,
                                  safeBounds.top() - 20.0 - (i % 2) * 12.0);
            const QPointF point = quadraticPoint(start, control, bagTarget, easeOutCubic(localT));
            const qreal scale = 1.12 - localT * 0.60;
            const qreal alpha = 255.0 * (1.0 - qMax<qreal>(0.0, localT - 0.84) / 0.16);

            painter.save();
            painter.translate(point);
            painter.rotate(-12.0 + i * 14.0 + localT * 320.0);
            painter.setBrush(QColor(255, 214, 92, qRound(alpha)));
            painter.setPen(QPen(QColor(255, 246, 190, qRound(alpha * 0.88)), 1.8));
            painter.drawEllipse(QRectF(-12.0 * scale, -12.0 * scale, 24.0 * scale, 24.0 * scale));
            painter.setPen(QPen(QColor(160, 98, 10, qRound(alpha * 0.72)), 1.0));
            painter.drawLine(QPointF(-6.0 * scale, 0.0), QPointF(6.0 * scale, 0.0));
            painter.restore();
        }

        const QRectF bankBadge(safeBounds.left() + 26.0, safeBounds.bottom() - 38.0, safeBounds.width() - 52.0, 28.0);
        painter.setBrush(QColor(9, 18, 34, 198));
        painter.setPen(QPen(QColor(255, 215, 118, 148), 1.4));
        painter.drawRoundedRect(bankBadge, 12.0, 12.0);
        painter.setPen(QColor(255, 241, 204));
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 12, QFont::Black));
        painter.drawText(bankBadge, Qt::AlignCenter, QStringLiteral("BANK: %1").arg(totalCoins));
        return pixmap;
    }

    const QRectF bagRect(safeBounds.left() + safeBounds.width() * 0.29,
                         safeBounds.top() + 24.0,
                         safeBounds.width() * 0.42,
                         safeBounds.height() * 0.60);

    QRadialGradient bagGlow(bagRect.center() + QPointF(10.0, 4.0), bagRect.width() * 1.05);
    bagGlow.setColorAt(0.0, QColor(255, 224, 118, 126 + qRound(anim * 50.0)));
    bagGlow.setColorAt(0.58, QColor(255, 176, 58, 42));
    bagGlow.setColorAt(1.0, QColor(255, 176, 58, 0));
    painter.setBrush(bagGlow);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QRectF(bagRect.left() - 34.0, bagRect.top() - 28.0, bagRect.width() + 82.0, bagRect.height() + 66.0));

    painter.setBrush(QColor(8, 12, 22, 92));
    painter.drawEllipse(QRectF(bagRect.left() - 28.0, bagRect.bottom() - 2.0, bagRect.width() + 60.0, 24.0));

    QPainterPath bodyPath;
    bodyPath.moveTo(bagRect.left() + bagRect.width() * 0.34, bagRect.top() + 20.0);
    bodyPath.cubicTo(bagRect.left() - 10.0, bagRect.top() + 44.0,
                     bagRect.left() - 8.0, bagRect.bottom() - 28.0,
                     bagRect.center().x() - 14.0, bagRect.bottom());
    bodyPath.lineTo(bagRect.center().x() + 14.0, bagRect.bottom());
    bodyPath.cubicTo(bagRect.right() + 10.0, bagRect.bottom() - 28.0,
                     bagRect.right() + 12.0, bagRect.top() + 44.0,
                     bagRect.left() + bagRect.width() * 0.66, bagRect.top() + 20.0);
    bodyPath.cubicTo(bagRect.center().x() + 14.0, bagRect.top() + 34.0,
                     bagRect.center().x() - 14.0, bagRect.top() + 34.0,
                     bagRect.left() + bagRect.width() * 0.34, bagRect.top() + 20.0);

    QLinearGradient bodyFill(bagRect.topLeft(), bagRect.bottomRight());
    bodyFill.setColorAt(0.0, QColor(255, 214, 96));
    bodyFill.setColorAt(0.34, QColor(247, 176, 46));
    bodyFill.setColorAt(0.72, QColor(221, 136, 24));
    bodyFill.setColorAt(1.0, QColor(168, 94, 12));
    painter.setBrush(bodyFill);
    painter.setPen(QPen(QColor(255, 236, 170), 3.0));
    painter.drawPath(bodyPath);

    QPainterPath crownPath;
    crownPath.moveTo(bagRect.left() + bagRect.width() * 0.34, bagRect.top() + 20.0);
    crownPath.cubicTo(bagRect.left() + bagRect.width() * 0.26, bagRect.top() - 16.0,
                      bagRect.left() + bagRect.width() * 0.20, bagRect.top() - 8.0,
                      bagRect.left() + bagRect.width() * 0.18, bagRect.top() + 8.0);
    crownPath.cubicTo(bagRect.left() + bagRect.width() * 0.34, bagRect.top() - 18.0,
                      bagRect.left() + bagRect.width() * 0.44, bagRect.top() - 18.0,
                      bagRect.center().x(), bagRect.top() + 2.0);
    crownPath.cubicTo(bagRect.left() + bagRect.width() * 0.56, bagRect.top() - 18.0,
                      bagRect.left() + bagRect.width() * 0.66, bagRect.top() - 18.0,
                      bagRect.left() + bagRect.width() * 0.82, bagRect.top() + 8.0);
    crownPath.cubicTo(bagRect.left() + bagRect.width() * 0.80, bagRect.top() - 8.0,
                      bagRect.left() + bagRect.width() * 0.74, bagRect.top() - 16.0,
                      bagRect.left() + bagRect.width() * 0.66, bagRect.top() + 20.0);
    crownPath.closeSubpath();
    painter.setBrush(QColor(244, 165, 34, 230));
    painter.setPen(QPen(QColor(255, 230, 158), 1.8));
    painter.drawPath(crownPath);

    const QRectF ropeRect(bagRect.left() + 10.0, bagRect.top() + 24.0, bagRect.width() - 20.0, 14.0);
    QLinearGradient ropeFill(ropeRect.topLeft(), ropeRect.bottomRight());
    ropeFill.setColorAt(0.0, QColor(186, 118, 28));
    ropeFill.setColorAt(1.0, QColor(118, 74, 18));
    painter.setBrush(ropeFill);
    painter.setPen(QPen(QColor(255, 218, 145, 120), 1.2));
    painter.drawRoundedRect(ropeRect, 7.0, 7.0);
    painter.setPen(QPen(QColor(126, 74, 18), 3.0));
    painter.drawLine(QPointF(ropeRect.left() + 18.0, ropeRect.center().y()),
                     QPointF(ropeRect.left() - 4.0, ropeRect.top() + 18.0));
    painter.drawLine(QPointF(ropeRect.right() - 18.0, ropeRect.center().y()),
                     QPointF(ropeRect.right() + 4.0, ropeRect.top() + 18.0));

    painter.setPen(QPen(QColor(176, 106, 22, 120), 3.2));
    for (int i = 0; i < 4; ++i) {
        const qreal foldX = bagRect.left() + bagRect.width() * (0.20 + i * 0.18);
        painter.drawLine(QPointF(foldX, bagRect.top() + 42.0),
                         QPointF(foldX + (i % 2 == 0 ? -5.0 : 5.0), bagRect.bottom() - 14.0));
    }

    painter.setBrush(QColor(255, 244, 198, 52));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QRectF(bagRect.left() + bagRect.width() * 0.17,
                               bagRect.top() + bagRect.height() * 0.22,
                               bagRect.width() * 0.22,
                               bagRect.height() * 0.16));

    const QRectF badgeRect(bagRect.center().x() - 34.0, bagRect.top() + bagRect.height() * 0.40, 68.0, 68.0);
    painter.setBrush(QColor(255, 228, 132, 110));
    painter.drawEllipse(badgeRect.adjusted(-8.0, -6.0, 8.0, 6.0));
    QRadialGradient badgeFill(badgeRect.center() + QPointF(-10.0, -12.0), badgeRect.width() * 0.62);
    badgeFill.setColorAt(0.0, QColor(255, 244, 206));
    badgeFill.setColorAt(0.45, QColor(255, 220, 112));
    badgeFill.setColorAt(1.0, QColor(240, 170, 44));
    painter.setBrush(badgeFill);
    painter.setPen(QPen(QColor(182, 116, 18), 2.2));
    painter.drawEllipse(badgeRect);
    painter.setPen(QPen(QColor(138, 82, 14), 2.0));
    painter.setFont(QFont(QStringLiteral("Segoe UI"), 30, QFont::Black));
    painter.drawText(badgeRect, Qt::AlignCenter, QStringLiteral("$"));

    const int stackCount = qBound(4, 4 + runCoins / 18, 6);
    for (int stack = 0; stack < stackCount; ++stack) {
        const int layers = 2 + (stack % 3);
        const qreal baseX = bounds.left() + 24.0 + stack * 22.0;
        const qreal baseY = bounds.bottom() - 34.0 + (stack % 2) * 4.0;
        for (int layer = 0; layer < layers; ++layer) {
            const QRectF coinRect(baseX, baseY - layer * 7.0, 34.0, 11.0);
            painter.setBrush(QColor(255, 214, 90));
            painter.setPen(QPen(QColor(255, 240, 176), 1.5));
            painter.drawRoundedRect(coinRect, 5.5, 5.5);
        }
    }

    const QList<QPointF> looseCoinOffsets = {
        QPointF(bagRect.right() + 18.0, bagRect.top() + 54.0),
        QPointF(bagRect.right() + 34.0, bagRect.top() + 82.0),
        QPointF(bagRect.left() - 18.0, bagRect.bottom() - 26.0)
    };
    for (int i = 0; i < looseCoinOffsets.size(); ++i) {
        painter.save();
        painter.translate(looseCoinOffsets.at(i));
        painter.rotate(24.0 - i * 18.0);
        painter.setBrush(QColor(255, 214, 92));
        painter.setPen(QPen(QColor(255, 246, 190), 1.7));
        painter.drawEllipse(QRectF(-14.0, -14.0, 28.0, 28.0));
        painter.restore();
    }

    const QPointF bagTarget = badgeRect.center();
    const int flyCoins = qBound(4, 4 + runCoins / 15, 8);
    for (int i = 0; i < flyCoins; ++i) {
        const qreal localT = qBound<qreal>(0.0, anim * 1.35 - i * 0.11, 1.0);
        if (localT <= 0.0) {
            continue;
        }

        const QPointF start(bounds.left() + 22.0 + (i % 3) * 26.0,
                            bounds.top() + 30.0 + i * 17.0);
        const QPointF control(bounds.center().x() - 18.0 + i * 7.0,
                              bounds.top() - 16.0 - (i % 2) * 12.0);
        const QPointF point = quadraticPoint(start, control, bagTarget, easeOutCubic(localT));
        const qreal scale = 1.14 - localT * 0.60;
        const qreal alpha = 255.0 * (1.0 - qMax<qreal>(0.0, localT - 0.84) / 0.16);

        painter.save();
        painter.translate(point);
        painter.rotate(-16.0 + i * 14.0 + localT * 320.0);
        painter.setBrush(QColor(255, 214, 92, qRound(alpha)));
        painter.setPen(QPen(QColor(255, 246, 190, qRound(alpha * 0.88)), 1.8));
        painter.drawEllipse(QRectF(-12.0 * scale, -12.0 * scale, 24.0 * scale, 24.0 * scale));
        painter.setPen(QPen(QColor(160, 98, 10, qRound(alpha * 0.7)), 1.0));
        painter.drawLine(QPointF(-6.0 * scale, 0.0), QPointF(6.0 * scale, 0.0));
        painter.restore();
    }

    drawTinyRewardCar(painter,
                      QRectF(bounds.right() - 92.0, bounds.bottom() - 56.0, 62.0, 24.0),
                      QColor(255, 192, 78));

    painter.setPen(QColor(255, 241, 204));
    painter.setFont(QFont(QStringLiteral("Segoe UI"), 13, QFont::Black));
    painter.drawText(QRectF(bounds.left(), bounds.bottom() - 24.0, bounds.width(), 18.0),
                     Qt::AlignCenter,
                     QStringLiteral("BANK: %1").arg(totalCoins));
    return pixmap;
}
constexpr int kSimulationStepMs = 33;
constexpr qreal kSimulationStepSeconds = kSimulationStepMs / 1000.0;

bool shouldSuppressStartupCollisionImpact(bool driveActive,
                                          bool countdownActive,
                                          qint64 sessionElapsedMs)
{
    return countdownActive
           || (driveActive
               && sessionElapsedMs >= 0
               && sessionElapsedMs < kRaceStartCollisionImpactGuardMs);
}

bool isDrivableSurface(TileType type)
{
    switch (type) {
    case TileType::Road:
    case TileType::Asphalt:
    case TileType::StartLine:
    case TileType::FinishLine:
        return true;
    default:
        return false;
    }
}

struct GateSpec {
    QVector2D center;
    qreal width = 0.0;
    qreal height = 0.0;
    bool valid = false;
};

// 在赛道直道段收集可行驶瓦片，生成横穿路面宽度的条状检查门
// gateSpansX=true：门沿 X 铺满（用于西/东竖直直道）；false：门沿 Y 铺满（用于北/南横向弧段）
GateSpec computeGateAcrossTrack(TrackData* track, int rowMin, int rowMax, int colMin, int colMax,
                                qreal tileSize, qreal gateThickness, bool gateSpansX)
{
    GateSpec gate;
    int minRow = 999;
    int maxRow = -1;
    int minCol = 999;
    int maxCol = -1;
    qreal sumX = 0.0;
    qreal sumY = 0.0;
    int count = 0;

    for (int row = rowMin; row <= rowMax; ++row) {
        for (int col = colMin; col <= colMax; ++col) {
            TrackTile* tile = track->getTileAt(row, col);
            if (!tile || !isDrivableSurface(tile->getType())) {
                continue;
            }
            minRow = qMin(minRow, row);
            maxRow = qMax(maxRow, row);
            minCol = qMin(minCol, col);
            maxCol = qMax(maxCol, col);
            sumX += col * tileSize + tileSize / 2.0;
            sumY += row * tileSize + tileSize / 2.0;
            ++count;
        }
    }

    if (count == 0) {
        return gate;
    }

    const qreal spanX = (maxCol - minCol + 1) * tileSize;
    const qreal spanY = (maxRow - minRow + 1) * tileSize;
    gate.center = QVector2D(sumX / count, sumY / count);
    gate.valid = true;

    if (gateSpansX) {
        gate.width = spanX;
        gate.height = gateThickness;
    } else {
        gate.width = gateThickness;
        gate.height = spanY;
    }
    return gate;
}

void addGateCheckpoint(TrackData* track, int id, int routeIndex,
                     const QVector2D& center, qreal width, qreal height)
{
    Checkpoint* cp = new Checkpoint(id, center, track);
    cp->setIndexInRoute(routeIndex);
    cp->setWidth(width);
    cp->setHeight(height);
    track->addCheckpoint(cp);
}

constexpr qreal kPhysicsMaxSpeed = 300.0;
constexpr qreal kDisplayMaxSpeedKmh = 120.0;
constexpr qreal kTileSize = 64.0;

QString visualAssetPath(const QString& relativePath)
{
    const QString assetRelative = QStringLiteral("visual_upgrade/phantomdrive_direct_use_assets/%1").arg(relativePath);
    const QStringList roots = {
        QCoreApplication::applicationDirPath() + QStringLiteral("/assets"),
        QCoreApplication::applicationDirPath() + QStringLiteral("/../assets"),
        QCoreApplication::applicationDirPath() + QStringLiteral("/../../assets"),
        QCoreApplication::applicationDirPath() + QStringLiteral("/../../../assets"),
        QDir::currentPath() + QStringLiteral("/assets"),
        QDir::currentPath() + QStringLiteral("/source/assets"),
        QDir::currentPath()
    };

    for (const QString& root : roots) {
        const QString candidate = QDir(root).filePath(assetRelative);
        if (QFileInfo::exists(candidate)) {
            return candidate;
        }
    }
    return QString();
}

class MenuBackgroundLayer : public QWidget
{
public:
    explicit MenuBackgroundLayer(QWidget* parent)
        : QWidget(parent)
        , m_background(visualAssetPath(QStringLiteral("menu/pd_main_menu_bg_phantomdrive_1920x1080.png")))
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAutoFillBackground(false);
    }

protected:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        if (watched == parentWidget() && event && event->type() == QEvent::Resize) {
            setGeometry(parentWidget()->rect());
            lower();
        }
        return QWidget::eventFilter(watched, event);
    }

    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter.fillRect(rect(), QColor(5, 8, 22));

        if (m_background.isNull()) {
            return;
        }

        const QSize scaledSize = m_background.size().scaled(size(), Qt::KeepAspectRatioByExpanding);
        const QRect target((width() - scaledSize.width()) / 2,
                           (height() - scaledSize.height()) / 2,
                           scaledSize.width(),
                           scaledSize.height());
        painter.drawPixmap(target, m_background);
    }

private:
    QPixmap m_background;
};

bool tileAtIsStartFinish(PhantomDrive::TrackData* track, const QVector2D& position)
{
    if (!track) {
        return false;
    }
    const QPoint tileCoord = PhantomDrive::TrackData::worldToTile(position, kTileSize);
    const int row = tileCoord.y();
    const int col = tileCoord.x();
    PhantomDrive::TrackTile* tile = track->getTileAt(row, col);
    if (!tile) {
        return false;
    }
    const auto type = tile->getType();
    return type == PhantomDrive::TileType::StartLine || type == PhantomDrive::TileType::FinishLine;
}

bool positionNearStartFinish(PhantomDrive::TrackData* track, const QVector2D& position, qreal maxDistance = 40.0)
{
    if (!track) {
        return false;
    }

    const qreal maxDistanceSq = maxDistance * maxDistance;
    for (int row = 0; row < track->getRowCount(); ++row) {
        for (int col = 0; col < track->getColCount(); ++col) {
            PhantomDrive::TrackTile* tile = track->getTileAt(row, col);
            if (!tile) {
                continue;
            }

            const auto type = tile->getType();
            if (type != PhantomDrive::TileType::StartLine && type != PhantomDrive::TileType::FinishLine) {
                continue;
            }

            const QVector2D center = PhantomDrive::TrackData::tileToWorldCenter(row, col);
            const QVector2D delta = position - center;
            if (delta.lengthSquared() <= maxDistanceSq) {
                return true;
            }
        }
    }

    return false;
}

bool positionInNorthGate(PhantomDrive::TrackData* track, const QVector2D& position)
{
    if (!track) {
        return false;
    }
    const QList<PhantomDrive::Checkpoint*> checkpoints = track->getCheckpointsInOrder();
    if (checkpoints.isEmpty() || !checkpoints.first()) {
        return false;
    }
    return checkpoints.first()->containsPoint(position);
}

bool crossedCheckpointGate(const PhantomDrive::Checkpoint* cp,
                           const QVector2D& from,
                           const QVector2D& to)
{
    if (!cp) {
        return false;
    }
    const QRectF bounds = cp->getBounds();
    if (bounds.contains(from.toPointF()) || bounds.contains(to.toPointF())) {
        return true;
    }

    constexpr int kSamples = 8;
    for (int i = 1; i < kSamples; ++i) {
        const qreal t = static_cast<qreal>(i) / static_cast<qreal>(kSamples);
        const QVector2D sample = from * (1.0 - t) + to * t;
        if (bounds.contains(sample.toPointF())) {
            return true;
        }
    }
    return false;
}

struct RaceHudEntry {
    QString id;
    int lap = 0;
    int checkpoint = 0;
    qreal progressPercent = 0.0;
    qreal distanceToTarget = 0.0;
    qreal absoluteProgress = 0.0;
    int insertionOrder = 0;
};

qreal progressOnSegment(const QVector2D& position, const QVector2D& start, const QVector2D& end)
{
    const QVector2D segment = end - start;
    const qreal lengthSq = segment.lengthSquared();
    if (lengthSq <= 0.001) {
        return 0.0;
    }

    return qBound<qreal>(0.0,
                         QVector2D::dotProduct(position - start, segment) / lengthSq,
                         1.0);
}

bool routeHasDuplicateEndpoint(const QList<Waypoint>& waypoints)
{
    return waypoints.size() > 1
        && (waypoints.first().position - waypoints.last().position).lengthSquared() <= 1.0;
}

int routeSegmentCountFor(const QList<Waypoint>& waypoints)
{
    if (waypoints.size() < 2) {
        return 1;
    }
    return routeHasDuplicateEndpoint(waypoints) ? waypoints.size() - 1 : waypoints.size();
}

qreal progressAlongWaypoints(const QVector2D& position, const QList<Waypoint>& waypoints)
{
    if (waypoints.size() < 2) {
        return 0.0;
    }

    const int segmentCount = routeSegmentCountFor(waypoints);
    qreal bestDistanceSq = std::numeric_limits<qreal>::max();
    qreal bestProgress = 0.0;
    for (int i = 0; i < segmentCount; ++i) {
        const QVector2D start = waypoints.at(i).position;
        const QVector2D end = waypoints.at((i + 1) % waypoints.size()).position;
        const QVector2D segment = end - start;
        const qreal lengthSq = segment.lengthSquared();
        if (lengthSq <= 0.001) {
            continue;
        }

        const qreal t = qBound<qreal>(0.0,
                                      QVector2D::dotProduct(position - start, segment) / lengthSq,
                                      1.0);
        const QVector2D projected = start + segment * t;
        const qreal distanceSq = (position - projected).lengthSquared();
        if (distanceSq < bestDistanceSq) {
            bestDistanceSq = distanceSq;
            bestProgress = static_cast<qreal>(i) + t;
        }
    }

    return bestProgress;
}

qreal progressAlongWaypointsNear(const QVector2D& position,
                                 const QList<Waypoint>& waypoints,
                                 qreal expectedAbsoluteProgress)
{
    if (waypoints.size() < 2) {
        return 0.0;
    }

    const int segmentCount = routeSegmentCountFor(waypoints);
    qreal bestScore = std::numeric_limits<qreal>::max();
    qreal bestProgress = qMax<qreal>(0.0, expectedAbsoluteProgress);

    for (int i = 0; i < segmentCount; ++i) {
        const QVector2D start = waypoints.at(i).position;
        const QVector2D end = waypoints.at((i + 1) % waypoints.size()).position;
        const QVector2D segment = end - start;
        const qreal lengthSq = segment.lengthSquared();
        if (lengthSq <= 0.001) {
            continue;
        }

        const qreal t = qBound<qreal>(0.0,
                                      QVector2D::dotProduct(position - start, segment) / lengthSq,
                                      1.0);
        qreal candidate = static_cast<qreal>(i) + t;
        while (candidate - expectedAbsoluteProgress > segmentCount * 0.5) {
            candidate -= segmentCount;
        }
        while (expectedAbsoluteProgress - candidate > segmentCount * 0.5) {
            candidate += segmentCount;
        }

        const QVector2D projected = start + segment * t;
        const qreal distanceSq = (position - projected).lengthSquared();
        const qreal progressDelta = candidate - expectedAbsoluteProgress;
        const qreal score = distanceSq + (progressDelta * progressDelta * 4096.0);
        if (score < bestScore) {
            bestScore = score;
            bestProgress = candidate;
        }
    }

    return bestProgress;
}

QString routeSignatureFor(const QList<Waypoint>& waypoints)
{
    QStringList parts;
    parts.reserve(waypoints.size());
    for (const Waypoint& waypoint : waypoints) {
        parts.append(QStringLiteral("%1,%2")
                         .arg(qRound(waypoint.position.x()))
                         .arg(qRound(waypoint.position.y())));
    }
    return parts.join(QLatin1Char('|'));
}

QList<Waypoint> firstOpponentWaypoints(PhantomDrive::AIOpponentManager* aiManager)
{
    if (!aiManager) {
        return {};
    }

    const QList<PhantomDrive::AIOpponent*> opponents = aiManager->getAllOpponents();
    for (const PhantomDrive::AIOpponent* ai : opponents) {
        if (ai && !ai->getWaypoints().isEmpty()) {
            return ai->getWaypoints();
        }
    }
    return {};
}

QList<Waypoint> routeWaypointsFromTrack(PhantomDrive::TrackData* track)
{
    QList<Waypoint> route;
    if (!track) {
        return route;
    }

    auto appendUnique = [&route](const QVector2D& point) {
        if (route.isEmpty() || (route.last().position - point).lengthSquared() > 1.0f) {
            route.append(Waypoint(point, 110.0, false, 0, route.size()));
        }
    };

    appendUnique(track->getStartPosition());
    const QList<PhantomDrive::Checkpoint*> checkpoints = track->getCheckpointsInOrder();
    for (PhantomDrive::Checkpoint* cp : checkpoints) {
        if (cp) {
            appendUnique(cp->getPosition());
        }
    }
    appendUnique(track->getStartPosition());
    return route;
}

QList<Waypoint> hudRankingWaypoints(PhantomDrive::AIOpponentManager* aiManager,
                                    PhantomDrive::TrackData* track)
{
    QList<Waypoint> waypoints = firstOpponentWaypoints(aiManager);
    if (waypoints.size() >= 2) {
        return waypoints;
    }
    return routeWaypointsFromTrack(track);
}

qreal distanceToCheckpoint(PhantomDrive::TrackData* track,
                           int checkpointIndex,
                           const QVector2D& position)
{
    if (!track) {
        return 0.0;
    }

    const QList<PhantomDrive::Checkpoint*> checkpoints = track->getCheckpointsInOrder();
    if (checkpointIndex >= 0 && checkpointIndex < checkpoints.size() && checkpoints.at(checkpointIndex)) {
        return (checkpoints.at(checkpointIndex)->getPosition() - position).length();
    }

    return (track->getStartPosition() - position).length();
}

qreal distanceToAiTarget(const PhantomDrive::AIOpponent* ai)
{
    if (!ai || ai->getWaypoints().isEmpty()) {
        return 0.0;
    }

    const Waypoint waypoint = ai->getCurrentWaypoint();
    return (waypoint.position - ai->getPosition()).length();
}

qreal playerRaceAbsoluteProgress(int lap,
                                 int nextCheckpointIndex,
                                 const QVector2D& position,
                                 const QList<Waypoint>& route,
                                 int totalCheckpoints)
{
    const int raceSegmentCount = qMax(1, totalCheckpoints + 1);
    const int routeSegmentCount = routeSegmentCountFor(route);
    const int boundedCheckpoint = qBound(0, nextCheckpointIndex, raceSegmentCount - 1);
    const qreal expectedWithinLap = (static_cast<qreal>(boundedCheckpoint) / raceSegmentCount)
        * routeSegmentCount;
    const qreal expectedAbsolute = (qMax(0, lap) * routeSegmentCount) + expectedWithinLap;

    if (route.size() >= 2) {
        return progressAlongWaypointsNear(position, route, expectedAbsolute);
    }

    return expectedAbsolute;
}

qreal aiRaceAbsoluteProgress(const PhantomDrive::AIOpponent* ai)
{
    if (!ai) {
        return 0.0;
    }

    const QList<Waypoint> waypoints = ai->getWaypoints();
    const int waypointCount = waypoints.size();
    const int routeSegmentCount = routeSegmentCountFor(waypoints);
    if (waypointCount < 2) {
        return qMax(0, ai->getCurrentLap()) * routeSegmentCount;
    }

    const int targetIndex = qBound(0, ai->getCurrentWaypointIndex(), waypointCount - 1);
    const int previousIndex = (targetIndex - 1 + waypointCount) % waypointCount;
    qreal progressWithinLap = 0.0;
    if (targetIndex == 0) {
        if (ai->getCurrentLap() == 0 && ai->getCheckpointsPassed() == 0) {
            progressWithinLap = progressOnSegment(ai->getPosition(),
                                                  waypoints.at(targetIndex).position,
                                                  waypoints.at((targetIndex + 1) % waypointCount).position);
        } else {
            progressWithinLap = (routeSegmentCount - 1)
                + progressOnSegment(ai->getPosition(),
                                    waypoints.at(previousIndex).position,
                                    waypoints.at(targetIndex).position);
        }
    } else {
        progressWithinLap = (targetIndex - 1)
            + progressOnSegment(ai->getPosition(),
                                waypoints.at(previousIndex).position,
                                waypoints.at(targetIndex).position);
    }

    return (qMax(0, ai->getCurrentLap()) * routeSegmentCount) + progressWithinLap;
}

QList<RaceHudEntry> buildRaceHudEntries(bool includeAi,
                                        bool useFormalRaceProgress,
                                        const QVector2D& p1Position,
                                        int p1Lap,
                                        int p1Checkpoint,
                                        bool includeP2,
                                        const QVector2D& p2Position,
                                        int p2Lap,
                                        int p2Checkpoint,
                                        PhantomDrive::AIOpponentManager* aiManager,
                                        PhantomDrive::TrackData* track,
                                        int totalCheckpoints,
                                        QHash<QString, qreal>* continuousProgressById,
                                        QString* routeSignature)
{
    QList<RaceHudEntry> entries;
    int order = 0;
    const int checkpointTotal = qMax(0, totalCheckpoints);
    const int segmentCount = qMax(1, checkpointTotal + 1);
    const QList<Waypoint> rankingWaypoints = hudRankingWaypoints(aiManager, track);
    const QString currentRouteSignature = routeSignatureFor(rankingWaypoints);
    if (routeSignature && *routeSignature != currentRouteSignature) {
        if (continuousProgressById) {
            continuousProgressById->clear();
        }
        *routeSignature = currentRouteSignature;
    }

    auto applyFreeRouteProgress = [&](RaceHudEntry& entry, const QVector2D& position) {
        if (rankingWaypoints.size() < 2) {
            return;
        }

        const int waypointSegmentCount = routeSegmentCountFor(rankingWaypoints);
        qreal continuousProgress = progressAlongWaypoints(position, rankingWaypoints);
        if (continuousProgressById) {
            const auto previousIt = continuousProgressById->constFind(entry.id);
            if (previousIt != continuousProgressById->constEnd()) {
                const qreal previous = previousIt.value();
                qreal candidate = progressAlongWaypointsNear(position, rankingWaypoints, previous);
                while (candidate - previous > waypointSegmentCount * 0.5) {
                    candidate -= waypointSegmentCount;
                }
                while (previous - candidate > waypointSegmentCount * 0.5) {
                    candidate += waypointSegmentCount;
                }
                continuousProgress = candidate;
            }
            continuousProgressById->insert(entry.id, continuousProgress);
        }

        if (checkpointTotal > 0
            && entry.checkpoint >= checkpointTotal
            && continuousProgress < ((entry.lap + 1) * waypointSegmentCount) - (waypointSegmentCount * 0.75)) {
            continuousProgress = qMax(continuousProgress, ((entry.lap + 1) * waypointSegmentCount) - 0.001);
            if (continuousProgressById) {
                continuousProgressById->insert(entry.id, continuousProgress);
            }
        }

        const qreal progressWithinLap = continuousProgress - (qFloor(continuousProgress / waypointSegmentCount) * waypointSegmentCount);
        entry.lap = qFloor(continuousProgress / waypointSegmentCount);
        entry.checkpoint = qFloor(progressWithinLap);
        entry.progressPercent = qBound(0.0,
                                       (progressWithinLap / waypointSegmentCount) * 100.0,
                                       100.0);
        entry.distanceToTarget = 1.0 - (progressWithinLap - qFloor(progressWithinLap));
        entry.absoluteProgress = continuousProgress;
    };

    auto applyFormalProgressFields = [&](RaceHudEntry& entry, qreal absoluteProgress) {
        entry.absoluteProgress = absoluteProgress;
        const qreal progressWithinLap = absoluteProgress - (qFloor(absoluteProgress / segmentCount) * segmentCount);
        entry.lap = qFloor(absoluteProgress / segmentCount);
        entry.checkpoint = qFloor(progressWithinLap);
        entry.progressPercent = qBound(0.0,
                                       (progressWithinLap / segmentCount) * 100.0,
                                       100.0);
        entry.distanceToTarget = 1.0 - (progressWithinLap - qFloor(progressWithinLap));
    };

    auto appendPlayer = [&](const QString& id, const QVector2D& position, int lap, int checkpoint) {
        const int boundedCheckpoint = qBound(0, checkpoint, checkpointTotal);
        RaceHudEntry entry;
        entry.id = id;
        entry.lap = qMax(0, lap);
        entry.checkpoint = boundedCheckpoint;
        entry.progressPercent = checkpointTotal > 0
            ? (static_cast<qreal>(boundedCheckpoint) / checkpointTotal) * 100.0
            : 0.0;
        entry.distanceToTarget = distanceToCheckpoint(track, boundedCheckpoint, position);
        entry.insertionOrder = order++;
        if (useFormalRaceProgress) {
            applyFormalProgressFields(entry,
                                      playerRaceAbsoluteProgress(lap,
                                                                 boundedCheckpoint,
                                                                 position,
                                                                 rankingWaypoints,
                                                                 checkpointTotal));
        } else {
            applyFreeRouteProgress(entry, position);
        }
        entries.append(entry);
    };

    appendPlayer(QStringLiteral("P1"), p1Position, p1Lap, p1Checkpoint);
    if (includeP2) {
        appendPlayer(QStringLiteral("P2"), p2Position, p2Lap, p2Checkpoint);
    }

    if (includeAi && aiManager) {
        const QList<PhantomDrive::AIOpponent*> opponents = aiManager->getAllOpponents();
        for (PhantomDrive::AIOpponent* ai : opponents) {
            if (!ai || (!ai->isActive() && !ai->hasFinished()) || ai->getWaypoints().isEmpty()) {
                continue;
            }

            RaceHudEntry entry;
            entry.id = ai->getId();
            entry.lap = qMax(0, ai->getCurrentLap());
            entry.checkpoint = qMax(0, ai->getCheckpointsPassed());
            entry.progressPercent = qBound(0.0, ai->getProgressPercentage(), 100.0);
            entry.distanceToTarget = distanceToAiTarget(ai);
            entry.insertionOrder = order++;
            if (useFormalRaceProgress) {
                applyFormalProgressFields(entry, aiRaceAbsoluteProgress(ai));
            } else {
                entry.absoluteProgress = aiRaceAbsoluteProgress(ai);
                const int waypointSegmentCount = routeSegmentCountFor(rankingWaypoints);
                const qreal progressWithinLap = entry.absoluteProgress
                    - (qFloor(entry.absoluteProgress / waypointSegmentCount) * waypointSegmentCount);
                entry.lap = qFloor(entry.absoluteProgress / waypointSegmentCount);
                entry.checkpoint = qFloor(progressWithinLap);
                entry.progressPercent = qBound(0.0,
                                               (progressWithinLap / waypointSegmentCount) * 100.0,
                                               100.0);
                entry.distanceToTarget = 1.0 - (progressWithinLap - qFloor(progressWithinLap));
            }
            entries.append(entry);
        }
    }

    std::stable_sort(entries.begin(), entries.end(), [](const RaceHudEntry& left,
                                                        const RaceHudEntry& right) {
        if (!qFuzzyCompare(left.absoluteProgress + 1.0, right.absoluteProgress + 1.0)) {
            return left.absoluteProgress > right.absoluteProgress;
        }
        return left.insertionOrder < right.insertionOrder;
    });

    return entries;
}

int raceHudPositionFor(const QList<RaceHudEntry>& entries, const QString& id)
{
    for (int i = 0; i < entries.size(); ++i) {
        if (entries.at(i).id == id) {
            return i + 1;
        }
    }
    return 1;
}

bool hasHudRankedAiOpponents(PhantomDrive::AIOpponentManager* aiManager)
{
    if (!aiManager) {
        return false;
    }

    const QList<PhantomDrive::AIOpponent*> opponents = aiManager->getAllOpponents();
    for (const PhantomDrive::AIOpponent* ai : opponents) {
        if (ai && (ai->isActive() || ai->hasFinished()) && !ai->getWaypoints().isEmpty()) {
            return true;
        }
    }
    return false;
}

void dumpCustomTrackLayoutForDebug(PhantomDrive::TrackData* track, const QString& label)
{
    Q_UNUSED(track);
    Q_UNUSED(label);
}

PhantomDrive::TrackData* cloneCustomTrackSnapshot(PhantomDrive::TrackData* source, QObject* parent)
{
    if (!source) {
        return nullptr;
    }

    auto* snapshot = new PhantomDrive::TrackData(parent);
    const int rows = source->getRowCount();
    const int cols = source->getColCount();

    snapshot->setName(source->getName());
    snapshot->setId(source->getId());
    snapshot->setAuthor(source->getAuthor());
    snapshot->setDescription(source->getDescription());
    snapshot->setDifficulty(source->getDifficulty());
    snapshot->setEstimatedLapTime(source->getEstimatedLapTime());
    snapshot->setTrackLength(source->getTrackLength());
    snapshot->setMaxLaps(1);
    snapshot->setSize(rows, cols);
    snapshot->setStartRotation(source->getStartRotation());

    // Editor rows grow downward. The existing driving renderer/physics path is y-up,
    // so flip rows once in the runtime snapshot instead of adding a custom input mode.
    for (int row = 0; row < rows; ++row) {
        const int engineRow = rows - 1 - row;
        for (int col = 0; col < cols; ++col) {
            const PhantomDrive::TrackTile* srcTile = source->getTileAt(row, col);
            PhantomDrive::TrackTile* dstTile = snapshot->getTileAt(engineRow, col);
            if (srcTile && dstTile) {
                dstTile->setType(srcTile->getType());
                dstTile->setDirection(srcTile->getDirection());
                dstTile->setFriction(srcTile->getFriction());
                dstTile->setDrivable(srcTile->isDrivable());
                dstTile->setCollisionTile(srcTile->isCollisionTile());
                dstTile->setAssetId(srcTile->getAssetId());
            }
        }
    }

    const qreal trackHeight = rows * PhantomDrive::TrackData::DefaultTileSize;
    auto flipWorldY = [trackHeight](const QVector2D& pos) {
        return QVector2D(pos.x(), trackHeight - pos.y());
    };

    snapshot->clearStartPositions();
    const QList<QVector2D> starts = source->getStartPositions();
    for (const QVector2D& start : starts) {
        snapshot->addStartPosition(flipWorldY(start));
    }
    snapshot->setStartPosition(starts.isEmpty() ? flipWorldY(source->getStartPosition()) : flipWorldY(starts.first()));

    const QList<PhantomDrive::Checkpoint*> checkpoints = source->getCheckpointsInOrder();
    for (int i = 0; i < checkpoints.size(); ++i) {
        const PhantomDrive::Checkpoint* srcCp = checkpoints.at(i);
        if (!srcCp) {
            continue;
        }
        auto* cp = new PhantomDrive::Checkpoint(srcCp->getId(), flipWorldY(srcCp->getPosition()), snapshot);
        cp->setIndexInRoute(srcCp->getIndexInRoute());
        cp->setWidth(srcCp->getWidth());
        cp->setHeight(srcCp->getHeight());
        cp->setActive(srcCp->isActive());
        cp->setMandatory(srcCp->isMandatory());
        cp->setRequiredLap(srcCp->getRequiredLap());
        snapshot->addCheckpoint(cp);
    }

    snapshot->clearItemBoxPositions();
    for (const QVector2D& item : source->getItemBoxPositions()) {
        snapshot->addItemBoxPosition(flipWorldY(item));
    }

    return snapshot;
}

QString lightColorToString(PhantomDrive::TrafficLightObject::LightColor color)
{
    switch (color) {
    case PhantomDrive::TrafficLightObject::LightColor::Red:
        return QStringLiteral("red");
    case PhantomDrive::TrafficLightObject::LightColor::Yellow:
        return QStringLiteral("yellow");
    case PhantomDrive::TrafficLightObject::LightColor::Green:
        return QStringLiteral("green");
    default:
        return QStringLiteral("green");
    }
}

bool isRealAiCoachReportMarkdown(const QString& markdown)
{
    const QString text = markdown.trimmed();
    if (text.isEmpty()) {
        return false;
    }
    if (text.contains(QStringLiteral("Local Fallback"), Qt::CaseInsensitive)
        || text.contains(QStringLiteral("AI API fallback reason"), Qt::CaseInsensitive)
        || text.contains(QStringLiteral("mode=mock"), Qt::CaseInsensitive)) {
        return false;
    }
    return true;
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_learningHUD(nullptr)
    , m_arcadeHUD(nullptr)
    , m_gameView(nullptr)
    , m_vehiclePhysics(nullptr)
    , m_player2Physics(nullptr)
    , m_drivingDataCollector(new DrivingDataCollector(this))
    , m_player2DataCollector(new DrivingDataCollector(this))
    , m_scoreManager(new ScoreManager(this))
    , m_player2ScoreManager(new ScoreManager(this))
    , m_aiManager(new AIOpponentManager(this))
    , m_trafficObjectManager(new TrafficObjectManager(this))
    , m_powerupWorld(new PowerupWorldRuntime(this))
    , m_reportWidget(nullptr)
    , m_reportPage(nullptr)
    , m_btnFinishDrive(nullptr)
    , m_btnPause(nullptr)
    , m_aiDifficultyCombo(nullptr)
    , m_btnLoadCustomTrack(nullptr)
    , m_btnCustomTrackMode(nullptr)
    , m_btnTwoPlayerRace(nullptr)
    , m_btnAdaptiveDemo(nullptr)
    , m_btnCoinChallenge(nullptr)
    , m_btnGarage(nullptr)
    , m_btnGuide(nullptr)
    , m_btnPlayCustomTrack(nullptr)
    , m_btnSaveCustomTrack(nullptr)
    , m_btnLoadCustomTrackForEdit(nullptr)
    , m_btnExportCustomTrackJson(nullptr)
    , m_customTrackEditor(nullptr)
    , m_customTrackMode(new CustomTrackMode(this))
    , m_collectibleManager(new CollectibleManager(this))
    , m_challengeObstacleManager(std::make_unique<ChallengeObstacleManager>())
    , m_blockerVehicleManager(std::make_unique<BlockerVehicleManager>())
    , m_garageWidget(nullptr)
    , m_mainMenuWidget(nullptr)
    , m_playerProfileStore(new PlayerProfileStore(this))
    , m_currencyManager(new CurrencyManager(this))
    , m_skinManager(new SkinManager(this))
    , m_defaultRaceTrack(nullptr)
    , m_selectedBuiltInTrack(nullptr)
    , m_runtimeCustomTrack(nullptr)
    , m_simTimer(nullptr)
    , m_learningSessionTimer(nullptr)
    , m_countdownFinishTimer(new QTimer(this))
    , m_currentMode("Arcade")
    , m_customTrackPath()
    , m_trackSelectCombo(nullptr)
    , m_playerCountCombo(nullptr)
    , m_trackDifficultyLabel(nullptr)
    , m_trackDescriptionLabel(nullptr)
    , m_coinChallengeHudLabel(nullptr)
    , m_coinChallengeSummaryOverlay(nullptr)
    , m_coinChallengeSummaryTitleLabel(nullptr)
    , m_coinChallengeSummaryFlavorLabel(nullptr)
    , m_coinChallengeSummaryRewardLabel(nullptr)
    , m_coinChallengeSummaryCoinBagLabel(nullptr)
    , m_coinChallengeSummaryDeltaLabel(nullptr)
    , m_coinChallengeSummaryStatsLabel(nullptr)
    , m_coinChallengePlayAgainButton(nullptr)
    , m_coinChallengeExitButton(nullptr)
    , m_coinChallengeSummaryAnimTimer(nullptr)
    , m_selectedTrackId(QStringLiteral("neon_loop"))
    , m_arcadeTrackId(QStringLiteral("neon_loop"))
    , m_twoPlayerTrackId(QStringLiteral("neon_loop"))
    , m_selectedAIDifficulty(QStringLiteral("medium"))
    , m_twoPlayerMode(false)
    , m_twoPlayerSetupActive(false)
    , m_currentSpeedLimit(60)
    , m_currentTrafficLightState("green")
    , m_driveActive(false)
    , m_countdownActive(false)
    , m_gamePaused(false)
    , m_sessionGeneration(0)
    , m_countdownRemainingMs(0)
    , m_countdownTimerStartedMs(0)
    , m_countdownSessionGeneration(0)
    , m_learningTimerRemainingMs(0)
    , m_learningTimerStartedMs(0)
    , m_arcadeRaceFinished(false)
    , m_customTrackPlaying(false)
    , m_coinChallengeActive(false)
    , m_coinChallengeSummaryVisible(false)
    , m_coinChallengeTenSecondWarningShown(false)
    , m_coinChallengeCountdownAnnouncedStage(-1)
    , m_lapsCompleted(0)
    , m_totalLaps(3)
    , m_coinChallengeDurationMs(kCoinChallengeDefaultDurationMs)
    , m_coinChallengeRemainingMs(kCoinChallengeDefaultDurationMs)
    , m_coinChallengeGoalCoins(kCoinChallengeGoalCoins)
    , m_coinChallengeMaxSpeedKmh(0)
    , m_coinChallengeLastAverageSpeedKmh(0)
    , m_coinChallengeLastEfficiencyCpm(0)
    , m_coinChallengeLastRunCoins(0)
    , m_coinChallengeWarningStartedMs(0)
    , m_coinChallengeSpeedSampleSumKmh(0)
    , m_coinChallengeSpeedSampleCount(0)
    , m_coinChallengeLastElapsedMs(0)
    , m_coinChallengeMagnetRemainingMs(0)
    , m_coinChallengeBalloonRushRemainingMs(0)
    , m_coinChallengeBalloonRushTriggerRemainingMs(0)
    , m_coinChallengeBalloonRushReturnRemainingMs(0)
    , m_coinChallengeBalloonRushIntroCountdownRemainingMs(0)
    , m_coinChallengeBalloonRushIntroCountdownAnnounced(-1)
    , m_coinChallengeBalloonRushCollectedCoins(0)
    , m_coinChallengeBalloonRushRecentGainValue(0)
    , m_coinChallengeBalloonRushGainFlashRemainingMs(0)
    , m_coinChallengeBalloonRushMilestoneRemainingMs(0)
    , m_coinChallengeBalloonRushMilestoneDurationMs(0)
    , m_coinChallengePowerupsUsed(0)
    , m_coinChallengeLoopsCompleted(0)
    , m_coinChallengeLoopCheckpointIndex(0)
    , m_coinChallengeMagnetSpawnStage(0)
    , m_coinChallengeFinalCountdownAnnouncedSecond(-1)
    , m_coinChallengeSummaryAnimatedRunCoins(0)
    , m_coinChallengeSummaryAnimatedTotalCoins(0)
    , m_coinChallengeSummaryTargetRunCoins(0)
    , m_coinChallengeSummaryTargetTotalCoins(0)
    , m_coinChallengeSummaryDepositSoundStage(0)
    , m_coinChallengeSummaryVisualProgress(0.0)
    , m_coinChallengeLastHazardSoundMs(-1000)
    , m_coinChallengeLeftStartZone(false)
    , m_coinChallengeWasOnStartZone(false)
    , m_coinChallengeBalloonRushSpawned(false)
    , m_coinChallengeMagnetLoopPlaying(false)
    , m_coinChallengeMagnetSpawnPool()
    , m_coinChallengeBalloonRushSpawnPool()
    , m_coinChallengeBalloonRushPhase(CoinChallengeBalloonRushPhase::Inactive)
    , m_coinChallengeBalloonRushSavedPosition(0.0f, 0.0f)
    , m_coinChallengeBalloonRushSavedRotation(0.0)
    , m_coinChallengeBalloonRushSavedSpeed(0.0)
    , m_coinChallengeBalloonRushLaneIndex(1)
    , m_coinChallengeBalloonRushLaneVisual(1.0)
    , m_coinChallengeBalloonRushRoadScroll(0.0)
    , m_coinChallengeBalloonRushSpawnAccumulatorMs(0.0)
    , m_coinChallengeBalloonRushPatternLane(1)
    , m_coinChallengeBalloonRushPatternBatchCount(0)
    , m_coinChallengeBalloonRushRecentSegmentLanes()
    , m_coinChallengeBalloonRushVisitedLanes()
    , m_coinChallengeBalloonRushCoinLayout()
    , m_coinChallengeBalloonRushMilestoneHeadline()
    , m_coinChallengeBalloonRushMilestoneDetail()
    , m_coinChallengeBalloonRushMilestoneAccent()
    , m_coinChallengeTriggeredMilestones()
    , m_simTick(0)
    , m_sessionElapsedMs(0)
    , m_currentLapStartMs(0)
    , m_bestLapMs(0)
    , m_playerPosition(320.0, 320.0)
    , m_previousPlayerPosition(320.0, 320.0)
    , m_playerRotation(-90.0)
    , m_playerSpeed(0.0)
    , m_player2Position(360.0, 320.0)
    , m_previousPlayer2Position(360.0, 320.0)
    , m_player2Rotation(-90.0)
    , m_player2Speed(0.0)
    , m_player2LapsCompleted(0)
    , m_player2NextCheckpointIndex(0)
    , m_player2WasInsideNextGate(false)
    , m_player1Finished(false)
    , m_player2Finished(false)
    , m_twoPlayerFinishHandled(false)
    , m_player1PendingReport()
    , m_player2PendingReport()
    , m_player1PendingSamples()
    , m_player2PendingSamples()
    , m_arcadeRaceLogicActive(false)
    , m_nextCheckpointIndex(0)
    , m_raceCheckpointTotal(0)
    , m_hasLeftNorthSector(false)
    , m_wasOnStartLine(false)
    , m_wasInNorthGate(false)
    , m_blockCheckpointsUntilLeaveNorth(false)
    , m_wasInsideNextGate(false)
{
    ui->setupUi(this);
    resize(1280, 900);

    // Apply consistent styles to the top HUD buttons
    if (ui->btn_Back) {
        ui->btn_Back->setFixedHeight(52);
        ui->btn_Back->setStyleSheet(R"(
            QPushButton {
                background: rgba(15, 40, 90, 220);
                color: #00CCFF;
                border: 2px solid #0088CC;
                border-radius: 8px;
                padding: 4px 16px;
                font-size: 12px;
                font-weight: bold;
                letter-spacing: 1px;
            }
            QPushButton:hover {
                background: rgba(20, 55, 120, 240);
                border-color: #00AAEE;
            }
            QPushButton:pressed {
                background: rgba(10, 30, 80, 240);
            }
        )");
    }
    if (ui->btn_FinishDrive_Top) {
        ui->btn_FinishDrive_Top->setFixedHeight(52);
        ui->btn_FinishDrive_Top->setStyleSheet(R"(
            QPushButton {
                background: rgba(180, 20, 40, 200);
                color: #FF6688;
                border: 2px solid #FF2255;
                border-radius: 8px;
                padding: 4px 14px;
                font-size: 12px;
                font-weight: bold;
                letter-spacing: 1px;
            }
            QPushButton:hover {
                background: rgba(220, 30, 50, 230);
                border-color: #FF4466;
            }
            QPushButton:pressed {
                background: rgba(140, 15, 30, 230);
            }
        )");
    }
    if (ui->btn_ExitGame_Top) {
        ui->btn_ExitGame_Top->setFixedHeight(52);
        ui->btn_ExitGame_Top->setStyleSheet(R"(
            QPushButton {
                background: rgba(140, 10, 25, 200);
                color: #FF9999;
                border: 2px solid #CC1133;
                border-radius: 8px;
                padding: 4px 14px;
                font-size: 12px;
                font-weight: bold;
                letter-spacing: 1px;
            }
            QPushButton:hover {
                background: rgba(180, 20, 35, 230);
                border-color: #FF2255;
            }
            QPushButton:pressed {
                background: rgba(100, 5, 15, 230);
            }
        )");
    }

    if (!m_btnPause && ui->hudLayout && ui->btn_Back) {
        m_btnPause = new QPushButton(QStringLiteral("Pause"), this);
        m_btnPause->setObjectName(QStringLiteral("btn_PauseGame"));
        m_btnPause->setFixedHeight(52);
        m_btnPause->setMinimumWidth(110);
        m_btnPause->setStyleSheet(R"(
            QPushButton {
                background: rgba(10, 38, 72, 220);
                color: #66F7FF;
                border: 2px solid #00D6FF;
                border-radius: 8px;
                padding: 4px 16px;
                font-size: 12px;
                font-weight: bold;
                letter-spacing: 1px;
            }
            QPushButton:hover {
                background: rgba(16, 58, 105, 240);
                border-color: #66F7FF;
            }
            QPushButton:pressed {
                background: rgba(8, 26, 58, 240);
            }
        )");
        const int backIndex = ui->hudLayout->indexOf(ui->btn_Back);
        ui->hudLayout->insertWidget(backIndex >= 0 ? backIndex : ui->hudLayout->count(), m_btnPause);
        m_btnPause->hide();
    }

    if (m_countdownFinishTimer) {
        m_countdownFinishTimer->setSingleShot(true);
        connect(m_countdownFinishTimer, &QTimer::timeout, this, [this]() {
            if (m_countdownSessionGeneration == m_sessionGeneration) {
                onRaceStart();
            }
        });
    }

    PhantomDrive::InteractiveFeedback::instance(this);
    PhantomDrive::SoundManager::instance(this);
    // Sound generation is handled once in main() — do not call again here.

    QWidget* gamePage = ui->stackedWidget && ui->stackedWidget->count() > 1
        ? ui->stackedWidget->widget(1)
        : nullptr;

    m_arcadeHUD = new PhantomDrive::ArcadeHUD(gamePage ? gamePage : this);
    m_arcadeHUD->setWindowFlags(Qt::Widget);
    m_arcadeHUD->hide();

    m_learningHUD = new LearningHUD(gamePage ? gamePage : this);
    m_learningHUD->hide();

    setupGameView();

    // Tell InteractiveFeedback exactly where the game-map widget is so all
    // notifications are centred inside it and never overlap the HUD panel.
    if (m_gameView) {
        PhantomDrive::InteractiveFeedback::instance(this).setGameView(m_gameView);
    }

    // Connect ArcadeHUD banner signal → InteractiveFeedback (map-area display).
    if (m_arcadeHUD) {
        connect(m_arcadeHUD, &PhantomDrive::ArcadeHUD::centerNotificationRequested,
                this, [this](const QString& text, int durationMs) {
                    showInteractiveFeedback(text, PhantomDrive::FeedbackType::Milestone);
                    Q_UNUSED(durationMs);
                });
    }

    // ---- LEGACY: LearningHUD is no longer used for any HUD feedback ----
    // All powerup and violation feedback now goes through ArcadeHUD and InteractiveFeedback.
    // (LearningHUD object is still created for backward compatibility but not connected.)

    setupVehiclePhysics();
    setupDemoControls();
    setupCustomTrackControls();
    setupRaceSetupControls();
    styleMainMenu();
    setupDataBindings();
    if (m_scoreManager && m_aiManager) {
        connect(m_scoreManager,
                &ScoreManager::qLearningFeedbackReady,
                m_aiManager,
                &AIOpponentManager::onQLearningFeedbackReady);
    }

    if (m_scoreManager) {
        connect(m_scoreManager,
                &ScoreManager::reportJsonReady,
                this,
                [](const QJsonObject& reportJson) {
                    SaveLoadManager::instance().saveReportJson(reportJson);
                });
    }

    if (m_player2ScoreManager) {
        connect(m_player2ScoreManager,
                &ScoreManager::reportJsonReady,
                this,
                [](const QJsonObject& reportJson) {
                    SaveLoadManager::instance().saveReportJson(reportJson);
                });
    }

    if (m_scoreManager) {
        connect(m_scoreManager,
                &ScoreManager::qLearningFeedbackReady,
                this,
                [this](const PhantomDrive::QLearningFeedback& feedback) {
                    if (!m_driveActive || m_countdownActive) {
                        return;
                    }
                    const QString scoreText = QString::number(feedback.normalizedScore * 100.0, 'f', 0);
                    const QString riskText = QString::number(feedback.safetyRisk, 'f', 2);
                    const QString complianceText = QString::number(feedback.ruleCompliance, 'f', 2);

                    const QString message = QStringLiteral("Adaptive adjusted: score %1, risk %2, rules %3")
                                                .arg(scoreText, riskText, complianceText);
                    statusBar()->showMessage(message, 5000);
                    showInteractiveFeedback(message, FeedbackType::Milestone);
                });

        connect(m_scoreManager, &ScoreManager::feedbackReady,
                this, [this](const QString& text, int, const QString& severity) {
                    if (!m_driveActive || m_countdownActive) {
                        return;
                    }
                    const FeedbackType type = severity == QStringLiteral("positive")
                        ? FeedbackType::Positive
                        : severity == QStringLiteral("danger")
                            ? FeedbackType::Critical
                            : FeedbackType::Warning;
                    showInteractiveFeedback(text, type);
                });
    }

    if (m_aiManager) {
        connect(m_aiManager, &AIOpponentManager::rankingsUpdated,
                this, [this](const QList<QString>&) {
                    updateRaceHud();
                });

        connect(m_aiManager, &AIOpponentManager::opponentFinished,
                this, [this](const QString& opponentId, int finalPosition) {
                    showInteractiveFeedback(QString("%1 Finished Rank #%2").arg(opponentId).arg(finalPosition),
                                            FeedbackType::Milestone);
                });
    }

    qApp->setStyleSheet(ThemeManager::getStyleSheet("dark") + ThemeManager::mainMenuNeonQss());

    // Build the report page (stackedWidget index 2) and embed the report widget.
    // We do this programmatically so it works regardless of whether the .ui
    // file has been regenerated after adding pageReport.
    {
        QWidget* pageReport = (ui->stackedWidget->count() > 2)
            ? ui->stackedWidget->widget(2)
            : nullptr;

        if (!pageReport) {
            pageReport = new QWidget();
            QVBoxLayout* lay = new QVBoxLayout(pageReport);
            lay->setContentsMargins(0, 0, 0, 0);
            lay->setSpacing(0);
            ui->stackedWidget->addWidget(pageReport);  // becomes index 2
        } else if (!pageReport->layout()) {
            QVBoxLayout* lay = new QVBoxLayout(pageReport);
            lay->setContentsMargins(0, 0, 0, 0);
            lay->setSpacing(0);
        }

        // Create the report widget with pageReport as parent so Qt's layout
        // system never places it outside the stackedWidget.
        m_reportWidget = new DrivingReportWidget(pageReport);
        m_reportWidget->setMockDataEnabled(false);
        pageReport->layout()->addWidget(m_reportWidget);
        m_reportPage = pageReport;   // cache for use in onGameFinished()

        connect(&SaveLoadManager::instance(),
                &SaveLoadManager::historyChanged,
                this,
                [this]() {
                    if (m_reportWidget) {
                        m_reportWidget->loadHistoryFromSaveLoadManager();
                    }
                });

    }

    connect(m_reportWidget, &DrivingReportWidget::backToMenuRequested,
            this, &MainWindow::onReportBackToMenu);
    connect(m_reportWidget, &DrivingReportWidget::newDriveRequested,
            this, &MainWindow::onReportNewDrive);

    loadPlayerProfile();
    setupGaragePage();

    m_scoreManager->setVehicleId("player_1");
    if (m_player2ScoreManager) {
        m_player2ScoreManager->setVehicleId(QStringLiteral("player_2"));
    }

    if (ui->btn_Arcade) {
        connect(ui->btn_Arcade, &QPushButton::clicked, this, [this]() {
            playSound(PhantomDrive::SoundEffect::ButtonClick);
            showArcadeSetupDialog();
        });
    }

    if (ui->btn_Learn) {
        connect(ui->btn_Learn, &QPushButton::clicked, this, [this]() {
            playSound(PhantomDrive::SoundEffect::ButtonClick);
            startDrivingSession("Learning");
        });
    }

    if (ui->btn_History) {
        connect(ui->btn_History, &QPushButton::clicked, this, [this]() {
            playSound(PhantomDrive::SoundEffect::ButtonClick);
            on_btn_History_clicked();
        });
    }

    if (ui->btn_Exit) {
        connect(ui->btn_Exit, &QPushButton::clicked, this, [this]() {
            playSound(PhantomDrive::SoundEffect::ButtonBack);
            close();
        });
    }

    if (ui->btn_Back) {
        connect(ui->btn_Back, &QPushButton::clicked, this, [this]() {
            playSound(PhantomDrive::SoundEffect::ButtonBack);
            returnToMainMenuFromGame(false);
        });
    }

    m_drivingDataCollector->setVehicleId("player_1");
    m_drivingDataCollector->setSamplingInterval(kSimulationStepMs);
    m_drivingDataCollector->setCurrentSpeedLimit(m_currentSpeedLimit, "main_route_speed_zone");
    if (m_drivingDataCollector->vehicleSensor()) {
        m_drivingDataCollector->vehicleSensor()->setSpeedLimitViolationEnabled(false);
    }
    if (m_player2DataCollector) {
        m_player2DataCollector->setVehicleId(QStringLiteral("player_2"));
        m_player2DataCollector->setSamplingInterval(kSimulationStepMs);
        m_player2DataCollector->setCurrentSpeedLimit(m_currentSpeedLimit, "main_route_speed_zone");
        if (m_player2DataCollector->vehicleSensor()) {
            m_player2DataCollector->vehicleSensor()->setSpeedLimitViolationEnabled(false);
        }
    }

    simulateGameLoop();
}

MainWindow::~MainWindow()
{
    if (m_drivingDataCollector) {
        m_drivingDataCollector->stopCollection();
    }
    if (m_player2DataCollector) {
        m_player2DataCollector->stopCollection();
    }
    delete ui;
}

void MainWindow::setDrivingDataCollector(DrivingDataCollector* collector)
{
    if (!collector || collector == m_drivingDataCollector) {
        return;
    }

    if (m_drivingDataCollector && m_drivingDataCollector->parent() == this) {
        m_drivingDataCollector->stopCollection();
        m_drivingDataCollector->deleteLater();
    }

    m_drivingDataCollector = collector;
    setupDataBindings();
}

void MainWindow::setupDataBindings()
{
    if (!m_drivingDataCollector) {
        return;
    }

    if (m_drivingDataCollector->vehicleSensor()) {
        m_drivingDataCollector->vehicleSensor()->setSpeedLimitViolationEnabled(false);
    }

    connect(m_drivingDataCollector, &IDrivingDataCollector::dataCollected,
            this, &MainWindow::onDrivingDataCollected, Qt::UniqueConnection);
    connect(m_drivingDataCollector, &IDrivingDataCollector::violationDetected,
            this, &MainWindow::onViolationDetected, Qt::UniqueConnection);
    connect(m_drivingDataCollector, &IDrivingDataCollector::violationDetected,
            m_scoreManager, &ScoreManager::onViolationDetected, Qt::UniqueConnection);

    if (m_trafficObjectManager) {
        connect(m_trafficObjectManager, &TrafficObjectManager::violationDetected,
                this, &MainWindow::handleTrafficViolation, Qt::UniqueConnection);
    }

    if (m_scoreManager) {
        m_scoreManager->setTrafficObjectManager(m_trafficObjectManager);
        connect(m_scoreManager, &ScoreManager::scoreReady,
                this, &MainWindow::onScoreReady, Qt::UniqueConnection);
        disconnect(m_scoreManager, &ScoreManager::coachReportReadyForVehicle, this, nullptr);
        connect(m_scoreManager, &ScoreManager::coachReportReadyForVehicle,
                this, [this](const QString& vehicleId, const QString& sessionId, const QString& markdown) {
                    if (isRealAiCoachReportMarkdown(markdown)) {
                        SaveLoadManager::instance().updateReportAiCoachReport(sessionId, vehicleId, markdown);
                    }
                    if (m_reportWidget) {
                        const int playerIndex = vehicleId == QStringLiteral("player_2") ? 2 : 1;
                        m_reportWidget->setCoachReportMarkdownForPlayer(playerIndex, sessionId, markdown);
                        m_reportWidget->hideLoading();
                    }
                });
        connect(m_scoreManager, &ScoreManager::scoringFailed,
                this, [this](const QString& reason) {
                    statusBar()->showMessage(QStringLiteral("Scoring failed: %1").arg(reason), 5000);
                });
    }

    if (m_player2ScoreManager) {
        disconnect(m_player2ScoreManager, &ScoreManager::coachReportReadyForVehicle, this, nullptr);
        connect(m_player2ScoreManager, &ScoreManager::coachReportReadyForVehicle,
                this, [this](const QString& vehicleId, const QString& sessionId, const QString& markdown) {
                    if (isRealAiCoachReportMarkdown(markdown)) {
                        SaveLoadManager::instance().updateReportAiCoachReport(sessionId, vehicleId, markdown);
                    }
                    if (m_reportWidget) {
                        const int playerIndex = vehicleId == QStringLiteral("player_1") ? 1 : 2;
                        m_reportWidget->setCoachReportMarkdownForPlayer(playerIndex, sessionId, markdown);
                        m_reportWidget->hideLoading();
                    }
                });
    }
}

void MainWindow::setupDemoControls()
{
    if (ui->btn_History) {
        ui->btn_History->setText(QStringLiteral("Driving Report / History"));
    }

    if (!m_btnLoadCustomTrack && ui->verticalLayout) {
        m_btnLoadCustomTrack = new QPushButton(QStringLiteral("Load Custom Track"), this);
        m_btnLoadCustomTrack->setObjectName(QStringLiteral("btn_LoadCustomTrack"));

        const int historyIndex = ui->btn_History ? ui->verticalLayout->indexOf(ui->btn_History) : -1;
        ui->verticalLayout->insertWidget(historyIndex >= 0 ? historyIndex : ui->verticalLayout->count(),
                                        m_btnLoadCustomTrack);

        connect(m_btnLoadCustomTrack, &QPushButton::clicked,
                this, &MainWindow::loadCustomTrack);
    }

    // Optional legacy button: most builds use the .ui top Finish Drive button.
    if (m_btnFinishDrive) {
        connect(m_btnFinishDrive, &QPushButton::clicked,
                this, &MainWindow::onGameFinished);
    }

    if (!m_btnGarage && ui->verticalLayout) {
        m_btnGarage = new QPushButton(QStringLiteral("Garage"), this);
        m_btnGarage->setObjectName(QStringLiteral("btn_Garage"));

        const int historyIndex = ui->btn_History ? ui->verticalLayout->indexOf(ui->btn_History) : -1;
        ui->verticalLayout->insertWidget(historyIndex >= 0 ? historyIndex : ui->verticalLayout->count(),
                                         m_btnGarage);

        connect(m_btnGarage, &QPushButton::clicked, this, [this]() {
            playSound(PhantomDrive::SoundEffect::ButtonClick);
            showGaragePage();
        });
    }

    if (!m_btnCoinChallenge && ui->verticalLayout) {
        m_btnCoinChallenge = new QPushButton(QStringLiteral("Coin Challenge"), this);
        m_btnCoinChallenge->setObjectName(QStringLiteral("btn_CoinChallenge"));

        const int arcadeIndex = ui->btn_Arcade ? ui->verticalLayout->indexOf(ui->btn_Arcade) : -1;
        ui->verticalLayout->insertWidget(arcadeIndex >= 0 ? arcadeIndex + 1 : ui->verticalLayout->count(),
                                         m_btnCoinChallenge);

        connect(m_btnCoinChallenge, &QPushButton::clicked, this, [this]() {
            playSound(PhantomDrive::SoundEffect::ButtonClick);
            showCoinChallengeTrackDialog();
        });
    }

    // Top HUD buttons (from .ui hudLayout): Back, Finish Drive, Exit Game
    if (ui->btn_Back) {
        // Back is already wired to handleReportBackToMenu in the btn_Back click handler above
    }
    if (m_btnPause) {
        connect(m_btnPause, &QPushButton::clicked,
                this, &MainWindow::toggleGamePaused);
    }
    if (ui->btn_FinishDrive_Top) {
        connect(ui->btn_FinishDrive_Top, &QPushButton::clicked,
                this, &MainWindow::onGameFinished);
    }
    if (ui->btn_ExitGame_Top) {
        connect(ui->btn_ExitGame_Top, &QPushButton::clicked,
                this, &MainWindow::exitApplicationFromGame);
    }
}

void MainWindow::setupGaragePage()
{
    if (!ui->stackedWidget || m_garageWidget) {
        return;
    }

    m_garageWidget = new GarageWidget(ui->stackedWidget);
    connect(m_garageWidget, &GarageWidget::backRequested, this, [this]() {
        if (ui->stackedWidget) {
            ui->stackedWidget->setCurrentIndex(0);
        }
        statusBar()->clearMessage();
    });
    connect(m_garageWidget, &GarageWidget::purchaseRequested,
            this, &MainWindow::handleGaragePurchase);
    connect(m_garageWidget, &GarageWidget::selectRequested,
            this, &MainWindow::handleGarageSelect);
    ui->stackedWidget->addWidget(m_garageWidget);
    refreshGaragePage();
}

void MainWindow::setupCustomTrackControls()
{
    if (!m_btnCustomTrackMode && ui->verticalLayout) {
        m_btnCustomTrackMode = new QPushButton(QStringLiteral("Custom Track Mode"), this);
        m_btnCustomTrackMode->setObjectName(QStringLiteral("btn_CustomTrackMode"));

        const int learningIndex = ui->btn_Learn ? ui->verticalLayout->indexOf(ui->btn_Learn) : -1;
        const int insertIndex = learningIndex >= 0 ? learningIndex + 1 : ui->verticalLayout->count();
        ui->verticalLayout->insertWidget(insertIndex, m_btnCustomTrackMode);

        connect(m_btnCustomTrackMode, &QPushButton::clicked,
                this, &MainWindow::showCustomTrackEditor);
    }

    if (!m_customTrackEditor && ui->stackedWidget && ui->stackedWidget->count() > 1) {
        QWidget* gamePage = ui->stackedWidget->widget(1);
        QVBoxLayout* pageLayout = gamePage ? qobject_cast<QVBoxLayout*>(gamePage->layout()) : nullptr;
        if (pageLayout) {
            m_customTrackEditor = new CustomTrackEditorWidget(gamePage);
            m_customTrackEditor->hide();
            m_customTrackEditor->setGeometry(gamePage->rect());
            m_customTrackEditor->raise();

            connect(m_customTrackEditor, &CustomTrackEditorWidget::playRequested,
                    this, &MainWindow::playCurrentCustomTrack);
            connect(m_customTrackEditor, &CustomTrackEditorWidget::saveRequested,
                    this, &MainWindow::saveCurrentCustomTrack);
            connect(m_customTrackEditor, &CustomTrackEditorWidget::loadRequested,
                    this, &MainWindow::loadCustomTrackIntoEditor);
            connect(m_customTrackEditor, &CustomTrackEditorWidget::exportJsonRequested,
                    this, &MainWindow::exportCurrentCustomTrackJson);
            connect(m_customTrackEditor, &CustomTrackEditorWidget::backRequested,
                    this, [this]() {
                        hideCustomTrackEditor();
                        if (ui->stackedWidget) {
                            ui->stackedWidget->setCurrentIndex(0);
                        }
                        statusBar()->clearMessage();
                    });
            connect(m_customTrackEditor, &CustomTrackEditorWidget::trackChanged,
                    this, [this](TrackData* track) {
                        if (m_customTrackMode) {
                            m_customTrackMode->setTrack(track);
                        }
                    });
        }
    }
}

void MainWindow::setupRaceSetupControls()
{
    if (!m_btnTwoPlayerRace && ui->verticalLayout) {
        m_btnTwoPlayerRace = new QPushButton(QStringLiteral("Two-Player Race"), this);
        m_btnTwoPlayerRace->setObjectName(QStringLiteral("btn_TwoPlayerRace"));

        const int arcadeIndex = ui->btn_Arcade ? ui->verticalLayout->indexOf(ui->btn_Arcade) : -1;
        ui->verticalLayout->insertWidget(arcadeIndex >= 0 ? arcadeIndex + 1 : ui->verticalLayout->count(),
                                        m_btnTwoPlayerRace);

        connect(m_btnTwoPlayerRace, &QPushButton::clicked, this, [this]() {
            playSound(PhantomDrive::SoundEffect::ButtonClick);
            showTwoPlayerSetupDialog();
        });
    }

    if (!m_btnAdaptiveDemo && ui->verticalLayout) {
        m_btnAdaptiveDemo = new QPushButton(QStringLiteral("Adaptive AI Demo"), this);
        m_btnAdaptiveDemo->setObjectName(QStringLiteral("btn_AdaptiveDemo"));

        const int twoPlayerIndex = m_btnTwoPlayerRace ? ui->verticalLayout->indexOf(m_btnTwoPlayerRace) : -1;
        const int arcadeIndex = ui->btn_Arcade ? ui->verticalLayout->indexOf(ui->btn_Arcade) : -1;
        const int insertIndex = twoPlayerIndex >= 0 ? twoPlayerIndex + 1
                                : (arcadeIndex >= 0 ? arcadeIndex + 1 : ui->verticalLayout->count());
        ui->verticalLayout->insertWidget(insertIndex, m_btnAdaptiveDemo);

        connect(m_btnAdaptiveDemo, &QPushButton::clicked, this, [this]() {
            playSound(PhantomDrive::SoundEffect::ButtonClick);
            m_twoPlayerMode = false;
            m_selectedAIDifficulty = QStringLiteral("adaptive");
            if (m_playerCountCombo) {
                const int singlePlayerIndex = m_playerCountCombo->findData(1);
                m_playerCountCombo->setCurrentIndex(singlePlayerIndex >= 0 ? singlePlayerIndex : 0);
            }
            if (m_aiDifficultyCombo) {
                const int adaptiveIndex = m_aiDifficultyCombo->findData(QStringLiteral("adaptive"));
                if (adaptiveIndex >= 0) {
                    m_aiDifficultyCombo->setCurrentIndex(adaptiveIndex);
                }
            }
            startBuiltInTrackSession(QStringLiteral("Arcade"));
        });
    }

    if (!m_btnGuide && ui->verticalLayout) {
        m_btnGuide = new QPushButton(QStringLiteral("Guide / Powerups"), this);
        m_btnGuide->setObjectName(QStringLiteral("btn_Guide"));

        const int historyIndex = ui->btn_History ? ui->verticalLayout->indexOf(ui->btn_History) : -1;
        ui->verticalLayout->insertWidget(historyIndex >= 0 ? historyIndex : ui->verticalLayout->count(), m_btnGuide);

        connect(m_btnGuide, &QPushButton::clicked, this, &MainWindow::showGuideDialog);
    }
}

void MainWindow::showArcadeSetupDialog()
{
    m_twoPlayerSetupActive = false;
    m_twoPlayerMode = false;

    if (!ui->stackedWidget) {
        m_selectedTrackId = m_arcadeTrackId;
        startBuiltInTrackSession(QStringLiteral("Arcade"));
        return;
    }

    QWidget* setupPage = ui->stackedWidget->findChild<QWidget*>(QStringLiteral("pageArcadeSetup"));
    if (!setupPage) {
        setupPage = new QWidget(ui->stackedWidget);
        setupPage->setObjectName(QStringLiteral("pageArcadeSetup"));
        setupPage->setStyleSheet(QStringLiteral(
            "QWidget#pageArcadeSetup{background:#050B16;color:#F4FAFF;}"
            "QFrame#setupPanel{background:#091426;border:1px solid #1D405D;border-radius:24px;}"
            "QFrame.setupCard{background:#0C1A2D;border:1px solid #1C3B58;border-radius:16px;}"
            "QFrame#trackCard{border:1px solid #23718A;}"
            "QLabel#label_ArcadeSetupTitle{color:#F7FBFF;font-size:30px;font-weight:900;letter-spacing:4px;}"
            "QLabel#label_ArcadeSetupEyebrow{color:#37D9F2;font-size:12px;font-weight:800;letter-spacing:4px;}"
            "QLabel#label_ArcadeSetupSubtitle{color:#7892AA;font-size:14px;}"
            "QLabel.arcadeSetupSection{color:#8DA6BC;font-size:12px;font-weight:800;letter-spacing:2px;}"
            "QLabel#label_TrackDifficulty{color:#FFD166;font-size:14px;font-weight:800;letter-spacing:1px;}"
            "QLabel#label_TrackDescription{color:#AFC6D8;font-size:14px;}"
            "QComboBox{background:#07111F;color:#F5FBFF;border:1px solid #2A4A64;"
            "border-radius:11px;padding:0 18px;font-size:18px;font-weight:750;min-height:52px;}"
            "QComboBox:hover{border-color:#37D9F2;background:#0A1828;}"
            "QComboBox:focus{border:2px solid #37D9F2;}"
            "QComboBox::drop-down{width:52px;border:0;border-left:1px solid #203C53;}"
            "QComboBox::down-arrow{image:url(assets/ui/chevron_down.svg);width:18px;height:18px;}"
            "QComboBox QAbstractItemView{background:#0B1728;color:#F5FBFF;border:1px solid #2A536E;"
            "selection-background-color:#126178;selection-color:#FFFFFF;padding:6px;outline:0;}"
            "QPushButton{background:#0A1627;color:#C9D8E5;border:1px solid #29445C;"
            "border-radius:11px;min-height:52px;font-size:16px;font-weight:800;padding:0 24px;}"
            "QPushButton:hover{background:#10243A;color:#FFFFFF;border-color:#4A7595;}"
            "QPushButton:pressed{background:#07101D;}"
            "QPushButton#btn_StartArcade{background:#16C7DF;border:1px solid #4BE8F7;color:#031018;"
            "font-size:17px;}"
            "QPushButton#btn_StartArcade:hover{background:#4BE8F7;border-color:#89F5FF;}"));

        auto* pageLayout = new QVBoxLayout(setupPage);
        pageLayout->setContentsMargins(42, 28, 42, 34);
        pageLayout->addStretch(1);

        auto* panel = new QFrame(setupPage);
        panel->setObjectName(QStringLiteral("setupPanel"));
        panel->setMinimumWidth(980);
        panel->setMaximumWidth(1180);
        auto* outer = new QVBoxLayout(panel);
        outer->setContentsMargins(46, 34, 46, 38);
        outer->setSpacing(0);

        auto* eyebrow = new QLabel(QStringLiteral("PHANTOM DRIVE  /  ARCADE"), panel);
        eyebrow->setObjectName(QStringLiteral("label_ArcadeSetupEyebrow"));
        eyebrow->setAlignment(Qt::AlignCenter);
        outer->addWidget(eyebrow);
        outer->addSpacing(8);

        auto* title = new QLabel(QStringLiteral("RACE SETUP"), panel);
        title->setObjectName(QStringLiteral("label_ArcadeSetupTitle"));
        title->setAlignment(Qt::AlignCenter);
        outer->addWidget(title);
        outer->addSpacing(7);

        auto* subtitle = new QLabel(QStringLiteral("Configure your grid, then take the line."), panel);
        subtitle->setObjectName(QStringLiteral("label_ArcadeSetupSubtitle"));
        subtitle->setAlignment(Qt::AlignCenter);
        outer->addWidget(subtitle);
        outer->addSpacing(28);

        auto* trackCard = new QFrame(panel);
        trackCard->setObjectName(QStringLiteral("trackCard"));
        trackCard->setProperty("class", QStringLiteral("setupCard"));
        auto* trackLayout = new QVBoxLayout(trackCard);
        trackLayout->setContentsMargins(22, 18, 22, 20);
        trackLayout->setSpacing(10);

        auto* trackSection = new QLabel(QStringLiteral("01  SELECT CIRCUIT"), trackCard);
        trackSection->setProperty("class", QStringLiteral("arcadeSetupSection"));
        trackLayout->addWidget(trackSection);

        m_trackSelectCombo = new QComboBox(trackCard);
        m_trackSelectCombo->setObjectName(QStringLiteral("combo_BuiltInTrack"));
        for (const BuiltInTrackInfo& info : BuiltInTrackFactory::tracks()) {
            m_trackSelectCombo->addItem(info.name, info.id);
        }
        trackLayout->addWidget(m_trackSelectCombo);

        m_trackDifficultyLabel = new QLabel(trackCard);
        m_trackDifficultyLabel->setObjectName(QStringLiteral("label_TrackDifficulty"));
        trackLayout->addWidget(m_trackDifficultyLabel);

        m_trackDescriptionLabel = new QLabel(trackCard);
        m_trackDescriptionLabel->setObjectName(QStringLiteral("label_TrackDescription"));
        m_trackDescriptionLabel->setWordWrap(true);
        m_trackDescriptionLabel->setMinimumHeight(24);
        trackLayout->addWidget(m_trackDescriptionLabel);
        outer->addWidget(trackCard);
        outer->addSpacing(16);

        auto* settingsRow = new QHBoxLayout();
        settingsRow->setSpacing(16);

        auto* playerCard = new QFrame(panel);
        playerCard->setProperty("class", QStringLiteral("setupCard"));
        auto* playerLayout = new QVBoxLayout(playerCard);
        playerLayout->setContentsMargins(22, 18, 22, 20);
        playerLayout->setSpacing(10);
        auto* playerLabel = new QLabel(QStringLiteral("02  GRID"), playerCard);
        playerLabel->setProperty("class", QStringLiteral("arcadeSetupSection"));
        playerLayout->addWidget(playerLabel);

        m_playerCountCombo = new QComboBox(playerCard);
        m_playerCountCombo->setObjectName(QStringLiteral("combo_PlayerCount"));
        m_playerCountCombo->addItem(QStringLiteral("1 Player + AI"), 1);
        m_playerCountCombo->addItem(QStringLiteral("2 Players + AI"), 2);
        playerLayout->addWidget(m_playerCountCombo);
        settingsRow->addWidget(playerCard, 1);

        auto* difficultyCard = new QFrame(panel);
        difficultyCard->setProperty("class", QStringLiteral("setupCard"));
        auto* difficultyLayout = new QVBoxLayout(difficultyCard);
        difficultyLayout->setContentsMargins(22, 18, 22, 20);
        difficultyLayout->setSpacing(10);
        auto* difficultyLabel = new QLabel(QStringLiteral("03  AI DIFFICULTY"), difficultyCard);
        difficultyLabel->setProperty("class", QStringLiteral("arcadeSetupSection"));
        difficultyLayout->addWidget(difficultyLabel);

        m_aiDifficultyCombo = new QComboBox(difficultyCard);
        m_aiDifficultyCombo->setObjectName(QStringLiteral("combo_AIDifficulty"));
        m_aiDifficultyCombo->addItem(QStringLiteral("Easy"), QStringLiteral("easy"));
        m_aiDifficultyCombo->addItem(QStringLiteral("Medium"), QStringLiteral("medium"));
        m_aiDifficultyCombo->addItem(QStringLiteral("Hard"), QStringLiteral("hard"));
        m_aiDifficultyCombo->addItem(QStringLiteral("Adaptive AI"), QStringLiteral("adaptive"));
        m_aiDifficultyCombo->setCurrentIndex(1);
        difficultyLayout->addWidget(m_aiDifficultyCombo);
        settingsRow->addWidget(difficultyCard, 1);
        outer->addLayout(settingsRow);
        outer->addSpacing(22);

        auto* buttonRow = new QHBoxLayout();
        buttonRow->setSpacing(12);

        auto* backButton = new QPushButton(QStringLiteral("BACK"), panel);
        backButton->setObjectName(QStringLiteral("btn_BackArcadeSetup"));
        backButton->setMaximumWidth(180);
        auto* startButton = new QPushButton(QStringLiteral("START RACE  >"), panel);
        startButton->setObjectName(QStringLiteral("btn_StartArcade"));
        buttonRow->addWidget(backButton);
        buttonRow->addStretch(1);
        buttonRow->addWidget(startButton, 1);
        outer->addLayout(buttonRow);

        pageLayout->addWidget(panel, 0, Qt::AlignHCenter);
        pageLayout->addStretch(1);

        connect(backButton, &QPushButton::clicked, this, [this]() {
            if (ui->stackedWidget) {
                ui->stackedWidget->setCurrentIndex(0);
            }
        });
        connect(startButton, &QPushButton::clicked, this, [this]() {
            if (m_trackSelectCombo) {
                m_selectedTrackId = m_trackSelectCombo->currentData().toString();
            }
            if (m_twoPlayerSetupActive) {
                m_twoPlayerTrackId = m_selectedTrackId;
            } else {
                m_arcadeTrackId = m_selectedTrackId;
            }
            m_selectedAIDifficulty = selectedAIDifficulty();
            m_twoPlayerMode = m_twoPlayerSetupActive || selectedPlayerCount() == 2;
            startBuiltInTrackSession(QStringLiteral("Arcade"));
        });

        connect(m_trackSelectCombo, &QComboBox::currentIndexChanged,
                this, [this](int) { updateArcadeSetupTrackInfo(); });
        connect(m_aiDifficultyCombo, &QComboBox::currentIndexChanged,
                this, [this](int) { m_selectedAIDifficulty = selectedAIDifficulty(); });

        ui->stackedWidget->addWidget(setupPage);
    }

    if (m_trackSelectCombo) {
        const QString setupTrackId = m_twoPlayerSetupActive
            ? m_twoPlayerTrackId
            : m_arcadeTrackId;
        const int trackIndex = m_trackSelectCombo->findData(setupTrackId);
        m_trackSelectCombo->setCurrentIndex(trackIndex >= 0 ? trackIndex : 0);
    }
    if (m_playerCountCombo) {
        m_playerCountCombo->setCurrentIndex(m_twoPlayerSetupActive ? 1 : 0);
        m_playerCountCombo->setEnabled(!m_twoPlayerSetupActive);
    }
    if (m_aiDifficultyCombo) {
        const int difficultyIndex = m_aiDifficultyCombo->findData(m_selectedAIDifficulty);
        m_aiDifficultyCombo->setCurrentIndex(difficultyIndex >= 0 ? difficultyIndex : 1);
    }
    if (QLabel* title =
            setupPage->findChild<QLabel*>(QStringLiteral("label_ArcadeSetupTitle"))) {
        title->setText(QStringLiteral("RACE SETUP"));
    }
    if (QLabel* eyebrow =
            setupPage->findChild<QLabel*>(QStringLiteral("label_ArcadeSetupEyebrow"))) {
        eyebrow->setText(QStringLiteral("PHANTOM DRIVE  /  ARCADE"));
    }
    updateArcadeSetupTrackInfo();
    ui->stackedWidget->setCurrentWidget(setupPage);
    statusBar()->clearMessage();
}

void MainWindow::showTwoPlayerSetupDialog()
{
    m_twoPlayerSetupActive = true;
    m_twoPlayerMode = true;

    showArcadeSetupDialog();

    // showArcadeSetupDialog initializes the shared setup page as single-player.
    // Switch it into the dedicated two-player context after creation.
    m_twoPlayerSetupActive = true;
    m_twoPlayerMode = true;
    if (m_trackSelectCombo) {
        const int trackIndex = m_trackSelectCombo->findData(m_twoPlayerTrackId);
        m_trackSelectCombo->setCurrentIndex(trackIndex >= 0 ? trackIndex : 0);
    }
    if (m_playerCountCombo) {
        const int twoPlayerIndex = m_playerCountCombo->findData(2);
        m_playerCountCombo->setCurrentIndex(twoPlayerIndex >= 0 ? twoPlayerIndex : 1);
        m_playerCountCombo->setEnabled(false);
    }
    if (ui->stackedWidget) {
        QWidget* setupPage =
            ui->stackedWidget->findChild<QWidget*>(QStringLiteral("pageArcadeSetup"));
        if (setupPage) {
            if (QLabel* title =
                    setupPage->findChild<QLabel*>(QStringLiteral("label_ArcadeSetupTitle"))) {
                title->setText(QStringLiteral("TWO-PLAYER SETUP"));
            }
            if (QLabel* eyebrow =
                    setupPage->findChild<QLabel*>(QStringLiteral("label_ArcadeSetupEyebrow"))) {
                eyebrow->setText(QStringLiteral("PHANTOM DRIVE  /  TWO-PLAYER"));
            }
        }
    }
    updateArcadeSetupTrackInfo();
}

void MainWindow::showCoinChallengeTrackDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("Coin Challenge Track"));
    dialog.setModal(true);
    dialog.setStyleSheet(QStringLiteral(
        "QDialog{background:#06101F;color:#EAFBFF;}"
        "QLabel#coinChallengeSetupTitle{color:#38F6FF;font-size:22px;font-weight:800;letter-spacing:5px;}"
        "QLabel.coinChallengeSetupCopy{color:#BCEEFF;font-size:14px;}"
        "QComboBox{background:#071126;color:#F3FBFF;border:2px solid #00CFE8;"
        "border-radius:10px;padding:8px 18px;font-size:18px;font-weight:700;min-height:44px;}"
        "QPushButton{background:#071126;color:#F3FBFF;border:2px solid #00CFE8;"
        "border-radius:10px;min-height:44px;font-size:16px;font-weight:800;padding:0 20px;}"
        "QPushButton:hover{background:#0A1C38;border-color:#38F6FF;}"
        "QPushButton#btn_StartCoinChallenge{background:#0B2734;border-color:#35F6FF;color:#FFFFFF;}"));

    auto* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(22, 22, 22, 18);
    layout->setSpacing(14);

    auto* title = new QLabel(QStringLiteral("COIN CHALLENGE"), &dialog);
    title->setObjectName(QStringLiteral("coinChallengeSetupTitle"));
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    auto* subtitle = new QLabel(QStringLiteral("Select a built-in track for this challenge run."), &dialog);
    subtitle->setProperty("class", QStringLiteral("coinChallengeSetupCopy"));
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setWordWrap(true);
    layout->addWidget(subtitle);

    auto* trackCombo = new QComboBox(&dialog);
    for (const BuiltInTrackInfo& info : BuiltInTrackFactory::tracks()) {
        trackCombo->addItem(QStringLiteral("%1  |  %2").arg(info.name, info.difficulty), info.id);
    }
    const int trackIndex = trackCombo->findData(m_selectedTrackId);
    trackCombo->setCurrentIndex(trackIndex >= 0 ? trackIndex : 0);
    layout->addWidget(trackCombo);

    auto* infoLabel = new QLabel(&dialog);
    infoLabel->setProperty("class", QStringLiteral("coinChallengeSetupCopy"));
    infoLabel->setWordWrap(true);
    layout->addWidget(infoLabel);

    auto updateInfo = [trackCombo, infoLabel]() {
        const QString selectedId = trackCombo->currentData().toString();
        for (const BuiltInTrackInfo& info : BuiltInTrackFactory::tracks()) {
            if (info.id == selectedId) {
                infoLabel->setText(QStringLiteral("%1\nDifficulty: %2")
                                       .arg(info.description, info.difficulty));
                return;
            }
        }
        infoLabel->clear();
    };
    updateInfo();
    connect(trackCombo, &QComboBox::currentIndexChanged, &dialog, [updateInfo](int index) {
        Q_UNUSED(index);
        updateInfo();
    });

    auto* buttonRow = new QHBoxLayout();
    buttonRow->setSpacing(12);
    auto* cancelButton = new QPushButton(QStringLiteral("Cancel"), &dialog);
    auto* startButton = new QPushButton(QStringLiteral("Start Challenge"), &dialog);
    startButton->setObjectName(QStringLiteral("btn_StartCoinChallenge"));
    buttonRow->addWidget(cancelButton);
    buttonRow->addWidget(startButton);
    layout->addLayout(buttonRow);

    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(startButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    m_selectedTrackId = trackCombo->currentData().toString();
    if (m_selectedTrackId.isEmpty()) {
        m_selectedTrackId = QStringLiteral("neon_loop");
    }
    startCoinChallengeMode();
}

void MainWindow::showGaragePage()
{
    if (!ui->stackedWidget) {
        return;
    }

    if (!m_garageWidget) {
        setupGaragePage();
    }

    refreshGaragePage();
    ui->stackedWidget->setCurrentWidget(m_garageWidget);
    statusBar()->showMessage(QStringLiteral("Garage ready"));
}

void MainWindow::startCoinChallengeMode()
{
    if (m_driveActive) {
        silentFinishSession();
    }
    clearTransientDrivingFeedback();
    hideCoinChallengeSummaryOverlay();
    m_coinChallengeMagnetRemainingMs = 0;
    m_coinChallengeBalloonRushRemainingMs = 0;
    m_coinChallengeBalloonRushTriggerRemainingMs = 0;
    m_coinChallengeBalloonRushPhase = CoinChallengeBalloonRushPhase::Inactive;
    syncCoinChallengeMagnetLoop();

    if (m_learningSessionTimer) {
        m_learningSessionTimer->stop();
        m_learningSessionTimer->deleteLater();
        m_learningSessionTimer = nullptr;
    }

    m_currentMode = QStringLiteral("Coin Challenge");
    m_twoPlayerMode = false;
    m_twoPlayerFinishHandled = false;
    m_player1Finished = false;
    m_player2Finished = false;
    m_arcadeRaceLogicActive = false;
    m_customTrackPlaying = false;
    m_coinChallengeActive = true;
    m_driveActive = true;
    m_arcadeRaceFinished = false;
    m_lapsCompleted = 0;
    m_totalLaps = 1;
    m_coinChallengeRemainingMs = m_coinChallengeDurationMs;
    m_simTick = 0;
    m_sessionElapsedMs = 0;
    m_currentLapStartMs = 0;
    m_bestLapMs = 0;
    m_playerSpeed = 0.0;
    m_player2Speed = 0.0;
    m_player2LapsCompleted = 0;
    m_player2NextCheckpointIndex = 0;
    m_player2WasInsideNextGate = false;
    resetCoinChallengeStats();

    if (m_drivingDataCollector) {
        m_drivingDataCollector->stopCollection();
        m_drivingDataCollector->clearData();
    }
    if (m_player2DataCollector) {
        m_player2DataCollector->stopCollection();
        m_player2DataCollector->clearData();
    }

    if (m_selectedBuiltInTrack) {
        m_selectedBuiltInTrack->deleteLater();
        m_selectedBuiltInTrack = nullptr;
    }
    const QString trackId = m_selectedTrackId.isEmpty() ? QStringLiteral("neon_loop") : m_selectedTrackId;
    m_selectedBuiltInTrack = BuiltInTrackFactory::createTrack(trackId, this);
    if (!m_selectedBuiltInTrack) {
        m_selectedTrackId = QStringLiteral("neon_loop");
        m_selectedBuiltInTrack = BuiltInTrackFactory::createTrack(m_selectedTrackId, this);
    }
    m_defaultRaceTrack = m_selectedBuiltInTrack;

    TrackManager* trackMgr = TrackManager::instance(this);
    if (trackMgr && m_defaultRaceTrack) {
        trackMgr->setCurrentTrack(m_defaultRaceTrack);
    }

    if (ui->stackedWidget) {
        ui->stackedWidget->setCurrentIndex(1);
    }
    if (m_customTrackEditor) {
        m_customTrackEditor->hide();
    }

    setGameHeaderVisible(true);
    updatePauseButtonState();
    if (m_btnFinishDrive) {
        m_btnFinishDrive->show();
        m_btnFinishDrive->setEnabled(false);
    }
    if (m_gameView) {
        m_gameView->show();
        m_gameView->clearAllAICars();
        m_gameView->clearCoins();
        m_gameView->clearCoinChallengeOverlayState();
        m_gameView->clearBalloonRushSceneState();
    }
    if (m_aiManager) {
        m_aiManager->destroyAllOpponents();
    }
    clearEBRuntimeObjects();

    restoreDefaultRaceTrack();
    focusGameViewForDriving();

    if (ui->label_ModeTitle) {
        ui->label_ModeTitle->setText(QStringLiteral("COIN CHALLENGE"));
        ui->label_ModeTitle->setStyleSheet(
            "QLabel{color:#F5C451;font-size:13px;font-weight:bold;letter-spacing:3px;}");
    }

    if (m_learningHUD) {
        m_learningHUD->hide();
    }
    if (m_arcadeHUD) {
        m_arcadeHUD->hide();
    }

    if (m_vehiclePhysics) {
        m_vehiclePhysics->reset();
        m_vehiclePhysics->resetRaceProgress();
        m_vehiclePhysics->setRaceLogicEnabled(false);
    }
    if (m_player2Physics) {
        m_player2Physics->reset();
        m_player2Physics->resetRaceProgress();
        m_player2Physics->setRaceLogicEnabled(false);
    }

    applyPlayerSpawnAtStartLine();
    resetArcadeRaceProgress();

    if (m_challengeObstacleManager) {
        m_challengeObstacleManager->setTrack(m_gameView ? m_gameView->trackData() : m_defaultRaceTrack);
        m_challengeObstacleManager->resetForCoinChallenge();
    }
    if (m_blockerVehicleManager) {
        m_blockerVehicleManager->setTrack(m_gameView ? m_gameView->trackData() : m_defaultRaceTrack);
        m_blockerVehicleManager->resetForCoinChallenge();
        m_blockerVehicleManager->update(0.0, 0, m_coinChallengeDurationMs / 1000, 0);
    }

    if (m_collectibleManager) {
        m_collectibleManager->setTrack(m_gameView ? m_gameView->trackData() : m_defaultRaceTrack);
        m_collectibleManager->setBlockedAreas(currentCoinChallengeBlockedAreas());
        m_collectibleManager->resetForCoinChallenge();
        m_collectibleManager->setBlockedAreas(currentCoinChallengeBlockedAreas());
    }
    setupCoinChallengePowerupBoxes();
    if (m_gameView && m_collectibleManager) {
        m_gameView->updateCoins(m_collectibleManager->coins());
    }
    refreshCoinChallengeHazardVisuals();

    m_countdownActive = true;
    m_countdownRemainingMs = 3200;

    updateCoinChallengeHud();

    showCountdown();
    statusBar()->showMessage(QStringLiteral("Coin Challenge: get ready"), 2500);
}

void MainWindow::finishCoinChallengeMode()
{
    if (!m_coinChallengeActive) {
        return;
    }

    m_coinChallengeActive = false;
    m_driveActive = false;
    m_arcadeRaceLogicActive = false;
    m_customTrackPlaying = false;
    m_twoPlayerMode = false;
    m_countdownActive = false;
    m_coinChallengeRemainingMs = m_coinChallengeDurationMs;
    m_coinChallengeMagnetRemainingMs = 0;
    m_coinChallengeBalloonRushRemainingMs = 0;
    m_coinChallengeBalloonRushTriggerRemainingMs = 0;
    m_coinChallengeBalloonRushPhase = CoinChallengeBalloonRushPhase::Inactive;
    syncCoinChallengeMagnetLoop();
    clearTransientDrivingFeedback();

    savePlayerProfile();
    refreshGaragePage();
    clearEBRuntimeObjects();

    if (m_aiManager) {
        m_aiManager->destroyAllOpponents();
    }
    if (m_gameView) {
        m_gameView->clearAllAICars();
        m_gameView->clearCoins();
        m_gameView->clearChallengeObstacles();
        m_gameView->clearBlockerVehicles();
        m_gameView->clearCoinChallengeOverlayState();
        m_gameView->clearBalloonRushSceneState();
    }
    if (m_collectibleManager) {
        m_collectibleManager->clear();
    }
    if (m_challengeObstacleManager) {
        m_challengeObstacleManager->clear();
    }
    if (m_blockerVehicleManager) {
        m_blockerVehicleManager->clear();
    }
    if (m_coinChallengeHudLabel) {
        m_coinChallengeHudLabel->hide();
        m_coinChallengeHudLabel->clear();
    }
    if (m_coinChallengeSummaryAnimTimer) {
        m_coinChallengeSummaryAnimTimer->stop();
    }
    hideCoinChallengeSummaryOverlay();
}

void MainWindow::styleMainMenu()
{
    if (!ui->pageMenu) {
        return;
    }

    if (!m_mainMenuWidget) {
        m_mainMenuWidget = new MainMenuWidget(ui->pageMenu);
        m_mainMenuWidget->setReferenceBackgrounds(QStringLiteral("assets/ui/menu/menu_reference_main.png"),
                                                 QStringLiteral("assets/ui/menu/menu_reference_start.png"));
        connect(m_mainMenuWidget, &MainMenuWidget::arcadeRequested, this, [this]() {
            playSound(PhantomDrive::SoundEffect::ButtonClick);
            showArcadeSetupDialog();
        });
        connect(m_mainMenuWidget, &MainMenuWidget::coinChallengeRequested, this, [this]() {
            playSound(PhantomDrive::SoundEffect::ButtonClick);
            showCoinChallengeTrackDialog();
        });
        connect(m_mainMenuWidget, &MainMenuWidget::learningRequested, this, [this]() {
            playSound(PhantomDrive::SoundEffect::ButtonClick);
            startDrivingSession(QStringLiteral("Learning"));
        });
        connect(m_mainMenuWidget, &MainMenuWidget::aiDemoRequested, this, [this]() {
            playSound(PhantomDrive::SoundEffect::ButtonClick);
            showTwoPlayerSetupDialog();
        });
        connect(m_mainMenuWidget, &MainMenuWidget::garageRequested, this, [this]() {
            playSound(PhantomDrive::SoundEffect::ButtonClick);
            showGaragePage();
        });
        connect(m_mainMenuWidget, &MainMenuWidget::trackStudioRequested, this, [this]() {
            playSound(PhantomDrive::SoundEffect::ButtonClick);
            showCustomTrackEditor();
        });
        connect(m_mainMenuWidget, &MainMenuWidget::recordsRequested, this, [this]() {
            playSound(PhantomDrive::SoundEffect::ButtonClick);
            on_btn_History_clicked();
        });
        connect(m_mainMenuWidget, &MainMenuWidget::guideRequested, this, [this]() {
            playSound(PhantomDrive::SoundEffect::ButtonClick);
            showGuideDialog();
        });
        connect(m_mainMenuWidget, &MainMenuWidget::exitRequested, this, [this]() {
            playSound(PhantomDrive::SoundEffect::ButtonBack);
            close();
        });

        if (m_currencyManager) {
            connect(m_currencyManager, &CurrencyManager::coinsChanged,
                    m_mainMenuWidget, &MainMenuWidget::setCoinCount);
            m_mainMenuWidget->setCoinCount(m_currencyManager->coins());
        }
    }

    if (ui->label) {
        ui->label->hide();
    }

    const QList<QWidget*> legacyMenuWidgets = {
        ui->btn_Arcade,
        ui->btn_Learn,
        ui->btn_History,
        ui->btn_Exit,
        m_btnCoinChallenge,
        m_btnGarage,
        m_btnLoadCustomTrack,
        m_btnCustomTrackMode,
        m_btnTwoPlayerRace,
        m_btnAdaptiveDemo,
        m_btnGuide
    };
    for (QWidget* widget : legacyMenuWidgets) {
        if (widget) {
            widget->hide();
            widget->setEnabled(false);
        }
    }

    if (ui->menuGridLayout) {
        ui->menuGridLayout->setContentsMargins(0, 0, 0, 0);
        ui->menuGridLayout->setSpacing(0);
        ui->menuGridLayout->setRowStretch(0, 1);
        ui->menuGridLayout->setRowStretch(1, 1);
        ui->menuGridLayout->setColumnStretch(0, 1);
        if (m_mainMenuWidget && ui->menuGridLayout->indexOf(m_mainMenuWidget) < 0) {
            ui->menuGridLayout->addWidget(m_mainMenuWidget, 0, 0, 2, 1);
        }
    }
    if (ui->verticalLayout) {
        ui->verticalLayout->setContentsMargins(0, 0, 0, 0);
        ui->verticalLayout->setSpacing(0);
    }

    ui->pageMenu->setStyleSheet(QStringLiteral("QWidget#pageMenu{background:#040816;}"));
    if (!ui->menuGridLayout) {
        m_mainMenuWidget->setGeometry(ui->pageMenu->rect());
    }
    m_mainMenuWidget->show();
    m_mainMenuWidget->raise();
}

void MainWindow::setGameHeaderVisible(bool visible)
{
    if (ui->label_Speed) {
        ui->label_Speed->setVisible(visible);
    }
    if (ui->label_Limit) {
        ui->label_Limit->setVisible(visible);
    }
    if (ui->label_) {
        ui->label_->setVisible(visible);
    }
    if (ui->label_ModeTitle) {
        ui->label_ModeTitle->setVisible(visible);
    }
    if (ui->btn_Back) {
        ui->btn_Back->setVisible(visible);
    }
    if (m_btnPause) {
        m_btnPause->setVisible(visible);
    }
    if (ui->btn_FinishDrive_Top) {
        ui->btn_FinishDrive_Top->setVisible(visible);
    }
    if (ui->btn_ExitGame_Top) {
        ui->btn_ExitGame_Top->setVisible(visible);
    }

    QWidget* gamePage = ui->stackedWidget && ui->stackedWidget->count() > 1
        ? ui->stackedWidget->widget(1)
        : nullptr;
    QVBoxLayout* pageLayout = gamePage ? qobject_cast<QVBoxLayout*>(gamePage->layout()) : nullptr;
    if (!pageLayout || pageLayout->count() == 0) {
        return;
    }

    pageLayout->setContentsMargins(visible ? 8 : 0,
                                   visible ? 8 : 0,
                                   visible ? 8 : 0,
                                   visible ? 8 : 0);
    pageLayout->setSpacing(visible ? 4 : 0);

    QLayoutItem* headerItem = pageLayout->itemAt(0);
    QHBoxLayout* hudLayout = headerItem ? qobject_cast<QHBoxLayout*>(headerItem->layout()) : nullptr;
    if (!hudLayout) {
        return;
    }

    hudLayout->setSpacing(visible ? 12 : 0);
    for (int i = 0; i < hudLayout->count(); ++i) {
        QLayoutItem* item = hudLayout->itemAt(i);
        QSpacerItem* spacer = item ? item->spacerItem() : nullptr;
        if (spacer) {
            spacer->changeSize(visible ? 20 : 0,
                               visible ? 20 : 0,
                               visible ? QSizePolicy::Expanding : QSizePolicy::Fixed,
                               QSizePolicy::Fixed);
        }
    }

    hudLayout->invalidate();
    pageLayout->invalidate();
}

void MainWindow::clearTransientDrivingFeedback()
{
    ++m_sessionGeneration;
    m_hudRaceProgressById.clear();
    m_hudRaceRouteSignature.clear();
    setGamePaused(false);
    m_countdownActive = false;
    m_countdownRemainingMs = 0;
    m_countdownTimerStartedMs = 0;
    m_countdownSessionGeneration = 0;
    m_learningTimerRemainingMs = 0;
    m_learningTimerStartedMs = 0;
    if (m_countdownFinishTimer) {
        m_countdownFinishTimer->stop();
    }
    PhantomDrive::InteractiveFeedback::instance(this).clearAll();
    if (m_learningHUD) {
        m_learningHUD->clearAllWarnings();
    }
    if (m_arcadeHUD) {
        m_arcadeHUD->clearPowerupState();
    }

}

void MainWindow::toggleGamePaused()
{
    if (!m_driveActive && !m_countdownActive) {
        return;
    }

    setGamePaused(!m_gamePaused);
}

void MainWindow::setGamePaused(bool paused)
{
    if (m_gamePaused == paused) {
        updatePauseButtonState();
        return;
    }

    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    m_gamePaused = paused;

    if (m_gamePaused) {
        auto releaseVehicleInputs = [](VehiclePhysics* physics) {
            if (!physics) {
                return;
            }
            const int keys[] = {
                Qt::Key_W, Qt::Key_A, Qt::Key_S, Qt::Key_D, Qt::Key_Space,
                Qt::Key_Up, Qt::Key_Down, Qt::Key_Left, Qt::Key_Right
            };
            for (int key : keys) {
                QKeyEvent releaseEvent(QEvent::KeyRelease, key, Qt::NoModifier);
                physics->handleKeyRelease(&releaseEvent);
            }
        };
        releaseVehicleInputs(m_vehiclePhysics);
        releaseVehicleInputs(m_player2Physics);

        if (m_countdownFinishTimer && m_countdownFinishTimer->isActive()) {
            const qint64 elapsed = qMax<qint64>(0, nowMs - m_countdownTimerStartedMs);
            m_countdownRemainingMs = qMax(1, m_countdownRemainingMs - static_cast<int>(elapsed));
            m_countdownFinishTimer->stop();
        }
        if (m_learningSessionTimer && m_learningSessionTimer->isActive()) {
            const qint64 elapsed = qMax<qint64>(0, nowMs - m_learningTimerStartedMs);
            m_learningTimerRemainingMs = qMax(1, m_learningTimerRemainingMs - static_cast<int>(elapsed));
            m_learningSessionTimer->stop();
        }
        PhantomDrive::InteractiveFeedback::instance(this).pause();
    } else {
        if (m_countdownActive && m_countdownRemainingMs > 0) {
            startCountdownFinishTimer(m_countdownRemainingMs);
        }
        if (m_learningSessionTimer && m_learningTimerRemainingMs > 0 && m_driveActive
            && m_currentMode == QStringLiteral("Learning")) {
            m_learningTimerStartedMs = nowMs;
            m_learningSessionTimer->start(m_learningTimerRemainingMs);
        }
        PhantomDrive::InteractiveFeedback::instance(this).resume();
    }

    if (m_gameView) {
        m_gameView->setPaused(m_gamePaused);
    }
    if (m_arcadeHUD) {
        m_arcadeHUD->setPaused(m_gamePaused);
    }
    syncCoinChallengeMagnetLoop();
    updatePauseButtonState();
}

void MainWindow::updatePauseButtonState()
{
    if (!m_btnPause) {
        return;
    }

    m_btnPause->setText(m_gamePaused ? QStringLiteral("Resume") : QStringLiteral("Pause"));
    m_btnPause->setEnabled(m_driveActive || m_countdownActive);
    m_btnPause->setStyleSheet(m_gamePaused ? R"(
        QPushButton {
            background: rgba(8, 80, 72, 235);
            color: #DDFFF5;
            border: 2px solid #00FFA0;
            border-radius: 8px;
            padding: 4px 16px;
            font-size: 12px;
            font-weight: bold;
            letter-spacing: 1px;
        }
        QPushButton:hover {
            background: rgba(12, 104, 92, 245);
            border-color: #66FFC8;
        }
        QPushButton:pressed {
            background: rgba(6, 55, 54, 245);
        }
    )" : R"(
        QPushButton {
            background: rgba(10, 38, 72, 220);
            color: #66F7FF;
            border: 2px solid #00D6FF;
            border-radius: 8px;
            padding: 4px 16px;
            font-size: 12px;
            font-weight: bold;
            letter-spacing: 1px;
        }
        QPushButton:hover {
            background: rgba(16, 58, 105, 240);
            border-color: #66F7FF;
        }
        QPushButton:pressed {
            background: rgba(8, 26, 58, 240);
        }
    )");
}

void MainWindow::startCountdownFinishTimer(int remainingMs)
{
    if (!m_countdownFinishTimer || remainingMs <= 0) {
        return;
    }

    m_countdownRemainingMs = remainingMs;
    m_countdownTimerStartedMs = QDateTime::currentMSecsSinceEpoch();
    m_countdownSessionGeneration = m_sessionGeneration;
    m_countdownFinishTimer->start(m_countdownRemainingMs);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);

    if (m_mainMenuWidget && ui->pageMenu) {
        if (!ui->menuGridLayout || ui->menuGridLayout->indexOf(m_mainMenuWidget) < 0) {
            m_mainMenuWidget->setGeometry(ui->pageMenu->rect());
        } else {
            m_mainMenuWidget->updateGeometry();
        }
        m_mainMenuWidget->raise();
    }

    if (m_btnGuide && ui->pageMenu) {
        m_btnGuide->setFixedSize(380, 52);
        m_btnGuide->move(qMax(24, ui->pageMenu->width() - m_btnGuide->width() - 64), 52);
        m_btnGuide->raise();
    }

    layoutCoinChallengeHud();
    if (m_coinChallengeSummaryOverlay && ui->stackedWidget && ui->stackedWidget->count() > 1) {
        if (QWidget* gamePage = ui->stackedWidget->widget(1)) {
            m_coinChallengeSummaryOverlay->setGeometry(gamePage->rect());
        }
    }

    if (!m_customTrackEditor || m_customTrackPlaying
        || m_currentMode != QStringLiteral("Custom Track")
        || !m_customTrackEditor->isVisible()
        || !ui->stackedWidget || ui->stackedWidget->currentIndex() != 1) {
        return;
    }

    QWidget* gamePage = ui->stackedWidget->widget(1);
    if (!gamePage) {
        return;
    }

    m_customTrackEditor->setGeometry(gamePage->rect());
    m_customTrackEditor->raise();
}

void MainWindow::returnToMainMenuFromGame(bool finishWithReport)
{
    QWidget* reportPage = m_reportPage ? m_reportPage
                                       : (ui->pageReport ? static_cast<QWidget*>(ui->pageReport)
                                                         : (ui->stackedWidget && ui->stackedWidget->count() > 2
                                                                ? ui->stackedWidget->widget(2)
                                                                : nullptr));
    if (reportPage && ui->stackedWidget && ui->stackedWidget->currentWidget() == reportPage) {
        onReportBackToMenu();
        return;
    }

    if (isCoinChallengeModeActive()) {
        exitCoinChallengeToMenu();
        return;
    }

    if (finishWithReport && m_driveActive) {
        onGameFinished();
        return;
    }

    if (m_driveActive) {
        silentFinishSession();
    }
    clearTransientDrivingFeedback();

    if (m_currentMode == QStringLiteral("Custom Track") && !m_customTrackPlaying) {
        hideCustomTrackEditor();
    }

    m_customTrackPlaying = false;
    m_arcadeRaceLogicActive = false;

    if (m_customTrackEditor) {
        m_customTrackEditor->hide();
        m_customTrackEditor->clearFocus();
    }
    if (m_gameView) {
        m_gameView->hide();
        m_gameView->clearAllAICars();
    }
    if (m_learningHUD) {
        m_learningHUD->hide();
    }
    if (m_arcadeHUD) {
        m_arcadeHUD->hide();
    }
    if (m_aiManager) {
        m_aiManager->destroyAllOpponents();
    }
    if (ui->btn_Back) {
        ui->btn_Back->setEnabled(true);
    }

    setGameHeaderVisible(false);
    if (ui->stackedWidget) {
        ui->stackedWidget->setCurrentIndex(0);
    }
    statusBar()->clearMessage();
}

void MainWindow::exitApplicationFromGame()
{
    if (m_driveActive) {
        silentFinishSession();
    }
    clearTransientDrivingFeedback();
    close();
}

void MainWindow::showCustomTrackEditor()
{
    if (m_driveActive) {
        silentFinishSession();
    }

    m_currentMode = QStringLiteral("Custom Track");
    m_customTrackPlaying = false;
    m_arcadeRaceLogicActive = false;
    clearTransientDrivingFeedback();

    if (m_customTrackMode) {
        m_customTrackMode->onEnter();
        m_customTrackMode->setCustomState(CustomTrackModeState::Editing);
        if (m_customTrackEditor) {
            m_customTrackMode->setTrack(m_customTrackEditor->trackData());
        }
    }

    if (ui->stackedWidget) {
        ui->stackedWidget->setCurrentIndex(1);
    }
    if (m_gameView) {
        m_gameView->hide();
    }
    if (m_arcadeHUD) {
        m_arcadeHUD->hide();
    }
    if (m_learningHUD) {
        m_learningHUD->hide();
    }
    if (m_btnFinishDrive) {
        m_btnFinishDrive->setEnabled(false);
    }
    setGameHeaderVisible(false);
    if (m_customTrackEditor) {
        QWidget* gamePage = ui->stackedWidget ? ui->stackedWidget->widget(1) : nullptr;
        if (gamePage) {
            m_customTrackEditor->setGeometry(gamePage->rect());
        }
        m_customTrackEditor->show();
        m_customTrackEditor->raise();
        m_customTrackEditor->setFocus();
    }

    clearEBRuntimeObjects();
    statusBar()->showMessage(QStringLiteral("Custom Track Mode: edit a 24 x 18 tile track"));
}

void MainWindow::hideCustomTrackEditor()
{
    if (m_customTrackEditor) {
        m_customTrackEditor->hide();
        m_customTrackEditor->clearFocus();
    }
    if (m_btnFinishDrive) {
        m_btnFinishDrive->show();
    }
    setGameHeaderVisible(true);
    updatePauseButtonState();
    if (m_customTrackMode) {
        m_customTrackMode->setCustomState(CustomTrackModeState::Editing);
    }
}

void MainWindow::playCurrentCustomTrack()
{
    TrackData* track = m_customTrackEditor ? m_customTrackEditor->trackData() : nullptr;
    if (!track) {
        QMessageBox::warning(this,
                             QStringLiteral("Custom Track"),
                             QStringLiteral("No custom track is available to play."));
        return;
    }

    const TrackValidationResult validation = TrackValidator::validateCustomTrack(*track);
    if (!validation.ok) {
        QMessageBox::warning(this,
                             QStringLiteral("Track Is Not Playable"),
                             validation.errors.join(QStringLiteral("\n")));
        return;
    }

    playSound(PhantomDrive::SoundEffect::ButtonClick);
    if (m_customTrackEditor) {
        m_twoPlayerMode = m_customTrackEditor->selectedPlayerCount() == 2;
        m_selectedAIDifficulty = m_customTrackEditor->selectedAiDifficulty();
    }
    dumpCustomTrackLayoutForDebug(track, QStringLiteral("Editor before Play snapshot"));
    if (m_runtimeCustomTrack) {
        m_runtimeCustomTrack->deleteLater();
        m_runtimeCustomTrack = nullptr;
    }
    m_runtimeCustomTrack = cloneCustomTrackSnapshot(track, this);
    dumpCustomTrackLayoutForDebug(m_runtimeCustomTrack, QStringLiteral("Runtime Play snapshot"));

    startCustomTrackSession(m_runtimeCustomTrack);
}

void MainWindow::saveCurrentCustomTrack()
{
    TrackData* track = m_customTrackEditor ? m_customTrackEditor->trackData() : nullptr;
    if (!track) {
        return;
    }

    const QString filePath = QFileDialog::getSaveFileName(
        this,
        QStringLiteral("Save Custom Track"),
        QString(),
        QStringLiteral("PhantomDrive Track (*.pdtrack)"));
    if (filePath.isEmpty()) {
        return;
    }

    QString finalPath = filePath;
    if (QFileInfo(finalPath).suffix().isEmpty()) {
        finalPath += QStringLiteral(".pdtrack");
    }

    TrackIO io;
    if (!io.saveTrack(track, finalPath)) {
        QMessageBox::warning(this,
                             QStringLiteral("Track Save Failed"),
                             io.getLastError());
        return;
    }

    statusBar()->showMessage(QStringLiteral("Saved custom track: %1").arg(finalPath), 5000);
}

void MainWindow::loadCustomTrackIntoEditor()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("Load Custom Track Into Editor"),
        QString(),
        QStringLiteral("PhantomDrive Track (*.pdtrack *.json)"));
    if (filePath.isEmpty()) {
        return;
    }

    TrackIO io;
    TrackData* track = io.loadTrack(filePath);
    if (!track) {
        QMessageBox::warning(this,
                             QStringLiteral("Track Load Failed"),
                             io.getLastError());
        return;
    }

    track->setMaxLaps(1);
    if (m_customTrackEditor) {
        m_customTrackEditor->setTrackData(track);
    }
    if (m_customTrackMode) {
        m_customTrackMode->setTrack(track);
    }
    statusBar()->showMessage(QStringLiteral("Loaded custom track into editor: %1").arg(filePath), 5000);
}

void MainWindow::exportCurrentCustomTrackJson()
{
    TrackData* track = m_customTrackEditor ? m_customTrackEditor->trackData() : nullptr;
    if (!track) {
        return;
    }

    const QString filePath = QFileDialog::getSaveFileName(
        this,
        QStringLiteral("Export Custom Track JSON"),
        QString(),
        QStringLiteral("JSON (*.json)"));
    if (filePath.isEmpty()) {
        return;
    }

    QString finalPath = filePath;
    if (QFileInfo(finalPath).suffix().isEmpty()) {
        finalPath += QStringLiteral(".json");
    }

    TrackIO io;
    if (!io.saveTrack(track, finalPath)) {
        QMessageBox::warning(this,
                             QStringLiteral("JSON Export Failed"),
                             io.getLastError());
        return;
    }

    statusBar()->showMessage(QStringLiteral("Exported custom track JSON: %1").arg(finalPath), 5000);
}

void MainWindow::restoreDefaultRaceTrack()
{
    if (!m_defaultRaceTrack || !m_gameView) {
        return;
    }

    m_gameView->setTrackData(m_defaultRaceTrack);
    TrackManager* trackMgr = TrackManager::instance(this);
    if (trackMgr) {
        trackMgr->setCurrentTrack(m_defaultRaceTrack);
        QList<QVector2D> waypoints;
        const QString trackId = m_defaultRaceTrack->getId();
        if (BuiltInTrackFactory::isBuiltInTrackId(trackId)) {
            waypoints = BuiltInTrackFactory::getAIDrivingWaypoints(trackId);
        }
        if (waypoints.isEmpty()) {
            trackMgr->rebuildWaypoints();
            waypoints = trackMgr->getWaypoints();
        }
        if (waypoints.isEmpty()) {
            for (Checkpoint* cp : m_defaultRaceTrack->getCheckpointsInOrder()) {
                if (cp) {
                    waypoints.append(cp->getPosition());
                }
            }
            waypoints.append(m_defaultRaceTrack->getStartPosition());
        }
        if (!waypoints.isEmpty()) {
            trackMgr->setWaypoints(waypoints);
        }
    }
}

void MainWindow::focusGameViewForDriving()
{
    if (!m_gameView) {
        return;
    }

    m_gameView->show();
    m_gameView->raise();
    m_gameView->activateWindow();
    m_gameView->setFocus(Qt::OtherFocusReason);

    if (!m_twoPlayerMode
        && (m_currentMode == QStringLiteral("Learning")
            || m_currentMode == QStringLiteral("Custom Track"))) {
        m_gameView->setCameraZoom(0.92);
        m_gameView->setCameraPosition(m_playerPosition);
    }
}

int MainWindow::selectedPlayerCount() const
{
    return m_playerCountCombo ? m_playerCountCombo->currentData().toInt() : 1;
}

bool MainWindow::isTwoPlayerSelected() const
{
    return selectedPlayerCount() == 2;
}

QString MainWindow::selectedAIDifficulty() const
{
    if (m_currentMode == QStringLiteral("Custom Track") || m_customTrackPlaying) {
        return m_selectedAIDifficulty.isEmpty() ? QStringLiteral("medium") : m_selectedAIDifficulty;
    }
    if (m_aiDifficultyCombo) {
        const QString value = m_aiDifficultyCombo->currentData().toString();
        if (!value.isEmpty()) {
            return value;
        }
    }
    return m_selectedAIDifficulty.isEmpty() ? QStringLiteral("medium") : m_selectedAIDifficulty;
}

void MainWindow::updateArcadeSetupTrackInfo()
{
    const QString trackId = m_trackSelectCombo
        ? m_trackSelectCombo->currentData().toString()
        : m_selectedTrackId;

    BuiltInTrackInfo selectedInfo;
    bool found = false;
    for (const BuiltInTrackInfo& info : BuiltInTrackFactory::tracks()) {
        if (info.id == trackId) {
            selectedInfo = info;
            found = true;
            break;
        }
    }
    if (!found) {
        return;
    }

    if (m_trackDifficultyLabel) {
        m_trackDifficultyLabel->setText(
            QStringLiteral("%1 | %2").arg(selectedInfo.name, selectedInfo.difficulty));
    }
    if (m_trackDescriptionLabel) {
        m_trackDescriptionLabel->setText(selectedInfo.description);
    }
}

void MainWindow::preparePlayerReportSystems()
{
    QStringList contextParts;
    if (m_twoPlayerMode) {
        contextParts << QStringLiteral("Two-Player");
    }
    if (m_customTrackPlaying || m_currentMode == QStringLiteral("Custom Track")) {
        contextParts << QStringLiteral("Custom Track");
    }
    if (m_currentMode == QStringLiteral("Learning")) {
        contextParts << QStringLiteral("Learning traffic rules");
    }
    if (m_selectedAIDifficulty == QStringLiteral("adaptive")) {
        contextParts << QStringLiteral("Adaptive AI");
    }
    const QString baseContext = contextParts.isEmpty()
        ? QStringLiteral("Standard single-player drive")
        : contextParts.join(QStringLiteral(" / "));

    if (m_drivingDataCollector) {
        m_drivingDataCollector->stopCollection();
        m_drivingDataCollector->clearData();
        m_drivingDataCollector->setVehicleId(QStringLiteral("player_1"));
        m_drivingDataCollector->setSamplingInterval(kSimulationStepMs);
        m_drivingDataCollector->setCurrentSpeedLimit(m_currentSpeedLimit, "main_route_speed_zone");
        if (m_drivingDataCollector->vehicleSensor()) {
            m_drivingDataCollector->vehicleSensor()->setSpeedLimitViolationEnabled(false);
        }
        m_drivingDataCollector->startCollection();
    }
    if (m_scoreManager) {
        m_scoreManager->setSessionContext(m_currentMode, m_twoPlayerMode
            ? baseContext + QStringLiteral(" / Player 1")
            : baseContext);
        m_scoreManager->startSession(QStringLiteral("player_1"));
    }

    if (!m_twoPlayerMode) {
        if (m_player2DataCollector) {
            m_player2DataCollector->stopCollection();
            m_player2DataCollector->clearData();
        }
        return;
    }

    if (m_player2DataCollector) {
        m_player2DataCollector->stopCollection();
        m_player2DataCollector->clearData();
        m_player2DataCollector->setVehicleId(QStringLiteral("player_2"));
        m_player2DataCollector->setSamplingInterval(kSimulationStepMs);
        m_player2DataCollector->setCurrentSpeedLimit(m_currentSpeedLimit, "main_route_speed_zone");
        if (m_player2DataCollector->vehicleSensor()) {
            m_player2DataCollector->vehicleSensor()->setSpeedLimitViolationEnabled(false);
        }
        m_player2DataCollector->startCollection();
    }
    if (m_player2ScoreManager) {
        m_player2ScoreManager->setSessionContext(m_currentMode,
                                                 baseContext + QStringLiteral(" / Player 2"));
        m_player2ScoreManager->startSession(QStringLiteral("player_2"));
    }
}

void MainWindow::applyPlayer2SpawnAtStartLine()
{
    if (!m_player2Physics) {
        return;
    }

    TrackData* track = m_gameView ? m_gameView->trackData() : nullptr;
    const QVector2D base = m_playerPosition;
    const qreal radians = qDegreesToRadians(m_playerRotation);
    const QVector2D right(qCos(radians), -qSin(radians));
    const QVector2D back(-qSin(radians), -qCos(radians));
    const QVector2D candidates[] = {
        base + right * 52.0,
        base - right * 52.0,
        base + back * 64.0,
        base - back * 64.0
    };

    QVector2D spawn = base + right * 52.0;
    if (track && track->getStartPositions().size() > 1) {
        spawn = track->getStartPositions().at(1);
    } else {
        for (const QVector2D& candidate : candidates) {
            if (m_player2Physics->isPositionFree(candidate)) {
                spawn = candidate;
                break;
            }
        }
    }

    m_player2Physics->setPosition(spawn);
    m_player2Physics->setRotation(m_playerRotation);
    m_player2Physics->clearDriveInput();
    m_player2Position = spawn;
    m_previousPlayer2Position = spawn;
    m_player2Rotation = m_playerRotation;
    m_player2Speed = 0.0;
    m_player2LapsCompleted = 0;
    m_player2NextCheckpointIndex = 0;
    m_player2WasInsideNextGate = false;
}

void MainWindow::updateTwoPlayerCamera()
{
    if (!m_gameView || !m_twoPlayerMode) {
        return;
    }
    const QVector2D midpoint = (m_playerPosition + m_player2Position) * 0.5;
    const qreal distance = (m_playerPosition - m_player2Position).length();
    const qreal targetZoom = qBound<qreal>(0.55, 1.1 - distance / 1600.0, 1.15);
    const qreal currentZoom = m_gameView->getRenderState().cameraZoom;
    const qreal zoom = qAbs(currentZoom - targetZoom) < 0.01
        ? currentZoom
        : currentZoom + (targetZoom - currentZoom) * 0.18;
    m_gameView->setCameraPosition(midpoint);
    m_gameView->setCameraZoom(zoom);
}

void MainWindow::showGuideDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("PhantomDrive Guide"));
    dialog.resize(760, 620);
    dialog.setStyleSheet(QStringLiteral(
        "QDialog{background:#06101F;color:#EAFBFF;}"
        "QTextEdit{background:#0B1830;color:#EAFBFF;border:1px solid #29E6FF;"
        "font-size:14px;padding:14px;selection-background-color:#FF2D75;}"));

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    auto* text = new QTextEdit(&dialog);
    text->setReadOnly(true);
    text->setHtml(QStringLiteral(
        "<h1 style='color:#4DFBFF'>PHANTOMDRIVE GUIDE</h1>"
        "<h2>Controls</h2>"
        "<p><b>P1</b>: W accelerate, S brake/reverse, A/D steer.</p>"
        "<p><b>P2</b>: Arrow Up accelerate, Down brake/reverse, Left/Right steer.</p>"
        "<h2>Race Goal</h2>"
        "<p>Pass checkpoints in order, then cross the start/finish gate to complete the route.</p>"
        "<h2>Powerups</h2>"
        "<ul>"
        "<li><b>Boost</b>: short speed burst.</li>"
        "<li><b>Shield</b>: reduces collision penalty and protects momentum.</li>"
        "<li><b>Missile</b>: launches at the best target ahead.</li>"
        "<li><b>Oil Slick</b>: drops a slipping hazard behind the car.</li>"
        "<li><b>EMP</b>: slows AI opponents in range.</li>"
        "<li><b>Invisibility</b>: temporary no-collision phase.</li>"
        "<li><b>Repair</b>: restores stability and speed.</li>"
        "<li><b>Teleport</b>: jumps forward along the current direction.</li>"
        "<li><b>Magnet</b>: increases nearby item collection radius.</li>"
        "<li><b>Random</b>: rolls one of the active powerups.</li>"
        "</ul>"));
    layout->addWidget(text);

    QPushButton* closeButton = new QPushButton(QStringLiteral("Close"), &dialog);
    closeButton->setMinimumHeight(44);
    closeButton->setStyleSheet(QStringLiteral(
        "QPushButton{background:#10264A;color:#EAFBFF;border:1px solid #29E6FF;"
        "border-radius:7px;font-weight:bold;} QPushButton:hover{background:#173B6C;}"));
    layout->addWidget(closeButton);
    connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    dialog.exec();
}

void MainWindow::loadPlayerProfile()
{
    if (!m_playerProfileStore || !m_currencyManager || !m_skinManager) {
        return;
    }

    const PlayerProfile profile = m_playerProfileStore->loadProfile();
    m_currencyManager->setCoins(profile.coins);
    m_skinManager->setUnlockedSkinIds(profile.ownedSkins);
    m_skinManager->setCurrentSkinId(profile.currentSkin);
}

bool MainWindow::savePlayerProfile()
{
    if (!m_playerProfileStore || !m_currencyManager || !m_skinManager) {
        return false;
    }

    PlayerProfile profile;
    profile.coins = m_currencyManager->coins();
    profile.ownedSkins = m_skinManager->unlockedSkinIds();
    profile.currentSkin = m_skinManager->currentSkinId();
    const bool saved = m_playerProfileStore->saveProfile(profile);
    if (!saved) {
        statusBar()->showMessage(QStringLiteral("Failed to save player profile"), 4000);
    }
    return saved;
}

void MainWindow::refreshGaragePage()
{
    if (!m_garageWidget || !m_currencyManager || !m_skinManager) {
        return;
    }

    m_garageWidget->setGarageState(m_skinManager->skins(),
                                   m_skinManager->currentSkinId(),
                                   m_currencyManager->coins());
}

void MainWindow::handleGaragePurchase(const QString& skinId)
{
    if (!m_skinManager || !m_currencyManager) {
        return;
    }

    if (!m_skinManager->purchaseSkin(skinId, *m_currencyManager)) {
        statusBar()->showMessage(QStringLiteral("Not enough coins or skin unavailable"), 3000);
        refreshGaragePage();
        return;
    }

    savePlayerProfile();
    refreshGaragePage();
    statusBar()->showMessage(QStringLiteral("Purchased skin: %1").arg(skinId), 3000);
}

void MainWindow::handleGarageSelect(const QString& skinId)
{
    if (!m_skinManager) {
        return;
    }

    if (!m_skinManager->selectSkin(skinId)) {
        refreshGaragePage();
        return;
    }

    savePlayerProfile();
    refreshGaragePage();
    statusBar()->showMessage(QStringLiteral("Selected skin: %1").arg(skinId), 3000);
}

void MainWindow::setupGameView()
{
    QWidget* gamePage = ui->stackedWidget && ui->stackedWidget->count() > 1
        ? ui->stackedWidget->widget(1)
        : nullptr;

    m_gameView = new GameViewWidget(gamePage ? gamePage : this);
    m_gameView->hide();

    if (gamePage) {
        QWidget* page = gamePage;
        if (page) {
            QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(page->layout());
            if (!layout) {
                layout = new QVBoxLayout(page);
                layout->setContentsMargins(0, 32, 0, 0);
                layout->setSpacing(0);
                page->setLayout(layout);
            }
            QHBoxLayout* playLayout = new QHBoxLayout();
            playLayout->setContentsMargins(0, 0, 0, 0);
            playLayout->setSpacing(8);
            playLayout->addWidget(m_gameView, 1);

            if (m_arcadeHUD) {
                // Fixed-width right panel — always 300px, no stretch.
                playLayout->addWidget(m_arcadeHUD, 0, Qt::AlignTop);
            }

            layout->addLayout(playLayout, 1);

            page->setFocusPolicy(Qt::StrongFocus);
            page->setFocus();

            if (!m_coinChallengeHudLabel) {
                m_coinChallengeHudLabel = new QLabel(page);
                m_coinChallengeHudLabel->setObjectName(QStringLiteral("coinChallengeHudLabel"));
                m_coinChallengeHudLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
                m_coinChallengeHudLabel->setTextFormat(Qt::RichText);
                m_coinChallengeHudLabel->setWordWrap(true);
                m_coinChallengeHudLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
                m_coinChallengeHudLabel->setStyleSheet(QStringLiteral(
                    "QLabel#coinChallengeHudLabel{"
                    "background:rgba(5,12,24,228);"
                    "border:2px solid rgba(245,196,81,220);"
                    "border-radius:20px;"
                    "padding:0px;"
                    "color:#FFF6D1;"
                    "}"));
                m_coinChallengeHudLabel->hide();
            }
            layoutCoinChallengeHud();
        }
    }

    TrackManager* trackMgr = TrackManager::instance(this);
    if (trackMgr && trackMgr->hasCurrentTrack()) {
        TrackData* loadedTrack = trackMgr->getCurrentTrack();
        if (!loadedTrack->getCheckpointsInOrder().isEmpty()) {
            m_defaultRaceTrack = loadedTrack;
            m_gameView->setTrackData(loadedTrack);
            if (loadedTrack->getStartPosition() == QVector2D(0, 0)) {
                const qreal tileSize = 64.0;
                const QVector2D fallback(15 * tileSize + tileSize / 2.0, 3 * tileSize + tileSize / 2.0);
                loadedTrack->setStartPosition(fallback);
                loadedTrack->setStartRotation(0.0);
            }
            syncRaceTrackToManager();
            setupEBRuntimeObjects();
            return;
        }
    }

    TrackData* testTrack = new TrackData();
    m_defaultRaceTrack = testTrack;
    testTrack->setId("main_city_training_route");
    testTrack->setName("Main City Training Route");
    testTrack->setSize(30, 30);

    const int trackCenterRow = 15;
    const int trackCenterCol = 15;
    const int trackOuterRadius = 12;
    const int trackInnerRadius = 8;
    const int startLineRow = trackCenterRow - trackOuterRadius;
    const int finishLineRow = startLineRow + 1;

    for (int row = 0; row < 30; ++row) {
        for (int col = 0; col < 30; ++col) {
            TrackTile* tile = new TrackTile();
            tile->setPosition(row, col);

            double distFromCenter = std::sqrt(
                std::pow(row - trackCenterRow, 2) + std::pow(col - trackCenterCol, 2)
            );

            const bool isOnTrack = (distFromCenter >= trackInnerRadius && distFromCenter <= trackOuterRadius);
            const bool isOuterWall = (distFromCenter > trackOuterRadius && distFromCenter <= trackOuterRadius + 1);
            const bool isInnerWall = (distFromCenter < trackInnerRadius && distFromCenter >= trackInnerRadius - 1);

            // Keep spawn on StartLine; draw FinishLine one tile lower as a separate visual marker.
            const bool isStartLine = (row == startLineRow
                                      && col >= trackCenterCol - 2
                                      && col <= trackCenterCol + 2);
            const bool isFinishLine = (row == finishLineRow
                                       && col >= trackCenterCol - 2
                                       && col <= trackCenterCol + 2);

            if (isStartLine) {
                tile->setType(TileType::StartLine);
            } else if (isFinishLine) {
                tile->setType(TileType::FinishLine);
            } else if (isOnTrack) {
                tile->setType(TileType::Road);
            } else if (isOuterWall || isInnerWall) {
                tile->setType(TileType::Wall);
            } else {
                tile->setType(TileType::Grass);
            }

            testTrack->setTileAt(row, col, tile);
        }
    }

    const qreal tileSize = 64.0;
    const int startRow = startLineRow;
    const int startCol = trackCenterCol;
    const QVector2D startPos(startCol * tileSize + tileSize / 2.0,
                             startRow * tileSize + tileSize / 2.0);
    testTrack->setStartPosition(startPos);
    // rotation=0：沿 +Y 驶入环道（与当前 VehiclePhysics 前进方向一致）
    testTrack->setStartRotation(0.0);
    testTrack->setMaxLaps(m_totalLaps > 0 ? m_totalLaps : 3);

    const qreal gateThickness = tileSize * 0.75;
    const qreal gateSpan = tileSize * 5.0;
    const int band = trackOuterRadius - trackInnerRadius + 1;

    // 北/南弧段：行驶方向大致沿 X，检查门沿 Y 横穿；西/东直道：沿 Y 行驶，门沿 X 横穿
    const GateSpec northGate = computeGateAcrossTrack(
        testTrack,
        trackCenterRow - trackOuterRadius,
        trackCenterRow - trackInnerRadius,
        trackCenterCol - band,
        trackCenterCol + band,
        tileSize,
        gateThickness,
        false);
    const GateSpec eastGate = computeGateAcrossTrack(
        testTrack,
        trackCenterRow - band,
        trackCenterRow + band,
        trackCenterCol + trackInnerRadius,
        trackCenterCol + trackOuterRadius,
        tileSize,
        gateThickness,
        true);
    const GateSpec southGate = computeGateAcrossTrack(
        testTrack,
        trackCenterRow + trackInnerRadius,
        trackCenterRow + trackOuterRadius,
        trackCenterCol - band,
        trackCenterCol + band,
        tileSize,
        gateThickness,
        false);
    const GateSpec westGate = computeGateAcrossTrack(
        testTrack,
        trackCenterRow - band,
        trackCenterRow + band,
        trackCenterCol - trackOuterRadius,
        trackCenterCol - trackInnerRadius,
        tileSize,
        gateThickness,
        true);

    const GateSpec gates[] = {northGate, eastGate, southGate, westGate};
    QList<QVector2D> checkpointPositions;
    for (int i = 0; i < 4; ++i) {
        if (!gates[i].valid) {
            continue;
        }
        qreal gateW = gates[i].width;
        qreal gateH = gates[i].height;
        const qreal minSpan = gateSpan;
        const qreal minThickness = gateThickness * 2.5;
        // 所有门加宽薄边，避免高速/贴边漏检
        if (gateW >= gateH) {
            gateW = qMax(gateW, minSpan);
            gateH = qMax(gateH, minThickness);
        } else {
            gateW = qMax(gateW, minThickness);
            gateH = qMax(gateH, minSpan);
        }
        addGateCheckpoint(testTrack, i, i, gates[i].center, gateW, gateH);
        checkpointPositions.append(gates[i].center);
    }

    if (trackMgr) {
        QList<QVector2D> waypoints = checkpointPositions;
        waypoints.append(startPos);
        trackMgr->setWaypoints(waypoints);
    }

    m_gameView->setTrackData(testTrack);
    if (trackMgr) {
        trackMgr->setCurrentTrack(testTrack);
    }
    syncRaceTrackToManager();

    setupEBRuntimeObjects();
}

QString MainWindow::powerupTypeToString(PhantomDrive::PowerupType type) const
{
    switch (type) {
    case PhantomDrive::PowerupType::Boost:
        return QStringLiteral("Boost");
    case PhantomDrive::PowerupType::Shield:
        return QStringLiteral("Shield");
    case PhantomDrive::PowerupType::Missile:
        return QStringLiteral("Missile");
    case PhantomDrive::PowerupType::OilSlick:
        return QStringLiteral("Oil Slick");
    case PhantomDrive::PowerupType::EMP:
        return QStringLiteral("EMP");
    case PhantomDrive::PowerupType::Invisibility:
        return QStringLiteral("Invisibility");
    case PhantomDrive::PowerupType::Repair:
        return QStringLiteral("Repair");
    case PhantomDrive::PowerupType::Teleport:
        return QStringLiteral("Teleport");
    case PhantomDrive::PowerupType::Magnet:
        return QStringLiteral("Magnet");
    case PhantomDrive::PowerupType::Custom:
        return QStringLiteral("Custom");
    default:
        return QStringLiteral("Powerup");
    }
}

void MainWindow::clearEBRuntimeObjects()
{
    qDeleteAll(m_powerupBoxes);
    m_powerupBoxes.clear();

    if (m_trafficObjectManager) {
        m_trafficObjectManager->clear();
    }

    if (m_gameView) {
        m_gameView->clearScenarioObjects();
        m_gameView->setPlayerEffectState(false, false, false, false);
        m_gameView->setWorldEffects({}, {}, {});
    }

    if (m_powerupWorld) {
        m_powerupWorld->clear();
    }

    // ArcadeHUD clears its powerup slots.
    if (m_arcadeHUD) {
        m_arcadeHUD->clearPowerupState();
    }

    m_currentSpeedLimit = 60;
    m_currentTrafficLightState = QStringLiteral("green");
}

void MainWindow::setupEBRuntimeObjects()
{
    clearEBRuntimeObjects();

    if (!m_gameView || !m_gameView->trackData() || !m_trafficObjectManager) {
        return;
    }

    TrackData* track = m_gameView->trackData();
    const QList<Checkpoint*> checkpoints = track->getCheckpointsInOrder();
    auto checkpointPosition = [&](int index, const QVector2D& fallback) {
        if (index >= 0 && index < checkpoints.size() && checkpoints.at(index)) {
            return checkpoints.at(index)->getPosition();
        }
        return fallback;
    };

    const QVector2D start = track->getStartPosition();
    const QVector2D north = checkpointPosition(0, start + QVector2D(0.0f, 120.0f));
    const QVector2D east = checkpointPosition(1, start + QVector2D(520.0f, 520.0f));
    const QVector2D south = checkpointPosition(2, start + QVector2D(0.0f, 1040.0f));
    const QVector2D west = checkpointPosition(3, start + QVector2D(-520.0f, 520.0f));

    auto addPowerupBox = [this](const QString& id, const QVector2D& position, PowerupType type) {
        auto* box = new PowerupBox(position, 54.0f, this);
        box->setObjectName(id);
        box->setFixedPowerupType(type);
        box->setRespawnTime(8.0f);
        m_powerupBoxes.append(box);

        const QString typeName = powerupTypeToString(type);
        if (m_gameView) {
            m_gameView->addPowerupBox(id, position, typeName);
        }

        connect(box, &PowerupBox::collected,
                this, [this, id](const QString& playerId, const PowerupType& collectedType) {
                    if (m_gameView) {
                        m_gameView->removePowerupBox(id);
                    }
                    handlePowerupCollectedForPlayer(collectedType, playerId == QStringLiteral("player_2") ? 2 : 1);
                });
        connect(box, &PowerupBox::respawned,
                this, [this, id, position, typeName]() {
                    if (m_gameView) {
                        m_gameView->addPowerupBox(id, position, typeName);
                    }
                });
    };

    addPowerupBox(QStringLiteral("eb_boost_box"), north + QVector2D(0.0f, 120.0f), PowerupType::Boost);
    addPowerupBox(QStringLiteral("eb_shield_box"), east, PowerupType::Shield);
    addPowerupBox(QStringLiteral("eb_emp_box"), west, PowerupType::EMP);
    addPowerupBox(QStringLiteral("eb_repair_box"), south + QVector2D(140.0f, 0.0f), PowerupType::Repair);
    addPowerupBox(QStringLiteral("eb_missile_box"), north + QVector2D(180.0f, 0.0f), PowerupType::Missile);
    addPowerupBox(QStringLiteral("eb_oil_box"), south + QVector2D(-160.0f, 0.0f), PowerupType::OilSlick);
    addPowerupBox(QStringLiteral("eb_invis_box"), east + QVector2D(0.0f, 180.0f), PowerupType::Invisibility);
    addPowerupBox(QStringLiteral("eb_teleport_box"), west + QVector2D(0.0f, -180.0f), PowerupType::Teleport);
    addPowerupBox(QStringLiteral("eb_magnet_box"), start + QVector2D(260.0f, 260.0f), PowerupType::Magnet);
    addPowerupBox(QStringLiteral("eb_custom_box"), south + QVector2D(0.0f, -120.0f), PowerupType::Custom);

    auto* light = new TrafficLightObject(QStringLiteral("eb_light_1"), m_trafficObjectManager);
    light->setPosition(north + QVector2D(0.0f, 190.0f));
    light->setBounds(QRectF(light->getPosition().x() - 95.0,
                            light->getPosition().y() - 95.0,
                            190.0,
                            190.0));
    light->setCurrentColor(TrafficLightObject::LightColor::Red);
    light->setRedDurationMs(7000);
    light->setGreenDurationMs(6000);
    light->setYellowDurationMs(2000);
    m_trafficObjectManager->registerTrafficObject(light);
    light->start();
    m_currentTrafficLightState = QStringLiteral("red");
    m_gameView->addTrafficLight(light->getId(), light->getPosition(), m_currentTrafficLightState);
    connect(light, &TrafficLightObject::colorChanged,
            this, [this, light](TrafficLightObject::LightColor, TrafficLightObject::LightColor color) {
                m_currentTrafficLightState = lightColorToString(color);
                if (m_gameView) {
                    m_gameView->updateTrafficLight(light->getId(), m_currentTrafficLightState);
                }
            });

    auto* sign = new SpeedLimitSignObject(QStringLiteral("eb_speed_zone_1"), m_trafficObjectManager);
    sign->setPosition(east + QVector2D(-70.0f, 0.0f));
    sign->setSpeedLimit(50.0);
    sign->setDetectionRadius(180.0);
    sign->setZoneId(QStringLiteral("eb_east_speed_zone"));
    m_trafficObjectManager->registerTrafficObject(sign);
    m_gameView->addSpeedLimitSign(sign->getId(), sign->getPosition(), static_cast<int>(sign->getSpeedLimit()));

    auto* crossing = new PedestrianCrossingObject(QStringLiteral("eb_crossing_1"), m_trafficObjectManager);
    const QSizeF crossingSize(240.0, 120.0);
    crossing->setPosition(south);
    crossing->setBounds(QRectF(south.x() - crossingSize.width() / 2.0,
                               south.y() - crossingSize.height() / 2.0,
                               crossingSize.width(),
                               crossingSize.height()));
    crossing->spawnPedestrian();
    m_trafficObjectManager->registerTrafficObject(crossing);
    m_gameView->addPedestrianCrossing(crossing->getId(), south, crossingSize);
}

void MainWindow::setupVehiclePhysics()
{
    m_vehiclePhysics = new VehiclePhysics(this);
    m_player2Physics = new VehiclePhysics(this);
    m_vehiclePhysics->setControlScheme(VehiclePhysics::ControlScheme::Wasd);
    m_player2Physics->setControlScheme(VehiclePhysics::ControlScheme::Arrows);

    TrackManager* trackMgr = TrackManager::instance(this);
    if (trackMgr) {
        m_vehiclePhysics->initialize(trackMgr);
        m_player2Physics->initialize(trackMgr);
    }

    if (m_gameView) {
        connect(m_gameView, &GameViewWidget::keyInputReceived,
                this, [this](QKeyEvent* event) {
                    if (isCoinChallengeBalloonRushSceneActive()) {
                        if (event && (event->key() == Qt::Key_A || event->key() == Qt::Key_Left)) {
                            moveCoinChallengeBalloonRushLane(-1);
                        } else if (event && (event->key() == Qt::Key_D || event->key() == Qt::Key_Right)) {
                            moveCoinChallengeBalloonRushLane(1);
                        }
                        return;
                    }
                    if (!m_gamePaused && !m_countdownActive && m_vehiclePhysics) {
                        m_vehiclePhysics->handleKeyPress(event);
                    }
                });
        connect(m_gameView, &GameViewWidget::keyReleased,
                this, [this](QKeyEvent* event) {
                    if (isCoinChallengeBalloonRushSceneActive()) {
                        Q_UNUSED(event);
                        return;
                    }
                    if (m_vehiclePhysics) {
                        m_vehiclePhysics->handleKeyRelease(event);
                    }
                });
        connect(m_gameView, &GameViewWidget::keyInputReceived,
                this, [this](QKeyEvent* event) {
                    if (isCoinChallengeBalloonRushSceneActive()) {
                        return;
                    }
                    if (!m_gamePaused && !m_countdownActive && m_player2Physics) {
                        m_player2Physics->handleKeyPress(event);
                    }
                });
        connect(m_gameView, &GameViewWidget::keyReleased,
                this, [this](QKeyEvent* event) {
                    if (isCoinChallengeBalloonRushSceneActive()) {
                        Q_UNUSED(event);
                        return;
                    }
                    if (m_player2Physics) {
                        m_player2Physics->handleKeyRelease(event);
                    }
                });
    } else {
        qWarning() << "MainWindow: game view is null; keyboard input connections skipped";
    }

    connect(m_vehiclePhysics, &VehiclePhysics::positionUpdated,
            this, [this](const QVector2D& position) {
                m_playerPosition = position;
                m_playerRotation = m_vehiclePhysics->getRotation();
                m_playerSpeed = m_vehiclePhysics->getSpeed();

                if (m_gameView && m_driveActive) {
                    m_gameView->setPlayerEffectState(m_vehiclePhysics->isSpeedBoostActive(),
                                                     m_vehiclePhysics->isShieldActive(),
                                                     m_vehiclePhysics->isInvisible(),
                                                     m_vehiclePhysics->isMagnetActive());
                    if (m_twoPlayerMode) {
                        m_gameView->updatePlayerCar(QStringLiteral("P1"),
                                                    m_playerPosition,
                                                    m_playerRotation,
                                                    displaySpeedKmh(),
                                                    currentPlayerSkinColor(),
                                                    m_vehiclePhysics->isSpeedBoostActive(),
                                                    m_vehiclePhysics->isShieldActive(),
                                                    m_vehiclePhysics->isInvisible(),
                                                    m_vehiclePhysics->isMagnetActive(),
                                                    currentPlayerSkinId());
                        updateTwoPlayerCamera();
                    } else {
                        m_gameView->updatePlayerCar(QStringLiteral("P1"),
                                                    m_playerPosition,
                                                    m_playerRotation,
                                                    displaySpeedKmh(),
                                                    currentPlayerSkinColor(),
                                                    m_vehiclePhysics->isSpeedBoostActive(),
                                                    m_vehiclePhysics->isShieldActive(),
                                                    m_vehiclePhysics->isInvisible(),
                                                    m_vehiclePhysics->isMagnetActive(),
                                                    currentPlayerSkinId());
                        m_gameView->setCameraPosition(m_playerPosition);
                    }
                }

                if (m_drivingDataCollector && m_drivingDataCollector->vehicleSensor()) {
                    VehicleSensor* sensor = m_drivingDataCollector->vehicleSensor();
                    sensor->updatePosition(m_playerPosition);
                    sensor->updateRotation(m_playerRotation);
                    qreal radians = qDegreesToRadians(m_playerRotation);
                    const qreal displaySpeed = displaySpeedKmh();
                    sensor->updateVelocity(QVector2D(qCos(radians) * displaySpeed,
                                                     qSin(radians) * displaySpeed));
                    sensor->updateSpeedLimit(m_currentSpeedLimit, "main_route_speed_zone");
                }
            });

    connect(m_player2Physics, &VehiclePhysics::positionUpdated,
            this, [this](const QVector2D& position) {
                m_player2Position = position;
                m_player2Rotation = m_player2Physics->getRotation();
                m_player2Speed = m_player2Physics->getSpeed();

                if (m_gameView && m_driveActive && m_twoPlayerMode) {
                    m_gameView->updatePlayerCar(QStringLiteral("P2"),
                                                m_player2Position,
                                                m_player2Rotation,
                                                speedToDisplayKmh(m_player2Speed),
                                                QColor(40, 220, 255));
                    updateTwoPlayerCamera();
                }

                if (m_player2DataCollector && m_player2DataCollector->vehicleSensor()) {
                    VehicleSensor* sensor = m_player2DataCollector->vehicleSensor();
                    sensor->updatePosition(m_player2Position);
                    sensor->updateRotation(m_player2Rotation);
                    qreal radians = qDegreesToRadians(m_player2Rotation);
                    const qreal displaySpeed = speedToDisplayKmh(m_player2Speed);
                    sensor->updateVelocity(QVector2D(qCos(radians) * displaySpeed,
                                                     qSin(radians) * displaySpeed));
                    sensor->updateSpeedLimit(m_currentSpeedLimit, "main_route_speed_zone");
                }
            });

    connect(m_vehiclePhysics, &VehiclePhysics::collisionOccurred,
            this, [this](const QString& objectType, const QVector2D& position, qreal impactForce) {
                if (m_gameView
                    && !shouldSuppressStartupCollisionImpact(m_driveActive,
                                                             m_countdownActive,
                                                             m_sessionElapsedMs)) {
                    m_gameView->showCollisionImpact(position, qBound<qreal>(0.5, impactForce / 90.0, 1.8));
                }
                onCollision();
                if (m_scoreManager) {
                    m_scoreManager->recordCollision(position.toPointF(), displaySpeedKmh(), objectType);
                }
            });

    connect(m_player2Physics, &VehiclePhysics::collisionOccurred,
            this, [this](const QString& objectType, const QVector2D& position, qreal impactForce) {
                if (!m_twoPlayerMode) {
                    return;
                }
                if (m_gameView
                    && !shouldSuppressStartupCollisionImpact(m_driveActive,
                                                             m_countdownActive,
                                                             m_sessionElapsedMs)) {
                    m_gameView->showCollisionImpact(position, qBound<qreal>(0.5, impactForce / 90.0, 1.8));
                }
                showInteractiveFeedback(QStringLiteral("P2 Wall Hit!"), PhantomDrive::FeedbackType::Critical);
                playSound(PhantomDrive::SoundEffect::Crash);
                if (m_player2ScoreManager) {
                    m_player2ScoreManager->recordCollision(position.toPointF(),
                                                           speedToDisplayKmh(m_player2Speed),
                                                           objectType);
                }
            });

    connect(m_vehiclePhysics, &VehiclePhysics::invisibilityRecoveryApplied,
            this, [this](const QVector2D& position) {
                m_playerPosition = position;
                if (m_gameView && m_driveActive) {
                    if (m_twoPlayerMode) {
                        m_gameView->updatePlayerCar(QStringLiteral("P1"),
                                                    m_playerPosition,
                                                    m_vehiclePhysics->getRotation(),
                                                    displaySpeedKmh(),
                                                    currentPlayerSkinColor(),
                                                    m_vehiclePhysics->isSpeedBoostActive(),
                                                    m_vehiclePhysics->isShieldActive(),
                                                    m_vehiclePhysics->isInvisible(),
                                                    m_vehiclePhysics->isMagnetActive(),
                                                    currentPlayerSkinId());
                        updateTwoPlayerCamera();
                    } else {
                        m_gameView->updatePlayerCar(QStringLiteral("P1"),
                                                    m_playerPosition,
                                                    m_vehiclePhysics->getRotation(),
                                                    displaySpeedKmh(),
                                                    currentPlayerSkinColor(),
                                                    m_vehiclePhysics->isSpeedBoostActive(),
                                                    m_vehiclePhysics->isShieldActive(),
                                                    m_vehiclePhysics->isInvisible(),
                                                    m_vehiclePhysics->isMagnetActive(),
                                                    currentPlayerSkinId());
                        m_gameView->setCameraPosition(m_playerPosition);
                    }
                }
                statusBar()->showMessage(
                    QStringLiteral("Invisibility ended: returned to the last safe item pickup position."),
                    3500);
            });

}

void MainWindow::initializeAIOpponents()
{
    if (!m_aiManager) {
        return;
    }

    m_aiManager->destroyAllOpponents();
    m_playerVehicleContacts.clear();
    m_aiManager->setRaceTotalLaps(m_totalLaps);
    m_aiManager->setPlayerPosition(m_playerPosition);
    m_aiManager->setPlayerRaceProgress(0, 0, 0.0, false, 0.0);
    m_aiManager->setTrackBounds(QRectF(0.0, 0.0, 1280.0, 1920.0));

    TrackManager* trackMgr = TrackManager::instance(this);
    if (trackMgr) {
        m_aiManager->setTrackManager(trackMgr);
    }
    if (trackMgr && trackMgr->hasCurrentTrack()) {
        m_aiManager->setTrackBounds(trackMgr->getCurrentTrack()->getBounds());
    }

    QList<Waypoint> aiWaypoints;
    QList<QVector2D> trackWaypoints;
    if (trackMgr && trackMgr->hasCurrentTrack()) {
        const QString trackId = trackMgr->getCurrentTrack()->getId();
        if (BuiltInTrackFactory::isBuiltInTrackId(trackId)) {
            trackWaypoints = BuiltInTrackFactory::getAIDrivingWaypoints(trackId);
        }
    }
    if (trackWaypoints.isEmpty() && trackMgr) {
        trackWaypoints = trackMgr->getWaypoints();
    }

    for (int i = 0; i < trackWaypoints.size(); ++i) {
        aiWaypoints.append(Waypoint(trackWaypoints.at(i), 110.0, false, 0, i));
    }

    if (aiWaypoints.isEmpty()) {
        aiWaypoints.append(Waypoint(QVector2D(200, 200), 90, false, 0, 0));
        aiWaypoints.append(Waypoint(QVector2D(800, 200), 115, false, 0, 1));
        aiWaypoints.append(Waypoint(QVector2D(800, 800), 70, true, 2, 2));
        aiWaypoints.append(Waypoint(QVector2D(200, 800), 100, true, 1, 3));
    }

    const QString difficulty = selectedAIDifficulty();

    AIStyle primaryStyle = AIStyle::Normal;
    AIStyle secondaryStyle = AIStyle::Defensive;
    qreal speedScale = 1.0;
    qreal accelerationScale = 1.0;
    if (difficulty == QStringLiteral("easy")) {
        primaryStyle = AIStyle::Conservative;
        secondaryStyle = AIStyle::Normal;
        speedScale = 0.86;
        accelerationScale = 0.9;
    } else if (difficulty == QStringLiteral("hard")) {
        primaryStyle = AIStyle::Aggressive;
        secondaryStyle = AIStyle::Defensive;
        speedScale = 1.14;
        accelerationScale = 1.08;
    } else if (difficulty == QStringLiteral("adaptive")) {
        // Adaptive reuses the existing adaptive demo mapping: Normal + Aggressive
        // styles with a mild speed lift, leaving the AI API/core algorithm intact.
        primaryStyle = AIStyle::Normal;
        secondaryStyle = AIStyle::Aggressive;
        speedScale = 1.06;
        accelerationScale = 1.03;
    }

    AIOpponent* ai1 = m_aiManager->createOpponent("ai_1", primaryStyle);
    AIOpponent* ai2 = m_aiManager->createOpponent("ai_2", secondaryStyle);
    const QList<AIOpponent*> opponents = {ai1, ai2};
    int spawnAnchorIndex = 0;
    if (!aiWaypoints.isEmpty() && trackMgr && trackMgr->hasCurrentTrack()) {
        const QVector2D startPosition = trackMgr->getCurrentTrack()->getStartPosition();
        qreal nearestDistance = std::numeric_limits<qreal>::max();
        for (int i = 0; i < aiWaypoints.size(); ++i) {
            const qreal distance = (aiWaypoints.at(i).position - startPosition).length();
            if (distance < nearestDistance) {
                nearestDistance = distance;
                spawnAnchorIndex = i;
            }
        }
    }
    const int waypointCount = aiWaypoints.size();
    const int spawnStep = waypointCount > 6 ? qMax(1, waypointCount / 18) : 1;

    for (int i = 0; i < opponents.size(); ++i) {
        AIOpponent* ai = opponents.at(i);
        if (!ai) {
            continue;
        }
        AIConfig config = ai->getConfig();
        config.maxSpeed *= speedScale;
        config.acceleration *= accelerationScale;
        ai->setConfig(config);
        ai->setMaxSpeed(config.maxSpeed);
        ai->setWaypoints(aiWaypoints);
        const int spawnIndex = waypointCount > 1
            ? (spawnAnchorIndex + (i * spawnStep)) % waypointCount
            : 0;
        const int targetIndex = waypointCount > 1
            ? (spawnIndex + 1) % waypointCount
            : spawnIndex;
        QVector2D spawnPos = aiWaypoints.at(spawnIndex).position;
        QVector2D spawnDirection(0.0f, 1.0f);
        if (waypointCount > 1) {
            spawnDirection = aiWaypoints.at(targetIndex).position - aiWaypoints.at(spawnIndex).position;
        }
        if (!spawnDirection.isNull()) {
            QVector2D lateral(-spawnDirection.y(), spawnDirection.x());
            if (!lateral.isNull()) {
                lateral.normalize();
                spawnPos += lateral * (i == 0 ? -14.0f : 14.0f);
            }
        }
        ai->setPosition(spawnPos);
        ai->setCurrentWaypointIndex(targetIndex);
        if (!spawnDirection.isNull()) {
            ai->setRotation(qRadiansToDegrees(std::atan2(spawnDirection.x(), spawnDirection.y())));
        } else {
            ai->setRotation(0.0);
        }
        ai->setVelocity(QVector2D(0.0, 0.0));
        ai->setCurrentLap(0);
        ai->setCheckpointsPassed(0);
        ai->setRacePosition(i + 2);
        ai->setFinished(false);
        ai->setState(AIState::Racing);
    }

    if (m_gameView) {
        for (AIOpponent* ai : opponents) {
            if (ai) {
                m_gameView->updateAICar(ai->getId(),
                                        ai->getPosition(),
                                        ai->getRotation(),
                                        speedToDisplayKmh(ai->getSpeed()));
            }
        }
    }
}

void MainWindow::applyAIDifficultySelection()
{
    m_selectedAIDifficulty = selectedAIDifficulty();
    if (m_currentMode == QStringLiteral("Arcade") && m_driveActive && !m_twoPlayerMode) {
        initializeAIOpponents();
    } else if (m_aiManager) {
        m_aiManager->destroyAllOpponents();
        if (m_gameView) {
            m_gameView->clearAllAICars();
        }
    }
}

void MainWindow::applyPlayerSpawnAtStartLine()
{
    TrackData* track = nullptr;
    if (m_gameView) {
        track = m_gameView->trackData();
    }
    if (!track) {
        TrackManager* trackMgr = TrackManager::instance(this);
        if (trackMgr && trackMgr->hasCurrentTrack()) {
            track = trackMgr->getCurrentTrack();
        }
    }

    QVector2D spawnPos(320.0, 320.0);
    qreal spawnRotation = 0.0;
    if (track) {
        const QList<QVector2D> startPositions = track->getStartPositions();
        spawnPos = startPositions.isEmpty() ? track->getStartPosition() : startPositions.first();
        spawnRotation = track->getStartRotation();
    }
    const QPoint spawnTile = TrackData::worldToTile(spawnPos);
    m_playerPosition = spawnPos;
    m_playerRotation = spawnRotation;
    m_playerSpeed = 0.0;
    m_coinChallengeLastSafePlayerPosition = spawnPos;
    m_coinChallengeLastSafePlayerRotation = spawnRotation;
    m_coinChallengeHasSafePlayerPosition = true;

    if (m_vehiclePhysics) {
        m_vehiclePhysics->setPosition(spawnPos);
        m_vehiclePhysics->setRotation(spawnRotation);
    }

    if (m_gameView) {
        m_gameView->updatePlayerCar(QStringLiteral("P1"),
                                    m_playerPosition,
                                    m_playerRotation,
                                    displaySpeedKmh(),
                                    currentPlayerSkinColor(),
                                    m_vehiclePhysics ? m_vehiclePhysics->isSpeedBoostActive() : false,
                                    m_vehiclePhysics ? m_vehiclePhysics->isShieldActive() : false,
                                    m_vehiclePhysics ? m_vehiclePhysics->isInvisible() : false,
                                    m_vehiclePhysics ? m_vehiclePhysics->isMagnetActive() : false,
                                    currentPlayerSkinId());
        m_gameView->setCameraPosition(m_playerPosition);
    }
}

void MainWindow::loadCustomTrack()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("Load PhantomDrive Track"),
        QString(),
        QStringLiteral("PhantomDrive Track (*.pdtrack *.json)"));

    if (filePath.isEmpty()) {
        return;
    }

    TrackManager* trackMgr = TrackManager::instance(this);
    if (!trackMgr || !trackMgr->loadTrackFromFile(filePath)) {
        QMessageBox::warning(this,
                             QStringLiteral("Track Load Failed"),
                             QStringLiteral("Could not load the selected track. Please choose a valid .pdtrack or JSON track file."));
        return;
    }

    m_customTrackPath = filePath;
    TrackData* loadedTrack = trackMgr->getCurrentTrack();
    if (m_gameView) {
        m_gameView->setTrackData(loadedTrack);
    }
    syncRaceTrackToManager();
    if (loadedTrack && loadedTrack->getStartPosition() == QVector2D(0, 0)) {
        const qreal tileSize = 64.0;
        loadedTrack->setStartPosition(QVector2D(15 * tileSize + tileSize / 2.0, 3 * tileSize + tileSize / 2.0));
        loadedTrack->setStartRotation(0.0);
        trackMgr->rebuildWaypoints();
    }
    setupEBRuntimeObjects();
    initializeAIOpponents();
    statusBar()->showMessage(QStringLiteral("Loaded custom track: %1").arg(filePath), 5000);
}

void MainWindow::setupCustomTrackRuntimeObjects(TrackData* track)
{
    clearEBRuntimeObjects();

    if (!track || !m_gameView) {
        return;
    }

    const QList<QVector2D> itemBoxes = track->getItemBoxPositions();
    for (int i = 0; i < itemBoxes.size(); ++i) {
        const QString id = QStringLiteral("custom_item_box_%1").arg(i + 1);
        auto* box = new PowerupBox(itemBoxes.at(i), 54.0f, this);
        box->setObjectName(id);
        box->setRespawnTime(8.0f);
        m_powerupBoxes.append(box);

        m_gameView->addPowerupBox(id, itemBoxes.at(i), QStringLiteral("Random"));

        connect(box, &PowerupBox::collected,
                this, [this, id](const QString& playerId, const PowerupType& collectedType) {
                    if (m_gameView) {
                        m_gameView->removePowerupBox(id);
                    }
                    handlePowerupCollectedForPlayer(collectedType, playerId == QStringLiteral("player_2") ? 2 : 1);
                });
        connect(box, &PowerupBox::respawned,
                this, [this, id, itemBoxes, i]() {
                    if (m_gameView) {
                        m_gameView->addPowerupBox(id, itemBoxes.at(i), QStringLiteral("Random"));
                    }
                });
    }
}

void MainWindow::startCustomTrackSession(TrackData* track)
{
    if (!track) {
        return;
    }

    if (m_driveActive) {
        silentFinishSession();
    }
    clearTransientDrivingFeedback();

    // Stop any previous learning session timer.
    if (m_learningSessionTimer) {
        m_learningSessionTimer->stop();
        m_learningSessionTimer->deleteLater();
        m_learningSessionTimer = nullptr;
    }

    m_currentMode = QStringLiteral("Custom Track");
    m_twoPlayerFinishHandled = false;
    m_player1Finished = false;
    m_player2Finished = false;
    m_customTrackPlaying = true;
    m_arcadeRaceLogicActive = true;
    m_driveActive = true;
    m_arcadeRaceFinished = false;
    m_lapsCompleted = 0;
    m_totalLaps = 1;
    m_simTick = 0;
    m_sessionElapsedMs = 0;
    m_currentLapStartMs = 0;
    m_bestLapMs = 0;
    m_playerSpeed = 0.0;
    m_player2Speed = 0.0;
    m_player2LapsCompleted = 0;
    m_player2NextCheckpointIndex = 0;
    m_player2WasInsideNextGate = false;
    track->setMaxLaps(1);

    if (m_customTrackMode) {
        m_customTrackMode->setTrack(track);
        m_customTrackMode->setCustomState(CustomTrackModeState::Playing);
    }

    if (m_customTrackEditor) {
        m_customTrackEditor->hide();
    }
    setGameHeaderVisible(true);
    updatePauseButtonState();
    if (ui->stackedWidget) {
        ui->stackedWidget->setCurrentIndex(1);
    }
    if (m_gameView) {
        m_gameView->show();
        m_gameView->setTrackData(track);
        m_gameView->clearAllAICars();
    }

    TrackManager* trackMgr = TrackManager::instance(this);
    if (trackMgr) {
        trackMgr->setCurrentTrack(track);
    }

    preparePlayerReportSystems();
    if (m_aiManager) {
        m_aiManager->destroyAllOpponents();
        m_aiManager->setRaceTotalLaps(1);
        m_aiManager->setPlayerRaceProgress(0, 0, 0.0, false, 0.0);
        m_aiManager->setTrackBounds(track->getBounds());

        initializeAIOpponents();
    }

    if (m_learningHUD) {
        m_learningHUD->hide();
    }
    if (m_arcadeHUD) {
        m_arcadeHUD->setGameMode("Custom Track");
        m_arcadeHUD->reset();
        m_arcadeHUD->setTwoPlayerMode(m_twoPlayerMode);
        m_arcadeHUD->show();
    }
    if (ui->label_ModeTitle) {
        ui->label_ModeTitle->setText("CUSTOM TRACK");
        ui->label_ModeTitle->setStyleSheet(
            "QLabel{color:#59F7FF;font-size:13px;font-weight:bold;letter-spacing:3px;}");
    }
    if (m_btnFinishDrive) {
        m_btnFinishDrive->show();
        m_btnFinishDrive->setEnabled(false);
    }

    setupCustomTrackRuntimeObjects(track);

    if (m_vehiclePhysics) {
        m_vehiclePhysics->reset();
        m_vehiclePhysics->resetRaceProgress();
        m_vehiclePhysics->setRaceLogicEnabled(false);
    }
    applyPlayerSpawnAtStartLine();
    if (m_twoPlayerMode) {
        applyPlayer2SpawnAtStartLine();
        if (m_gameView) {
            m_gameView->updatePlayerCar(QStringLiteral("P2"),
                                        m_player2Position,
                                        m_player2Rotation,
                                        speedToDisplayKmh(m_player2Speed),
                                        QColor(40, 220, 255));
            updateTwoPlayerCamera();
        }
    }
    resetArcadeRaceProgress();
    focusGameViewForDriving();

    m_countdownActive = true;
    showCountdown();

    statusBar()->showMessage(QStringLiteral("Custom Track Mode running"));
}

QString MainWindow::formatRaceTime(qint64 milliseconds) const
{
    const qint64 minutes = milliseconds / 60000;
    const qint64 seconds = (milliseconds % 60000) / 1000;
    const qint64 millis = milliseconds % 1000;
    return QStringLiteral("%1:%2.%3")
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 2, 10, QLatin1Char('0'))
        .arg(millis, 3, 10, QLatin1Char('0'));
}

qreal MainWindow::estimatePlayerProgress() const
{
    constexpr qint64 DemoLapDurationMs = 15000;
    const qint64 lapElapsed = qMax<qint64>(0, m_sessionElapsedMs - m_currentLapStartMs);
    return qBound(0.0, (static_cast<qreal>(lapElapsed) / DemoLapDurationMs) * 100.0, 100.0);
}

int MainWindow::displaySpeedKmh() const
{
    return speedToDisplayKmh(m_playerSpeed);
}

int MainWindow::speedToDisplayKmh(qreal physicsSpeed) const
{
    return qBound(0, qRound(qAbs(physicsSpeed) * kDisplayMaxSpeedKmh / kPhysicsMaxSpeed), 999);
}

void MainWindow::updateEBRuntime(qreal deltaSeconds)
{
    const qint64 deltaMs = static_cast<qint64>(deltaSeconds * 1000.0);
    const bool widenPowerupCollect = !isCoinChallengeModeActive();
    const float collectRadius = (widenPowerupCollect && m_vehiclePhysics && m_vehiclePhysics->isMagnetActive()) ? 96.0f : 54.0f;
    const float player2CollectRadius = (widenPowerupCollect && m_player2Physics && m_player2Physics->isMagnetActive()) ? 96.0f : 54.0f;

    for (PowerupBox* box : m_powerupBoxes) {
        if (!box) {
            continue;
        }
        box->update(static_cast<float>(deltaSeconds));
        if (box->isActive()) {
            box->tryCollect(m_playerPosition, QStringLiteral("player_1"), collectRadius);
        }
        if (m_twoPlayerMode && box->isActive()) {
            box->tryCollect(m_player2Position, QStringLiteral("player_2"), player2CollectRadius);
        }
    }

    if (m_powerupWorld) {
        m_powerupWorld->update(deltaMs, m_vehiclePhysics, m_aiManager, m_powerupBoxes, m_gameView);
    }

    if (!m_trafficObjectManager) {
        return;
    }

    if (m_currentMode != QStringLiteral("Learning")) {
        return;
    }

    const qreal speedKmh = displaySpeedKmh();
    m_trafficObjectManager->onVehicleSpeedChanged(speedKmh);
    m_trafficObjectManager->onVehiclePositionChanged(m_playerPosition);

    for (PedestrianCrossingObject* crossing : m_trafficObjectManager->getPedestrianCrossings()) {
        if (crossing && crossing->getPedestrianCount() == 0) {
            crossing->spawnPedestrian();
        }
    }

    m_trafficObjectManager->update(static_cast<qint64>(deltaSeconds * 1000.0));

    const qreal zoneLimit = m_trafficObjectManager->getCurrentSpeedLimit(m_playerPosition);
    if (zoneLimit > 0.0) {
        m_currentSpeedLimit = qRound(zoneLimit);
    } else {
        m_currentSpeedLimit = 80;
    }
    if (m_arcadeHUD) {
        m_arcadeHUD->updateSpeedLimit(m_currentSpeedLimit);
    }
}

void MainWindow::handlePowerupCollected(PhantomDrive::PowerupType type)
{
    handlePowerupCollectedForPlayer(type, 1);
}

void MainWindow::handlePowerupCollectedForPlayer(PhantomDrive::PowerupType type, int playerIndex)
{
    if (m_gamePaused) {
        return;
    }

    PowerupType effectiveType = type;
    if (type == PowerupType::Custom) {
        static const PowerupType kCustomPool[] = {
            PowerupType::Boost,
            PowerupType::Shield,
            PowerupType::Repair,
            PowerupType::Missile,
            PowerupType::EMP
        };
        const int index = QRandomGenerator::global()->bounded(static_cast<int>(sizeof(kCustomPool) / sizeof(kCustomPool[0])));
        effectiveType = kCustomPool[index];
    }

    const QString collectedTypeName = powerupTypeToString(type);
    const QString typeName = powerupTypeToString(effectiveType);
    const QString powerupId = QStringLiteral("eb_%1").arg(typeName.toLower().replace(QStringLiteral(" "), QStringLiteral("_")));
    VehiclePhysics* targetPhysics = (playerIndex == 2) ? m_player2Physics : m_vehiclePhysics;
    QVector2D* targetPosition = (playerIndex == 2) ? &m_player2Position : &m_playerPosition;
    qreal* targetRotation = (playerIndex == 2) ? &m_player2Rotation : &m_playerRotation;
    qreal* targetSpeed = (playerIndex == 2) ? &m_player2Speed : &m_playerSpeed;
    const QString playerLabel = playerIndex == 2 ? QStringLiteral("P2") : QStringLiteral("P1");

    onPowerupCollected(playerIndex == 2 ? QStringLiteral("P2 %1").arg(collectedTypeName) : collectedTypeName);

    // Unified powerup notification: ArcadeHUD only (LearningHUD is deprecated).
    auto notifyArcadeHUD = [this, playerIndex](const QString& type, int durationMs) {
        if (m_arcadeHUD) {
            const int remainingSecs = durationMs > 0 ? durationMs / 1000 : 0;
            if (m_twoPlayerMode) {
                m_arcadeHUD->updatePlayerPowerup(playerIndex, type, remainingSecs);
            } else {
                m_arcadeHUD->updatePowerupState(type, remainingSecs);
            }
        }
    };

    if (effectiveType == PowerupType::Boost && targetPhysics) {
        targetPhysics->activateSpeedBoost(1.35, 4000);
        playSound(SoundEffect::Boost);
        notifyArcadeHUD(QStringLiteral("Boost"), 4000);
    } else if (effectiveType == PowerupType::Shield && targetPhysics) {
        targetPhysics->activateShield(6000);
        notifyArcadeHUD(QStringLiteral("Shield"), 6000);
    } else if (effectiveType == PowerupType::EMP) {
        QList<QPair<QPointer<AIOpponent>, qreal>> affectedOpponents;
        if (m_aiManager) {
            for (AIOpponent* opponent : m_aiManager->getAllOpponents()) {
                if (!opponent || opponent->hasFinished()) {
                    continue;
                }
                const qreal originalSpeed = opponent->getMaxSpeed();
                affectedOpponents.append(qMakePair(QPointer<AIOpponent>(opponent), originalSpeed));
                opponent->setMaxSpeed(qMax<qreal>(35.0, originalSpeed * 0.45));
            }
        }
        notifyArcadeHUD(QStringLiteral("EMP"), 3000);
        QTimer::singleShot(3000, this, [this, affectedOpponents]() {
            for (const auto& entry : affectedOpponents) {
                if (entry.first) {
                    entry.first->setMaxSpeed(entry.second);
                }
            }
        });
    } else if (effectiveType == PowerupType::Repair && targetPhysics) {
        targetPhysics->activateRepair();
        notifyArcadeHUD(QStringLiteral("Repair"), 1200);
    } else if (effectiveType == PowerupType::Missile && m_powerupWorld && targetPhysics) {
        const QString targetId = m_powerupWorld->findBestMissileTarget(
            *targetPosition, targetPhysics->getRotation(), m_aiManager);
        if (!targetId.isEmpty()) {
            m_powerupWorld->spawnMissile(*targetPosition, targetId);
            notifyArcadeHUD(QStringLiteral("Missile"), 1500);
        } else if (m_arcadeHUD) {
            m_arcadeHUD->showRaceBanner(QStringLiteral("No target"));
        }
    } else if (effectiveType == PowerupType::OilSlick && m_powerupWorld && targetPhysics) {
        const qreal radians = qDegreesToRadians(targetPhysics->getRotation());
        const QVector2D dropPos = *targetPosition
            - QVector2D(qSin(radians), qCos(radians)) * 36.0;
        m_powerupWorld->spawnOilPuddle(dropPos, 72.0, 12000);
        notifyArcadeHUD(QStringLiteral("Oil"), 1500);
    } else if (effectiveType == PowerupType::Invisibility && targetPhysics) {
        targetPhysics->activateInvisibility(8000);
        notifyArcadeHUD(QStringLiteral("Invis"), 8000);
    } else if (effectiveType == PowerupType::Teleport && targetPhysics) {
        if (targetPhysics->teleportForward(280.0)) {
            *targetPosition = targetPhysics->getPosition();
            *targetRotation = targetPhysics->getRotation();
            *targetSpeed = targetPhysics->getSpeed();
            if (m_gameView) {
                if (playerIndex == 2) {
                    m_gameView->updatePlayerCar(QStringLiteral("P2"), m_player2Position, m_player2Rotation,
                                                speedToDisplayKmh(m_player2Speed), QColor(40, 220, 255));
                    updateTwoPlayerCamera();
                } else if (m_twoPlayerMode) {
                    m_gameView->updatePlayerCar(QStringLiteral("P1"), m_playerPosition, m_playerRotation,
                                                displaySpeedKmh(),
                                                currentPlayerSkinColor(),
                                                m_vehiclePhysics ? m_vehiclePhysics->isSpeedBoostActive() : false,
                                                m_vehiclePhysics ? m_vehiclePhysics->isShieldActive() : false,
                                                m_vehiclePhysics ? m_vehiclePhysics->isInvisible() : false,
                                                m_vehiclePhysics ? m_vehiclePhysics->isMagnetActive() : false,
                                                currentPlayerSkinId());
                    updateTwoPlayerCamera();
                } else {
                    m_gameView->updatePlayerCar(QStringLiteral("P1"),
                                                m_playerPosition,
                                                m_playerRotation,
                                                displaySpeedKmh(),
                                                currentPlayerSkinColor(),
                                                m_vehiclePhysics ? m_vehiclePhysics->isSpeedBoostActive() : false,
                                                m_vehiclePhysics ? m_vehiclePhysics->isShieldActive() : false,
                                                m_vehiclePhysics ? m_vehiclePhysics->isInvisible() : false,
                                                m_vehiclePhysics ? m_vehiclePhysics->isMagnetActive() : false,
                                                currentPlayerSkinId());
                    m_gameView->setCameraPosition(m_playerPosition);
                }
            }
            notifyArcadeHUD(QStringLiteral("Teleport"), 1200);
        } else {
            notifyArcadeHUD(QStringLiteral("Teleport"), 0);
        }
    } else if (effectiveType == PowerupType::Magnet && targetPhysics) {
        const int magnetDurationMs = isCoinChallengeModeActive() ? kCoinChallengeMagnetDurationMs : 6000;
        targetPhysics->activateMagnet(magnetDurationMs);
        if (isCoinChallengeModeActive() && playerIndex == 1) {
            m_coinChallengeMagnetRemainingMs = magnetDurationMs;
            ++m_coinChallengePowerupsUsed;
            syncCoinChallengeMagnetLoop();
        }
        notifyArcadeHUD(QStringLiteral("Magnet"), magnetDurationMs);
    }
}

void MainWindow::handleTrafficViolation(const PhantomDrive::ViolationEvent& violation)
{
    if (m_gamePaused) {
        return;
    }
    // Single unified path: InteractiveFeedback toast only (no duplicate statusBar).
    showInteractiveFeedback(violation.description, PhantomDrive::FeedbackType::Critical);
    playSound(SoundEffect::Violation);
}

void MainWindow::updateRaceHud()
{
    if (isCoinChallengeModeActive()) {
        updateCoinChallengeHud();
        return;
    }

    const bool raceRankingActive = m_arcadeRaceLogicActive
        && (m_currentMode == QStringLiteral("Arcade") || m_customTrackPlaying);
    const bool learningRaceHudActive = (m_currentMode == QStringLiteral("Learning"));
    const bool hudRaceRankingActive = raceRankingActive || learningRaceHudActive;
    const bool includeAiInHudRanking = hudRaceRankingActive && hasHudRankedAiOpponents(m_aiManager);

    if (m_aiManager && raceRankingActive && m_driveActive && !m_countdownActive && !m_gamePaused) {
        const int totalCheckpoints = qMax(1, m_raceCheckpointTotal);
        const int checkpointIndex = qBound(0, m_nextCheckpointIndex, totalCheckpoints);
        const qreal progressPercent = qBound(0.0,
                                             (static_cast<qreal>(checkpointIndex) / totalCheckpoints) * 100.0,
                                             100.0);
        const int lap = m_customTrackPlaying
            ? (m_arcadeRaceFinished ? 1 : 0)
            : m_lapsCompleted;
        m_aiManager->setPlayerRaceProgress(lap,
                                           checkpointIndex,
                                           progressPercent,
                                           m_arcadeRaceFinished,
                                           m_sessionElapsedMs / 1000.0);
    }

    // LearningHUD is fully deprecated; all HUD data goes to ArcadeHUD.

    if (!m_arcadeHUD) {
        return;
    }

    m_arcadeHUD->setTwoPlayerMode(m_twoPlayerMode);
    m_arcadeHUD->updateSpeed(displaySpeedKmh());

    const int raceCheckpointTotal = qMax(0, m_raceCheckpointTotal);
    const QList<RaceHudEntry> raceEntries = buildRaceHudEntries(includeAiInHudRanking,
                                                                hudRaceRankingActive,
                                                                m_playerPosition,
                                                                m_lapsCompleted,
                                                                m_nextCheckpointIndex,
                                                                m_twoPlayerMode,
                                                                m_player2Position,
                                                                m_player2LapsCompleted,
                                                                m_player2NextCheckpointIndex,
                                                                m_aiManager,
                                                                m_gameView ? m_gameView->trackData() : nullptr,
                                                                raceCheckpointTotal,
                                                                &m_hudRaceProgressById,
                                                                &m_hudRaceRouteSignature);
    const int totalRacers = qMax(1, raceEntries.size());
    const int p1Position = raceHudPositionFor(raceEntries, QStringLiteral("P1"));
    const int p2Position = raceHudPositionFor(raceEntries, QStringLiteral("P2"));

    if (m_twoPlayerMode) {
        const int totalLaps = m_customTrackPlaying ? 1 : qMax(1, m_totalLaps);
        m_arcadeHUD->updatePlayer1Status(displaySpeedKmh(),
                                         m_currentSpeedLimit,
                                         m_currentTrafficLightState,
                                         qBound(1, m_lapsCompleted + 1, totalLaps),
                                         totalLaps,
                                         p1Position,
                                         totalRacers);
        m_arcadeHUD->updatePlayer2Status(speedToDisplayKmh(m_player2Speed),
                                         m_currentSpeedLimit,
                                         m_currentTrafficLightState,
                                         qBound(1, m_player2LapsCompleted + 1, totalLaps),
                                         totalLaps,
                                         p2Position,
                                         totalRacers);
    } else if (m_customTrackPlaying) {
        const int total = qMax(0, m_raceCheckpointTotal);
        const int passed = qBound(0, m_nextCheckpointIndex, total);
        const QString nextTarget = passed >= total
            ? QStringLiteral("FINISH")
            : QStringLiteral("CP%1").arg(passed + 1);
        m_arcadeHUD->updateRouteProgress(passed, total, nextTarget);
    } else {
        m_arcadeHUD->updateLap(m_lapsCompleted, m_totalLaps);
    }
    m_arcadeHUD->updateTotalTime(formatRaceTime(m_sessionElapsedMs));
    m_arcadeHUD->updateLapTime(formatRaceTime(qMax<qint64>(0, m_sessionElapsedMs - m_currentLapStartMs)));

    if (!m_twoPlayerMode) {
        m_arcadeHUD->updatePosition(p1Position, totalRacers);
    }

    const AIOpponent* ai1 = m_aiManager ? m_aiManager->getOpponent(QStringLiteral("ai_1")) : nullptr;
    const AIOpponent* ai2 = m_aiManager ? m_aiManager->getOpponent(QStringLiteral("ai_2")) : nullptr;
    const bool ai1Visible = ai1 && ai1->isActive() && !ai1->hasFinished();
    const bool ai2Visible = ai2 && ai2->isActive() && !ai2->hasFinished();
    m_arcadeHUD->updateAISpeed1(ai1Visible ? speedToDisplayKmh(ai1->getSpeed()) : 0.0, ai1Visible);
    m_arcadeHUD->updateAISpeed2(ai2Visible ? speedToDisplayKmh(ai2->getSpeed()) : 0.0, ai2Visible);

    // Sync traffic light state to ArcadeHUD so the signal-dot and red-blink work.
    m_arcadeHUD->updateTrafficLight(m_currentTrafficLightState);

    // Sync speed limit so the speedometer turns red when speeding.
    m_arcadeHUD->updateSpeedLimit(m_currentSpeedLimit);

    // Show boost bar: 100% when boost is active, 0% otherwise.
    if (m_vehiclePhysics) {
        m_arcadeHUD->updateBoost(m_vehiclePhysics->isSpeedBoostActive() ? 100.0 : 0.0);
    }
}

void MainWindow::syncRaceTrackToManager()
{
    PhantomDrive::TrackManager* trackMgr = PhantomDrive::TrackManager::instance(this);
    if (trackMgr && m_gameView && m_gameView->trackData()) {
        trackMgr->setCurrentTrack(m_gameView->trackData());
    }
}

void MainWindow::resetArcadeRaceProgress()
{
    m_hudRaceProgressById.clear();
    m_hudRaceRouteSignature.clear();
    m_nextCheckpointIndex = 0;
    m_hasLeftNorthSector = false;
    m_blockCheckpointsUntilLeaveNorth = false;
    m_previousPlayerPosition = m_playerPosition;

    PhantomDrive::TrackData* track = m_gameView ? m_gameView->trackData() : nullptr;
    m_raceCheckpointTotal = track ? track->getCheckpointsInOrder().size() : 0;

    m_wasOnStartLine = tileAtIsStartFinish(track, m_playerPosition);
    m_wasInNorthGate = positionInNorthGate(track, m_playerPosition);
    m_wasInsideNextGate = false;
    if (m_nextCheckpointIndex < m_raceCheckpointTotal && track) {
        const QList<PhantomDrive::Checkpoint*> checkpoints = track->getCheckpointsInOrder();
        if (m_nextCheckpointIndex < checkpoints.size() && checkpoints.at(m_nextCheckpointIndex)) {
            m_wasInsideNextGate = checkpoints.at(m_nextCheckpointIndex)->containsPoint(m_playerPosition);
        }
    }
}

void MainWindow::updateArcadeRaceProgress(const QVector2D& positionBefore)
{
    const bool learningRaceHudActive = (m_currentMode == QStringLiteral("Learning"));
    if ((!m_arcadeRaceLogicActive && !learningRaceHudActive)
        || !m_driveActive || m_countdownActive || m_gamePaused || m_arcadeRaceFinished) {
        return;
    }

    PhantomDrive::TrackData* track = m_gameView ? m_gameView->trackData() : nullptr;
    if (!track) {
        return;
    }

    const QList<PhantomDrive::Checkpoint*> checkpoints = track->getCheckpointsInOrder();
    if (m_raceCheckpointTotal <= 0) {
        m_raceCheckpointTotal = checkpoints.size();
    }
    if (m_raceCheckpointTotal <= 0) {
        return;
    }

    const QVector2D& pos = m_playerPosition;
    const bool onStartLine = tileAtIsStartFinish(track, pos);
    const bool nearStartFinish = m_customTrackPlaying && positionNearStartFinish(track, pos);
    const bool inNorthGate = !m_customTrackPlaying && positionInNorthGate(track, pos);
    const bool inNorthSector = m_customTrackPlaying ? onStartLine : (onStartLine || inNorthGate);
    const bool wasInNorthSector = m_customTrackPlaying ? m_wasOnStartLine : (m_wasOnStartLine || m_wasInNorthGate);

    if (!inNorthSector) {
        m_hasLeftNorthSector = true;
        m_blockCheckpointsUntilLeaveNorth = false;
    }

    const bool allCheckpointsCollected = m_nextCheckpointIndex >= m_raceCheckpointTotal;

    if (allCheckpointsCollected && m_hasLeftNorthSector) {
        const bool enteredStartLine = onStartLine && !m_wasOnStartLine;
        const bool enteredNorthGate = !m_customTrackPlaying && inNorthGate && !m_wasInNorthGate;
        const bool reachedCustomTrackFinish = m_customTrackPlaying && (onStartLine || nearStartFinish);
        if (enteredStartLine || enteredNorthGate || reachedCustomTrackFinish) {
            if (m_customTrackPlaying) {
                finishCustomTrackRoute();
            } else {
                const int completedLap = m_lapsCompleted + 1;
                const qint64 lapMs = qMax<qint64>(0, m_sessionElapsedMs - m_currentLapStartMs);

                onLapCompleted(completedLap);
                m_lapsCompleted = completedLap;

                if (lapMs > 0 && (m_bestLapMs == 0 || lapMs < m_bestLapMs)) {
                    m_bestLapMs = lapMs;
                }

                if (learningRaceHudActive && m_lapsCompleted >= m_totalLaps) {
                    m_lapsCompleted = 0;
                }

                if (m_lapsCompleted < m_totalLaps) {
                    m_currentLapStartMs = m_sessionElapsedMs;
                    updateRaceHud();
                }

                m_nextCheckpointIndex = 0;
                m_hasLeftNorthSector = false;
                m_blockCheckpointsUntilLeaveNorth = true;
            }
        }
    }

    if (!m_blockCheckpointsUntilLeaveNorth && m_nextCheckpointIndex < checkpoints.size()) {
        if (m_nextCheckpointIndex == 0) {
            PhantomDrive::Checkpoint* cp0 = checkpoints.first();
            const bool leavingNorth = !m_customTrackPlaying && !inNorthSector && wasInNorthSector;
            const bool insideCp0 = cp0 && cp0->containsPoint(pos);
            const bool enteredCp0 = insideCp0 && !m_wasInsideNextGate;
            const bool crossedCp0 = (m_customTrackPlaying || m_hasLeftNorthSector)
                && cp0
                && crossedCheckpointGate(cp0, positionBefore, pos);
            if (leavingNorth || enteredCp0 || crossedCp0) {
                onCheckpointReached(0);
                m_nextCheckpointIndex = 1;
                m_wasInsideNextGate = false;
            }
        } else {
            PhantomDrive::Checkpoint* nextCp = checkpoints.at(m_nextCheckpointIndex);
            const bool insideNext = nextCp && nextCp->containsPoint(pos);
            const bool enteredNext = insideNext && !m_wasInsideNextGate;
            const bool crossedNext = crossedCheckpointGate(nextCp, positionBefore, pos);
            if (enteredNext || crossedNext) {
                onCheckpointReached(m_nextCheckpointIndex);
                ++m_nextCheckpointIndex;
                m_wasInsideNextGate = false;
            }
        }
    }

    m_wasOnStartLine = onStartLine;
    m_wasInNorthGate = inNorthGate;

    if (m_nextCheckpointIndex < checkpoints.size()) {
        PhantomDrive::Checkpoint* nextCp = checkpoints.at(m_nextCheckpointIndex);
        m_wasInsideNextGate = nextCp && nextCp->containsPoint(pos);
    } else {
        m_wasInsideNextGate = false;
    }
}

void MainWindow::finishCustomTrackRoute()
{
    if (m_arcadeRaceFinished) {
        return;
    }
    if (m_twoPlayerMode) {
        markTwoPlayerFinished(1);
        return;
    }

    m_arcadeRaceFinished = true;
    m_lapsCompleted = 1;
    m_nextCheckpointIndex = m_raceCheckpointTotal;
    const qint64 elapsedMs = qMax<qint64>(0, m_sessionElapsedMs - m_currentLapStartMs);
    if (elapsedMs > 0 && (m_bestLapMs == 0 || elapsedMs < m_bestLapMs)) {
        m_bestLapMs = elapsedMs;
    }

    if (m_aiManager) {
        m_aiManager->setPlayerRaceProgress(1, m_raceCheckpointTotal, 100.0, true, m_sessionElapsedMs / 1000.0);
    }

    if (m_arcadeHUD) {
        m_arcadeHUD->updateRouteProgress(m_raceCheckpointTotal, m_raceCheckpointTotal, QStringLiteral("FINISH"));
        m_arcadeHUD->showRaceBanner(QStringLiteral("Custom Track Finished"));
        m_arcadeHUD->showRaceFinished(1, formatRaceTime(m_sessionElapsedMs));
    }

    playSound(PhantomDrive::SoundEffect::RaceFinish);
    // Finish banner is already shown via ArcadeHUD above.

    // Wait 2 s so the player can see the finish banner, then show report.
    QTimer::singleShot(2000, this, [this]() {
        if (m_driveActive) {
            onGameFinished();
        }
    });
}

void MainWindow::updatePlayer2RaceProgress(const QVector2D& positionBefore)
{
    if (!m_twoPlayerMode || !m_arcadeRaceLogicActive || !m_driveActive || m_countdownActive || m_gamePaused
        || m_arcadeRaceFinished || m_twoPlayerFinishHandled) {
        return;
    }

    TrackData* track = m_gameView ? m_gameView->trackData() : nullptr;
    if (!track) {
        return;
    }

    const QList<Checkpoint*> checkpoints = track->getCheckpointsInOrder();
    const int total = checkpoints.size();
    if (total <= 0) {
        return;
    }

    const QVector2D pos = m_player2Position;
    if (m_player2Finished) {
        return;
    }
    if (m_player2NextCheckpointIndex < total) {
        Checkpoint* nextCp = checkpoints.at(m_player2NextCheckpointIndex);
        const bool insideNext = nextCp && nextCp->containsPoint(pos);
        const bool enteredNext = insideNext && !m_player2WasInsideNextGate;
        const bool crossedNext = crossedCheckpointGate(nextCp, positionBefore, pos);
        if (enteredNext || crossedNext) {
            if (m_arcadeHUD) {
                m_arcadeHUD->showRaceBanner(
                    QStringLiteral("P2 CP %1/%2 passed")
                        .arg(m_player2NextCheckpointIndex + 1)
                        .arg(total));
            }
            playSound(PhantomDrive::SoundEffect::Checkpoint);
            ++m_player2NextCheckpointIndex;
            m_player2WasInsideNextGate = false;
        }
    }

    if (m_player2NextCheckpointIndex >= total && tileAtIsStartFinish(track, pos)) {
        if (m_customTrackPlaying) {
            markTwoPlayerFinished(2);
            return;
        }
        ++m_player2LapsCompleted;
        if (m_player2LapsCompleted >= m_totalLaps) {
            markTwoPlayerFinished(2);
            return;
        }
        m_player2NextCheckpointIndex = 0;
    }

    if (m_player2NextCheckpointIndex < checkpoints.size()) {
        Checkpoint* nextCp = checkpoints.at(m_player2NextCheckpointIndex);
        m_player2WasInsideNextGate = nextCp && nextCp->containsPoint(pos);
    } else {
        m_player2WasInsideNextGate = false;
    }
}

void MainWindow::markTwoPlayerFinished(int playerIndex)
{
    if (m_twoPlayerFinishHandled) {
        return;
    }

    bool& finishedFlag = playerIndex == 2 ? m_player2Finished : m_player1Finished;
    if (finishedFlag) {
        return;
    }
    finishedFlag = true;

    if (playerIndex == 1) {
        m_lapsCompleted = m_totalLaps;
        m_nextCheckpointIndex = m_raceCheckpointTotal;
    } else {
        m_player2LapsCompleted = m_totalLaps;
        m_player2NextCheckpointIndex = m_raceCheckpointTotal;
    }

    const QString playerName = playerIndex == 2 ? QStringLiteral("Player 2") : QStringLiteral("Player 1");
    if (m_arcadeHUD) {
        m_arcadeHUD->showRaceBanner(
            m_player1Finished && m_player2Finished
                ? QStringLiteral("Both Players Finished")
                : QStringLiteral("%1 Finished - waiting for other player").arg(playerName));
        updateRaceHud();
    }

    playSound(PhantomDrive::SoundEffect::RaceFinish);

    if (m_player1Finished && m_player2Finished) {
        finishTwoPlayerRace();
    }
}

void MainWindow::finishTwoPlayerRace()
{
    if (m_twoPlayerFinishHandled) {
        return;
    }
    m_twoPlayerFinishHandled = true;
    m_arcadeRaceFinished = true;

    if (m_arcadeHUD) {
        m_arcadeHUD->showRaceBanner(QStringLiteral("Both Players Finished"));
        m_arcadeHUD->showRaceFinished(1, formatRaceTime(m_sessionElapsedMs));
    }
    // Final banner is already shown via ArcadeHUD above.

    QTimer::singleShot(2000, this, [this]() {
        if (m_driveActive) {
            onGameFinished();
        }
    });
}

void MainWindow::resolvePlayerAiVehicleContact(AIOpponent* ai)
{
    if (!ai || !m_vehiclePhysics) {
        return;
    }

    const QString aiId = ai->getId();

    if (m_vehiclePhysics->isInvisible()) {
        m_playerVehicleContacts.remove(aiId);
        return;
    }

    if (ai->hasFinished()) {
        static QHash<QString, qint64> lastFinishedAiContactFeedbackMs;

        const QVector2D playerPos = m_vehiclePhysics->getPosition();
        const QVector2D aiPos = ai->getPosition();
        const QVector2D delta = playerPos - aiPos;
        const qreal dist = delta.length();

        if (dist >= kVehicleContactReleaseDistance) {
            m_playerVehicleContacts.remove(aiId);
            return;
        }

        const bool wasInContact = m_playerVehicleContacts.contains(aiId);
        const bool inContact = dist < kVehicleImpactDistance;
        const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
        const qint64 lastFeedbackMs = lastFinishedAiContactFeedbackMs.value(aiId, -kFinishedAiContactCooldownMs);

        if (inContact) {
            m_playerVehicleContacts.insert(aiId);
        }

        if (inContact
            && !wasInContact
            && nowMs - lastFeedbackMs >= kFinishedAiContactCooldownMs) {
            const qreal impactForce = qMax<qreal>(5.0, qAbs(m_playerSpeed - ai->getSpeed()) * 0.25);
            if (m_gameView
                && !shouldSuppressStartupCollisionImpact(m_driveActive,
                                                         m_countdownActive,
                                                         m_sessionElapsedMs)) {
                m_gameView->showCollisionImpact((playerPos + aiPos) * 0.5,
                                                qBound<qreal>(0.5, impactForce / 90.0, 1.8));
            }
            lastFinishedAiContactFeedbackMs.insert(aiId, nowMs);
        }
        return;
    }

    static QHash<QString, qint64> lastAiContactImpactMs;
    auto* simpleAi = qobject_cast<SimpleAIOpponent*>(ai);

    QVector2D playerPos = m_vehiclePhysics->getPosition();
    QVector2D aiPos = ai->getPosition();
    QVector2D delta = playerPos - aiPos;
    qreal dist = delta.length();

    if (dist >= kVehicleContactReleaseDistance) {
        m_playerVehicleContacts.remove(aiId);
    }

    const bool wasInContactBeforeSeparation = m_playerVehicleContacts.contains(aiId);
    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    const qint64 lastImpactMs = lastAiContactImpactMs.value(aiId, -kVehicleImpactVisualCooldownMs);
    if (dist < kVehicleSeparationDistance
        && !wasInContactBeforeSeparation
        && nowMs - lastImpactMs >= kVehicleImpactVisualCooldownMs) {
        const qreal visualImpactForce = qAbs(m_playerSpeed - ai->getSpeed());
        if (m_gameView
            && !shouldSuppressStartupCollisionImpact(m_driveActive,
                                                     m_countdownActive,
                                                     m_sessionElapsedMs)) {
            m_gameView->showCollisionImpact((playerPos + aiPos) * 0.5,
                                            qBound<qreal>(0.6, visualImpactForce / 80.0, 1.8));
        }
        lastAiContactImpactMs.insert(aiId, nowMs);
    }

    if (dist < kVehicleSeparationDistance) {
        QVector2D dir = dist > 0.001 ? delta / dist : QVector2D(1.0, 0.0);
        const qreal overlap = kVehicleSeparationDistance - dist;
        const QVector2D playerNew = playerPos + dir * (overlap * 0.5);
        const QVector2D aiNew = aiPos - dir * (overlap * 0.5);

        const bool playerFree = m_vehiclePhysics->isPositionFree(playerNew);
        const bool aiFree = !simpleAi || simpleAi->isPositionFree(aiNew);

        if (playerFree && aiFree) {
            m_vehiclePhysics->setPosition(playerNew);
            m_playerPosition = playerNew;
            ai->setPosition(aiNew);
        } else if (playerFree) {
            const QVector2D pushedPlayer = playerPos + dir * overlap;
            if (m_vehiclePhysics->isPositionFree(pushedPlayer)) {
                m_vehiclePhysics->setPosition(pushedPlayer);
                m_playerPosition = pushedPlayer;
            }
        } else if (aiFree) {
            const QVector2D pushedAi = aiPos - dir * overlap;
            ai->setPosition(pushedAi);
        }
    }

    playerPos = m_vehiclePhysics->getPosition();
    aiPos = ai->getPosition();
    delta = playerPos - aiPos;
    dist = delta.length();

    const bool wasInContact = m_playerVehicleContacts.contains(aiId);
    const bool inContact = dist < kVehicleImpactDistance;

    if (inContact && !wasInContact && dist > 0.001) {
        const QVector2D normal = delta / dist;
        const qreal impactForce = qAbs(m_playerSpeed - ai->getSpeed());

        if (m_gameView
            && !shouldSuppressStartupCollisionImpact(m_driveActive,
                                                     m_countdownActive,
                                                     m_sessionElapsedMs)) {
            m_gameView->showCollisionImpact((playerPos + aiPos) * 0.5,
                                            qBound<qreal>(0.5, impactForce / 90.0, 1.8));
        }
        if (!m_vehiclePhysics->isColliding()) {
            m_vehiclePhysics->handleCollision(normal, impactForce);
        }
        if (simpleAi && simpleAi->usesVehiclePhysics() && !simpleAi->isPhysicsColliding()) {
            simpleAi->applyExternalCollision(-normal, impactForce);
        }
        m_aiManager->onPlayerCollision(aiId, playerPos);
        m_playerVehicleContacts.insert(aiId);
    }
}

void MainWindow::simulateGameLoop()
{
    m_simTimer = new QTimer(this);

    connect(m_simTimer, &QTimer::timeout, this, [this]() {
        if (!m_driveActive) {
            return;
        }
        if (m_gamePaused) {
            if (m_gameView) {
                m_gameView->update();
            }
            return;
        }
        if (m_countdownActive) {
            if (m_countdownTimerStartedMs > 0) {
                const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
                const int elapsedMs = static_cast<int>(qMax<qint64>(0, nowMs - m_countdownTimerStartedMs));
                m_countdownRemainingMs = qMax(0, 3200 - elapsedMs);
            }

            if (isCoinChallengeModeActive()) {
                const int countdownStage = coinChallengeCountdownStage();
                if (countdownStage >= 0 && countdownStage != m_coinChallengeCountdownAnnouncedStage) {
                    m_coinChallengeCountdownAnnouncedStage = countdownStage;
                    playSound(coinChallengeCountdownSoundForStage(countdownStage));
                }
                updateCoinChallengeHud();
            }
            if (m_gameView) {
                m_gameView->update();
            }
            return;
        }

        if (isCoinChallengeModeActive() && isCoinChallengeBalloonRushSequenceActive()) {
            updateCoinChallengeBalloonRush(kSimulationStepMs);
            updateCoinChallengeHud();
            if (m_gameView) {
                m_gameView->update();
            }
            return;
        }

        if (isCoinChallengeModeActive() && m_coinChallengeForcedEndActive) {
            updateCoinChallengeForcedEndSequence(kSimulationStepMs);
            updateCoinChallengeHud();
            if (m_gameView) {
                m_gameView->update();
            }
            return;
        }

        ++m_simTick;
        m_sessionElapsedMs += kSimulationStepMs;

        const QVector2D positionBeforeUpdate = m_playerPosition;
        const QVector2D player2PositionBeforeUpdate = m_player2Position;

        if (m_vehiclePhysics) {
            m_vehiclePhysics->update(kSimulationStepMs);
        }
        if (m_twoPlayerMode && m_player2Physics) {
            m_player2Physics->update(kSimulationStepMs);
        }

        if (m_arcadeRaceLogicActive || m_currentMode == QStringLiteral("Learning")) {
            updateArcadeRaceProgress(positionBeforeUpdate);
            if (m_twoPlayerMode) {
                updatePlayer2RaceProgress(player2PositionBeforeUpdate);
            }
        }

        m_previousPlayerPosition = m_playerPosition;
        m_previousPlayer2Position = m_player2Position;

        updateEBRuntime(kSimulationStepSeconds);
        updateTrafficAndHud(m_simTick);

        if (isCoinChallengeModeActive()) {
            const int currentSpeedKmh = displaySpeedKmh();
            const int runCoinsBeforeUpdate = m_collectibleManager ? m_collectibleManager->runCoins() : 0;
            m_coinChallengeRemainingMs = qMax(0, m_coinChallengeRemainingMs - kSimulationStepMs);
            m_coinChallengeMagnetRemainingMs = qMax(0, m_coinChallengeMagnetRemainingMs - kSimulationStepMs);
            m_coinChallengeDamageFlashRemainingMs = qMax(0, m_coinChallengeDamageFlashRemainingMs - kSimulationStepMs);
            m_coinChallengeDamagePopupRemainingMs = qMax(0, m_coinChallengeDamagePopupRemainingMs - kSimulationStepMs);
            m_coinChallengeLowIntegrityWarningRemainingMs = qMax(0,
                                                                 m_coinChallengeLowIntegrityWarningRemainingMs - kSimulationStepMs);
            if (m_coinChallengeDamagePopupRemainingMs <= 0) {
                m_coinChallengeRecentDamageAmount = 0;
                m_coinChallengeDamagePopupText.clear();
                m_coinChallengeDamagePopupDetail.clear();
            }
            syncCoinChallengeMagnetLoop();
            updateCoinChallengeStats(currentSpeedKmh);
            updateCoinChallengeLoopProgress(positionBeforeUpdate);
            const int remainingSeconds = qMax(0, static_cast<int>(qCeil(m_coinChallengeRemainingMs / 1000.0)));
            if (m_challengeObstacleManager) {
                m_challengeObstacleManager->updateProgress(runCoinsBeforeUpdate,
                                                           remainingSeconds,
                                                           m_sessionElapsedMs);
            }
            if (m_blockerVehicleManager) {
                m_blockerVehicleManager->update(kSimulationStepSeconds,
                                                runCoinsBeforeUpdate,
                                                remainingSeconds,
                                                m_sessionElapsedMs);
            }
            refreshCoinChallengeHazardVisuals();
            if (m_collectibleManager) {
                m_collectibleManager->setBlockedAreas(currentCoinChallengeBlockedAreas());
            }
            updateCoinChallengeLastSafePosition();
            applyCoinChallengeHazardCollision(positionBeforeUpdate);
            if (m_coinChallengeForcedEndActive) {
                updateCoinChallengeHud();
                if (m_gameView) {
                    m_gameView->update();
                }
                return;
            }
            if (m_collectibleManager) {
                m_collectibleManager->setTargetActiveCoinCount(coinChallengeDesiredActiveCoins());
            }
            updateHUD(currentSpeedKmh, QStringLiteral("Driving"));

            if (!m_coinChallengeTenSecondWarningShown && m_coinChallengeRemainingMs > 0 && m_coinChallengeRemainingMs <= 10000) {
                m_coinChallengeTenSecondWarningShown = true;
                m_coinChallengeWarningStartedMs = QDateTime::currentMSecsSinceEpoch();
                playSound(PhantomDrive::SoundEffect::FinalLap);
            }

            if (m_coinChallengeRemainingMs > 0 && remainingSeconds > 0 && remainingSeconds <= 3) {
                if (remainingSeconds != m_coinChallengeFinalCountdownAnnouncedSecond) {
                    m_coinChallengeFinalCountdownAnnouncedSecond = remainingSeconds;
                    playSound(coinChallengeCountdownSoundForStage(remainingSeconds));
                }
            } else if (remainingSeconds > 3) {
                m_coinChallengeFinalCountdownAnnouncedSecond = -1;
            }

            const QRectF playerCollectRect = playerCoinCollectRect();
            bool coinsChanged = false;
            if (m_collectibleManager && m_currencyManager) {
                coinsChanged = m_collectibleManager->update(playerCollectRect,
                                                            m_playerPosition.toPointF(),
                                                            kSimulationStepSeconds,
                                                            *m_currencyManager,
                                                            m_coinChallengeMagnetRemainingMs > 0,
                                                            kCoinChallengeMagnetRadius,
                                                            kCoinChallengeMagnetPullSpeed);
            }
            const QList<CoinItem> collectedCoins = m_collectibleManager
                ? m_collectibleManager->collectedCoinsThisFrame()
                : QList<CoinItem>();
            const bool collectedCoin = m_collectibleManager
                && m_collectibleManager->runCoins() > runCoinsBeforeUpdate;
            const int runCoinsAfterUpdate = m_collectibleManager ? m_collectibleManager->runCoins() : runCoinsBeforeUpdate;

            if (coinsChanged) {
                if (m_gameView && m_collectibleManager) {
                    m_gameView->updateCoins(m_collectibleManager->coins());
                }
            }

            if (collectedCoin) {
                if (m_gameView) {
                    for (const CoinItem& coin : collectedCoins) {
                        m_gameView->triggerCoinGoalAnimation(coin.position, coin.value);
                    }
                }
                handleCoinChallengeMilestones(runCoinsBeforeUpdate, runCoinsAfterUpdate);
                playSound(PhantomDrive::SoundEffect::PowerupCollect);
                refreshGaragePage();
                savePlayerProfile();
            }

            maybeSpawnCoinChallengeMagnet();
            maybeSpawnCoinChallengeBalloonRush();
            updateCoinChallengeHud();

            if (m_coinChallengeRemainingMs <= 0) {
                showCoinChallengeSummaryOverlay();
            }
            return;
        }

        if (m_twoPlayerMode && m_vehiclePhysics && m_player2Physics) {
            static bool playersInContact = false;
            static qint64 lastTwoPlayerImpactMs = -kVehicleImpactVisualCooldownMs;

            if (m_vehiclePhysics->isInvisible() || m_player2Physics->isInvisible()) {
                playersInContact = false;
            } else {
                QVector2D player1Pos = m_vehiclePhysics->getPosition();
                QVector2D player2Pos = m_player2Physics->getPosition();
                QVector2D delta = player1Pos - player2Pos;
                qreal dist = delta.length();
                const bool wasInContactBeforeSeparation = playersInContact;
                const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();

                if (dist >= kVehicleContactReleaseDistance) {
                    playersInContact = false;
                }

                bool positionsAdjusted = false;
                if (dist < kVehicleSeparationDistance) {
                    const QVector2D dir = dist > 0.001 ? delta / dist : QVector2D(1.0, 0.0);
                    const qreal overlap = kVehicleSeparationDistance - dist;
                    const QVector2D player1New = player1Pos + dir * (overlap * 0.5);
                    const QVector2D player2New = player2Pos - dir * (overlap * 0.5);

                    const bool player1Free = m_vehiclePhysics->isPositionFree(player1New);
                    const bool player2Free = m_player2Physics->isPositionFree(player2New);

                    if (player1Free && player2Free) {
                        m_vehiclePhysics->setPosition(player1New);
                        m_player2Physics->setPosition(player2New);
                        positionsAdjusted = true;
                    } else if (player1Free) {
                        const QVector2D pushedPlayer1 = player1Pos + dir * overlap;
                        if (m_vehiclePhysics->isPositionFree(pushedPlayer1)) {
                            m_vehiclePhysics->setPosition(pushedPlayer1);
                            positionsAdjusted = true;
                        }
                    } else if (player2Free) {
                        const QVector2D pushedPlayer2 = player2Pos - dir * overlap;
                        if (m_player2Physics->isPositionFree(pushedPlayer2)) {
                            m_player2Physics->setPosition(pushedPlayer2);
                            positionsAdjusted = true;
                        }
                    }
                }

                player1Pos = m_vehiclePhysics->getPosition();
                player2Pos = m_player2Physics->getPosition();
                delta = player1Pos - player2Pos;
                dist = delta.length();

                const bool inContact = dist < kVehicleImpactDistance;
                if (inContact
                    && !wasInContactBeforeSeparation
                    && dist > 0.001) {
                    const QVector2D normal = delta / dist;
                    const qreal impactForce = qAbs(m_playerSpeed - m_player2Speed);

                    if (m_gameView
                        && nowMs - lastTwoPlayerImpactMs >= kVehicleImpactVisualCooldownMs
                        && !shouldSuppressStartupCollisionImpact(m_driveActive,
                                                                 m_countdownActive,
                                                                 m_sessionElapsedMs)) {
                        m_gameView->showCollisionImpact((player1Pos + player2Pos) * 0.5,
                                                        qBound<qreal>(0.6, impactForce / 80.0, 1.8));
                    }
                    if (!m_vehiclePhysics->isColliding()) {
                        m_vehiclePhysics->handleCollision(normal, impactForce);
                    }
                    if (!m_player2Physics->isColliding()) {
                        m_player2Physics->handleCollision(-normal, impactForce);
                    }
                    lastTwoPlayerImpactMs = nowMs;
                    playersInContact = true;
                } else if (inContact) {
                    playersInContact = true;
                }

                m_playerPosition = m_vehiclePhysics->getPosition();
                m_player2Position = m_player2Physics->getPosition();
                m_playerRotation = m_vehiclePhysics->getRotation();
                m_player2Rotation = m_player2Physics->getRotation();
                m_playerSpeed = m_vehiclePhysics->getSpeed();
                m_player2Speed = m_player2Physics->getSpeed();

                if ((positionsAdjusted || inContact) && m_gameView) {
                    m_gameView->updatePlayerCar(QStringLiteral("P1"),
                                                m_playerPosition,
                                                m_playerRotation,
                                                displaySpeedKmh(),
                                                currentPlayerSkinColor(),
                                                m_vehiclePhysics->isSpeedBoostActive(),
                                                m_vehiclePhysics->isShieldActive(),
                                                m_vehiclePhysics->isInvisible(),
                                                m_vehiclePhysics->isMagnetActive(),
                                                currentPlayerSkinId());
                    m_gameView->updatePlayerCar(QStringLiteral("P2"),
                                                m_player2Position,
                                                m_player2Rotation,
                                                speedToDisplayKmh(m_player2Speed),
                                                QColor(40, 220, 255));
                    updateTwoPlayerCamera();
                }
            }
        }

        if (m_aiManager && m_driveActive) {
            m_aiManager->setPlayerPosition(m_playerPosition);
            m_aiManager->update(kSimulationStepMs);

            QList<AIOpponent*> opponents = m_aiManager->getAllOpponents();

            for (AIOpponent* ai : opponents) {
                if (ai && m_gameView) {
                    resolvePlayerAiVehicleContact(ai);

                    m_gameView->updateAICar(
                        ai->getId(),
                        ai->getPosition(),
                        ai->getRotation(),
                        speedToDisplayKmh(ai->getSpeed())
                    );
                }
            }
        }

        if (m_drivingDataCollector && m_drivingDataCollector->vehicleSensor()) {
            VehicleSensor* sensor = m_drivingDataCollector->vehicleSensor();
            sensor->updatePosition(m_playerPosition);
            qreal rad = m_playerRotation * 3.14159265 / 180.0;
            const qreal displaySpeed = displaySpeedKmh();
            QVector2D velocity(std::cos(rad) * displaySpeed, std::sin(rad) * displaySpeed);
            sensor->updateVelocity(velocity);
            sensor->updateRotation(m_playerRotation);
            sensor->updateSpeedLimit(m_currentSpeedLimit, "main_route_speed_zone");
            sensor->updateAcceleratorState(displaySpeed >= m_drivingDataCollector->getCurrentData().speed);
            sensor->updateBrakeState(displaySpeed < m_drivingDataCollector->getCurrentData().speed);
        }
        if (m_twoPlayerMode && m_player2DataCollector && m_player2DataCollector->vehicleSensor()) {
            VehicleSensor* sensor = m_player2DataCollector->vehicleSensor();
            sensor->updatePosition(m_player2Position);
            qreal rad = m_player2Rotation * 3.14159265 / 180.0;
            const qreal displaySpeed = speedToDisplayKmh(m_player2Speed);
            QVector2D velocity(std::cos(rad) * displaySpeed, std::sin(rad) * displaySpeed);
            sensor->updateVelocity(velocity);
            sensor->updateRotation(m_player2Rotation);
            sensor->updateSpeedLimit(m_currentSpeedLimit, "main_route_speed_zone");
            sensor->updateAcceleratorState(displaySpeed >= m_player2DataCollector->getCurrentData().speed);
            sensor->updateBrakeState(displaySpeed < m_player2DataCollector->getCurrentData().speed);
        }

        if (m_reportWidget && m_reportWidget->isVisible()) {
            m_reportWidget->addSpeedData(displaySpeedKmh(), m_simTick);
        }

        if (m_scoreManager) {
            m_scoreManager->recordSafeDrivingTick(QDateTime::currentMSecsSinceEpoch(), displaySpeedKmh());
        }

        updateRaceHud();
    });

    m_simTimer->start(kSimulationStepMs);
}

void MainWindow::startDrivingSession(const QString& mode)
{
    if (m_driveActive) {
        silentFinishSession();
    }
    clearTransientDrivingFeedback();

    // Stop any previous learning session timer.
    if (m_learningSessionTimer) {
        m_learningSessionTimer->stop();
        m_learningSessionTimer->deleteLater();
        m_learningSessionTimer = nullptr;
    }

    m_currentMode = mode;
    m_twoPlayerMode = (mode == QStringLiteral("Arcade")) && (m_twoPlayerMode || isTwoPlayerSelected());
    m_twoPlayerFinishHandled = false;
    m_player1Finished = false;
    m_player2Finished = false;
    m_arcadeRaceLogicActive = (mode == QStringLiteral("Arcade"));
    m_customTrackPlaying = false;
    m_driveActive = true;
    m_arcadeRaceFinished = false;
    m_lapsCompleted = 0;
    m_totalLaps = 3;
    m_simTick = 0;
    m_sessionElapsedMs = 0;
    m_currentLapStartMs = 0;
    m_bestLapMs = 0;
    m_playerSpeed = 0.0;
    m_player2Speed = 0.0;
    m_player2LapsCompleted = 0;
    m_player2NextCheckpointIndex = 0;
    m_player2WasInsideNextGate = false;

    // Learning mode always uses its canonical default track. Never inherit a
    // circuit selected by Arcade, Two-Player, or Coin Challenge.
    if (mode == QStringLiteral("Learning")) {
        if (m_selectedBuiltInTrack) {
            m_selectedBuiltInTrack->deleteLater();
            m_selectedBuiltInTrack = nullptr;
        }
        m_selectedBuiltInTrack =
            BuiltInTrackFactory::createTrack(QStringLiteral("neon_loop"), this);
        m_defaultRaceTrack = m_selectedBuiltInTrack;
        TrackManager* trackMgr = TrackManager::instance(this);
        if (trackMgr && m_defaultRaceTrack) {
            trackMgr->setCurrentTrack(m_defaultRaceTrack);
        }
    }

    if (m_gameView) {
        m_gameView->clearAllAICars();
    }

    preparePlayerReportSystems();

    ui->stackedWidget->setCurrentIndex(1);
    if (m_customTrackEditor) {
        m_customTrackEditor->hide();
    }
    setGameHeaderVisible(true);
    updatePauseButtonState();
    if (m_btnFinishDrive) {
        m_btnFinishDrive->show();
        m_btnFinishDrive->setEnabled(false);
    }
    if (m_gameView) {
        m_gameView->show();
    }

    restoreDefaultRaceTrack();
    focusGameViewForDriving();

    // Update the top HUD mode title label
    if (ui->label_ModeTitle) {
        if (mode == "Learning") {
            ui->label_ModeTitle->setText("LEARNING MODE");
            ui->label_ModeTitle->setStyleSheet(
                "QLabel{color:#00FFA0;font-size:13px;font-weight:bold;letter-spacing:3px;}");
        } else if (mode == "Custom Track") {
            ui->label_ModeTitle->setText("CUSTOM TRACK");
            ui->label_ModeTitle->setStyleSheet(
                "QLabel{color:#59F7FF;font-size:13px;font-weight:bold;letter-spacing:3px;}");
        } else {
            ui->label_ModeTitle->setText("ARCADE MODE");
            ui->label_ModeTitle->setStyleSheet(
                "QLabel{color:#FF3366;font-size:13px;font-weight:bold;letter-spacing:3px;}");
        }
    }

    // Show the right-side HUD in ALL driving modes (Arcade, Learning, Custom Track)
    if (m_arcadeHUD) {
        m_arcadeHUD->setGameMode(mode);
        m_arcadeHUD->reset();
        m_arcadeHUD->setTwoPlayerMode(m_twoPlayerMode);
        m_arcadeHUD->show();

        // Position the HUD panel on the right side of the game canvas.
        // ArcadeHUD uses minimumHeight to size itself naturally; no setFixedHeight needed.
        QWidget* gamePage = ui->stackedWidget ? ui->stackedWidget->widget(1) : nullptr;
        if (gamePage && m_gameView) {
            const int hw = 300;
            const int hudBarHeight = 42;
            const int hx = gamePage->width() - hw - 8;
            const int hy = hudBarHeight + 8;
            // Constrain HUD height to fit inside gamePage, but allow it to be shorter.
            const int maxH = gamePage->height() - hy - 8;
            m_arcadeHUD->setMaximumHeight(maxH);
            m_arcadeHUD->move(hx, hy);
            m_arcadeHUD->raise();
        }
    }
    // Hide the old LearningHUD (replaced by ArcadeHUD)
    if (m_learningHUD) {
        m_learningHUD->hide();
    }
    updateRaceHud();
    setupEBRuntimeObjects();

    if (m_vehiclePhysics) {
        m_vehiclePhysics->reset();
        m_vehiclePhysics->resetRaceProgress();
        m_vehiclePhysics->setRaceLogicEnabled(false);
    }
    if (m_player2Physics) {
        m_player2Physics->reset();
        m_player2Physics->resetRaceProgress();
        m_player2Physics->setRaceLogicEnabled(false);
    }
    applyPlayerSpawnAtStartLine();
    if (m_twoPlayerMode) {
        applyPlayer2SpawnAtStartLine();
        if (m_gameView) {
            m_gameView->updatePlayerCar(QStringLiteral("P2"),
                                        m_player2Position,
                                        m_player2Rotation,
                                        speedToDisplayKmh(m_player2Speed),
                                        QColor(40, 220, 255));
            updateTwoPlayerCamera();
        }
    }
    resetArcadeRaceProgress();

    focusGameViewForDriving();
    initializeAIOpponents();

    m_countdownActive = true;
    showCountdown();

    // Learning Mode: auto-finish after 5 minutes so the report always appears.
    if (mode == QStringLiteral("Learning")) {
        constexpr int kLearningMaxMs = 5 * 60 * 1000;
        m_learningSessionTimer = new QTimer(this);
        m_learningSessionTimer->setSingleShot(true);
        connect(m_learningSessionTimer, &QTimer::timeout, this, [this]() {
            if (m_driveActive && m_currentMode == QStringLiteral("Learning")) {
                showInteractiveFeedback(QStringLiteral("Learning Session Complete!"), FeedbackType::Milestone);
                playSound(PhantomDrive::SoundEffect::RaceFinish);
                QTimer::singleShot(1500, this, [this]() {
                    if (m_driveActive) {
                        onGameFinished();
                    }
                });
            }
        });
        m_learningTimerRemainingMs = kLearningMaxMs;
        m_learningTimerStartedMs = QDateTime::currentMSecsSinceEpoch();
        m_learningSessionTimer->start(kLearningMaxMs);
    }

    statusBar()->showMessage(QStringLiteral("%1 mode running").arg(m_currentMode));
}

void MainWindow::startBuiltInTrackSession(const QString& mode)
{
    const QString trackId = m_trackSelectCombo
        ? m_trackSelectCombo->currentData().toString()
        : m_selectedTrackId;
    m_selectedTrackId = trackId.isEmpty() ? QStringLiteral("neon_loop") : trackId;

    if (m_selectedBuiltInTrack) {
        m_selectedBuiltInTrack->deleteLater();
        m_selectedBuiltInTrack = nullptr;
    }
    m_selectedBuiltInTrack = BuiltInTrackFactory::createTrack(m_selectedTrackId, this);
    m_defaultRaceTrack = m_selectedBuiltInTrack;
    TrackManager* trackMgr = TrackManager::instance(this);
    if (trackMgr && m_defaultRaceTrack) {
        trackMgr->setCurrentTrack(m_defaultRaceTrack);
    }

    startDrivingSession(mode);
}

void MainWindow::onGameFinished()
{
    if (isCoinChallengeModeActive()) {
        if (!m_coinChallengeSummaryVisible && !m_coinChallengeForcedEndActive) {
            m_coinChallengeEndReason = CoinChallengeEndReason::ForcedFinish;
            m_coinChallengeEndTitle = QStringLiteral("Forced Finish");
            m_coinChallengeEndDetail = QStringLiteral("Run ended manually. Banking collected coins.");
        }
        showCoinChallengeSummaryOverlay();
        return;
    }

    if (!m_driveActive || !m_scoreManager || !m_drivingDataCollector) {
        return;
    }

    m_driveActive = false;
    clearTransientDrivingFeedback();

    // Cancel the learning session auto-end timer if it is still running.
    if (m_learningSessionTimer) {
        m_learningSessionTimer->stop();
        m_learningSessionTimer->deleteLater();
        m_learningSessionTimer = nullptr;
    }

    m_drivingDataCollector->stopCollection();
    if (m_player2DataCollector) {
        m_player2DataCollector->stopCollection();
    }
    if (m_btnFinishDrive) {
        m_btnFinishDrive->setEnabled(false);
    }
    // Disable btn_Back while the report page is shown so it cannot accidentally
    // navigate away before the user has seen the report.
    if (ui->btn_Back) {
        ui->btn_Back->setEnabled(false);
    }

    if (m_learningHUD) {
        m_learningHUD->hide();
    }
    if (m_arcadeHUD) {
        m_arcadeHUD->hide();
    }
    if (m_gameView) {
        m_gameView->hide();
    }
    setGameHeaderVisible(false);

    // Grab all data synchronously before anything can mutate the collector.
    const QList<DrivingData> speedSamples = m_drivingDataCollector->getCollectedData();
    const ScoreReport report = m_scoreManager->finishSession(m_drivingDataCollector);
    QList<DrivingData> player2SpeedSamples;
    ScoreReport player2Report;
    if (m_twoPlayerMode && m_player2DataCollector && m_player2ScoreManager) {
        player2SpeedSamples = m_player2DataCollector->getCollectedData();
        player2Report = m_player2ScoreManager->finishSession(m_player2DataCollector);
    }

    // Populate the report widget and switch to the report page.
    if (!m_reportWidget || !m_reportPage) {
        qWarning() << "[onGameFinished] m_reportWidget or m_reportPage is null – report page not set up"
                   << "m_reportWidget=" << m_reportWidget
                   << "m_reportPage=" << m_reportPage;
        // Still clean up and return to menu to avoid getting stuck
        m_driveActive = false;
        if (ui->stackedWidget && ui->stackedWidget->count() > 0) {
            ui->stackedWidget->setCurrentIndex(0);
        }
        if (ui->btn_Back) {
            ui->btn_Back->setEnabled(true);
        }
        return;
    }

    m_reportWidget->hideLoading();

    m_reportWidget->loadHistoryFromSaveLoadManager();

    if (m_twoPlayerMode && !player2Report.sessionId.isEmpty()) {
        m_reportWidget->setPlayerReports(report, speedSamples, player2Report, player2SpeedSamples);
    } else {
        m_reportWidget->setSessionSpeedSamples(speedSamples);
        m_reportWidget->setCurrentReport(report);
    }

    ui->stackedWidget->setCurrentWidget(m_reportPage);
    m_reportPage->show();
    m_reportPage->raise();
    m_reportWidget->show();
    m_reportWidget->raise();
    ui->stackedWidget->update();
    // Re-enable btn_Back so the user can return to menu from the report page.
    if (ui->btn_Back) {
        ui->btn_Back->setEnabled(true);
    }

    playSound(PhantomDrive::SoundEffect::Victory);
    // Report is now displayed in the report widget — no statusBar needed.
}

void MainWindow::finishDrivingSession()
{
    onGameFinished();
}

void MainWindow::silentFinishSession()
{
    // End the current session without showing the report panel.
    // Used when the player starts a new game while one is already running.
    if (!m_driveActive) {
        return;
    }
    if (isCoinChallengeModeActive()) {
        finishCoinChallengeMode();
        return;
    }
    m_driveActive = false;
    clearTransientDrivingFeedback();

    if (m_learningSessionTimer) {
        m_learningSessionTimer->stop();
        m_learningSessionTimer->deleteLater();
        m_learningSessionTimer = nullptr;
    }

    if (m_drivingDataCollector) {
        m_drivingDataCollector->stopCollection();
    }
    if (m_btnFinishDrive) {
        m_btnFinishDrive->setEnabled(false);
    }
    // Restore btn_Back so the next session can use it normally.
    if (ui->btn_Back) {
        ui->btn_Back->setEnabled(true);
    }
    if (m_learningHUD) {
        m_learningHUD->hide();
    }
    if (m_arcadeHUD) {
        m_arcadeHUD->hide();
    }
    if (m_gameView) {
        m_gameView->hide();
    }
    setGameHeaderVisible(false);
    if (m_scoreManager && m_drivingDataCollector) {
        m_scoreManager->finishSession(m_drivingDataCollector);
    }
    if (m_twoPlayerMode && m_player2ScoreManager && m_player2DataCollector) {
        m_player2ScoreManager->finishSession(m_player2DataCollector);
    }
}

void MainWindow::showReportWindow(const ScoreReport* report)
{
    if (!m_reportWidget) {
        qWarning() << "[showReportWindow] m_reportWidget is null";
        return;
    }
    m_reportWidget->hideLoading();
    m_reportWidget->loadHistoryFromSaveLoadManager();

    if (report) {
        if (m_drivingDataCollector) {
            m_reportWidget->setSessionSpeedSamples(m_drivingDataCollector->getCollectedData());
        }
        m_reportWidget->setCurrentReport(*report);
    } else {
        const ScoreReport latest = m_scoreManager ? m_scoreManager->latestReport() : ScoreReport();
        if (!latest.sessionId.isEmpty()) {
            if (m_drivingDataCollector) {
                m_reportWidget->setSessionSpeedSamples(m_drivingDataCollector->getCollectedData());
            }
            m_reportWidget->setCurrentReport(latest);
        }
    }

    QWidget* reportPage = m_reportPage ? m_reportPage
                                       : (ui->pageReport ? static_cast<QWidget*>(ui->pageReport)
                                                         : ui->stackedWidget->widget(2));
    ui->stackedWidget->setCurrentWidget(reportPage);
    reportPage->show();
    reportPage->raise();
    m_reportWidget->show();
    m_reportWidget->raise();
}

void MainWindow::fadeInReportWindow()
{
    QWidget* reportPage = m_reportPage ? m_reportPage
                                       : (ui->pageReport ? static_cast<QWidget*>(ui->pageReport)
                                                         : ui->stackedWidget->widget(2));
    ui->stackedWidget->setCurrentWidget(reportPage);
    reportPage->show();
    reportPage->raise();
    if (m_reportWidget) {
        m_reportWidget->show();
        m_reportWidget->raise();
    }
}

void MainWindow::onReportBackToMenu()
{
    if (m_coinChallengeHudLabel) {
        m_coinChallengeHudLabel->hide();
    }
    hideCoinChallengeSummaryOverlay();
    if (m_gameView) {
        m_gameView->clearCoinChallengeOverlayState();
        m_gameView->hide();
    }
    if (m_learningHUD) {
        m_learningHUD->hide();
    }
    if (m_arcadeHUD) {
        m_arcadeHUD->hide();
    }
    setGameHeaderVisible(false);
    // Re-enable btn_Back now that we are returning to the menu.
    if (ui->btn_Back) {
        ui->btn_Back->setEnabled(true);
    }
    ui->stackedWidget->setCurrentIndex(0);
    statusBar()->clearMessage();
}

void MainWindow::onReportNewDrive()
{
    // Re-enable btn_Back before starting a new session.
    if (ui->btn_Back) {
        ui->btn_Back->setEnabled(true);
    }
    const QString mode = m_currentMode.isEmpty() ? QStringLiteral("Arcade") : m_currentMode;
    if (mode == QStringLiteral("Coin Challenge")) {
        startCoinChallengeMode();
    } else if (mode == QStringLiteral("Custom Track") && m_runtimeCustomTrack) {
        startCustomTrackSession(m_runtimeCustomTrack);
    } else {
        startDrivingSession(mode);
    }
}

void MainWindow::updateTrafficAndHud(int tick)
{
    // LearningHUD is fully deprecated; all HUD data goes to ArcadeHUD.
}

bool MainWindow::isCoinChallengeModeActive() const
{
    return m_coinChallengeActive;
}

QRectF MainWindow::playerCoinCollectRect() const
{
    constexpr qreal width = 36.0;
    constexpr qreal height = 56.0;
    return QRectF(m_playerPosition.x() - width / 2.0,
                  m_playerPosition.y() - height / 2.0,
                  width,
                  height);
}

void MainWindow::resetCoinChallengeStats()
{
    m_coinChallengeSummaryVisible = false;
    m_coinChallengeTenSecondWarningShown = false;
    m_coinChallengeCountdownAnnouncedStage = -1;
    m_coinChallengeMaxSpeedKmh = 0;
    m_coinChallengeLastAverageSpeedKmh = 0;
    m_coinChallengeLastEfficiencyCpm = 0;
    m_coinChallengeLastRunCoins = 0;
    m_coinChallengeWarningStartedMs = 0;
    m_coinChallengeSpeedSampleSumKmh = 0;
    m_coinChallengeSpeedSampleCount = 0;
    m_coinChallengeLastElapsedMs = 0;
    m_coinChallengeMagnetRemainingMs = 0;
    m_coinChallengeBalloonRushRemainingMs = 0;
    m_coinChallengeBalloonRushTriggerRemainingMs = 0;
    m_coinChallengeBalloonRushReturnRemainingMs = 0;
    m_coinChallengeBalloonRushIntroCountdownRemainingMs = 0;
    m_coinChallengeBalloonRushIntroCountdownAnnounced = -1;
    m_coinChallengeBalloonRushCollectedCoins = 0;
    m_coinChallengeBalloonRushRecentGainValue = 0;
    m_coinChallengeBalloonRushGainFlashRemainingMs = 0;
    m_coinChallengeBalloonRushMilestoneRemainingMs = 0;
    m_coinChallengeBalloonRushMilestoneDurationMs = 0;
    m_coinChallengePowerupsUsed = 0;
    m_coinChallengeLoopsCompleted = 0;
    m_coinChallengeLoopCheckpointIndex = 0;
    m_coinChallengeMagnetSpawnStage = 0;
    m_coinChallengeFinalCountdownAnnouncedSecond = -1;
    m_coinChallengeSummaryAnimatedRunCoins = 0;
    m_coinChallengeSummaryAnimatedTotalCoins = 0;
    m_coinChallengeSummaryTargetRunCoins = 0;
    m_coinChallengeSummaryTargetTotalCoins = 0;
    m_coinChallengeSummaryDepositSoundStage = 0;
    m_coinChallengeVehicleIntegrity = kCoinChallengeIntegrityMax;
    m_coinChallengeObstacleHits = 0;
    m_coinChallengeAICollisionCount = 0;
    m_coinChallengeDamageTaken = 0;
    m_coinChallengeRecentDamageAmount = 0;
    m_coinChallengeDamageFlashRemainingMs = 0;
    m_coinChallengeDamagePopupRemainingMs = 0;
    m_coinChallengeLowIntegrityWarningRemainingMs = 0;
    m_coinChallengeLeftStartZone = false;
    m_coinChallengeWasOnStartZone = false;
    m_coinChallengeBalloonRushSpawned = false;
    m_coinChallengeMagnetLoopPlaying = false;
    m_coinChallengeLastHazardSoundMs = -1000;
    m_coinChallengeLastDamageMs = -1000;
    m_coinChallengeForcedEndActive = false;
    m_coinChallengeForcedEndRemainingMs = 0;
    m_coinChallengeEndReason = CoinChallengeEndReason::None;
    m_coinChallengeDamagePopupText.clear();
    m_coinChallengeDamagePopupDetail.clear();
    m_coinChallengeEndTitle.clear();
    m_coinChallengeEndDetail.clear();
    m_coinChallengeBlockerDamageContacts.clear();
    m_coinChallengeObstacleDamageContacts.clear();
    m_coinChallengeLastSafePlayerPosition = QVector2D(0.0f, 0.0f);
    m_coinChallengeLastSafePlayerRotation = 0.0;
    m_coinChallengeHasSafePlayerPosition = false;
    m_coinChallengeMagnetSpawnPool.clear();
    m_coinChallengeBalloonRushSpawnPool.clear();
    m_coinChallengeBalloonRushPhase = CoinChallengeBalloonRushPhase::Inactive;
    m_coinChallengeBalloonRushSavedPosition = QVector2D(0.0f, 0.0f);
    m_coinChallengeBalloonRushSavedRotation = 0.0;
    m_coinChallengeBalloonRushSavedSpeed = 0.0;
    m_coinChallengeBalloonRushLaneIndex = 1;
    m_coinChallengeBalloonRushLaneVisual = 1.0;
    m_coinChallengeBalloonRushRoadScroll = 0.0;
    m_coinChallengeBalloonRushIntroCountdownRemainingMs = 0;
    m_coinChallengeBalloonRushIntroCountdownAnnounced = -1;
    m_coinChallengeBalloonRushSpawnAccumulatorMs = 0.0;
    m_coinChallengeBalloonRushPatternLane = 1;
    m_coinChallengeBalloonRushPatternBatchCount = 0;
    m_coinChallengeBalloonRushRecentSegmentLanes.clear();
    m_coinChallengeBalloonRushVisitedLanes.clear();
    m_coinChallengeBalloonRushCoinLayout.clear();
    m_coinChallengeBalloonRushMilestoneHeadline.clear();
    m_coinChallengeBalloonRushMilestoneDetail.clear();
    m_coinChallengeBalloonRushMilestoneAccent = QColor();
    m_coinChallengeTriggeredMilestones.clear();
}

void MainWindow::updateCoinChallengeStats(int currentSpeedKmh)
{
    if (!m_coinChallengeActive || m_countdownActive) {
        return;
    }

    const int speedKmh = qMax(0, currentSpeedKmh);
    m_coinChallengeSpeedSampleSumKmh += speedKmh;
    ++m_coinChallengeSpeedSampleCount;
    m_coinChallengeMaxSpeedKmh = qMax(m_coinChallengeMaxSpeedKmh, speedKmh);
    m_coinChallengeLastElapsedMs = qMax<qint64>(m_coinChallengeLastElapsedMs, m_sessionElapsedMs);
    m_coinChallengeLastAverageSpeedKmh = coinChallengeAverageSpeedKmh();
    m_coinChallengeLastEfficiencyCpm = coinChallengeEfficiencyCpm();
    m_coinChallengeLastRunCoins = m_collectibleManager ? m_collectibleManager->runCoins() : 0;
}

int MainWindow::coinChallengeAverageSpeedKmh() const
{
    if (m_coinChallengeSpeedSampleCount <= 0) {
        return m_coinChallengeLastAverageSpeedKmh;
    }

    return qMax(0, qRound(static_cast<qreal>(m_coinChallengeSpeedSampleSumKmh)
                          / static_cast<qreal>(m_coinChallengeSpeedSampleCount)));
}

int MainWindow::coinChallengeMaxSpeedDisplayKmh() const
{
    return qMax(m_coinChallengeMaxSpeedKmh, 0);
}

int MainWindow::coinChallengeEfficiencyCpm() const
{
    const qint64 elapsedMs = m_coinChallengeLastElapsedMs > 0 ? m_coinChallengeLastElapsedMs : m_sessionElapsedMs;
    if (elapsedMs <= 0) {
        return m_coinChallengeLastEfficiencyCpm;
    }

    const int runCoins = m_collectibleManager ? m_collectibleManager->runCoins() : m_coinChallengeLastRunCoins;
    const qreal elapsedSeconds = static_cast<qreal>(elapsedMs) / 1000.0;
    if (elapsedSeconds <= 0.001) {
        return m_coinChallengeLastEfficiencyCpm;
    }

    return qMax(0, qRound((static_cast<qreal>(runCoins) / elapsedSeconds) * 60.0));
}

qreal MainWindow::coinChallengeGoalProgress() const
{
    const int goalCoins = qMax(1, m_coinChallengeGoalCoins);
    const int runCoins = m_collectibleManager ? m_collectibleManager->runCoins() : m_coinChallengeLastRunCoins;
    return qBound<qreal>(0.0,
                         static_cast<qreal>(runCoins) / static_cast<qreal>(goalCoins),
                         1.0);
}

int MainWindow::coinChallengeDesiredActiveCoins() const
{
    const int averageSpeedKmh = coinChallengeAverageSpeedKmh();
    return qBound(12, 12 + qMax(0, averageSpeedKmh - 35) / 25, 16);
}

int MainWindow::coinChallengeCountdownStage() const
{
    if (!m_countdownActive) {
        return -1;
    }

    if (m_countdownRemainingMs > 2200) {
        return 3;
    }
    if (m_countdownRemainingMs > 1200) {
        return 2;
    }
    if (m_countdownRemainingMs > kCoinChallengeCountdownGoWindowMs) {
        return 1;
    }
    return 0;
}

qreal MainWindow::coinChallengeCountdownStageProgress() const
{
    if (!m_countdownActive) {
        return 1.0;
    }

    const int stage = coinChallengeCountdownStage();
    if (stage == 3) {
        return qBound<qreal>(0.0,
                             static_cast<qreal>(3200 - m_countdownRemainingMs) / 1000.0,
                             1.0);
    }
    if (stage == 2) {
        return qBound<qreal>(0.0,
                             static_cast<qreal>(2200 - m_countdownRemainingMs) / 1000.0,
                             1.0);
    }
    if (stage == 1) {
        return qBound<qreal>(0.0,
                             static_cast<qreal>(1200 - m_countdownRemainingMs) / 850.0,
                             1.0);
    }
    return qBound<qreal>(0.0,
                         static_cast<qreal>(kCoinChallengeCountdownGoWindowMs - m_countdownRemainingMs)
                             / static_cast<qreal>(kCoinChallengeCountdownGoWindowMs),
                         1.0);
}

int MainWindow::coinChallengeIntegrityStage() const
{
    if (m_coinChallengeVehicleIntegrity <= 0) {
        return 4;
    }
    if (m_coinChallengeVehicleIntegrity <= kCoinChallengeCriticalIntegrityThreshold) {
        return 3;
    }
    if (m_coinChallengeVehicleIntegrity <= 50) {
        return 2;
    }
    if (m_coinChallengeVehicleIntegrity <= 75) {
        return 1;
    }
    return 0;
}

void MainWindow::applyCoinChallengeIntegrityDamage(int amount,
                                                   const QString& popupText,
                                                   const QString& popupDetail)
{
    if (!m_coinChallengeActive || m_coinChallengeForcedEndActive || amount <= 0) {
        return;
    }

    const int previousIntegrity = qBound(0, m_coinChallengeVehicleIntegrity, kCoinChallengeIntegrityMax);
    m_coinChallengeVehicleIntegrity = qMax(0, previousIntegrity - amount);
    const int appliedDamage = previousIntegrity - m_coinChallengeVehicleIntegrity;
    if (appliedDamage <= 0) {
        return;
    }

    m_coinChallengeDamageTaken += appliedDamage;
    m_coinChallengeRecentDamageAmount = appliedDamage;
    m_coinChallengeDamageFlashRemainingMs = kCoinChallengeDamageFlashDurationMs;
    m_coinChallengeDamagePopupRemainingMs = kCoinChallengeDamagePopupDurationMs;
    m_coinChallengeDamagePopupText = popupText.isEmpty()
        ? QStringLiteral("-%1").arg(appliedDamage)
        : popupText;
    m_coinChallengeDamagePopupDetail = popupDetail.isEmpty()
        ? coinChallengeIntegrityStatusLabel(m_coinChallengeVehicleIntegrity)
        : popupDetail;

    const bool enteredCriticalIntegrity = previousIntegrity > kCoinChallengeCriticalIntegrityThreshold
        && m_coinChallengeVehicleIntegrity > 0
        && m_coinChallengeVehicleIntegrity <= kCoinChallengeCriticalIntegrityThreshold;
    if (enteredCriticalIntegrity) {
        m_coinChallengeLowIntegrityWarningRemainingMs = kCoinChallengeLowIntegrityWarningDurationMs;
        showInteractiveFeedback(QStringLiteral("INTEGRITY CRITICAL"), PhantomDrive::FeedbackType::Critical);
    }

    if (m_coinChallengeVehicleIntegrity <= 0) {
        triggerCoinChallengeForcedEnd(CoinChallengeEndReason::VehicleDestroyed,
                                      QStringLiteral("Vehicle Destroyed"),
                                      QStringLiteral("Integrity dropped to 0%. Tow truck dispatched."));
    }
}

void MainWindow::triggerCoinChallengeForcedEnd(CoinChallengeEndReason reason,
                                               const QString& title,
                                               const QString& detail)
{
    if (!m_coinChallengeActive || m_coinChallengeSummaryVisible || m_coinChallengeForcedEndActive) {
        return;
    }

    m_coinChallengeForcedEndActive = true;
    m_coinChallengeForcedEndRemainingMs = kCoinChallengeForcedEndDurationMs;
    m_coinChallengeEndReason = reason;
    m_coinChallengeEndTitle = title;
    m_coinChallengeEndDetail = detail;
    m_coinChallengeDamageFlashRemainingMs = qMax(m_coinChallengeDamageFlashRemainingMs,
                                                 kCoinChallengeDamageFlashDurationMs);
    m_coinChallengeDamagePopupRemainingMs = qMax(m_coinChallengeDamagePopupRemainingMs,
                                                 kCoinChallengeDamagePopupDurationMs);

    if (reason == CoinChallengeEndReason::FatalCrash || reason == CoinChallengeEndReason::VehicleDestroyed) {
        m_coinChallengeVehicleIntegrity = 0;
    }

    m_coinChallengeMagnetRemainingMs = 0;
    syncCoinChallengeMagnetLoop();

    if (m_vehiclePhysics) {
        m_vehiclePhysics->clearDriveInput();
        m_vehiclePhysics->setSpeed(0.0);
    }

    m_playerSpeed = 0.0;
    if (m_btnPause) {
        m_btnPause->setEnabled(false);
    }
    if (m_btnFinishDrive) {
        m_btnFinishDrive->setEnabled(false);
    }

    playSound(PhantomDrive::SoundEffect::Crash);
    QTimer::singleShot(120, this, [this, reason]() {
        if (!m_coinChallengeForcedEndActive) {
            return;
        }
        playSound(reason == CoinChallengeEndReason::FatalCrash
                      ? PhantomDrive::SoundEffect::Collision
                      : PhantomDrive::SoundEffect::Fail);
    });
}

void MainWindow::updateCoinChallengeForcedEndSequence(int deltaMs)
{
    if (!m_coinChallengeForcedEndActive) {
        return;
    }

    m_coinChallengeForcedEndRemainingMs = qMax(0, m_coinChallengeForcedEndRemainingMs - deltaMs);
    m_coinChallengeDamageFlashRemainingMs = qMax(0, m_coinChallengeDamageFlashRemainingMs - deltaMs);
    m_coinChallengeDamagePopupRemainingMs = qMax(0, m_coinChallengeDamagePopupRemainingMs - deltaMs);
    m_coinChallengeLowIntegrityWarningRemainingMs = qMax(0,
                                                         m_coinChallengeLowIntegrityWarningRemainingMs - deltaMs);

    if (m_vehiclePhysics) {
        m_vehiclePhysics->clearDriveInput();
        m_vehiclePhysics->setSpeed(0.0);
    }
    m_playerSpeed = 0.0;

    if (m_coinChallengeDamagePopupRemainingMs <= 0) {
        m_coinChallengeRecentDamageAmount = 0;
        m_coinChallengeDamagePopupText.clear();
        m_coinChallengeDamagePopupDetail.clear();
    }

    if (m_coinChallengeForcedEndRemainingMs <= 0) {
        m_coinChallengeForcedEndActive = false;
        showCoinChallengeSummaryOverlay();
    }
}

void MainWindow::updateCoinChallengeHud()
{
    const int totalCoins = m_currencyManager ? m_currencyManager->coins() : 0;
    const int runCoins = m_collectibleManager ? m_collectibleManager->runCoins() : m_coinChallengeLastRunCoins;
    const bool balloonRushActive = isCoinChallengeBalloonRushSequenceActive();
    const int integrity = qBound(0, m_coinChallengeVehicleIntegrity, kCoinChallengeIntegrityMax);
    const int activeCoins = balloonRushActive
        ? 0
        : (m_collectibleManager ? m_collectibleManager->activeCoinCount() : 0);
    const int remainingSeconds = qMax(0, static_cast<int>(qCeil(m_coinChallengeRemainingMs / 1000.0)));
    const int countdownStage = coinChallengeCountdownStage();
    const int countdownSeconds = countdownStage > 0 ? countdownStage : 0;
    const int speedKmh = displaySpeedKmh();
    const int averageSpeedKmh = coinChallengeAverageSpeedKmh();
    const int maxSpeedKmh = coinChallengeMaxSpeedDisplayKmh();
    const int efficiencyCpm = coinChallengeEfficiencyCpm();
    const qreal goalProgress = coinChallengeGoalProgress();
    const qreal timeProgress = m_coinChallengeDurationMs > 0
        ? qBound<qreal>(0.0,
                        static_cast<qreal>(m_coinChallengeRemainingMs) / static_cast<qreal>(m_coinChallengeDurationMs),
                        1.0)
        : 0.0;

    CoinChallengeOverlayState overlayState;
    overlayState.visible = m_coinChallengeActive && !m_coinChallengeSummaryVisible && !balloonRushActive;
    overlayState.countdownActive = m_countdownActive;
    overlayState.tenSecondWarningVisible = m_coinChallengeTenSecondWarningShown
        && !m_countdownActive
        && !m_coinChallengeSummaryVisible
        && remainingSeconds > 3
        && m_coinChallengeRemainingMs > 0;
    overlayState.goalComplete = runCoins >= m_coinChallengeGoalCoins;
    overlayState.criticalIntegrity = integrity > 0 && integrity <= kCoinChallengeCriticalIntegrityThreshold;
    overlayState.destroyed = integrity <= 0;
    overlayState.forcedEndActive = m_coinChallengeForcedEndActive;
    overlayState.remainingSeconds = remainingSeconds;
    overlayState.countdownSeconds = countdownSeconds;
    overlayState.countdownStage = countdownStage;
    overlayState.totalCoins = totalCoins;
    overlayState.runCoins = runCoins;
    overlayState.activeCoins = activeCoins;
    overlayState.goalCoins = m_coinChallengeGoalCoins;
    overlayState.speedKmh = speedKmh;
    overlayState.averageSpeedKmh = averageSpeedKmh;
    overlayState.maxSpeedKmh = maxSpeedKmh;
    overlayState.efficiencyCoinsPerMinute = efficiencyCpm;
    overlayState.goalProgress = goalProgress;
    overlayState.timeProgress = timeProgress;
    overlayState.countdownStageProgress = coinChallengeCountdownStageProgress();
    overlayState.integrityPercent = integrity;
    overlayState.integrityStage = coinChallengeIntegrityStage();
    overlayState.recentDamageAmount = m_coinChallengeRecentDamageAmount;
    overlayState.damageFlashProgress = qBound<qreal>(0.0,
                                                     static_cast<qreal>(m_coinChallengeDamageFlashRemainingMs)
                                                         / static_cast<qreal>(kCoinChallengeDamageFlashDurationMs),
                                                     1.0);
    overlayState.lowIntegrityPulseProgress = qBound<qreal>(
        0.0,
        static_cast<qreal>(m_coinChallengeLowIntegrityWarningRemainingMs)
            / static_cast<qreal>(kCoinChallengeLowIntegrityWarningDurationMs),
        1.0);
    overlayState.integrityStatusText = coinChallengeIntegrityStatusLabel(integrity);
    overlayState.damagePopupText = m_coinChallengeDamagePopupRemainingMs > 0 ? m_coinChallengeDamagePopupText : QString();
    overlayState.damagePopupDetail = m_coinChallengeDamagePopupRemainingMs > 0 ? m_coinChallengeDamagePopupDetail : QString();
    overlayState.forcedEndProgress = m_coinChallengeForcedEndActive
        ? 1.0 - (static_cast<qreal>(m_coinChallengeForcedEndRemainingMs)
                 / static_cast<qreal>(qMax(1, kCoinChallengeForcedEndDurationMs)))
        : 0.0;
    overlayState.forcedEndHeadline = m_coinChallengeEndTitle;
    overlayState.forcedEndDetail = m_coinChallengeEndDetail;
    const bool magnetActive = m_coinChallengeMagnetRemainingMs > 0;
    overlayState.powerupActive = balloonRushActive || magnetActive;
    overlayState.powerupLabel = balloonRushActive
        ? QStringLiteral("BALLOON RUSH")
        : (magnetActive ? QStringLiteral("MAGNET ACTIVE")
                        : QStringLiteral("Powerup: --"));
    overlayState.powerupRemainingSeconds = balloonRushActive
        ? qMax(1, static_cast<int>(qCeil(m_coinChallengeBalloonRushRemainingMs / 1000.0)))
        : (magnetActive
               ? qMax(1, static_cast<int>(qCeil(m_coinChallengeMagnetRemainingMs / 1000.0)))
               : 0);
    overlayState.loopsCompleted = m_coinChallengeLoopsCompleted;
    overlayState.routeHintText = m_coinChallengeForcedEndActive
        ? QStringLiteral("RECOVERY IN PROGRESS  |  CONTROL LOCKED")
        : (balloonRushActive
        ? QStringLiteral("BALLOON RUSH  |  BONUS SCENE ACTIVE")
        : QStringLiteral("FOLLOW THE COIN TRAIL  |  LOOPS %1")
              .arg(m_coinChallengeLoopsCompleted));
    if (overlayState.tenSecondWarningVisible && m_coinChallengeWarningStartedMs > 0) {
        overlayState.tenSecondWarningProgress = qBound<qreal>(
            0.0,
            static_cast<qreal>(QDateTime::currentMSecsSinceEpoch() - m_coinChallengeWarningStartedMs) / 1800.0,
            1.0);
    }

    if (m_gameView) {
        if (overlayState.visible) {
            m_gameView->setCoinChallengeOverlayState(overlayState);
        } else {
            m_gameView->clearCoinChallengeOverlayState();
        }
    }

    if (m_coinChallengeHudLabel) {
        const QString statusText = m_countdownActive
            ? QStringLiteral("START SEQUENCE")
            : QStringLiteral("LIVE RUN");
        const QString timeValue = m_countdownActive
            ? QStringLiteral("%1").arg(countdownSeconds)
            : QStringLiteral("%1s").arg(remainingSeconds);

        m_coinChallengeHudLabel->setText(QStringLiteral(
            "<div style='padding:14px 16px;'>"
            "<div style='color:#F5C451;font-size:11px;font-weight:800;letter-spacing:2px;'>COIN CHALLENGE</div>"
            "<div style='margin-top:3px;color:#8FEFFF;font-size:10px;font-weight:700;'>%1</div>"
            "<div style='margin-top:10px;color:#EAFBFF;font-size:15px;font-weight:700;'>Time: %2</div>"
            "<div style='margin-top:6px;color:#7DFFB8;font-size:14px;font-weight:700;'>Run Coins: +%3</div>"
            "<div style='margin-top:4px;color:#FFF6D1;font-size:13px;font-weight:600;'>Total Coins: %4</div>"
            "</div>")
                                             .arg(statusText,
                                                  timeValue,
                                                  QString::number(runCoins),
                                                  QString::number(totalCoins)));
        m_coinChallengeHudLabel->hide();
    }
}

void MainWindow::layoutCoinChallengeHud()
{
    if (!m_coinChallengeHudLabel || !ui->stackedWidget || ui->stackedWidget->count() <= 1) {
        return;
    }

    QWidget* gamePage = ui->stackedWidget->widget(1);
    if (!gamePage) {
        return;
    }

    const int width = 308;
    const int height = 148;
    const int margin = 18;
    m_coinChallengeHudLabel->setGeometry(margin, 54, width, height);
}

void MainWindow::ensureCoinChallengeSummaryOverlay()
{
    if (m_coinChallengeSummaryOverlay || !ui->stackedWidget || ui->stackedWidget->count() <= 1) {
        return;
    }

    QWidget* gamePage = ui->stackedWidget->widget(1);
    if (!gamePage) {
        return;
    }

    m_coinChallengeSummaryOverlay = new QWidget(gamePage);
    m_coinChallengeSummaryOverlay->setObjectName(QStringLiteral("coinChallengeSummaryOverlay"));
    m_coinChallengeSummaryOverlay->setStyleSheet(QStringLiteral(
        "QWidget#coinChallengeSummaryOverlay{background:rgba(2,8,20,188);}"
        "QWidget#coinChallengeSummaryPanel{background:qlineargradient(x1:0,y1:0,x2:1,y2:1,"
        "stop:0 rgba(22,18,58,246), stop:0.55 rgba(14,18,38,242), stop:1 rgba(40,18,14,240));"
        "border:3px solid rgba(255,214,102,225);border-radius:30px;}"
        "QLabel#coinChallengeSummaryTitle{color:#F9D66F;font-size:30px;font-weight:900;letter-spacing:5px;}"
        "QLabel#coinChallengeSummaryFlavor{color:#FFF2D0;font-size:14px;font-weight:700;letter-spacing:2px;}"
        "QLabel#coinChallengeSummaryDelta{color:#7DFFB8;font-size:24px;font-weight:900;letter-spacing:2px;}"
        "QLabel#coinChallengeSummaryStats{color:#F7FBFF;font-size:15px;font-weight:600;line-height:145%;}"
        "QPushButton#coinChallengePlayAgainButton{background:#0B2734;color:#FFFFFF;border:2px solid #35F6FF;"
        "border-radius:12px;min-height:50px;font-size:17px;font-weight:800;padding:0 26px;}"
        "QPushButton#coinChallengePlayAgainButton:hover{background:#123652;border-color:#7EFFFF;}"
        "QPushButton#coinChallengeExitButton{background:#2B1020;color:#FFD5DE;border:2px solid #FF5E88;"
        "border-radius:12px;min-height:50px;font-size:17px;font-weight:800;padding:0 26px;}"
        "QPushButton#coinChallengeExitButton:hover{background:#3A152A;border-color:#FF86A7;}"));

    auto* overlayLayout = new QVBoxLayout(m_coinChallengeSummaryOverlay);
    overlayLayout->setContentsMargins(28, 26, 28, 26);
    overlayLayout->addStretch(1);

    auto* panel = new QWidget(m_coinChallengeSummaryOverlay);
    panel->setObjectName(QStringLiteral("coinChallengeSummaryPanel"));
    panel->setMinimumWidth(1040);
    panel->setMaximumWidth(1140);

    auto* panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(42, 34, 42, 30);
    panelLayout->setSpacing(18);

    m_coinChallengeSummaryTitleLabel = new QLabel(QStringLiteral("Challenge Complete"), panel);
    m_coinChallengeSummaryTitleLabel->setObjectName(QStringLiteral("coinChallengeSummaryTitle"));
    m_coinChallengeSummaryTitleLabel->setAlignment(Qt::AlignCenter);
    m_coinChallengeSummaryTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    panelLayout->addWidget(m_coinChallengeSummaryTitleLabel);

    m_coinChallengeSummaryFlavorLabel = new QLabel(QStringLiteral("Treasure payout incoming"), panel);
    m_coinChallengeSummaryFlavorLabel->setObjectName(QStringLiteral("coinChallengeSummaryFlavor"));
    m_coinChallengeSummaryFlavorLabel->setAlignment(Qt::AlignCenter);
    m_coinChallengeSummaryFlavorLabel->setWordWrap(true);
    m_coinChallengeSummaryFlavorLabel->setMinimumHeight(34);
    panelLayout->addWidget(m_coinChallengeSummaryFlavorLabel);

    auto* rewardRow = new QHBoxLayout();
    rewardRow->setSpacing(18);

    m_coinChallengeSummaryRewardLabel = new QLabel(panel);
    m_coinChallengeSummaryRewardLabel->setMinimumSize(260, 204);
    m_coinChallengeSummaryRewardLabel->setMaximumWidth(300);
    m_coinChallengeSummaryRewardLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_coinChallengeSummaryRewardLabel->setAlignment(Qt::AlignCenter);
    rewardRow->addWidget(m_coinChallengeSummaryRewardLabel, 1);

    auto* centerColumn = new QVBoxLayout();
    centerColumn->setSpacing(12);
    centerColumn->setAlignment(Qt::AlignCenter);

    m_coinChallengeSummaryDeltaLabel = new QLabel(QStringLiteral("TREASURE DROP"), panel);
    m_coinChallengeSummaryDeltaLabel->setObjectName(QStringLiteral("coinChallengeSummaryDelta"));
    m_coinChallengeSummaryDeltaLabel->setAlignment(Qt::AlignCenter);
    m_coinChallengeSummaryDeltaLabel->setWordWrap(true);
    m_coinChallengeSummaryDeltaLabel->setMinimumHeight(58);
    centerColumn->addWidget(m_coinChallengeSummaryDeltaLabel);

    m_coinChallengeSummaryStatsLabel = new QLabel(panel);
    m_coinChallengeSummaryStatsLabel->setObjectName(QStringLiteral("coinChallengeSummaryStats"));
    m_coinChallengeSummaryStatsLabel->setAlignment(Qt::AlignCenter);
    m_coinChallengeSummaryStatsLabel->setTextFormat(Qt::RichText);
    m_coinChallengeSummaryStatsLabel->setWordWrap(true);
    m_coinChallengeSummaryStatsLabel->setMinimumSize(320, 250);
    m_coinChallengeSummaryStatsLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    centerColumn->addWidget(m_coinChallengeSummaryStatsLabel);
    rewardRow->addLayout(centerColumn, 1);

    m_coinChallengeSummaryCoinBagLabel = new QLabel(panel);
    m_coinChallengeSummaryCoinBagLabel->setMinimumSize(368, 272);
    m_coinChallengeSummaryCoinBagLabel->setMaximumWidth(430);
    m_coinChallengeSummaryCoinBagLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_coinChallengeSummaryCoinBagLabel->setAlignment(Qt::AlignCenter);
    rewardRow->addWidget(m_coinChallengeSummaryCoinBagLabel, 2);

    panelLayout->addLayout(rewardRow);

    auto* buttonRow = new QHBoxLayout();
    buttonRow->setSpacing(14);
    buttonRow->addStretch(1);
    m_coinChallengePlayAgainButton = new QPushButton(QStringLiteral("Play Again"), panel);
    m_coinChallengePlayAgainButton->setObjectName(QStringLiteral("coinChallengePlayAgainButton"));
    m_coinChallengeExitButton = new QPushButton(QStringLiteral("Exit"), panel);
    m_coinChallengeExitButton->setObjectName(QStringLiteral("coinChallengeExitButton"));
    buttonRow->addWidget(m_coinChallengePlayAgainButton);
    buttonRow->addWidget(m_coinChallengeExitButton);
    buttonRow->addStretch(1);
    panelLayout->addLayout(buttonRow);

    overlayLayout->addWidget(panel, 0, Qt::AlignHCenter);
    overlayLayout->addStretch(1);

    m_coinChallengeSummaryAnimTimer = new QTimer(m_coinChallengeSummaryOverlay);
    m_coinChallengeSummaryAnimTimer->setInterval(28);
    connect(m_coinChallengeSummaryAnimTimer, &QTimer::timeout, this, [this]() {
        animateCoinChallengeSummaryStep();
    });

    connect(m_coinChallengePlayAgainButton, &QPushButton::clicked, this, [this]() {
        hideCoinChallengeSummaryOverlay();
        startCoinChallengeMode();
    });
    connect(m_coinChallengeExitButton, &QPushButton::clicked, this, [this]() {
        exitCoinChallengeToMenu();
    });

    m_coinChallengeSummaryOverlay->setGeometry(gamePage->rect());
    m_coinChallengeSummaryOverlay->hide();
}

void MainWindow::showCoinChallengeSummaryOverlay()
{
    if (!m_coinChallengeActive || m_coinChallengeSummaryVisible) {
        return;
    }

    updateCoinChallengeStats(displaySpeedKmh());
    ensureCoinChallengeSummaryOverlay();
    if (!m_coinChallengeSummaryOverlay || !m_coinChallengeSummaryStatsLabel || !m_coinChallengeSummaryTitleLabel) {
        return;
    }

    m_coinChallengeSummaryVisible = true;
    m_driveActive = false;
    m_arcadeRaceLogicActive = false;
    m_countdownActive = false;
    m_coinChallengeTenSecondWarningShown = false;
    m_coinChallengeWarningStartedMs = 0;
    m_coinChallengeMagnetRemainingMs = 0;
    m_coinChallengeBalloonRushRemainingMs = 0;
    m_coinChallengeBalloonRushTriggerRemainingMs = 0;
    m_coinChallengeBalloonRushPhase = CoinChallengeBalloonRushPhase::Inactive;
    syncCoinChallengeMagnetLoop();

    if (m_btnFinishDrive) {
        m_btnFinishDrive->setEnabled(false);
    }
    if (m_btnPause) {
        m_btnPause->setEnabled(false);
    }
    if (m_coinChallengeHudLabel) {
        m_coinChallengeHudLabel->hide();
    }
    if (m_gameView) {
        m_gameView->clearCoinChallengeOverlayState();
        m_gameView->clearBalloonRushSceneState();
    }

    const int totalCoins = m_currencyManager ? m_currencyManager->coins() : 0;
    const int runCoins = m_collectibleManager ? m_collectibleManager->runCoins() : m_coinChallengeLastRunCoins;
    const bool goalComplete = runCoins >= m_coinChallengeGoalCoins;
    if (m_coinChallengeEndReason == CoinChallengeEndReason::None) {
        m_coinChallengeEndReason = goalComplete
            ? CoinChallengeEndReason::GoalComplete
            : CoinChallengeEndReason::Timeout;
    }
    if (m_coinChallengeEndTitle.isEmpty()) {
        m_coinChallengeEndTitle = coinChallengeEndReasonLabel(m_coinChallengeEndReason, goalComplete);
    }
    if (m_coinChallengeEndDetail.isEmpty()) {
        switch (m_coinChallengeEndReason) {
        case CoinChallengeEndReason::GoalComplete:
            m_coinChallengeEndDetail = QStringLiteral("Treasure chest packed. Coins are dropping into the bag.");
            break;
        case CoinChallengeEndReason::ForcedFinish:
            m_coinChallengeEndDetail = QStringLiteral("Run ended manually. Banking collected coins.");
            break;
        case CoinChallengeEndReason::VehicleDestroyed:
            m_coinChallengeEndDetail = QStringLiteral("Integrity reached 0%. Recovery team pulled you off the road.");
            break;
        case CoinChallengeEndReason::FatalCrash:
            m_coinChallengeEndDetail = QStringLiteral("AI collision totalled the car. Recovery team secured the haul.");
            break;
        case CoinChallengeEndReason::Timeout:
        case CoinChallengeEndReason::None:
        default:
            m_coinChallengeEndDetail = QStringLiteral("Time is up. Count the haul and bank every coin.");
            break;
        }
    }

    m_coinChallengeSummaryTitleLabel->setText(m_coinChallengeEndTitle);
    if (m_coinChallengeSummaryFlavorLabel) {
        m_coinChallengeSummaryFlavorLabel->setText(m_coinChallengeEndDetail);
    }

    m_coinChallengeSummaryTargetRunCoins = runCoins;
    m_coinChallengeSummaryTargetTotalCoins = totalCoins;
    m_coinChallengeSummaryAnimatedRunCoins = 0;
    m_coinChallengeSummaryAnimatedTotalCoins = qMax(0, totalCoins - runCoins);
    m_coinChallengeSummaryDepositSoundStage = 0;
    m_coinChallengeSummaryVisualProgress = 0.0;
    refreshCoinChallengeSummaryVisuals();
    if (m_coinChallengeSummaryAnimTimer) {
        m_coinChallengeSummaryAnimTimer->start();
    }

    playSound((m_coinChallengeEndReason == CoinChallengeEndReason::FatalCrash
               || m_coinChallengeEndReason == CoinChallengeEndReason::VehicleDestroyed)
                  ? PhantomDrive::SoundEffect::Fail
                  : PhantomDrive::SoundEffect::RaceFinish);
    if (goalComplete && m_coinChallengeEndReason != CoinChallengeEndReason::FatalCrash) {
        QTimer::singleShot(180, this, [this]() {
            playSound(PhantomDrive::SoundEffect::Victory);
        });
    }
    if (runCoins > 0) {
        QTimer::singleShot(260, this, [this]() {
            if (m_coinChallengeSummaryVisible) {
                playSound(PhantomDrive::SoundEffect::PowerupCollect);
            }
        });
    }

    m_coinChallengeSummaryOverlay->setGeometry(ui->stackedWidget->widget(1)->rect());
    m_coinChallengeSummaryOverlay->show();
    m_coinChallengeSummaryOverlay->raise();
}

void MainWindow::refreshCoinChallengeSummaryVisuals()
{
    if (!m_coinChallengeSummaryStatsLabel) {
        return;
    }

    const bool goalComplete = m_coinChallengeSummaryTargetRunCoins >= m_coinChallengeGoalCoins;
    const QString endReason = coinChallengeEndReasonLabel(m_coinChallengeEndReason, goalComplete);
    const int averageSpeed = coinChallengeAverageSpeedKmh();
    const int maxSpeed = coinChallengeMaxSpeedDisplayKmh();
    const int efficiency = coinChallengeEfficiencyCpm();
    const int baggedCoins = qMax(0, m_coinChallengeSummaryAnimatedRunCoins);
    const int bankCoins = qMax(0, m_coinChallengeSummaryAnimatedTotalCoins);

    if (m_coinChallengeSummaryDeltaLabel) {
        QString deltaText = goalComplete
            ? QStringLiteral("TREASURE CHEST SECURED")
            : QStringLiteral("BANK DEPOSIT COMPLETE");
        QString deltaColor = goalComplete ? QStringLiteral("#A6FFC6") : QStringLiteral("#FFD470");
        if (m_coinChallengeEndReason == CoinChallengeEndReason::ForcedFinish) {
            deltaText = QStringLiteral("RUN FORCE-CLOSED");
            deltaColor = QStringLiteral("#8FEFFF");
        } else if (m_coinChallengeEndReason == CoinChallengeEndReason::VehicleDestroyed) {
            deltaText = QStringLiteral("RECOVERY TOW COMPLETE");
            deltaColor = QStringLiteral("#FFB36B");
        } else if (m_coinChallengeEndReason == CoinChallengeEndReason::FatalCrash) {
            deltaText = QStringLiteral("FATAL CRASH LOGGED");
            deltaColor = QStringLiteral("#FF6E7B");
        }
        m_coinChallengeSummaryDeltaLabel->setText(deltaText);
        m_coinChallengeSummaryDeltaLabel->setStyleSheet(QStringLiteral(
            "QLabel#coinChallengeSummaryDelta{color:%1;font-size:24px;font-weight:900;letter-spacing:2px;}")
                                                         .arg(deltaColor));
    }

    if (m_coinChallengeSummaryRewardLabel) {
        m_coinChallengeSummaryRewardLabel->setPixmap(makeCoinChallengeChestVisual(
            m_coinChallengeSummaryRewardLabel->size(),
            baggedCoins,
            goalComplete,
            m_coinChallengeSummaryVisualProgress));
    }
    if (m_coinChallengeSummaryCoinBagLabel) {
        m_coinChallengeSummaryCoinBagLabel->setPixmap(makeCoinChallengeBagVisual(
            m_coinChallengeSummaryCoinBagLabel->size(),
            bankCoins,
            baggedCoins,
            m_coinChallengeSummaryVisualProgress));
    }

    m_coinChallengeSummaryStatsLabel->setText(QStringLiteral(
        "<div style='text-align:center;'>"
        "<div style='font-size:40px;font-weight:900;color:#F9D66F;line-height:112%;'>+%1</div>"
        "<div style='font-size:13px;font-weight:800;color:#FFF4D2;letter-spacing:2px;'>RUN PAYOUT</div>"
        "<div style='margin-top:14px;display:inline-block;padding:14px 18px;border-radius:18px;"
        "background:rgba(7,18,38,182);border:1px solid rgba(255,216,120,130);text-align:left;'>"
        "<div style='font-size:14px;color:#EAFBFF;'><b>End Reason:</b> %2</div>"
        "<div style='font-size:14px;color:#EAFBFF;'><b>Coins Collected:</b> %1</div>"
        "<div style='font-size:14px;color:#EAFBFF;'><b>Total Coins:</b> %3</div>"
        "<div style='font-size:14px;color:#EAFBFF;'><b>Final Integrity:</b> %4%% (%5)</div>"
        "<div style='font-size:14px;color:#EAFBFF;'><b>Obstacle Hits:</b> %6</div>"
        "<div style='font-size:14px;color:#EAFBFF;'><b>AI Collisions:</b> %7</div>"
        "<div style='font-size:14px;color:#EAFBFF;'><b>Damage Taken:</b> %8%%</div>"
        "<div style='font-size:14px;color:#EAFBFF;'><b>Average Speed:</b> %9 km/h</div>"
        "<div style='font-size:14px;color:#EAFBFF;'><b>Max Speed:</b> %10 km/h</div>"
        "<div style='font-size:14px;color:#EAFBFF;'><b>Efficiency:</b> %11 coins/min</div>"
        "<div style='font-size:14px;color:#EAFBFF;'><b>Powerups Used:</b> %12</div>"
        "</div>"
        "</div>")
                                             .arg(baggedCoins)
                                             .arg(endReason)
                                             .arg(bankCoins)
                                             .arg(m_coinChallengeVehicleIntegrity)
                                             .arg(coinChallengeIntegrityStatusLabel(m_coinChallengeVehicleIntegrity))
                                             .arg(m_coinChallengeObstacleHits)
                                             .arg(m_coinChallengeAICollisionCount)
                                             .arg(m_coinChallengeDamageTaken)
                                             .arg(averageSpeed)
                                             .arg(maxSpeed)
                                             .arg(efficiency)
                                             .arg(m_coinChallengePowerupsUsed));
}

void MainWindow::animateCoinChallengeSummaryStep()
{
    const int runTarget = qMax(0, m_coinChallengeSummaryTargetRunCoins);
    const int totalTarget = qMax(0, m_coinChallengeSummaryTargetTotalCoins);
    bool changed = false;

    if (m_coinChallengeSummaryAnimatedRunCoins < runTarget) {
        const int gap = runTarget - m_coinChallengeSummaryAnimatedRunCoins;
        m_coinChallengeSummaryAnimatedRunCoins += qMax(1, gap / 10);
        m_coinChallengeSummaryAnimatedRunCoins = qMin(m_coinChallengeSummaryAnimatedRunCoins, runTarget);
        changed = true;
    }
    if (m_coinChallengeSummaryAnimatedTotalCoins < totalTarget) {
        const int gap = totalTarget - m_coinChallengeSummaryAnimatedTotalCoins;
        m_coinChallengeSummaryAnimatedTotalCoins += qMax(1, gap / 11);
        m_coinChallengeSummaryAnimatedTotalCoins = qMin(m_coinChallengeSummaryAnimatedTotalCoins, totalTarget);
        changed = true;
    }
    if (m_coinChallengeSummaryVisualProgress < 1.0) {
        m_coinChallengeSummaryVisualProgress = qMin<qreal>(1.0, m_coinChallengeSummaryVisualProgress + 0.022);
        changed = true;
    }

    if (runTarget > 0) {
        static const qreal kDepositThresholds[] = {0.10, 0.24, 0.40, 0.58, 0.76, 0.92};
        static const SoundEffect kDepositSounds[] = {
            PhantomDrive::SoundEffect::PowerupCollect,
            PhantomDrive::SoundEffect::Checkpoint,
            PhantomDrive::SoundEffect::PowerupCollect,
            PhantomDrive::SoundEffect::Checkpoint,
            PhantomDrive::SoundEffect::PowerupCollect,
            PhantomDrive::SoundEffect::PowerupCollect
        };
        while (m_coinChallengeSummaryDepositSoundStage < 6
               && static_cast<qreal>(m_coinChallengeSummaryAnimatedRunCoins) / static_cast<qreal>(runTarget)
                      >= kDepositThresholds[m_coinChallengeSummaryDepositSoundStage]) {
            playSound(kDepositSounds[m_coinChallengeSummaryDepositSoundStage]);
            ++m_coinChallengeSummaryDepositSoundStage;
        }
    }

    if (changed) {
        refreshCoinChallengeSummaryVisuals();
        return;
    }

    if (m_coinChallengeSummaryAnimTimer) {
        m_coinChallengeSummaryAnimTimer->stop();
    }
}

void MainWindow::hideCoinChallengeSummaryOverlay()
{
    m_coinChallengeSummaryVisible = false;
    if (m_coinChallengeSummaryAnimTimer) {
        m_coinChallengeSummaryAnimTimer->stop();
    }
    if (m_coinChallengeSummaryOverlay) {
        m_coinChallengeSummaryOverlay->hide();
    }
}

void MainWindow::exitCoinChallengeToMenu()
{
    hideCoinChallengeSummaryOverlay();
    finishCoinChallengeMode();
    setGameHeaderVisible(false);
    if (ui->stackedWidget) {
        ui->stackedWidget->setCurrentIndex(0);
    }
    statusBar()->clearMessage();
}

void MainWindow::setupCoinChallengePowerupBoxes()
{
    if (!m_gameView || !m_gameView->trackData()) {
        return;
    }

    qDeleteAll(m_powerupBoxes);
    m_powerupBoxes.clear();
    m_coinChallengeMagnetSpawnPool.clear();
    m_coinChallengeBalloonRushSpawnPool.clear();
    m_coinChallengeMagnetSpawnStage = 0;
    m_coinChallengeBalloonRushSpawned = false;

    TrackData* track = m_gameView->trackData();
    const QVector2D start = track->getStartPosition();
    QList<QRectF> blockedAreas;
    if (m_challengeObstacleManager) {
        blockedAreas.append(m_challengeObstacleManager->allObstacleBounds());
    }
    if (m_blockerVehicleManager) {
        blockedAreas.append(m_blockerVehicleManager->allVehicleBounds());
    }
    QList<QVector2D> candidates;

    auto appendCandidate = [this, &candidates, start, blockedAreas, track](const QVector2D& candidate) {
        QVector2D snappedCandidate;
        if (!snapToNearestCoinChallengeRoadPoint(track, candidate, snappedCandidate)) {
            return;
        }
        if (!isCoinChallengePowerupSpawnPointValid(snappedCandidate, 260.0f)) {
            return;
        }
        if ((snappedCandidate - start).length() < 260.0f) {
            return;
        }
        for (const QRectF& rect : blockedAreas) {
            if (rect.adjusted(-28.0, -28.0, 28.0, 28.0).contains(snappedCandidate.toPointF())) {
                return;
            }
        }
        for (const QVector2D& existing : candidates) {
            if ((existing - snappedCandidate).length() < 150.0f) {
                return;
            }
        }
        candidates.append(snappedCandidate);
    };

    for (const QVector2D& itemBox : track->getItemBoxPositions()) {
        appendCandidate(itemBox);
    }

    const QList<Checkpoint*> checkpoints = track->getCheckpointsInOrder();
    for (const Checkpoint* checkpoint : checkpoints) {
        if (checkpoint) {
            appendCandidate(checkpoint->getPosition());
        }
    }

    if (candidates.isEmpty()) {
        appendCandidate(start + QVector2D(320.0f, 180.0f));
        appendCandidate(start + QVector2D(-280.0f, 360.0f));
        appendCandidate(start + QVector2D(120.0f, 520.0f));
    }

    if (candidates.isEmpty()) {
        return;
    }

    std::shuffle(candidates.begin(), candidates.end(), *QRandomGenerator::global());
    const int spawnCount = qMin(4, candidates.size());
    for (int i = 0; i < spawnCount; ++i) {
        if (spawnCount >= 3 && i == spawnCount - 1) {
            m_coinChallengeBalloonRushSpawnPool.append(candidates.at(i));
        } else {
            m_coinChallengeMagnetSpawnPool.append(candidates.at(i));
        }
    }

    if (m_coinChallengeBalloonRushSpawnPool.isEmpty() && !candidates.isEmpty()) {
        m_coinChallengeBalloonRushSpawnPool.append(candidates.constLast());
    }
}

bool MainWindow::isCoinChallengePowerupSpawnPointValid(const QVector2D& position, qreal minDistanceFromPlayers) const
{
    if (!m_gameView) {
        return false;
    }

    TrackData* track = m_gameView->trackData();
    if (!track || !track->containsPoint(position)) {
        return false;
    }

    const QPoint tileCoord = TrackData::worldToTile(position, kTileSize);
    TrackTile* tile = track->getTileAt(tileCoord.y(), tileCoord.x());
    if (!tile || !isDrivableSurface(tile->getType())) {
        return false;
    }

    if (positionNearStartFinish(track, position, 88.0) || tileAtIsStartFinish(track, position)) {
        return false;
    }

    const QList<Checkpoint*> checkpoints = track->getCheckpointsInOrder();
    for (const Checkpoint* checkpoint : checkpoints) {
        if (checkpoint && checkpoint->containsPoint(position)) {
            return false;
        }
    }

    for (const QRectF& rect : currentCoinChallengeBlockedAreas()) {
        if (rect.adjusted(-32.0, -32.0, 32.0, 32.0).contains(position.toPointF())) {
            return false;
        }
    }

    if ((position - m_playerPosition).length() < minDistanceFromPlayers) {
        return false;
    }
    if (m_twoPlayerMode && (position - m_player2Position).length() < minDistanceFromPlayers) {
        return false;
    }

    for (PowerupBox* box : m_powerupBoxes) {
        if (!box || !box->isActive()) {
            continue;
        }
        if ((box->position() - position).length() < 120.0f) {
            return false;
        }
    }

    return true;
}

bool MainWindow::tryTakeCoinChallengePowerupSpawnPoint(QList<QVector2D>& spawnPool, QVector2D& outPosition) const
{
    if (spawnPool.isEmpty()) {
        return false;
    }

    QList<QVector2D> deferred;
    while (!spawnPool.isEmpty()) {
        const QVector2D candidate = spawnPool.takeFirst();
        if (isCoinChallengePowerupSpawnPointValid(candidate)) {
            deferred.append(spawnPool);
            spawnPool = deferred;
            outPosition = candidate;
            return true;
        }
        deferred.append(candidate);
    }

    spawnPool = deferred;
    return false;
}

void MainWindow::updateCoinChallengeLoopProgress(const QVector2D& positionBefore)
{
    if (!isCoinChallengeModeActive() || !m_gameView || !m_gameView->trackData()) {
        return;
    }

    TrackData* track = m_gameView->trackData();
    const QList<Checkpoint*> checkpoints = track->getCheckpointsInOrder();
    const bool onStartZone = tileAtIsStartFinish(track, m_playerPosition)
        || positionNearStartFinish(track, m_playerPosition, 72.0);

    if (!onStartZone) {
        m_coinChallengeLeftStartZone = true;
    }

    if (m_coinChallengeLoopCheckpointIndex < checkpoints.size()) {
        Checkpoint* nextCheckpoint = checkpoints.at(m_coinChallengeLoopCheckpointIndex);
        if (nextCheckpoint && crossedCheckpointGate(nextCheckpoint, positionBefore, m_playerPosition)) {
            ++m_coinChallengeLoopCheckpointIndex;
        }
    }

    const bool readyForLoopComplete = checkpoints.isEmpty()
        ? m_coinChallengeLeftStartZone
        : (m_coinChallengeLeftStartZone && m_coinChallengeLoopCheckpointIndex >= checkpoints.size());
    const bool crossedStartZone = readyForLoopComplete && onStartZone && !m_coinChallengeWasOnStartZone;
    if (crossedStartZone) {
        ++m_coinChallengeLoopsCompleted;
        m_coinChallengeLoopCheckpointIndex = 0;
        m_coinChallengeLeftStartZone = false;
        playSound(PhantomDrive::SoundEffect::Checkpoint);
    }

    m_coinChallengeWasOnStartZone = onStartZone;
}

void MainWindow::maybeSpawnCoinChallengeMagnet()
{
    if (!isCoinChallengeModeActive() || !m_gameView) {
        return;
    }

    const QList<PowerupBox*> activeBoxes = m_powerupBoxes;
    for (PowerupBox* box : activeBoxes) {
        if (box && box->isActive()) {
            return;
        }
    }

    if (m_coinChallengeMagnetSpawnPool.isEmpty()) {
        return;
    }

    const int runCoins = m_collectibleManager ? m_collectibleManager->runCoins() : 0;
    const bool spawnFirstMagnet = m_coinChallengeMagnetSpawnStage == 0 && m_coinChallengeLoopsCompleted >= 1;
    const bool spawnSecondMagnet = m_coinChallengeMagnetSpawnStage == 1 && runCoins >= m_coinChallengeGoalCoins;

    if (!spawnFirstMagnet && !spawnSecondMagnet) {
        return;
    }

    QVector2D spawnPos;
    if (!tryTakeCoinChallengePowerupSpawnPoint(m_coinChallengeMagnetSpawnPool, spawnPos)) {
        return;
    }
    spawnCoinChallengeMagnetBox(spawnPos);
    ++m_coinChallengeMagnetSpawnStage;
}

void MainWindow::maybeSpawnCoinChallengeBalloonRush()
{
    if (!isCoinChallengeModeActive() || !m_gameView || m_coinChallengeBalloonRushSpawned) {
        return;
    }

    const QList<PowerupBox*> activeBoxes = m_powerupBoxes;
    for (PowerupBox* box : activeBoxes) {
        if (box && box->isActive()) {
            return;
        }
    }

    if (m_coinChallengeBalloonRushSpawnPool.isEmpty()) {
        return;
    }

    const int runCoins = m_collectibleManager ? m_collectibleManager->runCoins() : 0;
    if (runCoins < kCoinChallengeBalloonRushMilestone || m_coinChallengeLoopsCompleted < 1) {
        return;
    }

    QVector2D spawnPos;
    if (!tryTakeCoinChallengePowerupSpawnPoint(m_coinChallengeBalloonRushSpawnPool, spawnPos)) {
        return;
    }
    spawnCoinChallengeBalloonRushBox(spawnPos);
    m_coinChallengeBalloonRushSpawned = true;
}

void MainWindow::spawnCoinChallengeMagnetBox(const QVector2D& position)
{
    if (!m_gameView) {
        return;
    }

    const QString id = QStringLiteral("coin_magnet_%1").arg(m_coinChallengeMagnetSpawnStage + 1);
    const QString typeName = powerupTypeToString(PowerupType::Magnet);
    auto* box = new PowerupBox(position, 54.0f, this);
    box->setObjectName(id);
    box->setFixedPowerupType(PowerupType::Magnet);
    box->setRespawnTime(kCoinChallengePowerupRespawnSeconds);
    m_powerupBoxes.append(box);
    m_gameView->addPowerupBox(id, position, typeName);

    connect(box, &PowerupBox::collected,
            this, [this, id](const QString& playerId, const PowerupType& collectedType) {
                if (m_gameView) {
                    m_gameView->removePowerupBox(id);
                }
                if (playerId == QStringLiteral("player_1")) {
                    handlePowerupCollectedForPlayer(collectedType, 1);
                }
            });
    connect(box, &PowerupBox::respawned,
            this, [this, id, typeName, box]() {
                if (m_gameView && box) {
                    m_gameView->addPowerupBox(id, box->position(), typeName);
                }
            });
}

void MainWindow::spawnCoinChallengeBalloonRushBox(const QVector2D& position)
{
    if (!m_gameView) {
        return;
    }

    const QString id = QStringLiteral("coin_balloon_rush");
    const QString typeName = QStringLiteral("Balloon Rush");
    auto* box = new PowerupBox(position, 56.0f, this);
    box->setObjectName(id);
    box->setFixedPowerupType(PowerupType::Custom);
    box->setRespawnTime(kCoinChallengePowerupRespawnSeconds);
    m_powerupBoxes.append(box);
    m_gameView->addPowerupBox(id, position, typeName);

    connect(box, &PowerupBox::collected,
            this, [this, id](const QString& playerId, const PowerupType&) {
                if (m_gameView) {
                    m_gameView->removePowerupBox(id);
                }
                if (playerId == QStringLiteral("player_1")) {
                    activateCoinChallengeBalloonRush();
                }
            });
}

void MainWindow::activateCoinChallengeBalloonRush()
{
    if (!isCoinChallengeModeActive() || isCoinChallengeBalloonRushSequenceActive()) {
        return;
    }

    ++m_coinChallengePowerupsUsed;
    m_coinChallengeBalloonRushPhase = CoinChallengeBalloonRushPhase::Trigger;
    m_coinChallengeBalloonRushTriggerRemainingMs = kCoinChallengeBalloonRushTriggerDurationMs;
    m_coinChallengeBalloonRushRemainingMs = kCoinChallengeBalloonRushSceneDurationMs;
    m_coinChallengeBalloonRushIntroCountdownRemainingMs = 0;
    m_coinChallengeBalloonRushIntroCountdownAnnounced = -1;
    m_coinChallengeBalloonRushCollectedCoins = 0;
    m_coinChallengeBalloonRushIntroCountdownRemainingMs = 0;
    m_coinChallengeBalloonRushIntroCountdownAnnounced = -1;
    m_coinChallengeBalloonRushRecentGainValue = 0;
    m_coinChallengeBalloonRushGainFlashRemainingMs = 0;
    m_coinChallengeBalloonRushMilestoneRemainingMs = 0;
    m_coinChallengeBalloonRushMilestoneDurationMs = 0;
    m_coinChallengeBalloonRushReturnRemainingMs = 0;
    m_coinChallengeBalloonRushSavedPosition = m_playerPosition;
    m_coinChallengeBalloonRushSavedRotation = m_playerRotation;
    m_coinChallengeBalloonRushSavedSpeed = m_playerSpeed;
    m_coinChallengeBalloonRushLaneIndex = 1;
    m_coinChallengeBalloonRushLaneVisual = 1.0;
    m_coinChallengeBalloonRushRoadScroll = 0.0;
    m_coinChallengeBalloonRushSpawnAccumulatorMs = 0.0;
    m_coinChallengeBalloonRushPatternLane = 1;
    m_coinChallengeBalloonRushPatternBatchCount = 0;
    m_coinChallengeBalloonRushRecentSegmentLanes.clear();
    m_coinChallengeBalloonRushVisitedLanes.clear();
    m_coinChallengeBalloonRushCoinLayout.clear();
    m_coinChallengeBalloonRushMilestoneHeadline.clear();
    m_coinChallengeBalloonRushMilestoneDetail.clear();
    m_coinChallengeBalloonRushMilestoneAccent = QColor();

    if (m_vehiclePhysics) {
        m_vehiclePhysics->clearDriveInput();
    }
    if (m_player2Physics) {
        m_player2Physics->clearDriveInput();
    }

    playSound(PhantomDrive::SoundEffect::PowerupCustom);
    QTimer::singleShot(110, this, [this]() {
        if (isCoinChallengeModeActive() && isCoinChallengeBalloonRushSequenceActive()) {
            playSound(PhantomDrive::SoundEffect::Checkpoint);
        }
    });

    showInteractiveFeedback(QStringLiteral("Balloon Rush!"), PhantomDrive::FeedbackType::Powerup);
    syncCoinChallengeMagnetLoop();
    refreshCoinChallengeHazardVisuals();
    refreshCoinChallengeBalloonRushScene();
    updateCoinChallengeHud();
}

bool MainWindow::isCoinChallengeBalloonRushSequenceActive() const
{
    return m_coinChallengeBalloonRushPhase != CoinChallengeBalloonRushPhase::Inactive;
}

bool MainWindow::isCoinChallengeBalloonRushSceneActive() const
{
    return m_coinChallengeBalloonRushPhase == CoinChallengeBalloonRushPhase::BonusScene;
}

void MainWindow::updateCoinChallengeBalloonRush(int deltaMs)
{
    if (!isCoinChallengeModeActive() || !isCoinChallengeBalloonRushSequenceActive()) {
        return;
    }

    if (m_coinChallengeBalloonRushPhase == CoinChallengeBalloonRushPhase::Trigger) {
        m_coinChallengeBalloonRushTriggerRemainingMs = qMax(0, m_coinChallengeBalloonRushTriggerRemainingMs - deltaMs);
        if (m_coinChallengeBalloonRushTriggerRemainingMs <= 0) {
            enterCoinChallengeBalloonRushBonusScene();
            return;
        }

        refreshCoinChallengeBalloonRushScene();
        return;
    }

    if (m_coinChallengeBalloonRushPhase == CoinChallengeBalloonRushPhase::Return) {
        m_coinChallengeBalloonRushReturnRemainingMs = qMax(0, m_coinChallengeBalloonRushReturnRemainingMs - deltaMs);
        if (m_coinChallengeBalloonRushReturnRemainingMs <= 0) {
            finishCoinChallengeBalloonRush();
            return;
        }

        refreshCoinChallengeBalloonRushScene();
        return;
    }

    if (m_coinChallengeBalloonRushPhase != CoinChallengeBalloonRushPhase::BonusScene) {
        finishCoinChallengeBalloonRush();
        return;
    }

    const qreal deltaSeconds = qMax<qreal>(0.0, static_cast<qreal>(deltaMs) / 1000.0);
    const int runCoinsBeforeFrame = m_collectibleManager ? m_collectibleManager->runCoins() : 0;
    int collectedThisFrame = 0;
    m_coinChallengeBalloonRushRemainingMs = qMax(0, m_coinChallengeBalloonRushRemainingMs - deltaMs);
    m_coinChallengeBalloonRushLaneVisual += (static_cast<qreal>(m_coinChallengeBalloonRushLaneIndex) - m_coinChallengeBalloonRushLaneVisual)
        * qMin<qreal>(1.0, deltaSeconds * 10.0);
    m_coinChallengeBalloonRushGainFlashRemainingMs = qMax(0, m_coinChallengeBalloonRushGainFlashRemainingMs - deltaMs);
    m_coinChallengeBalloonRushMilestoneRemainingMs = qMax(0, m_coinChallengeBalloonRushMilestoneRemainingMs - deltaMs);
    if (m_coinChallengeBalloonRushGainFlashRemainingMs <= 0) {
        m_coinChallengeBalloonRushRecentGainValue = 0;
    }

    if (m_coinChallengeBalloonRushMilestoneRemainingMs <= 0) {
        m_coinChallengeBalloonRushMilestoneDurationMs = 0;
        m_coinChallengeBalloonRushMilestoneHeadline.clear();
        m_coinChallengeBalloonRushMilestoneDetail.clear();
        m_coinChallengeBalloonRushMilestoneAccent = QColor();
    }

    const int introCountdownStage = balloonRushIntroCountdownStage(m_coinChallengeBalloonRushRemainingMs);
    if (introCountdownStage >= 0 && introCountdownStage != m_coinChallengeBalloonRushIntroCountdownAnnounced) {
        m_coinChallengeBalloonRushIntroCountdownAnnounced = introCountdownStage;
        playSound(coinChallengeCountdownSoundForStage(introCountdownStage));
    }

    m_coinChallengeBalloonRushRoadScroll += deltaSeconds * 1.85;
    m_coinChallengeBalloonRushSpawnAccumulatorMs += deltaMs;
    while (m_coinChallengeBalloonRushSpawnAccumulatorMs >= kCoinChallengeBalloonRushCoinSpawnIntervalMs) {
        m_coinChallengeBalloonRushSpawnAccumulatorMs -= kCoinChallengeBalloonRushCoinSpawnIntervalMs;
        if (m_coinChallengeBalloonRushPatternBatchCount <= 0) {
            int nextLane = m_coinChallengeBalloonRushPatternLane;
            if (m_coinChallengeBalloonRushVisitedLanes.isEmpty()) {
                nextLane = 1;
            } else {
                QList<int> laneCandidates;
                QList<int> missingLanes;
                for (int lane = 0; lane < 3; ++lane) {
                    if (!m_coinChallengeBalloonRushVisitedLanes.contains(lane)) {
                        missingLanes.append(lane);
                    }
                }

                if (!missingLanes.isEmpty()) {
                    for (const int lane : missingLanes) {
                        if (lane != m_coinChallengeBalloonRushPatternLane) {
                            laneCandidates.append(lane);
                        }
                    }
                    if (laneCandidates.isEmpty()) {
                        laneCandidates = missingLanes;
                    }
                } else {
                    for (int lane = 0; lane < 3; ++lane) {
                        if (lane == m_coinChallengeBalloonRushPatternLane) {
                            continue;
                        }
                        if (!m_coinChallengeBalloonRushRecentSegmentLanes.contains(lane)) {
                            laneCandidates.append(lane);
                        }
                    }
                    if (laneCandidates.isEmpty()) {
                        for (int lane = 0; lane < 3; ++lane) {
                            if (lane != m_coinChallengeBalloonRushPatternLane) {
                                laneCandidates.append(lane);
                            }
                        }
                    }
                }

                if (!laneCandidates.isEmpty()) {
                    nextLane = laneCandidates.at(QRandomGenerator::global()->bounded(laneCandidates.size()));
                }
            }
            m_coinChallengeBalloonRushPatternLane = qBound(0, nextLane, 2);
            m_coinChallengeBalloonRushVisitedLanes.insert(m_coinChallengeBalloonRushPatternLane);
            m_coinChallengeBalloonRushRecentSegmentLanes.append(m_coinChallengeBalloonRushPatternLane);
            while (m_coinChallengeBalloonRushRecentSegmentLanes.size() > 2) {
                m_coinChallengeBalloonRushRecentSegmentLanes.removeFirst();
            }
            m_coinChallengeBalloonRushPatternBatchCount = 3 + QRandomGenerator::global()->bounded(3);
        }

        const int primaryLane = qBound(0, m_coinChallengeBalloonRushPatternLane, 2);
        const qreal baseDepth = 0.10 + 0.010 * QRandomGenerator::global()->bounded(3);
        m_coinChallengeBalloonRushCoinLayout.append(QPointF(primaryLane, baseDepth));
        --m_coinChallengeBalloonRushPatternBatchCount;
    }

    const qreal travelStep = kCoinChallengeBalloonRushCoinSpeedPerSecond * deltaSeconds;
    for (int i = m_coinChallengeBalloonRushCoinLayout.size() - 1; i >= 0; --i) {
        QPointF coin = m_coinChallengeBalloonRushCoinLayout.at(i);
        coin.setY(coin.y() + travelStep);

        const bool inCatchWindow = coin.y() >= 0.82 && coin.y() <= 1.06;
        const bool laneMatched = qAbs(coin.x() - static_cast<qreal>(m_coinChallengeBalloonRushLaneIndex)) <= 0.28;
        if (inCatchWindow && laneMatched) {
            m_coinChallengeBalloonRushCoinLayout.removeAt(i);
            ++m_coinChallengeBalloonRushCollectedCoins;
            ++collectedThisFrame;
            if (m_collectibleManager && m_currencyManager) {
                m_collectibleManager->addRunCoins(1, *m_currencyManager);
            }
            playSound(PhantomDrive::SoundEffect::PowerupCollect);
            refreshGaragePage();
            savePlayerProfile();
            continue;
        }

        if (coin.y() > 1.14) {
            m_coinChallengeBalloonRushCoinLayout.removeAt(i);
            continue;
        }

        m_coinChallengeBalloonRushCoinLayout[i] = coin;
    }

    if (collectedThisFrame > 0) {
        m_coinChallengeBalloonRushRecentGainValue = collectedThisFrame;
        m_coinChallengeBalloonRushGainFlashRemainingMs = 620;
        const int runCoinsAfterFrame = m_collectibleManager ? m_collectibleManager->runCoins() : runCoinsBeforeFrame;
        handleCoinChallengeMilestones(runCoinsBeforeFrame, runCoinsAfterFrame);
    }

    if (m_coinChallengeBalloonRushRemainingMs <= 0) {
        m_coinChallengeBalloonRushPhase = CoinChallengeBalloonRushPhase::Return;
        m_coinChallengeBalloonRushReturnRemainingMs = kCoinChallengeBalloonRushReturnTransitionDurationMs;
        m_coinChallengeBalloonRushCoinLayout.clear();
        m_coinChallengeBalloonRushRecentGainValue = 0;
        m_coinChallengeBalloonRushGainFlashRemainingMs = 0;
        finishCoinChallengeBalloonRush(false);
        refreshCoinChallengeBalloonRushScene();
        return;
    }

    refreshCoinChallengeBalloonRushScene();
}

void MainWindow::enterCoinChallengeBalloonRushBonusScene()
{
    if (!isCoinChallengeModeActive()) {
        return;
    }

    m_coinChallengeBalloonRushPhase = CoinChallengeBalloonRushPhase::BonusScene;
    m_coinChallengeBalloonRushRemainingMs = kCoinChallengeBalloonRushSceneDurationMs;
    m_coinChallengeBalloonRushReturnRemainingMs = 0;
    m_coinChallengeBalloonRushLaneIndex = 1;
    m_coinChallengeBalloonRushLaneVisual = 1.0;
    m_coinChallengeBalloonRushRoadScroll = 0.0;
    m_coinChallengeBalloonRushIntroCountdownRemainingMs = 0;
    m_coinChallengeBalloonRushIntroCountdownAnnounced = -1;
    m_coinChallengeBalloonRushSpawnAccumulatorMs = 0.0;
    m_coinChallengeBalloonRushPatternLane = 1;
    m_coinChallengeBalloonRushPatternBatchCount = 0;
    m_coinChallengeBalloonRushRecentSegmentLanes.clear();
    m_coinChallengeBalloonRushVisitedLanes.clear();
    m_coinChallengeBalloonRushCoinLayout.clear();
    syncCoinChallengeMagnetLoop();
    refreshCoinChallengeBalloonRushScene();
}

void MainWindow::finishCoinChallengeBalloonRush(bool completeSequence)
{
    if (!isCoinChallengeModeActive()) {
        return;
    }

    if (m_vehiclePhysics) {
        m_vehiclePhysics->clearDriveInput();
        QVector2D restorePosition = m_coinChallengeBalloonRushSavedPosition;
        auto restorePositionIsSafe = [&](const QVector2D& candidate) {
            return isCoinChallengePlayerPositionSafe(candidate);
        };

        if (!restorePositionIsSafe(restorePosition)) {
            QVector2D snapped;
            if (m_gameView && snapToNearestCoinChallengeRoadPoint(m_gameView->trackData(), restorePosition, snapped, 5)) {
                restorePosition = snapped;
            }
            if (!restorePositionIsSafe(restorePosition)) {
                static const QVector2D kRestoreOffsets[] = {
                    QVector2D(0.0f, 0.0f),
                    QVector2D(64.0f, 0.0f),
                    QVector2D(-64.0f, 0.0f),
                    QVector2D(0.0f, 64.0f),
                    QVector2D(0.0f, -64.0f),
                    QVector2D(96.0f, 32.0f),
                    QVector2D(-96.0f, 32.0f)
                };
                for (const QVector2D& offset : kRestoreOffsets) {
                    const QVector2D candidate = restorePosition + offset;
                    if (restorePositionIsSafe(candidate)) {
                        restorePosition = candidate;
                        break;
                    }
                }
            }
        }

        const qreal restoreSpeed = qBound<qreal>(42.0, m_coinChallengeBalloonRushSavedSpeed, 84.0);
        m_vehiclePhysics->setPosition(restorePosition);
        m_vehiclePhysics->setRotation(m_coinChallengeBalloonRushSavedRotation);
        m_vehiclePhysics->setSpeed(restoreSpeed);
        m_playerPosition = restorePosition;
        m_playerRotation = m_coinChallengeBalloonRushSavedRotation;
        m_playerSpeed = restoreSpeed;
        if (restorePositionIsSafe(restorePosition)) {
            m_coinChallengeLastSafePlayerPosition = restorePosition;
            m_coinChallengeLastSafePlayerRotation = m_playerRotation;
            m_coinChallengeHasSafePlayerPosition = true;
        }
    }

    m_previousPlayerPosition = m_playerPosition;
    if (m_gameView) {
        m_gameView->updatePlayerCar(QStringLiteral("P1"),
                                    m_playerPosition,
                                    m_playerRotation,
                                    displaySpeedKmh(),
                                    currentPlayerSkinColor(),
                                    m_vehiclePhysics && m_vehiclePhysics->isSpeedBoostActive(),
                                    m_vehiclePhysics && m_vehiclePhysics->isShieldActive(),
                                    m_vehiclePhysics && m_vehiclePhysics->isInvisible(),
                                    m_vehiclePhysics && m_vehiclePhysics->isMagnetActive(),
                                    currentPlayerSkinId());
        m_gameView->setCameraPosition(m_playerPosition);
        if (completeSequence) {
            m_gameView->clearBalloonRushSceneState();
        }
    }

    if (!completeSequence) {
        if (m_collectibleManager) {
            m_collectibleManager->setBlockedAreas(currentCoinChallengeBlockedAreas());
        }
        refreshCoinChallengeHazardVisuals();
        updateCoinChallengeHud();
        return;
    }

    m_coinChallengeBalloonRushPhase = CoinChallengeBalloonRushPhase::Inactive;
    m_coinChallengeBalloonRushRemainingMs = 0;
    m_coinChallengeBalloonRushTriggerRemainingMs = 0;
    m_coinChallengeBalloonRushReturnRemainingMs = 0;
    m_coinChallengeBalloonRushIntroCountdownRemainingMs = 0;
    m_coinChallengeBalloonRushIntroCountdownAnnounced = -1;
    m_coinChallengeBalloonRushCollectedCoins = 0;
    m_coinChallengeBalloonRushRecentGainValue = 0;
    m_coinChallengeBalloonRushGainFlashRemainingMs = 0;
    m_coinChallengeBalloonRushMilestoneRemainingMs = 0;
    m_coinChallengeBalloonRushMilestoneDurationMs = 0;
    m_coinChallengeBalloonRushLaneIndex = 1;
    m_coinChallengeBalloonRushLaneVisual = 1.0;
    m_coinChallengeBalloonRushRoadScroll = 0.0;
    m_coinChallengeBalloonRushSpawnAccumulatorMs = 0.0;
    m_coinChallengeBalloonRushPatternBatchCount = 0;
    m_coinChallengeBalloonRushRecentSegmentLanes.clear();
    m_coinChallengeBalloonRushVisitedLanes.clear();
    m_coinChallengeBalloonRushCoinLayout.clear();
    m_coinChallengeBalloonRushMilestoneHeadline.clear();
    m_coinChallengeBalloonRushMilestoneDetail.clear();
    m_coinChallengeBalloonRushMilestoneAccent = QColor();
    syncCoinChallengeMagnetLoop();
    refreshCoinChallengeHazardVisuals();
    updateCoinChallengeHud();
}

void MainWindow::refreshCoinChallengeBalloonRushScene()
{
    if (!m_gameView) {
        return;
    }

    if (!isCoinChallengeBalloonRushSequenceActive()) {
        m_gameView->clearBalloonRushSceneState();
        return;
    }

    BalloonRushSceneState sceneState;
    sceneState.active = true;
    sceneState.triggerVisible = m_coinChallengeBalloonRushPhase == CoinChallengeBalloonRushPhase::Trigger;
    sceneState.bonusSceneVisible = m_coinChallengeBalloonRushPhase == CoinChallengeBalloonRushPhase::BonusScene;
    sceneState.returnTransitionVisible = m_coinChallengeBalloonRushPhase == CoinChallengeBalloonRushPhase::Return;
    sceneState.introCountdownVisible = sceneState.bonusSceneVisible
        && balloonRushIntroCountdownStage(m_coinChallengeBalloonRushRemainingMs) >= 0;
    sceneState.remainingSeconds = qMax(0, static_cast<int>(qCeil(m_coinChallengeBalloonRushRemainingMs / 1000.0)));
    sceneState.introCountdownValue = sceneState.introCountdownVisible
        ? balloonRushIntroCountdownStage(m_coinChallengeBalloonRushRemainingMs)
        : 0;
    sceneState.collectedCoins = m_coinChallengeBalloonRushCollectedCoins;
    sceneState.runCoins = m_collectibleManager ? m_collectibleManager->runCoins() : m_coinChallengeLastRunCoins;
    sceneState.goalCoins = m_coinChallengeGoalCoins;
    sceneState.recentGainValue = m_coinChallengeBalloonRushRecentGainValue;
    sceneState.laneIndex = m_coinChallengeBalloonRushLaneIndex;
    sceneState.laneVisual = m_coinChallengeBalloonRushLaneVisual;
    sceneState.goalProgress = coinChallengeGoalProgress();
    sceneState.gainFlashProgress = qBound<qreal>(0.0,
                                                 static_cast<qreal>(m_coinChallengeBalloonRushGainFlashRemainingMs) / 620.0,
                                                 1.0);
    sceneState.introCountdownProgress = sceneState.introCountdownVisible
        ? balloonRushIntroCountdownStageProgress(m_coinChallengeBalloonRushRemainingMs)
        : 0.0;
    sceneState.introCountdownLabel = sceneState.introCountdownVisible
        ? balloonRushIntroCountdownLabel(m_coinChallengeBalloonRushRemainingMs)
        : QString();
    sceneState.roadScroll = m_coinChallengeBalloonRushRoadScroll;
    sceneState.coinLayout = m_coinChallengeBalloonRushCoinLayout;
    sceneState.returnTransitionProgress = sceneState.returnTransitionVisible
        ? 1.0 - (static_cast<qreal>(m_coinChallengeBalloonRushReturnRemainingMs)
                 / static_cast<qreal>(qMax(1, kCoinChallengeBalloonRushReturnTransitionDurationMs)))
        : 0.0;
    sceneState.milestonePopupVisible = sceneState.bonusSceneVisible && m_coinChallengeBalloonRushMilestoneRemainingMs > 0;
    sceneState.milestonePopupProgress = sceneState.milestonePopupVisible
        ? 1.0 - (static_cast<qreal>(m_coinChallengeBalloonRushMilestoneRemainingMs)
                 / static_cast<qreal>(qMax(1, m_coinChallengeBalloonRushMilestoneDurationMs)))
        : 0.0;
    sceneState.milestoneHeadline = m_coinChallengeBalloonRushMilestoneHeadline;
    sceneState.milestoneDetail = m_coinChallengeBalloonRushMilestoneDetail;
    sceneState.milestoneAccent = m_coinChallengeBalloonRushMilestoneAccent;

    if (sceneState.triggerVisible) {
        const int viewHeight = m_gameView ? m_gameView->height() : height();
        const qreal progress = 1.0 - (static_cast<qreal>(m_coinChallengeBalloonRushTriggerRemainingMs)
            / static_cast<qreal>(qMax(1, kCoinChallengeBalloonRushTriggerDurationMs)));
        const qreal pop = progress < 0.45
            ? easeOutCubic(progress / 0.45)
            : 1.0;
        const qreal lift = progress > 0.52
            ? easeOutCubic((progress - 0.52) / 0.48)
            : 0.0;
        sceneState.triggerProgress = qBound<qreal>(0.0, progress, 1.0);
        sceneState.badgeOpacity = progress < 0.82 ? 1.0 : qMax<qreal>(0.0, (1.0 - progress) / 0.18);
        sceneState.badgeScale = 0.62 + pop * 0.62 - lift * 0.08;
        sceneState.badgeYOffset = -lift * viewHeight * 0.22;
        sceneState.headline = QStringLiteral("BALLOON RUSH");
        sceneState.detail = QStringLiteral("Bonus Scene incoming");
    } else if (sceneState.returnTransitionVisible) {
        sceneState.headline = QStringLiteral("RETURNING TO TRACK");
        sceneState.detail = QStringLiteral("Dropping back from Balloon Rush");
    } else if (sceneState.bonusSceneVisible) {
        sceneState.headline = QStringLiteral("BALLOON RUSH");
        sceneState.detail = QStringLiteral("Sweep the coin lanes");
    }

    m_gameView->setBalloonRushSceneState(sceneState);
}

void MainWindow::moveCoinChallengeBalloonRushLane(int direction)
{
    if (!isCoinChallengeBalloonRushSceneActive()) {
        return;
    }

    const int nextLane = qBound(0, m_coinChallengeBalloonRushLaneIndex + direction, 2);
    if (nextLane == m_coinChallengeBalloonRushLaneIndex) {
        return;
    }

    m_coinChallengeBalloonRushLaneIndex = nextLane;
    playSound(PhantomDrive::SoundEffect::Checkpoint);
    refreshCoinChallengeBalloonRushScene();
}

void MainWindow::syncCoinChallengeMagnetLoop()
{
    const bool shouldPlay = isCoinChallengeModeActive()
        && !m_coinChallengeSummaryVisible
        && !m_gamePaused
        && !isCoinChallengeBalloonRushSequenceActive()
        && m_coinChallengeMagnetRemainingMs > 0;

    SoundManager& soundManager = SoundManager::instance(this);
    if (shouldPlay && !m_coinChallengeMagnetLoopPlaying) {
        soundManager.playLoop(PhantomDrive::SoundEffect::PowerupMagnet);
        m_coinChallengeMagnetLoopPlaying = true;
    } else if (!shouldPlay && m_coinChallengeMagnetLoopPlaying) {
        soundManager.stopLoop(PhantomDrive::SoundEffect::PowerupMagnet);
        m_coinChallengeMagnetLoopPlaying = false;
    }
}

void MainWindow::refreshCoinChallengeHazardVisuals()
{
    if (!m_gameView) {
        return;
    }

    if (m_coinChallengeBalloonRushPhase == CoinChallengeBalloonRushPhase::BonusScene) {
        m_gameView->clearChallengeObstacles();
        m_gameView->clearBlockerVehicles();
        return;
    }

    if (m_challengeObstacleManager) {
        m_gameView->updateChallengeObstacles(m_challengeObstacleManager->activeObstacles());
    } else {
        m_gameView->clearChallengeObstacles();
    }

    if (m_blockerVehicleManager) {
        m_gameView->updateBlockerVehicles(m_blockerVehicleManager->activeVehicles());
    } else {
        m_gameView->clearBlockerVehicles();
    }
}

QList<QRectF> MainWindow::currentCoinChallengeBlockedAreas() const
{
    if (m_coinChallengeBalloonRushPhase == CoinChallengeBalloonRushPhase::BonusScene) {
        return {};
    }

    QList<QRectF> blockedAreas;
    if (m_challengeObstacleManager) {
        blockedAreas.append(m_challengeObstacleManager->activeObstacleBounds());
    }
    if (m_blockerVehicleManager) {
        blockedAreas.append(m_blockerVehicleManager->activeVehicleBounds());
    }
    return blockedAreas;
}

QRectF MainWindow::playerHazardCollisionRect(const QVector2D& position) const
{
    return QRectF(position.x() - 14.0,
                  position.y() - 21.0,
                  28.0,
                  42.0);
}

bool MainWindow::isCoinChallengePlayerPositionSafe(const QVector2D& position) const
{
    if (!isCoinChallengeModeActive() || !m_vehiclePhysics || !m_gameView || !m_gameView->trackData()) {
        return false;
    }

    if (m_coinChallengeBalloonRushPhase == CoinChallengeBalloonRushPhase::BonusScene
        || m_coinChallengeForcedEndActive) {
        return false;
    }

    TrackData* track = m_gameView->trackData();
    const QPoint tileCoord = TrackData::worldToTile(position, kTileSize);
    TrackTile* tile = track->getTileAt(tileCoord.y(), tileCoord.x());
    if (!tile || !isCoinChallengeDrivableSurface(tile->getType())) {
        return false;
    }

    const QRectF candidateRect = playerHazardCollisionRect(position);
    for (const QRectF& bounds : currentCoinChallengeBlockedAreas()) {
        if (candidateRect.intersects(bounds)) {
            return false;
        }
    }

    return m_vehiclePhysics->isPositionFree(position);
}

void MainWindow::updateCoinChallengeLastSafePosition()
{
    if (!isCoinChallengePlayerPositionSafe(m_playerPosition)) {
        return;
    }

    m_coinChallengeLastSafePlayerPosition = m_playerPosition;
    m_coinChallengeLastSafePlayerRotation = m_playerRotation;
    m_coinChallengeHasSafePlayerPosition = true;
}

void MainWindow::applyCoinChallengeHazardCollision(const QVector2D& positionBeforeUpdate)
{
    if (!isCoinChallengeModeActive() || !m_vehiclePhysics || m_coinChallengeForcedEndActive) {
        return;
    }

    const QRectF playerRect = playerHazardCollisionRect(m_playerPosition);
    const QList<QRectF> hazardBounds = currentCoinChallengeBlockedAreas();
    const QVector2D fallbackPosition = m_coinChallengeHasSafePlayerPosition
        ? m_coinChallengeLastSafePlayerPosition
        : positionBeforeUpdate;
    const qreal fallbackRotation = m_coinChallengeHasSafePlayerPosition
        ? m_coinChallengeLastSafePlayerRotation
        : m_playerRotation;
    const QSizeF playerCollisionSize(playerRect.width(), playerRect.height());

    auto retainActiveContacts = [&playerRect](const QSet<QString>& contacts,
                                              const auto& hazards,
                                              qreal releasePadding) {
        QSet<QString> retainedContacts;
        for (const auto& hazard : hazards) {
            const QRectF releaseZone = hazard.bounds().adjusted(-releasePadding,
                                                                -releasePadding,
                                                                releasePadding,
                                                                releasePadding);
            if (playerRect.intersects(releaseZone)) {
                retainedContacts.insert(hazard.id);
            }
        }

        QSet<QString> filteredContacts;
        for (const QString& contactId : contacts) {
            if (retainedContacts.contains(contactId)) {
                filteredContacts.insert(contactId);
            }
        }
        return filteredContacts;
    };

    auto syncPlayerStateFromPhysics = [&]() {
        m_playerPosition = m_vehiclePhysics->getPosition();
        m_previousPlayerPosition = m_playerPosition;
        m_playerRotation = m_vehiclePhysics->getRotation();
        m_playerSpeed = m_vehiclePhysics->getSpeed();
        if (isCoinChallengePlayerPositionSafe(m_playerPosition)) {
            m_coinChallengeLastSafePlayerPosition = m_playerPosition;
            m_coinChallengeLastSafePlayerRotation = m_playerRotation;
            m_coinChallengeHasSafePlayerPosition = true;
        }
        if (m_gameView) {
            m_gameView->updatePlayerCar(QStringLiteral("P1"),
                                        m_playerPosition,
                                        m_playerRotation,
                                        displaySpeedKmh(),
                                        currentPlayerSkinColor(),
                                        m_vehiclePhysics->isSpeedBoostActive(),
                                        m_vehiclePhysics->isShieldActive(),
                                        m_vehiclePhysics->isInvisible(),
                                        m_vehiclePhysics->isMagnetActive(),
                                        currentPlayerSkinId());
        }
    };

    if (m_blockerVehicleManager) {
        m_coinChallengeBlockerDamageContacts = retainActiveContacts(m_coinChallengeBlockerDamageContacts,
                                                                    m_blockerVehicleManager->activeVehicles(),
                                                                    18.0);
    } else {
        m_coinChallengeBlockerDamageContacts.clear();
    }

    if (m_challengeObstacleManager) {
        m_coinChallengeObstacleDamageContacts = retainActiveContacts(m_coinChallengeObstacleDamageContacts,
                                                                     m_challengeObstacleManager->activeObstacles(),
                                                                     14.0);
    } else {
        m_coinChallengeObstacleDamageContacts.clear();
    }

    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();

    if (m_blockerVehicleManager) {
        for (const BlockerVehicle& blocker : m_blockerVehicleManager->activeVehicles()) {
            const QRectF bounds = blocker.bounds();
            if (!playerRect.intersects(bounds)) {
                continue;
            }

            if (m_coinChallengeBlockerDamageContacts.contains(blocker.id)) {
                continue;
            }

            if (!m_vehiclePhysics->resolveHazardCollision(bounds,
                                                          hazardBounds,
                                                          positionBeforeUpdate,
                                                          fallbackPosition,
                                                          fallbackRotation,
                                                          playerCollisionSize,
                                                          1.15)) {
                continue;
            }

            m_coinChallengeBlockerDamageContacts.insert(blocker.id);
            syncPlayerStateFromPhysics();
            m_coinChallengeLastDamageMs = nowMs;
            ++m_coinChallengeAICollisionCount;
            applyCoinChallengeIntegrityDamage(kCoinChallengeBlockerCollisionDamage,
                                              QStringLiteral("-%1").arg(kCoinChallengeBlockerCollisionDamage),
                                              QStringLiteral("AI vehicle impact"));

            if (m_gameView) {
                m_gameView->showCollisionImpact((m_playerPosition + blocker.position) * 0.5f, 1.9);
            }
            if (nowMs - m_coinChallengeLastHazardSoundMs >= 280) {
                m_coinChallengeLastHazardSoundMs = nowMs;
                playSound(PhantomDrive::SoundEffect::Collision);
            }
            return;
        }
    }

    if (m_challengeObstacleManager) {
        for (const ChallengeObstacle& obstacle : m_challengeObstacleManager->activeObstacles()) {
            const QRectF bounds = obstacle.bounds();
            if (!playerRect.intersects(bounds)) {
                continue;
            }

            if (m_coinChallengeObstacleDamageContacts.contains(obstacle.id)) {
                continue;
            }

            const int damage = coinChallengeObstacleDamage(obstacle.type);
            if (!m_vehiclePhysics->resolveHazardCollision(bounds,
                                                          hazardBounds,
                                                          positionBeforeUpdate,
                                                          fallbackPosition,
                                                          fallbackRotation,
                                                          playerCollisionSize,
                                                          0.76)) {
                continue;
            }

            m_coinChallengeObstacleDamageContacts.insert(obstacle.id);
            syncPlayerStateFromPhysics();
            m_coinChallengeLastDamageMs = nowMs;
            ++m_coinChallengeObstacleHits;
            applyCoinChallengeIntegrityDamage(damage,
                                              QStringLiteral("-%1").arg(damage),
                                              coinChallengeObstacleDamageDetail(obstacle.type));

            if (m_gameView) {
                m_gameView->showCollisionImpact(m_playerPosition,
                                                damage >= 15 ? 1.45 : 1.1);
            }
            if (nowMs - m_coinChallengeLastHazardSoundMs >= 280) {
                m_coinChallengeLastHazardSoundMs = nowMs;
                playSound(damage >= 15 ? PhantomDrive::SoundEffect::Collision
                                       : PhantomDrive::SoundEffect::Crash);
            }
            return;
        }
    }
}

void MainWindow::handleCoinChallengeMilestones(int runCoinsBeforeUpdate, int runCoinsAfterUpdate)
{
    static const QList<int> milestones = {50, 70};
    for (const int milestone : milestones) {
        if (runCoinsBeforeUpdate < milestone
            && runCoinsAfterUpdate >= milestone
            && !m_coinChallengeTriggeredMilestones.contains(milestone)) {
            m_coinChallengeTriggeredMilestones.insert(milestone);
            triggerCoinChallengeMilestone(milestone);
        }
    }
}

void MainWindow::triggerCoinChallengeMilestone(int milestoneCoins)
{
    if (!m_gameView) {
        return;
    }

    QString headline;
    QString detail;
    QColor accent;

    switch (milestoneCoins) {
    case 50:
        headline = QStringLiteral("MILESTONE 1");
        detail = QStringLiteral("50 / 70 COINS");
        accent = QColor(255, 178, 66);
        break;
    case 70:
        headline = QStringLiteral("MILESTONE 2");
        detail = QStringLiteral("GOAL COMPLETE - 70 COINS");
        accent = QColor(125, 255, 184);
        break;
    default:
        return;
    }

    if (isCoinChallengeBalloonRushSceneActive()) {
        m_coinChallengeBalloonRushMilestoneHeadline = headline;
        m_coinChallengeBalloonRushMilestoneDetail = detail;
        m_coinChallengeBalloonRushMilestoneAccent = accent;
        m_coinChallengeBalloonRushMilestoneDurationMs = detail.contains(QStringLiteral("70")) ? 2250 : 1850;
        m_coinChallengeBalloonRushMilestoneRemainingMs = m_coinChallengeBalloonRushMilestoneDurationMs;
        refreshCoinChallengeBalloonRushScene();
        return;
    }

    m_gameView->triggerMilestoneCelebration(headline, detail, accent);
}

QString MainWindow::currentPlayerSkinId() const
{
    return m_skinManager ? m_skinManager->currentSkinId() : QStringLiteral("default");
}

QColor MainWindow::currentPlayerSkinColor() const
{
    const QString skinId = currentPlayerSkinId();
    if (skinId == QStringLiteral("blue")) {
        return QColor(44, 118, 255);
    }
    if (skinId == QStringLiteral("neon")) {
        return QColor(40, 246, 224);
    }
    if (skinId == QStringLiteral("gold")) {
        return QColor(247, 196, 81);
    }
    if (skinId == QStringLiteral("aurora")) {
        return QColor(51, 168, 255);
    }
    if (skinId == QStringLiteral("splitfire")) {
        return QColor(255, 97, 77);
    }
    if (skinId == QStringLiteral("violet")) {
        return QColor(108, 92, 255);
    }
    return QColor(228, 234, 242);
}

void MainWindow::onDrivingDataCollected(const DrivingData& data)
{
    if (m_gamePaused) {
        return;
    }

    if (m_gameView && m_driveActive) {
        if (m_twoPlayerMode) {
            m_gameView->updatePlayerCar(QStringLiteral("P1"),
                                        m_playerPosition,
                                        m_playerRotation,
                                        displaySpeedKmh(),
                                        currentPlayerSkinColor(),
                                        m_vehiclePhysics ? m_vehiclePhysics->isSpeedBoostActive() : false,
                                        m_vehiclePhysics ? m_vehiclePhysics->isShieldActive() : false,
                                        m_vehiclePhysics ? m_vehiclePhysics->isInvisible() : false,
                                        m_vehiclePhysics ? m_vehiclePhysics->isMagnetActive() : false,
                                        currentPlayerSkinId());
            m_gameView->updatePlayerCar(QStringLiteral("P2"),
                                        m_player2Position,
                                        m_player2Rotation,
                                        speedToDisplayKmh(m_player2Speed),
                                        QColor(40, 220, 255),
                                        m_player2Physics ? m_player2Physics->isSpeedBoostActive() : false,
                                        m_player2Physics ? m_player2Physics->isShieldActive() : false,
                                        m_player2Physics ? m_player2Physics->isInvisible() : false,
                                        m_player2Physics ? m_player2Physics->isMagnetActive() : false);
            updateTwoPlayerCamera();
        } else {
            m_gameView->updatePlayerCar(QStringLiteral("P1"),
                                        m_playerPosition,
                                        m_playerRotation,
                                        displaySpeedKmh(),
                                        currentPlayerSkinColor(),
                                        m_vehiclePhysics ? m_vehiclePhysics->isSpeedBoostActive() : false,
                                        m_vehiclePhysics ? m_vehiclePhysics->isShieldActive() : false,
                                        m_vehiclePhysics ? m_vehiclePhysics->isInvisible() : false,
                                        m_vehiclePhysics ? m_vehiclePhysics->isMagnetActive() : false,
                                        currentPlayerSkinId());
            m_gameView->setCameraPosition(m_playerPosition);
        }
    }
    updateHUD(displaySpeedKmh(), data.isBraking ? QStringLiteral("Braking") : QStringLiteral("Driving"));
}

void MainWindow::onViolationDetected(const ViolationEvent& violation)
{
    if (m_gamePaused) {
        return;
    }

    // Single unified path: InteractiveFeedback toast only (no duplicate statusBar).
    showInteractiveFeedback(violation.description, PhantomDrive::FeedbackType::Warning);
    playSound(PhantomDrive::SoundEffect::Violation);
}

void MainWindow::onScoreReady(const ScoreReport& report)
{
    Q_UNUSED(report);
    // finishDrivingSession() now handles saving and opening the report window
    // directly for every mode. Keep this slot as a compatibility hook only.
}

void MainWindow::onCoachReportReady(const QString& markdown)
{
    if (m_reportWidget) {
        m_reportWidget->setCoachReportMarkdownForPlayer(1, markdown);
        // Coach report is the last async piece – ensure skeleton is dismissed.
        m_reportWidget->hideLoading();
    }
}

void MainWindow::updateGameViewFromData(const DrivingData& data)
{
    if (!m_gameView) {
        return;
    }
    if (m_gamePaused) {
        return;
    }

    if (m_twoPlayerMode) {
        m_gameView->updatePlayerCar(QStringLiteral("P1"),
                                    m_playerPosition,
                                    m_playerRotation,
                                    displaySpeedKmh(),
                                    currentPlayerSkinColor(),
                                    m_vehiclePhysics ? m_vehiclePhysics->isSpeedBoostActive() : false,
                                    m_vehiclePhysics ? m_vehiclePhysics->isShieldActive() : false,
                                    m_vehiclePhysics ? m_vehiclePhysics->isInvisible() : false,
                                    m_vehiclePhysics ? m_vehiclePhysics->isMagnetActive() : false,
                                    currentPlayerSkinId());
        m_gameView->updatePlayerCar(QStringLiteral("P2"),
                                    m_player2Position,
                                    m_player2Rotation,
                                    speedToDisplayKmh(m_player2Speed),
                                    QColor(40, 220, 255),
                                    m_player2Physics ? m_player2Physics->isSpeedBoostActive() : false,
                                    m_player2Physics ? m_player2Physics->isShieldActive() : false,
                                    m_player2Physics ? m_player2Physics->isInvisible() : false,
                                    m_player2Physics ? m_player2Physics->isMagnetActive() : false);
        updateTwoPlayerCamera();
    } else {
        m_gameView->updatePlayerCar(QStringLiteral("P1"),
                                    data.position,
                                    data.rotation,
                                    speedToDisplayKmh(data.speed),
                                    currentPlayerSkinColor(),
                                    m_vehiclePhysics ? m_vehiclePhysics->isSpeedBoostActive() : false,
                                    m_vehiclePhysics ? m_vehiclePhysics->isShieldActive() : false,
                                    m_vehiclePhysics ? m_vehiclePhysics->isInvisible() : false,
                                    m_vehiclePhysics ? m_vehiclePhysics->isMagnetActive() : false,
                                    currentPlayerSkinId());
        m_gameView->setCameraPosition(data.position);
    }
}

void MainWindow::updateHUD(int speed, const QString &status)
{
    if (ui->label_Speed) {
        ui->label_Speed->setText(QString("Speed: %1 km/h").arg(speed));
        ui->label_Speed->setStyleSheet(speed > m_currentSpeedLimit
            ? "color: #e74c3c; font-size: 16px; font-weight: bold;"
            : "color: #27ae60; font-size: 16px; font-weight: bold;");
    }

    if (ui->label_Limit) {
        ui->label_Limit->setText(QString("Limit: %1 km/h").arg(m_currentSpeedLimit));
    }

    if (ui->label_) {
        ui->label_->setText(QString("Light: %1").arg(m_currentTrafficLightState));
    }

    // Note: statusBar is no longer updated here — ArcadeHUD handles all driving feedback.
}

void MainWindow::on_btn_History_clicked()
{
    // Manual history open should immediately show the latest saved report
    // and history trend, never stay in a loading state.
    showReportWindow(nullptr);
}

void MainWindow::showInteractiveFeedback(const QString& message, PhantomDrive::FeedbackType type)
{
    if (!m_driveActive || m_gamePaused) {
        return;
    }

    PhantomDrive::InteractiveFeedback& feedback = PhantomDrive::InteractiveFeedback::instance(this);
    if (m_gameView) {
        feedback.setGameView(m_gameView);
    }
    feedback.showFeedback(message, type);
}

void MainWindow::playSound(PhantomDrive::SoundEffect effect)
{
    PhantomDrive::SoundManager::instance(this).play(effect);
}

void MainWindow::showCountdown()
{
    const int sessionGeneration = m_sessionGeneration;
    if (!isCoinChallengeModeActive() && m_arcadeHUD) {
        m_arcadeHUD->show();
        updateRaceHud();
    } else {
        updateCoinChallengeHud();
    }

    if (!isCoinChallengeModeActive()) {
        PhantomDrive::InteractiveFeedback::instance(this).showCountdown(3);
    } else {
        m_coinChallengeCountdownAnnouncedStage = -1;
    }

    if (!isCoinChallengeModeActive()) {
        PhantomDrive::SoundManager::instance(this).play(PhantomDrive::SoundEffect::RaceStart);
    }

    m_countdownSessionGeneration = sessionGeneration;
    startCountdownFinishTimer(3200);
}

void MainWindow::onRaceStart()
{
    if (!m_driveActive || !m_countdownActive || m_gamePaused) {
        return;
    }

    m_countdownActive = false;
    m_countdownRemainingMs = 0;
    m_countdownTimerStartedMs = 0;
    m_lapsCompleted = 0;
    m_currentLapStartMs = m_sessionElapsedMs;
    m_bestLapMs = 0;
    m_arcadeRaceFinished = false;

    if (isCoinChallengeModeActive()) {
        focusGameViewForDriving();
        QTimer::singleShot(50, this, [this]() {
            focusGameViewForDriving();
        });

        if (m_learningHUD) {
            m_learningHUD->hide();
        }
        if (m_arcadeHUD) {
            m_arcadeHUD->hide();
        }
        if (m_btnFinishDrive) {
            m_btnFinishDrive->setEnabled(true);
        }
        updateCoinChallengeHud();
        statusBar()->showMessage(QStringLiteral("Coin Challenge live"), 1800);
        return;
    }

    syncRaceTrackToManager();
    resetArcadeRaceProgress();

    if (m_vehiclePhysics) {
        m_vehiclePhysics->resetRaceProgress();
        m_vehiclePhysics->setRaceLogicEnabled(false);
    }
    focusGameViewForDriving();
    QTimer::singleShot(50, this, [this]() {
        focusGameViewForDriving();
    });

    const int checkpointCount = m_gameView && m_gameView->trackData()
        ? m_gameView->trackData()->getCheckpointsInOrder().size()
        : 0;
    // Note: GO notification is handled by ArcadeHUD's showGo() which was already called.
    // statusBar is no longer used for game event feedback.

    updateRaceHud();

    if (m_learningHUD) {
        m_learningHUD->hide();
    }
    if (m_arcadeHUD) {
        m_arcadeHUD->show();
    }

    if (m_btnFinishDrive) {
        m_btnFinishDrive->setEnabled(true);
    }

    // GO! is handled by ArcadeHUD's showGo() call.
    // Custom Track objective is shown via updateRaceHud -> updateCurrentObjective.
}

void MainWindow::onLapCompleted(int lapNumber)
{
    if (m_arcadeHUD) {
        m_arcadeHUD->showLapCompleted(lapNumber);
    }

    playSound(PhantomDrive::SoundEffect::LapComplete);

    if (lapNumber == m_totalLaps - 1) {
        playSound(PhantomDrive::SoundEffect::FinalLap);
    }

    if (lapNumber < m_totalLaps) {
        return;
    }

    if (m_twoPlayerMode) {
        markTwoPlayerFinished(1);
        return;
    }

    m_arcadeRaceFinished = true;

    if (m_aiManager) {
        m_aiManager->setPlayerRaceProgress(m_totalLaps, 0, 100.0, true, m_sessionElapsedMs / 1000.0);
    }

    if (m_arcadeHUD) {
        m_arcadeHUD->showRaceFinished(m_aiManager ? m_aiManager->getPlayerRacePosition() : 1,
                                      formatRaceTime(m_sessionElapsedMs));
    }

    playSound(PhantomDrive::SoundEffect::RaceFinish);

    // Wait 2 s so the player can see the "Race Finished" banner, then show report.
    QTimer::singleShot(2000, this, [this]() {
        if (m_driveActive) {
            onGameFinished();
        }
    });
}

void MainWindow::onCheckpointReached(int checkpointNumber)
{
    const int displayIndex = checkpointNumber + 1;
    const int totalGates = m_raceCheckpointTotal > 0 ? m_raceCheckpointTotal : 4;

    if (m_customTrackPlaying) {
        const QString nextTarget = displayIndex >= totalGates
            ? QStringLiteral("FINISH")
            : QStringLiteral("CP%1").arg(displayIndex + 1);
        if (m_arcadeHUD) {
            m_arcadeHUD->showRaceBanner(
                QStringLiteral("CP %1/%2 passed | Next: %3")
                    .arg(displayIndex)
                    .arg(totalGates)
                    .arg(nextTarget));
        }
        playSound(PhantomDrive::SoundEffect::Checkpoint);
        return;
    }

    if (m_arcadeHUD) {
        m_arcadeHUD->showRaceBanner(
            QStringLiteral("Checkpoint %1/%2 passed").arg(displayIndex).arg(totalGates));
    }

    playSound(PhantomDrive::SoundEffect::Checkpoint);
}

void MainWindow::onCollision()
{
    playSound(PhantomDrive::SoundEffect::Collision);
}

void MainWindow::onPowerupCollected(const QString& powerupType)
{
    if (!m_driveActive || m_countdownActive || m_gamePaused) {
        return;
    }

    QString displayText;
    PhantomDrive::FeedbackType type = PhantomDrive::FeedbackType::Powerup;
    const QString normalizedType = powerupType.toLower();

    if (normalizedType.contains("boost")) {
        displayText = "Boost Collected!";
    } else if (normalizedType.contains("shield")) {
        displayText = "Shield Active!";
    } else if (normalizedType.contains("emp")) {
        displayText = "EMP Pulse!";
    } else if (normalizedType.contains("repair")) {
        displayText = "Repair Kit!";
    } else {
        displayText = QString("%1 Collected!").arg(powerupType);
    }

    auto soundForPowerup = [](const QString& key) {
        if (key.contains("boost") || key.contains("speed")) {
            return PhantomDrive::SoundEffect::PowerupBoost;
        }
        if (key.contains("shield")) {
            return PhantomDrive::SoundEffect::PowerupShield;
        }
        if (key.contains("emp")) {
            return PhantomDrive::SoundEffect::PowerupEMP;
        }
        if (key.contains("repair") || key.contains("health") || key.contains("fix")) {
            return PhantomDrive::SoundEffect::PowerupRepair;
        }
        if (key.contains("oil")) {
            return PhantomDrive::SoundEffect::PowerupOil;
        }
        if (key.contains("magnet")) {
            return PhantomDrive::SoundEffect::PowerupMagnet;
        }
        if (key.contains("custom") || key.contains("mystery")) {
            return PhantomDrive::SoundEffect::PowerupCustom;
        }
        if (key.contains("missile") || key.contains("rocket")) {
            return PhantomDrive::SoundEffect::PowerupMissile;
        }
        if (key.contains("invisible") || key.contains("invisibility") || key.contains("stealth")) {
            return PhantomDrive::SoundEffect::PowerupInvisibility;
        }
        if (key.contains("teleport") || key.contains("blink")) {
            return PhantomDrive::SoundEffect::PowerupTeleport;
        }
        return PhantomDrive::SoundEffect::PowerupGeneric;
    };

    showInteractiveFeedback(displayText, type);
    playSound(soundForPowerup(normalizedType));
}

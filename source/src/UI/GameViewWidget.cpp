#include "UI/GameViewWidget.h"
#include "track/TrackTile.h"
#include "track/Checkpoint.h"

#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QDateTime>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPen>
#include <QBrush>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QFontMetrics>
#include <QHash>
#include <QRadialGradient>
#include <QSet>
#include <QTimer>
#include <QPixmap>
#include <QtMath>
#include <QDebug>

namespace PhantomDrive {

namespace {

QColor playerColorForId(const QString& playerId)
{
    const QString id = playerId.trimmed().toUpper();
    if (id == QStringLiteral("P2") || id == QStringLiteral("2")) {
        return QColor(255, 45, 126);
    }
    if (id == QStringLiteral("P3") || id == QStringLiteral("3")) {
        return QColor(255, 213, 74);
    }
    if (id == QStringLiteral("P4") || id == QStringLiteral("4")) {
        return QColor(97, 255, 149);
    }
    return QColor(228, 234, 242);
}

QString normalizedPlayerId(const QString& playerId)
{
    const QString trimmed = playerId.trimmed();
    return trimmed.isEmpty() ? QStringLiteral("P1") : trimmed;
}

struct SkinPalette {
    QColor primary;
    QColor secondary;
    QColor canopy;
    QColor glow;
    bool splitSides = false;
    bool diagonalAccent = false;
};

SkinPalette skinPaletteForId(const QString& skinId, const QColor& fallback)
{
    const QString id = skinId.trimmed().toLower();
    if (id == QStringLiteral("blue")) {
        return {QColor(QStringLiteral("#2E74FF")),
                QColor(QStringLiteral("#8B59FF")),
                QColor(QStringLiteral("#D9E9FF")),
                QColor(QStringLiteral("#67A8FF")),
                true,
                false};
    }
    if (id == QStringLiteral("neon")) {
        return {QColor(QStringLiteral("#23F0D7")),
                QColor(QStringLiteral("#B044FF")),
                QColor(QStringLiteral("#D8FFF8")),
                QColor(QStringLiteral("#27FFD8")),
                false,
                true};
    }
    if (id == QStringLiteral("gold")) {
        return {QColor(QStringLiteral("#F5C451")),
                QColor(QStringLiteral("#FFF1A8")),
                QColor(QStringLiteral("#FFF8D7")),
                QColor(QStringLiteral("#FFD56E")),
                false,
                false};
    }
    if (id == QStringLiteral("aurora")) {
        return {QColor(QStringLiteral("#33A8FF")),
                QColor(QStringLiteral("#8852FF")),
                QColor(QStringLiteral("#E0F3FF")),
                QColor(QStringLiteral("#8C6BFF")),
                true,
                true};
    }
    if (id == QStringLiteral("splitfire")) {
        return {QColor(QStringLiteral("#FF614D")),
                QColor(QStringLiteral("#111827")),
                QColor(QStringLiteral("#FFE4D5")),
                QColor(QStringLiteral("#FF865A")),
                true,
                false};
    }
    if (id == QStringLiteral("violet")) {
        return {QColor(QStringLiteral("#6C5CFF")),
                QColor(QStringLiteral("#1FD3FF")),
                QColor(QStringLiteral("#E8E4FF")),
                QColor(QStringLiteral("#A06DFF")),
                false,
                true};
    }

    return {fallback,
            fallback.lighter(135),
            QColor(QStringLiteral("#F8FBFF")),
            fallback.lighter(118),
            false,
            false};
}

QColor coinChallengeIntegrityColor(int integrity)
{
    if (integrity <= 0) {
        return QColor(QStringLiteral("#8E1C2E"));
    }
    if (integrity <= 20) {
        return QColor(QStringLiteral("#FF4D5A"));
    }
    if (integrity <= 40) {
        return QColor(QStringLiteral("#FF8C42"));
    }
    if (integrity <= 70) {
        return QColor(QStringLiteral("#F6C343"));
    }
    return QColor(QStringLiteral("#32E889"));
}

qreal easeOutCubic(qreal t)
{
    const qreal clamped = qBound<qreal>(0.0, t, 1.0);
    const qreal inv = 1.0 - clamped;
    return 1.0 - inv * inv * inv;
}

constexpr int kCollisionImpactDurationMs = 320;
constexpr int kCollisionImpactCooldownMs = 200;
constexpr int kMaxCollisionImpacts = 5;
constexpr qreal kTrafficLightVisualScale = 1.45;

QString visualAssetPath(const QString& relativePath)
{
    const QStringList assetRelatives = {
        relativePath,
        QStringLiteral("visual_upgrade/phantomdrive_extra_assets_bundle_fixed_transparent/%1").arg(relativePath),
        QStringLiteral("visual_upgrade/phantomdrive_direct_use_assets/%1").arg(relativePath)
    };
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
        for (const QString& assetRelative : assetRelatives) {
            const QString candidate = QDir(root).filePath(assetRelative);
            if (QFileInfo::exists(candidate)) {
                return candidate;
            }
        }
    }
    return QString();
}

const QPixmap& visualPixmap(const QString& relativePath)
{
    static QHash<QString, QPixmap> cache;
    auto it = cache.find(relativePath);
    if (it == cache.end()) {
        it = cache.insert(relativePath, QPixmap(visualAssetPath(relativePath)));
    }
    return it.value();
}

QString powerupIconPath(const QString& powerupType)
{
    const QString type = powerupType.trimmed().toLower();
    if (type.contains(QStringLiteral("balloon"))) return QStringLiteral("carballoon.png");
    if (type.contains(QStringLiteral("golden"))) return QStringLiteral("powerups/pd_powerup_custom.png");
    if (type.contains(QStringLiteral("custom")) || type.contains(QStringLiteral("random"))) return QStringLiteral("powerups/pd_powerup_custom.png");
    if (type.contains(QStringLiteral("boost"))) return QStringLiteral("powerups/pd_powerup_boost.png");
    if (type.contains(QStringLiteral("shield"))) return QStringLiteral("powerups/pd_powerup_shield.png");
    if (type.contains(QStringLiteral("missile"))) return QStringLiteral("powerups/pd_powerup_missile.png");
    if (type.contains(QStringLiteral("oil"))) return QStringLiteral("powerups/pd_powerup_oil.png");
    if (type.contains(QStringLiteral("emp"))) return QStringLiteral("powerups/pd_powerup_emp.png");
    if (type.contains(QStringLiteral("invis"))) return QStringLiteral("powerups/pd_powerup_invisibility.png");
    if (type.contains(QStringLiteral("repair"))) return QStringLiteral("powerups/pd_powerup_repair.png");
    if (type.contains(QStringLiteral("teleport"))) return QStringLiteral("powerups/pd_powerup_teleport.png");
    if (type.contains(QStringLiteral("magnet"))) return QStringLiteral("powerups/pd_powerup_magnet.png");
    return QString();
}

int nearestSpeedLimitSignValue(int limit)
{
    static const QList<int> supportedLimits = {20, 30, 40, 50, 60, 70, 80, 90, 100, 120};
    int nearest = supportedLimits.first();
    int nearestDelta = qAbs(limit - nearest);
    for (const int supported : supportedLimits) {
        const int delta = qAbs(limit - supported);
        if (delta < nearestDelta || (delta == nearestDelta && supported > nearest)) {
            nearest = supported;
            nearestDelta = delta;
        }
    }
    return nearest;
}

QString speedLimitImagePath(int limit)
{
    return QStringLiteral("traffic/pd_speed_limit_%1.png").arg(nearestSpeedLimitSignValue(limit));
}

bool isRoadVisualTile(const TrackTile* tile)
{
    if (!tile) {
        return false;
    }

    const TileType type = tile->getType();
    return tile->isDrivable()
        || type == TileType::Road
        || type == TileType::Asphalt
        || type == TileType::StartLine
        || type == TileType::FinishLine;
}

bool roadRunsVerticallyFromTiles(const TrackData* track, const QVector2D& worldPos)
{
    if (!track) {
        return false;
    }

    const QPoint tilePos = TrackData::worldToTile(worldPos);
    const int row = tilePos.y();
    const int col = tilePos.x();

    int verticalScore = 0;
    int horizontalScore = 0;
    for (int distance = 1; distance <= 2; ++distance) {
        verticalScore += isRoadVisualTile(track->getTileAt(row - distance, col)) ? 1 : 0;
        verticalScore += isRoadVisualTile(track->getTileAt(row + distance, col)) ? 1 : 0;
        horizontalScore += isRoadVisualTile(track->getTileAt(row, col - distance)) ? 1 : 0;
        horizontalScore += isRoadVisualTile(track->getTileAt(row, col + distance)) ? 1 : 0;
    }

    return verticalScore > horizontalScore;
}

bool roadRunsVerticallyAt(const TrackData* track, const QVector2D& worldPos, const QSizeF& crossingSize)
{
    Q_UNUSED(track);
    Q_UNUSED(worldPos);

    const qreal width = qMax<qreal>(1.0, crossingSize.width());
    const qreal height = qMax<qreal>(1.0, crossingSize.height());

    // Pedestrian crossing bounds describe the stripe zone that spans across the road.
    // A wide crossing zone belongs to a vertical road; a tall crossing zone belongs to a horizontal road.
    return width >= height;
}

QVector2D roadsideVisualOffset(const TrackData* track, const QVector2D& worldPos, qreal tileSize)
{
    if (!track) {
        return QVector2D(-tileSize * 0.95f, -tileSize * 0.45f);
    }

    const QPoint tilePos = TrackData::worldToTile(worldPos);
    const int row = tilePos.y();
    const int col = tilePos.x();
    const bool roadVertical = roadRunsVerticallyFromTiles(track, worldPos);

    struct OffsetCandidate {
        int dRow;
        int dCol;
        QVector2D offset;
    };

    const QList<OffsetCandidate> candidates = roadVertical
        ? QList<OffsetCandidate>{{0, -1, QVector2D(-tileSize * 1.08f, 0.0f)},
                                 {0, 1, QVector2D(tileSize * 1.08f, 0.0f)},
                                 {-1, 0, QVector2D(0.0f, -tileSize * 1.08f)},
                                 {1, 0, QVector2D(0.0f, tileSize * 1.08f)}}
        : QList<OffsetCandidate>{{-1, 0, QVector2D(0.0f, -tileSize * 1.08f)},
                                 {1, 0, QVector2D(0.0f, tileSize * 1.08f)},
                                 {0, -1, QVector2D(-tileSize * 1.08f, 0.0f)},
                                 {0, 1, QVector2D(tileSize * 1.08f, 0.0f)}};

    for (const OffsetCandidate& candidate : candidates) {
        if (!isRoadVisualTile(track->getTileAt(row + candidate.dRow, col + candidate.dCol))) {
            return candidate.offset;
        }
    }

    return roadVertical ? QVector2D(-tileSize * 1.08f, 0.0f) : QVector2D(0.0f, -tileSize * 1.08f);
}

void dumpTrackLayoutForDebug(const TrackData* track, const QString& label)
{
    Q_UNUSED(track);
    Q_UNUSED(label);
}

} // namespace

GameViewWidget::GameViewWidget(QWidget* parent)
    : QWidget(parent)
    , m_currentTrack(nullptr)
    , m_tileSize(64.0)
    , m_backgroundColor(QColor(34, 49, 63))
    , m_showGrid(false)
    , m_playerBoostActive(false)
    , m_playerShieldActive(false)
    , m_playerInvisibleActive(false)
    , m_playerMagnetActive(false)
    , m_paused(false)
    , m_collisionImpactTimer(new QTimer(this))
    , m_lastCollisionImpactMs(-kCollisionImpactCooldownMs)
    , m_coinFlyAnimTimer(new QTimer(this))
    , m_milestoneAnimTimer(new QTimer(this))
    , m_coinChallengeDisplayedRunCoins(0)
    , m_coinChallengeTargetRunCoins(0)
    , m_coinGoalFlashProgress(0.0)
    , m_coinGoalDisplayInitialized(false)
{
    setMinimumSize(400, 300);
    setMouseTracking(true);
    setBackgroundRole(QPalette::Dark);
    setAutoFillBackground(true);
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
    setAttribute(Qt::WA_InputMethodEnabled, true);

    m_collisionImpactTimer->setInterval(16);
    connect(m_collisionImpactTimer, &QTimer::timeout, this, [this]() {
        const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
        for (int i = m_collisionImpactEffects.size() - 1; i >= 0; --i) {
            const CollisionImpactEffect& effect = m_collisionImpactEffects.at(i);
            if (nowMs - effect.startMs >= effect.durationMs) {
                m_collisionImpactEffects.removeAt(i);
            }
        }

        if (m_collisionImpactEffects.isEmpty()) {
            m_collisionImpactTimer->stop();
        }
        update();
    });

    m_coinFlyAnimTimer->setInterval(16);
    connect(m_coinFlyAnimTimer, &QTimer::timeout, this, [this]() {
        advanceCoinGoalAnimations();
    });

    m_milestoneAnimTimer->setInterval(16);
    connect(m_milestoneAnimTimer, &QTimer::timeout, this, [this]() {
        advanceMilestoneCelebrations();
    });
}

GameViewWidget::~GameViewWidget()
{
}

void GameViewWidget::setTrackData(TrackData* track)
{
    m_currentTrack = track;
    dumpTrackLayoutForDebug(track, QStringLiteral("GameView::setTrackData"));
    updateRenderState();
    update();
}

void GameViewWidget::updatePlayerCar(const QVector2D& position, qreal rotation, qreal speed)
{
    updatePlayerCar(QStringLiteral("P1"),
                    position,
                    rotation,
                    speed,
                    QColor(228, 234, 242),
                    m_playerBoostActive,
                    m_playerShieldActive,
                    m_playerInvisibleActive,
                    m_playerMagnetActive);

    for (int i = m_renderState.playerCars.size() - 1; i >= 0; --i) {
        if (m_renderState.playerCars.at(i).extraData.value(QStringLiteral("playerId")).toString() != QStringLiteral("P1")) {
            m_renderState.playerCars.removeAt(i);
        }
    }
    update();
}

void GameViewWidget::updatePlayerCar(const QString& playerId,
                                     const QVector2D& position,
                                     qreal rotation,
                                     qreal speed,
                                     const QColor& color,
                                     bool boostActive,
                                     bool shieldActive,
                                     bool invisibleActive,
                                     bool magnetActive,
                                     const QString& skinId)
{
    GameRenderObject car;
    car.type = RenderObjectType::PlayerCar;
    car.position = position;
    car.rotation = rotation;
    car.size = QSizeF(28, 46);
    const QString id = normalizedPlayerId(playerId);
    car.color = color.isValid() ? color : playerColorForId(id);
    car.label = QStringLiteral("%1  %2 km/h").arg(id).arg(qRound(speed));
    car.extraData[QStringLiteral("playerId")] = id;
    car.extraData[QStringLiteral("speed")] = speed;
    car.extraData[QStringLiteral("boostActive")] = boostActive;
    car.extraData[QStringLiteral("shieldActive")] = shieldActive;
    car.extraData[QStringLiteral("invisibleActive")] = invisibleActive;
    car.extraData[QStringLiteral("magnetActive")] = magnetActive;
    if (!skinId.trimmed().isEmpty()) {
        car.extraData[QStringLiteral("skinId")] = skinId.trimmed().toLower();
    }

    for (GameRenderObject& existing : m_renderState.playerCars) {
        if (existing.extraData.value(QStringLiteral("playerId")).toString() == id) {
            if (!car.extraData.contains(QStringLiteral("skinId"))
                && existing.extraData.contains(QStringLiteral("skinId"))) {
                car.extraData[QStringLiteral("skinId")] = existing.extraData.value(QStringLiteral("skinId"));
            }
            existing = car;
            update();
            return;
        }
    }

    m_renderState.playerCars.append(car);
    update();
}

void GameViewWidget::updateAICar(const QString& carId, const QVector2D& position, qreal rotation, qreal speed)
{
    for (int i = 0; i < m_renderState.aiCars.size(); ++i) {
        if (m_renderState.aiCars[i].extraData["id"].toString() == carId) {
            m_renderState.aiCars[i].position = position;
            m_renderState.aiCars[i].rotation = rotation;
            m_renderState.aiCars[i].label = QString("AI: %1 (%2 km/h)").arg(carId).arg(qRound(speed));
            m_renderState.aiCars[i].extraData["speed"] = speed;
            update();
            return;
        }
    }

    GameRenderObject car;
    car.type = RenderObjectType::AICar;
    car.position = position;
    car.rotation = rotation;
    car.size = QSizeF(22, 38);
    car.color = QColor(52, 152, 219);
    car.label = QString("AI: %1 (%2 km/h)").arg(carId).arg(qRound(speed));
    car.extraData["id"] = carId;
    car.extraData["speed"] = speed;

    m_renderState.aiCars.append(car);
    update();
}

void GameViewWidget::removeAICar(const QString& carId)
{
    for (int i = m_renderState.aiCars.size() - 1; i >= 0; --i) {
        if (m_renderState.aiCars[i].extraData["id"].toString() == carId) {
            m_renderState.aiCars.removeAt(i);
            update();
            return;
        }
    }
}

void GameViewWidget::clearAllAICars()
{
    m_renderState.aiCars.clear();
    update();
}

void GameViewWidget::updateCoins(const QList<CoinItem>& coins)
{
    m_renderState.coins.clear();

    for (const CoinItem& coin : coins) {
        if (!coin.active || coin.collected) {
            continue;
        }

        GameRenderObject renderCoin;
        renderCoin.type = RenderObjectType::CollectibleCoin;
        renderCoin.position = QVector2D(static_cast<float>(coin.position.x()),
                                        static_cast<float>(coin.position.y()));
        renderCoin.rotation = 0.0;
        renderCoin.size = QSizeF(coin.radius * 2.0, coin.radius * 2.0);
        renderCoin.color = QColor(246, 195, 58);
        renderCoin.extraData[QStringLiteral("radius")] = coin.radius;
        renderCoin.extraData[QStringLiteral("value")] = coin.value;
        renderCoin.extraData[QStringLiteral("phaseSeed")] = coin.phaseSeed;
        m_renderState.coins.append(renderCoin);
    }

    update();
}

void GameViewWidget::clearCoins()
{
    m_renderState.coins.clear();
    update();
}

void GameViewWidget::updateChallengeObstacles(const QList<ChallengeObstacle>& obstacles)
{
    m_renderState.challengeObstacles.clear();

    for (const ChallengeObstacle& obstacle : obstacles) {
        GameRenderObject renderObstacle;
        renderObstacle.type = RenderObjectType::ChallengeObstacle;
        renderObstacle.position = obstacle.position;
        renderObstacle.rotation = obstacle.rotation;
        renderObstacle.size = obstacle.size;
        renderObstacle.label = obstacle.id;
        renderObstacle.extraData[QStringLiteral("obstacleType")] = static_cast<int>(obstacle.type);
        renderObstacle.extraData[QStringLiteral("circular")] = obstacle.circular;
        renderObstacle.extraData[QStringLiteral("radius")] = obstacle.radius;
        m_renderState.challengeObstacles.append(renderObstacle);
    }

    update();
}

void GameViewWidget::clearChallengeObstacles()
{
    m_renderState.challengeObstacles.clear();
    update();
}

void GameViewWidget::updateBlockerVehicles(const QList<BlockerVehicle>& vehicles)
{
    m_renderState.blockerVehicles.clear();
    const QList<QColor> blockerPalette = {
        QColor(52, 152, 219),
        QColor(228, 234, 242),
        QColor(72, 188, 255)
    };

    for (int i = 0; i < vehicles.size(); ++i) {
        const BlockerVehicle& vehicle = vehicles.at(i);
        GameRenderObject renderVehicle;
        renderVehicle.type = RenderObjectType::BlockerVehicle;
        renderVehicle.position = vehicle.position;
        renderVehicle.rotation = vehicle.rotation;
        renderVehicle.size = vehicle.size;
        renderVehicle.label.clear();
        renderVehicle.color = blockerPalette.at(i % blockerPalette.size());
        renderVehicle.extraData[QStringLiteral("speed")] = vehicle.speed;
        renderVehicle.extraData[QStringLiteral("progress")] = vehicle.progress;
        m_renderState.blockerVehicles.append(renderVehicle);
    }

    update();
}

void GameViewWidget::clearBlockerVehicles()
{
    m_renderState.blockerVehicles.clear();
    update();
}

void GameViewWidget::addPowerupBox(const QString& boxId, const QVector2D& position, const QString& powerupType)
{
    for (const auto& box : m_renderState.powerupBoxes) {
        if (box.extraData["id"].toString() == boxId) {
            return;
        }
    }

    GameRenderObject box;
    box.type = RenderObjectType::PowerupBox;
    box.position = position;
    box.rotation = 0;
    box.size = QSizeF(28, 28);
    const QString normalizedType = powerupType.toLower();
    if (normalizedType.contains(QStringLiteral("balloon"))) {
        box.color = QColor(255, 190, 92);
        box.label = QStringLiteral("BR");
        box.size = QSizeF(38, 38);
    } else if (normalizedType.contains(QStringLiteral("boost"))) {
        box.color = QColor(46, 204, 113);
        box.label = QStringLiteral(">>");
    } else if (normalizedType.contains(QStringLiteral("shield"))) {
        box.color = QColor(52, 152, 219);
        box.label = QStringLiteral("SH");
    } else if (normalizedType.contains(QStringLiteral("emp"))) {
        box.color = QColor(155, 89, 182);
        box.label = QStringLiteral("EMP");
    } else if (normalizedType.contains(QStringLiteral("repair"))) {
        box.color = QColor(231, 76, 60);
        box.label = QStringLiteral("+");
    } else if (normalizedType.contains(QStringLiteral("missile"))) {
        box.color = QColor(192, 57, 43);
        box.label = QStringLiteral("MS");
    } else if (normalizedType.contains(QStringLiteral("oil"))) {
        box.color = QColor(44, 62, 80);
        box.label = QStringLiteral("OIL");
    } else if (normalizedType.contains(QStringLiteral("invis"))) {
        box.color = QColor(149, 165, 166);
        box.label = QStringLiteral("INV");
    } else if (normalizedType.contains(QStringLiteral("teleport"))) {
        box.color = QColor(26, 188, 156);
        box.label = QStringLiteral("TP");
    } else if (normalizedType.contains(QStringLiteral("magnet"))) {
        box.color = QColor(230, 126, 34);
        box.label = QStringLiteral("MG");
    } else if (normalizedType.contains(QStringLiteral("custom")) || normalizedType.contains(QStringLiteral("random"))) {
        box.color = QColor(155, 89, 182);
        box.label = QStringLiteral("?");
    } else {
        box.color = QColor(241, 196, 15);
        box.label = powerupType.isEmpty() ? "?" : powerupType;
    }
    box.extraData["id"] = boxId;
    box.extraData["powerupType"] = powerupType;

    m_renderState.powerupBoxes.append(box);
    update();
}

void GameViewWidget::removePowerupBox(const QString& boxId)
{
    for (int i = m_renderState.powerupBoxes.size() - 1; i >= 0; --i) {
        if (m_renderState.powerupBoxes[i].extraData["id"].toString() == boxId) {
            m_renderState.powerupBoxes.removeAt(i);
            update();
            return;
        }
    }
}

void GameViewWidget::updatePowerupBoxPosition(const QString& boxId, const QVector2D& position)
{
    for (GameRenderObject& box : m_renderState.powerupBoxes) {
        if (box.extraData["id"].toString() == boxId) {
            box.position = position;
            update();
            return;
        }
    }
}

void GameViewWidget::addTrafficLight(const QString& lightId, const QVector2D& position, const QString& state)
{
    GameRenderObject light;
    light.type = RenderObjectType::TrafficLight;
    light.position = position;
    light.rotation = 0;
    light.size = QSizeF(20, 40);
    light.color = QColor(44, 62, 80);
    light.label = state;
    light.extraData["id"] = lightId;
    light.extraData["state"] = state;

    m_renderState.trafficLights.append(light);
    update();
}

void GameViewWidget::updateTrafficLight(const QString& lightId, const QString& state)
{
    for (auto& light : m_renderState.trafficLights) {
        if (light.extraData["id"].toString() == lightId) {
            light.extraData["state"] = state;
            light.label = state;
            update();
            return;
        }
    }
}

void GameViewWidget::addSpeedLimitSign(const QString& signId, const QVector2D& position, int limit)
{
    GameRenderObject sign;
    sign.type = RenderObjectType::SpeedLimitSign;
    sign.position = position;
    sign.rotation = 0;
    sign.size = QSizeF(30, 30);
    sign.color = QColor(231, 76, 60);
    sign.label = QString::number(limit);
    sign.extraData["id"] = signId;
    sign.extraData["limit"] = limit;

    m_renderState.speedLimitSigns.append(sign);
    update();
}

void GameViewWidget::updateSpeedLimitSign(const QString& signId, int limit)
{
    for (auto& sign : m_renderState.speedLimitSigns) {
        if (sign.extraData["id"].toString() == signId) {
            sign.label = QString::number(limit);
            sign.extraData["limit"] = limit;
            update();
            return;
        }
    }
}

void GameViewWidget::addPedestrianCrossing(const QString& crossingId, const QVector2D& position, const QSizeF& size)
{
    GameRenderObject crossing;
    crossing.type = RenderObjectType::PedestrianCrossing;
    crossing.position = position;
    crossing.rotation = 0;
    crossing.size = size;
    crossing.color = QColor(255, 255, 255);
    crossing.label = "Pedestrian Zone";
    crossing.extraData["id"] = crossingId;
    crossing.extraData["pedestrianCount"] = 3;

    m_renderState.pedestrianCrossings.append(crossing);
    update();
}

void GameViewWidget::clearScenarioObjects()
{
    m_renderState.powerupBoxes.clear();
    m_renderState.challengeObstacles.clear();
    m_renderState.blockerVehicles.clear();
    m_renderState.trafficLights.clear();
    m_renderState.speedLimitSigns.clear();
    m_renderState.pedestrianCrossings.clear();
    m_oilPuddlePositions.clear();
    m_oilPuddleRadii.clear();
    m_missilePositions.clear();
    update();
}

void GameViewWidget::setPlayerEffectState(bool boostActive, bool shieldActive,
                                          bool invisibleActive, bool magnetActive)
{
    if (m_playerBoostActive == boostActive
        && m_playerShieldActive == shieldActive
        && m_playerInvisibleActive == invisibleActive
        && m_playerMagnetActive == magnetActive) {
        return;
    }
    m_playerBoostActive = boostActive;
    m_playerShieldActive = shieldActive;
    m_playerInvisibleActive = invisibleActive;
    m_playerMagnetActive = magnetActive;
    update();
}

void GameViewWidget::setWorldEffects(const QList<QVector2D>& oilPositions,
                                     const QList<qreal>& oilRadii,
                                     const QList<QVector2D>& missilePositions)
{
    m_oilPuddlePositions = oilPositions;
    m_oilPuddleRadii = oilRadii;
    m_missilePositions = missilePositions;
    update();
}

void GameViewWidget::setRenderState(const RenderState& state)
{
    m_renderState = state;
    update();
}

void GameViewWidget::setCoinChallengeOverlayState(const CoinChallengeOverlayState& state)
{
    m_renderState.coinChallengeOverlay = state;
    m_coinChallengeTargetRunCoins = qMax(0, state.runCoins);

    const bool shouldSyncDisplay = !state.visible
        || !m_coinGoalDisplayInitialized
        || state.countdownActive
        || m_coinChallengeDisplayedRunCoins > m_coinChallengeTargetRunCoins
        || (m_coinFlyAnimations.isEmpty() && m_coinGoalFlashProgress <= 0.001);

    if (shouldSyncDisplay) {
        m_coinChallengeDisplayedRunCoins = m_coinChallengeTargetRunCoins;
    }

    m_coinGoalDisplayInitialized = state.visible;
    if ((!m_coinFlyAnimations.isEmpty() || m_coinGoalFlashProgress > 0.0) && !m_coinFlyAnimTimer->isActive()) {
        m_coinFlyAnimTimer->start();
    }
    update();
}

void GameViewWidget::setBalloonRushSceneState(const BalloonRushSceneState& state)
{
    m_renderState.balloonRushScene = state;
    update();
}

void GameViewWidget::clearCoinChallengeOverlayState()
{
    m_renderState.coinChallengeOverlay = CoinChallengeOverlayState();
    m_coinChallengeDisplayedRunCoins = 0;
    m_coinChallengeTargetRunCoins = 0;
    m_coinGoalDisplayInitialized = false;
    clearCoinGoalAnimations();
    clearMilestoneCelebrations();
    update();
}

void GameViewWidget::clearBalloonRushSceneState()
{
    m_renderState.balloonRushScene = BalloonRushSceneState();
    update();
}

void GameViewWidget::setCameraPosition(const QVector2D& pos)
{
    m_renderState.cameraPosition = pos;
    updateRenderState();
    update();
}

void GameViewWidget::setCameraZoom(qreal zoom)
{
    m_renderState.cameraZoom = qMax(0.25, qMin(3.0, zoom));
    updateRenderState();
    update();
}

void GameViewWidget::resetCamera()
{
    m_renderState.cameraPosition = QVector2D(0, 0);
    m_renderState.cameraZoom = 1.0;
    updateRenderState();
    update();
}

void GameViewWidget::showCollisionImpact(const QVector2D& worldPosition, qreal intensity)
{
    if (m_paused) {
        return;
    }

    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    if (nowMs - m_lastCollisionImpactMs < kCollisionImpactCooldownMs) {
        return;
    }

    CollisionImpactEffect effect;
    effect.worldPosition = worldPosition;
    effect.startMs = nowMs;
    effect.durationMs = kCollisionImpactDurationMs;
    effect.intensity = qBound<qreal>(0.4, intensity, 1.8);

    while (m_collisionImpactEffects.size() >= kMaxCollisionImpacts) {
        m_collisionImpactEffects.removeFirst();
    }

    m_collisionImpactEffects.append(effect);
    m_lastCollisionImpactMs = nowMs;

    if (!m_collisionImpactTimer->isActive()) {
        m_collisionImpactTimer->start();
    }
    update();
}

void GameViewWidget::triggerCoinGoalAnimation(const QPointF& worldPosition, int value)
{
    if (!m_renderState.coinChallengeOverlay.visible) {
        return;
    }

    const QPointF start = worldToScreen(QVector2D(static_cast<float>(worldPosition.x()),
                                                  static_cast<float>(worldPosition.y())));
    const QPointF end = coinGoalAnimationTarget(value);
    const QPointF delta = end - start;
    const qreal distance = qSqrt(delta.x() * delta.x() + delta.y() * delta.y());

    CoinFlyAnim anim;
    anim.start = start;
    anim.end = end;
    anim.control = QPointF((start.x() + end.x()) * 0.5 - qBound<qreal>(20.0, distance * 0.07, 52.0),
                           qMin(start.y(), end.y()) - qBound<qreal>(76.0, distance * 0.22, 168.0));
    anim.duration = qBound<qreal>(0.56, 0.42 + distance / 900.0, 0.94);
    anim.rotation = static_cast<qreal>((QDateTime::currentMSecsSinceEpoch() / 7) % 360);
    anim.rotationSpeed = 640.0 + (m_coinFlyAnimations.size() % 3) * 120.0;
    anim.value = qMax(1, value);

    m_coinGoalDisplayInitialized = true;
    m_coinFlyAnimations.append(anim);
    if (!m_coinFlyAnimTimer->isActive()) {
        m_coinFlyAnimTimer->start();
    }
    update();
}

void GameViewWidget::clearCoinGoalAnimations()
{
    m_coinFlyAnimations.clear();
    m_coinGoalFlashProgress = 0.0;
    if (m_coinFlyAnimTimer->isActive()) {
        m_coinFlyAnimTimer->stop();
    }
    if (m_coinGoalDisplayInitialized) {
        m_coinChallengeDisplayedRunCoins = m_coinChallengeTargetRunCoins;
    }
}

void GameViewWidget::triggerMilestoneCelebration(const QString& headline,
                                                 const QString& detail,
                                                 const QColor& accent)
{
    MilestoneCelebration celebration;
    const bool balloonRushCompact = m_renderState.balloonRushScene.bonusSceneVisible;
    celebration.headline = headline.trimmed();
    celebration.detail = detail.trimmed();
    celebration.channelLabel = balloonRushCompact
        ? QStringLiteral("BALLOON RUSH")
        : QStringLiteral("COIN CHALLENGE");
    celebration.accent = accent.isValid() ? accent : QColor(0, 214, 255);
    celebration.progress = 0.0;
    celebration.compact = balloonRushCompact;
    celebration.duration = balloonRushCompact
        ? (celebration.detail.contains(QStringLiteral("70")) ? 2.05 : 1.75)
        : (celebration.detail.contains(QStringLiteral("70")) ? 2.95 : 2.55);

    m_milestoneCelebrations.append(celebration);
    if (!m_milestoneAnimTimer->isActive()) {
        m_milestoneAnimTimer->start();
    }
    update();
}

void GameViewWidget::clearMilestoneCelebrations()
{
    m_milestoneCelebrations.clear();
    if (m_milestoneAnimTimer->isActive()) {
        m_milestoneAnimTimer->stop();
    }
}

void GameViewWidget::setPaused(bool paused)
{
    if (m_paused == paused) {
        return;
    }

    m_paused = paused;
    if (m_paused) {
        m_collisionImpactTimer->stop();
    } else if (!m_collisionImpactEffects.isEmpty()) {
        const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
        for (CollisionImpactEffect& effect : m_collisionImpactEffects) {
            effect.startMs = nowMs;
        }
        m_collisionImpactTimer->start();
    }
    update();
}

void GameViewWidget::clearAll()
{
    m_renderState.clear();
    m_currentTrack = nullptr;
    m_collisionImpactEffects.clear();
    m_collisionImpactTimer->stop();
    m_coinChallengeDisplayedRunCoins = 0;
    m_coinChallengeTargetRunCoins = 0;
    m_coinGoalDisplayInitialized = false;
    clearCoinGoalAnimations();
    clearMilestoneCelebrations();
    m_paused = false;
    update();
}

void GameViewWidget::refresh()
{
    update();
}

void GameViewWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    const bool balloonRushSceneVisible = m_renderState.balloonRushScene.bonusSceneVisible;
    painter.fillRect(rect(), m_backgroundColor);
    if (!balloonRushSceneVisible) {
        drawOuterBackdrop(painter);

        drawTrack(painter);
        drawStartFinishMarkers(painter);
        drawCheckpoints(painter);

        for (const auto& coin : m_renderState.coins) {
            drawCoin(painter, coin);
        }

        for (const auto& obstacle : m_renderState.challengeObstacles) {
            drawChallengeObstacle(painter, obstacle);
        }

        for (const auto& blocker : m_renderState.blockerVehicles) {
            drawBlockerVehicle(painter, blocker);
        }

        for (const auto& crossing : m_renderState.pedestrianCrossings) {
            drawPedestrianCrossing(painter, crossing);
        }

        for (const auto& box : m_renderState.powerupBoxes) {
            drawPowerupBox(painter, box);
        }

        drawWorldEffects(painter);

        for (const auto& light : m_renderState.trafficLights) {
            drawTrafficLight(painter, light);
        }

        for (const auto& sign : m_renderState.speedLimitSigns) {
            drawSpeedLimitSign(painter, sign);
        }

        for (const auto& car : m_renderState.aiCars) {
            drawAICar(painter, car);
        }

        for (const auto& car : m_renderState.playerCars) {
            drawPlayerCar(painter, car);
        }

        drawCollisionImpacts(painter);
    }

    drawCoinChallengeOverlay(painter);
    drawBalloonRushSceneOverlay(painter);
    if (!balloonRushSceneVisible) {
        drawCoinGoalAnimations(painter);
    }
    drawMilestoneCelebrations(painter);

    if (m_paused) {
        drawPausedOverlay(painter);
    }
}

void GameViewWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateRenderState();
}

void GameViewWidget::mousePressEvent(QMouseEvent* event)
{
    QVector2D worldPos = screenToWorld(event->pos());

    for (const auto& obj : m_renderState.playerCars) {
        if (QRectF(obj.position.x() - obj.size.width()/2, obj.position.y() - obj.size.height()/2,
                   obj.size.width(), obj.size.height()).contains(worldPos.toPointF())) {
            emit objectClicked(obj);
            return;
        }
    }

    for (const auto& obj : m_renderState.aiCars) {
        if (QRectF(obj.position.x() - obj.size.width()/2, obj.position.y() - obj.size.height()/2,
                   obj.size.width(), obj.size.height()).contains(worldPos.toPointF())) {
            emit objectClicked(obj);
            return;
        }
    }
}

void GameViewWidget::drawOuterBackdrop(QPainter& painter)
{
    const QPixmap& backdrop = visualPixmap(QStringLiteral("backgrounds/pd_outer_background.png"));
    if (backdrop.isNull()) {
        return;
    }

    painter.save();
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);

    const qreal tileW = qMax<qreal>(1.0, backdrop.width());
    const qreal tileH = qMax<qreal>(1.0, backdrop.height());
    qreal offsetX = m_renderState.cameraPosition.x() * 0.08;
    qreal offsetY = -m_renderState.cameraPosition.y() * 0.08;
    offsetX -= qFloor(offsetX / tileW) * tileW;
    offsetY -= qFloor(offsetY / tileH) * tileH;

    painter.drawTiledPixmap(rect(), backdrop, QPointF(offsetX, offsetY));
    painter.fillRect(rect(), QColor(2, 6, 14, 112));
    painter.restore();
}

void GameViewWidget::drawTrack(QPainter& painter)
{
    if (!m_currentTrack) {
        painter.setPen(QColor(70, 238, 255));
        painter.setFont(QFont("Arial", 14));
        painter.drawText(rect(), Qt::AlignCenter, "No Track Loaded");
        return;
    }

    int rows = m_currentTrack->getRowCount();
    int cols = m_currentTrack->getColCount();
    const QRectF viewport = rect();
    const QRectF worldBounds(0.0, 0.0, cols * m_tileSize, rows * m_tileSize);

    drawCyberGrid(painter, worldBounds);

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            TrackTile* tile = m_currentTrack->getTileAt(row, col);
            if (!tile) continue;

            const QPointF cornerA = worldToScreen(QVector2D(col * m_tileSize, row * m_tileSize));
            const QPointF cornerB = worldToScreen(QVector2D((col + 1) * m_tileSize, (row + 1) * m_tileSize));
            const QRectF tileRect = QRectF(cornerA, cornerB).normalized();

            if (!viewport.intersects(tileRect)) continue;

            QColor baseColor;
            QColor innerColor;
            QColor edgeColor;
            QColor accentColor;
            bool roadLike = false;
            bool wallLike = false;
            switch (tile->getType()) {
                case TileType::Road:
                    baseColor = QColor(29, 31, 34);
                    innerColor = QColor(47, 49, 52);
                    edgeColor = QColor(97, 107, 112, 120);
                    accentColor = QColor(175, 184, 184, 52);
                    roadLike = true;
                    break;
                case TileType::Grass:
                    baseColor = QColor(18, 74, 35);
                    innerColor = QColor(48, 130, 56);
                    edgeColor = QColor(57, 136, 62, 72);
                    accentColor = QColor(145, 196, 88, 72);
                    break;
                case TileType::Sand:
                    baseColor = QColor(88, 59, 22);
                    innerColor = QColor(214, 169, 71);
                    edgeColor = QColor(255, 210, 92, 95);
                    accentColor = QColor(255, 242, 120, 70);
                    break;
                case TileType::Asphalt:
                    baseColor = QColor(24, 26, 29);
                    innerColor = QColor(41, 44, 48);
                    edgeColor = QColor(92, 100, 105, 112);
                    accentColor = QColor(165, 174, 174, 48);
                    roadLike = true;
                    break;
                case TileType::StartLine:
                    baseColor = QColor(7, 16, 24);
                    innerColor = QColor(14, 78, 88);
                    edgeColor = QColor(68, 255, 214, 210);
                    accentColor = QColor(255, 255, 255, 165);
                    roadLike = true;
                    break;
                case TileType::FinishLine:
                    baseColor = QColor(22, 8, 18);
                    innerColor = QColor(78, 18, 44);
                    edgeColor = QColor(255, 49, 132, 220);
                    accentColor = QColor(255, 255, 255, 185);
                    roadLike = true;
                    break;
                case TileType::Wall:
                case TileType::Barrier:
                    baseColor = QColor(70, 72, 75);
                    innerColor = QColor(128, 132, 136);
                    edgeColor = QColor(192, 200, 202, 138);
                    accentColor = QColor(235, 232, 210, 82);
                    wallLike = true;
                    break;
                default:
                    baseColor = QColor(22, 40, 34);
                    innerColor = QColor(58, 96, 76);
                    edgeColor = QColor(120, 220, 170, 70);
                    accentColor = QColor(220, 255, 230, 55);
                    break;
            }

            QLinearGradient fill(tileRect.topLeft(), tileRect.bottomRight());
            fill.setColorAt(0.0, innerColor.lighter(118));
            fill.setColorAt(0.42, innerColor);
            fill.setColorAt(1.0, baseColor);
            painter.fillRect(tileRect, fill);

            if (roadLike) {
                const qreal markWidth = qMax<qreal>(1.0, 1.35 * m_renderState.cameraZoom);
                painter.fillRect(tileRect.adjusted(tileRect.width() * 0.47, 0, -tileRect.width() * 0.47, 0),
                                 QColor(210, 214, 204, 24));
                painter.setPen(QPen(QColor(9, 11, 13, 62), markWidth));
                painter.drawLine(QPointF(tileRect.left(), tileRect.top() + tileRect.height() * 0.16),
                                 QPointF(tileRect.right(), tileRect.top() + tileRect.height() * 0.16));
                painter.drawLine(QPointF(tileRect.left(), tileRect.bottom() - tileRect.height() * 0.14),
                                 QPointF(tileRect.right(), tileRect.bottom() - tileRect.height() * 0.14));
                painter.setPen(QPen(QColor(185, 190, 182, 46), qMax<qreal>(1.0, 1.0 * m_renderState.cameraZoom)));
                for (int i = 0; i < 5; ++i) {
                    const qreal seed = row * 17.0 + col * 29.0 + i * 11.0;
                    const QPointF p(tileRect.left() + qreal((int(seed) % 82) + 9) / 100.0 * tileRect.width(),
                                    tileRect.top() + qreal((int(seed * 1.7) % 76) + 12) / 100.0 * tileRect.height());
                    painter.drawPoint(p);
                }
            } else if (wallLike) {
                painter.fillRect(tileRect.adjusted(3, 3, -3, -3), QColor(255, 255, 255, 22));
                painter.setPen(QPen(QColor(45, 49, 52, 120), qMax<qreal>(1.0, 1.4 * m_renderState.cameraZoom)));
                const qreal blockH = tileRect.height() / 3.0;
                painter.drawLine(QPointF(tileRect.left(), tileRect.top() + blockH),
                                 QPointF(tileRect.right(), tileRect.top() + blockH));
                painter.drawLine(QPointF(tileRect.left(), tileRect.top() + blockH * 2.0),
                                 QPointF(tileRect.right(), tileRect.top() + blockH * 2.0));
                painter.drawLine(QPointF(tileRect.center().x(), tileRect.top()),
                                 QPointF(tileRect.center().x(), tileRect.top() + blockH));
                painter.drawLine(QPointF(tileRect.left() + tileRect.width() * 0.32, tileRect.top() + blockH),
                                 QPointF(tileRect.left() + tileRect.width() * 0.32, tileRect.top() + blockH * 2.0));
                painter.drawLine(QPointF(tileRect.left() + tileRect.width() * 0.68, tileRect.top() + blockH * 2.0),
                                 QPointF(tileRect.left() + tileRect.width() * 0.68, tileRect.bottom()));
                painter.setPen(QPen(QColor(238, 232, 206, 70), qMax<qreal>(1.0, 1.0 * m_renderState.cameraZoom)));
                painter.drawLine(tileRect.topLeft() + QPointF(4, 4), tileRect.topRight() + QPointF(-5, 4));
                painter.setPen(QPen(QColor(32, 35, 38, 120), qMax<qreal>(1.0, 1.2 * m_renderState.cameraZoom)));
                painter.drawLine(QPointF(tileRect.left() + tileRect.width() * 0.24, tileRect.top() + tileRect.height() * 0.30),
                                 QPointF(tileRect.left() + tileRect.width() * 0.38, tileRect.top() + tileRect.height() * 0.48));
                painter.drawLine(QPointF(tileRect.left() + tileRect.width() * 0.62, tileRect.top() + tileRect.height() * 0.56),
                                 QPointF(tileRect.left() + tileRect.width() * 0.76, tileRect.top() + tileRect.height() * 0.70));
            } else if (tile->getType() == TileType::Grass) {
                const int variant = qAbs((row * 37 + col * 19) % 5);
                painter.setPen(QPen(QColor(26, 86 + variant * 9, 34, 86), qMax<qreal>(1.0, 1.4 * m_renderState.cameraZoom)));
                for (int i = 0; i < 7; ++i) {
                    const qreal seed = row * 31.0 + col * 43.0 + i * 13.0;
                    const qreal x = tileRect.left() + qreal((int(seed) % 74) + 13) / 100.0 * tileRect.width();
                    const qreal y = tileRect.top() + qreal((int(seed * 1.9) % 70) + 15) / 100.0 * tileRect.height();
                    const qreal blade = tileRect.height() * (0.08 + (i % 3) * 0.025);
                    painter.drawLine(QPointF(x, y + blade), QPointF(x + ((i % 2) ? -blade * 0.32 : blade * 0.28), y - blade));
                }
                painter.setPen(QPen(QColor(153, 196, 82, 78), qMax<qreal>(1.0, 1.2 * m_renderState.cameraZoom)));
                painter.drawArc(tileRect.adjusted(tileRect.width() * 0.18, tileRect.height() * 0.24,
                                                  -tileRect.width() * 0.50, -tileRect.height() * 0.46),
                                20 * 16, 130 * 16);
                painter.drawArc(tileRect.adjusted(tileRect.width() * 0.52, tileRect.height() * 0.54,
                                                  -tileRect.width() * 0.16, -tileRect.height() * 0.14),
                                200 * 16, 120 * 16);
            }

            painter.setPen(QPen(edgeColor, qMax<qreal>(1.0, 1.5 * m_renderState.cameraZoom)));
            painter.drawRect(tileRect.adjusted(0.5, 0.5, -0.5, -0.5));
            painter.setPen(QPen(accentColor, 1));
            painter.drawRect(tileRect.adjusted(3, 3, -3, -3));

            if (m_showGrid) {
                painter.setPen(QPen(QColor(255, 255, 255, 55), 1));
                painter.drawRect(tileRect);
            }
        }
    }

    const QPointF topLeft = worldToScreen(QVector2D(0, 0));
    const QPointF bottomRight = worldToScreen(QVector2D(worldBounds.width(), worldBounds.height()));
    const QRectF trackRect(topLeft, bottomRight);
    painter.setPen(QPen(QColor(43, 245, 255, 190), 3));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(trackRect.normalized().adjusted(-4, -4, 4, 4), 10, 10);
}

void GameViewWidget::drawCyberGrid(QPainter& painter, const QRectF& worldBounds)
{
    painter.save();

    const QPointF topLeft = worldToScreen(QVector2D(worldBounds.left(), worldBounds.top()));
    const QPointF bottomRight = worldToScreen(QVector2D(worldBounds.right(), worldBounds.bottom()));
    const QRectF screenBounds(topLeft, bottomRight);
    const QRectF bounds = screenBounds.normalized().adjusted(-m_tileSize * m_renderState.cameraZoom,
                                                            -m_tileSize * m_renderState.cameraZoom,
                                                            m_tileSize * m_renderState.cameraZoom,
                                                            m_tileSize * m_renderState.cameraZoom);
    painter.setClipRect(rect());
    painter.setPen(QPen(QColor(0, 220, 255, 28), 1));

    const qreal step = qMax<qreal>(16.0, m_tileSize * m_renderState.cameraZoom);
    for (qreal x = bounds.left(); x <= bounds.right(); x += step) {
        painter.drawLine(QPointF(x, 0), QPointF(x, height()));
    }
    for (qreal y = bounds.top(); y <= bounds.bottom(); y += step) {
        painter.drawLine(QPointF(0, y), QPointF(width(), y));
    }

    painter.setPen(QPen(QColor(255, 45, 126, 24), 1));
    for (qreal x = bounds.left() + step * 0.5; x <= bounds.right(); x += step * 2.0) {
        painter.drawLine(QPointF(x, 0), QPointF(x, height()));
    }

    painter.restore();
}

void GameViewWidget::drawCheckpoints(QPainter& painter)
{
    if (!m_currentTrack) {
        return;
    }

    const QList<Checkpoint*> checkpoints = m_currentTrack->getCheckpointsInOrder();
    for (int i = 0; i < checkpoints.size(); ++i) {
        const Checkpoint* cp = checkpoints.at(i);
        if (!cp) {
            continue;
        }

        const QRectF worldBounds = cp->getBounds();
        const QPointF cornerA = worldToScreen(QVector2D(worldBounds.left(), worldBounds.top()));
        const QPointF cornerB = worldToScreen(QVector2D(worldBounds.right(), worldBounds.bottom()));
        const QRectF screenRect = QRectF(cornerA, cornerB).normalized();
        const QPixmap& checkpointImage = visualPixmap(QStringLiteral("track_markers/pd_checkpoint.png"));

        painter.save();
        painter.setPen(QPen(QColor(255, 216, 80, 80), 8));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(screenRect.adjusted(-2, -2, 2, 2), 8, 8);
        painter.setPen(QPen(QColor(255, 216, 80), 3));
        painter.setBrush(QColor(255, 216, 80, 55));
        painter.drawRoundedRect(screenRect, 6, 6);

        if (!checkpointImage.isNull()) {
            const qreal aspect = screenRect.height() > 0.0 ? screenRect.width() / screenRect.height() : 1.0;
            const qreal iconScale = aspect > 1.45 ? 0.76 : 0.64;
            const qreal iconSize = qMin(qMin(screenRect.width(), screenRect.height()) * iconScale,
                                        m_tileSize * m_renderState.cameraZoom * 1.6);
            const QRectF iconRect(screenRect.center().x() - iconSize / 2.0,
                                  screenRect.center().y() - iconSize / 2.0,
                                  iconSize,
                                  iconSize);
            painter.drawPixmap(iconRect, checkpointImage, checkpointImage.rect());
        }

        painter.restore();
    }
}

void GameViewWidget::drawStartFinishMarkers(QPainter& painter)
{
    if (!m_currentTrack) {
        return;
    }

    painter.save();

    const QList<QVector2D> starts = m_currentTrack->getStartPositions();
    if (!starts.isEmpty()) {
        const QPointF center = worldToScreen(starts.first());
        const QPixmap& startImage = visualPixmap(QStringLiteral("track_markers/pd_start.png"));
        if (!startImage.isNull()) {
            const qreal markerW = m_tileSize * m_renderState.cameraZoom * 1.8;
            const qreal markerH = m_tileSize * m_renderState.cameraZoom * 1.1;
            const QRectF marker(center.x() - markerW / 2.0, center.y() - markerH / 2.0, markerW, markerH);
            painter.drawPixmap(marker, startImage, startImage.rect());
        } else {
            const qreal glowRadius = 26.0;
            painter.setBrush(QColor(0, 230, 255, 55));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(center, glowRadius, glowRadius);
            const qreal radius = 18.0;
            const QRectF marker(center.x() - radius, center.y() - radius, radius * 2.0, radius * 2.0);
            painter.setBrush(QColor(0, 205, 255, 235));
            painter.setPen(QPen(QColor(226, 255, 255), 3));
            painter.drawEllipse(marker);
            painter.setPen(Qt::white);
            painter.setFont(QFont(QStringLiteral("Arial"), 8, QFont::Bold));
            painter.drawText(marker, Qt::AlignCenter, QStringLiteral("S"));
            const QRectF tag(center.x() - 34, center.y() + 20, 68, 20);
            painter.setBrush(QColor(5, 18, 32, 230));
            painter.setPen(QPen(QColor(0, 230, 255), 1));
            painter.drawRoundedRect(tag, 6, 6);
            painter.setPen(QColor(220, 255, 255));
            painter.setFont(QFont(QStringLiteral("Arial"), 8, QFont::Bold));
            painter.drawText(tag, Qt::AlignCenter, QStringLiteral("START"));
        }
    }

    for (int row = 0; row < m_currentTrack->getRowCount(); ++row) {
        for (int col = 0; col < m_currentTrack->getColCount(); ++col) {
            const TrackTile* tile = m_currentTrack->getTileAt(row, col);
            if (!tile) {
                continue;
            }
            const TileType type = tile->getType();
            if (type != TileType::FinishLine && type != TileType::StartLine) {
                continue;
            }

            const QPointF center = worldToScreen(TrackData::tileToWorldCenter(row, col, m_tileSize));
            const QString markerPath = type == TileType::StartLine
                ? QStringLiteral("track_markers/pd_start.png")
                : QStringLiteral("track_markers/pd_finish.png");
            const QPixmap& markerImage = visualPixmap(markerPath);
            if (!markerImage.isNull()) {
                const qreal markerW = m_tileSize * m_renderState.cameraZoom * 1.75;
                const qreal markerH = m_tileSize * m_renderState.cameraZoom * 1.05;
                const QRectF marker(center.x() - markerW / 2.0, center.y() - markerH / 2.0, markerW, markerH);
                painter.drawPixmap(marker, markerImage, markerImage.rect());
            } else {
                painter.setBrush(QColor(255, 70, 110, 55));
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(center, 30.0, 30.0);
                const QRectF flag(center.x() - 34.0, center.y() - 15.0, 68.0, 30.0);
                painter.setBrush(QColor(255, 255, 255, 235));
                painter.setPen(QPen(QColor(255, 66, 104), 3));
                painter.drawRoundedRect(flag, 4, 4);
                painter.setPen(QColor(231, 76, 60));
                painter.setFont(QFont(QStringLiteral("Arial"), 8, QFont::Bold));
                painter.drawText(flag, Qt::AlignCenter,
                                 type == TileType::StartLine ? QStringLiteral("START") : QStringLiteral("FINISH"));
            }
        }
    }

    painter.restore();
}

void GameViewWidget::drawPlayerCar(QPainter& painter, const GameRenderObject& car)
{
    drawRaceCar(painter, car, true);
}

void GameViewWidget::drawAICar(QPainter& painter, const GameRenderObject& car)
{
    drawRaceCar(painter, car, false);
}

void GameViewWidget::drawRaceCar(QPainter& painter, const GameRenderObject& car, bool playerCar)
{
    const CoinChallengeOverlayState& coinOverlay = m_renderState.coinChallengeOverlay;
    const bool coinDamageVisible = playerCar && coinOverlay.visible;
    const int damageStage = coinDamageVisible ? coinOverlay.integrityStage : 0;
    const bool wrecked = coinDamageVisible && coinOverlay.destroyed;
    const QPointF center = worldToScreen(car.position);
    const qreal halfW = car.size.width() * m_renderState.cameraZoom / 2.0;
    const qreal halfH = car.size.height() * m_renderState.cameraZoom / 2.0;
    const QColor fallbackBody = car.color.isValid() ? car.color : (playerCar ? QColor(228, 234, 242) : QColor(73, 153, 255));
    const SkinPalette palette = skinPaletteForId(car.extraData.value(QStringLiteral("skinId")).toString(),
                                                 fallbackBody);
    const QColor bodyColor = palette.primary;
    const QColor accentColor = palette.secondary;
    const QColor canopyColor = palette.canopy;
    const QColor glowBase = palette.glow;
    const QColor glowColor(glowBase.red(), glowBase.green(), glowBase.blue(), playerCar ? 96 : 56);

    painter.save();
    painter.translate(center);
    painter.rotate(car.rotation);

    if (car.extraData.value(QStringLiteral("shieldActive")).toBool()) {
        painter.setBrush(QColor(70, 210, 255, 42));
        painter.setPen(QPen(QColor(120, 240, 255, 190), qMax<qreal>(2.0, 2.5 * m_renderState.cameraZoom)));
        painter.drawEllipse(QPointF(0, 0), halfH * 1.32, halfH * 1.32);
    }

    if (car.extraData.value(QStringLiteral("magnetActive")).toBool()) {
        painter.setBrush(QColor(255, 188, 58, 36));
        painter.setPen(QPen(QColor(255, 202, 71, 180), qMax<qreal>(1.5, 2.0 * m_renderState.cameraZoom), Qt::DashLine));
        painter.drawEllipse(QPointF(0, 0), halfH * 1.55, halfH * 1.55);
    }

    if (car.extraData.value(QStringLiteral("boostActive")).toBool()) {
        QLinearGradient flame(0, halfH * 0.75, 0, halfH * 1.85);
        flame.setColorAt(0.0, QColor(255, 255, 255, 190));
        flame.setColorAt(0.36, QColor(0, 238, 255, 170));
        flame.setColorAt(1.0, QColor(255, 45, 126, 35));
        painter.setBrush(flame);
        painter.setPen(Qt::NoPen);
        QPolygonF flameShape;
        flameShape << QPointF(-halfW * 0.58, halfH * 0.68)
                   << QPointF(0, halfH * 1.92)
                   << QPointF(halfW * 0.58, halfH * 0.68)
                   << QPointF(0, halfH * 1.18);
        painter.drawPolygon(flameShape);
    }

    painter.setBrush(glowColor);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(0, 0), halfW * 1.5, halfH * 1.22);

    if (car.extraData.value(QStringLiteral("invisibleActive")).toBool()) {
        painter.setOpacity(0.42);
    }

    painter.setBrush(QColor(5, 8, 15, 235));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(QRectF(-halfW * 1.16, -halfH * 0.72, halfW * 0.38, halfH * 0.44), 2, 2);
    painter.drawRoundedRect(QRectF(halfW * 0.78, -halfH * 0.72, halfW * 0.38, halfH * 0.44), 2, 2);
    painter.drawRoundedRect(QRectF(-halfW * 1.16, halfH * 0.34, halfW * 0.38, halfH * 0.44), 2, 2);
    painter.drawRoundedRect(QRectF(halfW * 0.78, halfH * 0.34, halfW * 0.38, halfH * 0.44), 2, 2);

    QPainterPath body;
    body.moveTo(0, -halfH);
    body.lineTo(halfW * 0.72, -halfH * 0.58);
    body.lineTo(halfW * 0.96, halfH * 0.22);
    body.lineTo(halfW * 0.54, halfH * 0.94);
    body.lineTo(-halfW * 0.54, halfH * 0.94);
    body.lineTo(-halfW * 0.96, halfH * 0.22);
    body.lineTo(-halfW * 0.72, -halfH * 0.58);
    body.closeSubpath();

    QLinearGradient bodyGradient(0, -halfH, 0, halfH);
    bodyGradient.setColorAt(0.0, bodyColor.lighter(145));
    bodyGradient.setColorAt(0.36, bodyColor);
    bodyGradient.setColorAt(1.0, bodyColor.darker(185));
    painter.setBrush(bodyGradient);
    painter.setPen(QPen(QColor(238, 255, 255, playerCar ? 235 : 185), playerCar ? 2.4 : 1.7));
    painter.drawPath(body);

    QPainterPath stripePath;
    stripePath.moveTo(0, -halfH * 0.88);
    stripePath.lineTo(halfW * 0.16, -halfH * 0.18);
    stripePath.lineTo(halfW * 0.08, halfH * 0.88);
    stripePath.lineTo(-halfW * 0.08, halfH * 0.88);
    stripePath.lineTo(-halfW * 0.16, -halfH * 0.18);
    stripePath.closeSubpath();

    QLinearGradient stripeGradient(0, -halfH, 0, halfH);
    stripeGradient.setColorAt(0.0, accentColor.lighter(138));
    stripeGradient.setColorAt(1.0, accentColor.darker(120));
    painter.setBrush(stripeGradient);
    painter.setPen(Qt::NoPen);
    painter.drawPath(stripePath);

    if (palette.splitSides) {
        painter.setBrush(QColor(accentColor.red(), accentColor.green(), accentColor.blue(), 208));
        QPolygonF leftBlade;
        leftBlade << QPointF(-halfW * 0.72, -halfH * 0.1)
                  << QPointF(-halfW * 0.5, -halfH * 0.48)
                  << QPointF(-halfW * 0.3, halfH * 0.18)
                  << QPointF(-halfW * 0.52, halfH * 0.68);
        QPolygonF rightBlade;
        rightBlade << QPointF(halfW * 0.72, -halfH * 0.1)
                   << QPointF(halfW * 0.5, -halfH * 0.48)
                   << QPointF(halfW * 0.3, halfH * 0.18)
                   << QPointF(halfW * 0.52, halfH * 0.68);
        painter.drawPolygon(leftBlade);
        painter.drawPolygon(rightBlade);
    }

    if (palette.diagonalAccent) {
        painter.setBrush(QColor(255, 255, 255, 86));
        QPolygonF slash;
        slash << QPointF(-halfW * 0.08, -halfH * 0.72)
              << QPointF(halfW * 0.44, -halfH * 0.3)
              << QPointF(halfW * 0.18, halfH * 0.08)
              << QPointF(-halfW * 0.34, -halfH * 0.34);
        painter.drawPolygon(slash);
    }

    painter.setPen(QPen(QColor(255, 255, 255, 210), qMax<qreal>(1.0, 1.2 * m_renderState.cameraZoom)));
    painter.drawLine(QPointF(0, -halfH * 0.82), QPointF(0, halfH * 0.78));
    painter.setPen(QPen(accentColor.lighter(125), qMax<qreal>(1.5, 2.0 * m_renderState.cameraZoom)));
    painter.drawLine(QPointF(-halfW * 0.68, -halfH * 0.12), QPointF(-halfW * 0.48, halfH * 0.54));
    painter.drawLine(QPointF(halfW * 0.68, -halfH * 0.12), QPointF(halfW * 0.48, halfH * 0.54));

    QPainterPath cockpit;
    cockpit.addRoundedRect(QRectF(-halfW * 0.45, -halfH * 0.42, halfW * 0.9, halfH * 0.52), 4, 4);
    QLinearGradient canopyGradient(0, -halfH * 0.42, 0, halfH * 0.1);
    canopyGradient.setColorAt(0.0, canopyColor);
    canopyGradient.setColorAt(1.0, QColor(30, 64, 92, 210));
    painter.setBrush(canopyGradient);
    painter.setPen(QPen(QColor(170, 250, 255, 190), 1));
    painter.drawPath(cockpit);

    if (damageStage > 0 || wrecked) {
        const int sootAlpha = wrecked ? 138 : (damageStage >= 3 ? 110 : (damageStage == 2 ? 78 : 46));
        painter.setBrush(QColor(12, 14, 18, sootAlpha));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPointF(-halfW * 0.38, halfH * 0.08), halfW * 0.42, halfH * 0.30);
        painter.drawEllipse(QPointF(halfW * 0.34, halfH * 0.22), halfW * 0.34, halfH * 0.24);

        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(QColor(255, 255, 255, wrecked ? 180 : 138),
                            wrecked ? 2.2 : 1.5,
                            Qt::SolidLine,
                            Qt::RoundCap));
        painter.drawLine(QPointF(-halfW * 0.24, -halfH * 0.1), QPointF(halfW * 0.16, halfH * 0.14));
        painter.drawLine(QPointF(halfW * 0.05, -halfH * 0.28), QPointF(halfW * 0.34, halfH * 0.28));
        if (damageStage >= 2 || wrecked) {
            painter.drawLine(QPointF(-halfW * 0.44, halfH * 0.12), QPointF(-halfW * 0.06, halfH * 0.44));
            painter.drawLine(QPointF(halfW * 0.28, -halfH * 0.48), QPointF(halfW * 0.48, -halfH * 0.08));
        }
        if (damageStage >= 3 || wrecked) {
            painter.setPen(QPen(QColor(255, 163, 92, 210), 2.0, Qt::SolidLine, Qt::RoundCap));
            painter.drawLine(QPointF(-halfW * 0.12, halfH * 0.56), QPointF(-halfW * 0.24, halfH * 0.82));
            painter.drawLine(QPointF(halfW * 0.24, halfH * 0.52), QPointF(halfW * 0.36, halfH * 0.78));
            painter.setBrush(QColor(72, 72, 78, 126));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(QPointF(0.0, -halfH * 1.02), halfW * 0.46, halfH * 0.20);
            painter.drawEllipse(QPointF(-halfW * 0.12, -halfH * 1.28), halfW * 0.34, halfH * 0.16);
        }
        if (wrecked) {
            painter.setBrush(QColor(255, 76, 76, 68));
            painter.setPen(QPen(QColor(255, 188, 188, 175), 2.0));
            painter.drawRoundedRect(QRectF(-halfW * 0.54, -halfH * 0.02, halfW * 1.08, halfH * 0.20), 3.0, 3.0);
        }
    }

    painter.setBrush(QColor(255, 255, 255, 230));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(-halfW * 0.32, -halfH * 0.76), halfW * 0.12, halfW * 0.12);
    painter.drawEllipse(QPointF(halfW * 0.32, -halfH * 0.76), halfW * 0.12, halfW * 0.12);

    painter.setOpacity(1.0);
    painter.restore();

    painter.save();
    const QString id = car.extraData.value(QStringLiteral("playerId")).toString();
    const QString labelText = playerCar && !id.isEmpty() ? id : car.label;
    if (labelText.trimmed().isEmpty()) {
        painter.restore();
        return;
    }
    const qreal badgeW = qMax<qreal>(32.0, 10.0 + QFontMetrics(QFont(QStringLiteral("Arial"), 9, QFont::Bold)).horizontalAdvance(labelText));
    const QRectF badge(center.x() - badgeW / 2.0,
                       center.y() - halfH - (playerCar ? 28.0 : 24.0),
                       badgeW,
                       20.0);
    painter.setBrush(QColor(4, 10, 22, 220));
    painter.setPen(QPen(bodyColor.lighter(130), 1.5));
    painter.drawRoundedRect(badge, 6, 6);
    painter.setPen(QColor(235, 255, 255));
    painter.setFont(QFont(QStringLiteral("Arial"), 9, QFont::Bold));
    painter.drawText(badge, Qt::AlignCenter, labelText);
    painter.restore();
}

void GameViewWidget::drawPowerupBox(QPainter& painter, const GameRenderObject& box)
{
    QPointF center = worldToScreen(box.position);
    qreal halfW = box.size.width() * m_renderState.cameraZoom / 2;
    qreal halfH = box.size.height() * m_renderState.cameraZoom / 2;

    QRectF boxRect(center.x() - halfW, center.y() - halfH, halfW * 2, halfH * 2);
    const QString powerupType = box.extraData.value(QStringLiteral("powerupType")).toString();
    const bool balloonRushPowerup = powerupType.trimmed().contains(QStringLiteral("balloon"), Qt::CaseInsensitive);
    const QString iconPath = powerupIconPath(powerupType);
    const QPixmap icon = iconPath.isEmpty() ? QPixmap() : visualPixmap(iconPath);

    if (balloonRushPowerup) {
        const qreal auraRadius = qMin<qreal>(54.0, qMax<qreal>(36.0, 42.0 * m_renderState.cameraZoom));
        QRadialGradient aura(center, auraRadius);
        aura.setColorAt(0.0, QColor(255, 224, 150, 90));
        aura.setColorAt(0.55, QColor(96, 196, 255, 64));
        aura.setColorAt(1.0, QColor(96, 196, 255, 0));
        painter.setBrush(aura);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(center, auraRadius, auraRadius);

        painter.setBrush(QColor(6, 18, 36, 196));
        painter.setPen(QPen(QColor(255, 224, 148, 214), qMax<qreal>(1.6, 2.0 * m_renderState.cameraZoom)));
        painter.drawRoundedRect(boxRect.adjusted(-4.0, -4.0, 4.0, 4.0), 10.0, 10.0);

        const qreal iconSize = qMin<qreal>(78.0, qMax<qreal>(50.0, 66.0 * m_renderState.cameraZoom));
        const QRectF iconRect(center.x() - iconSize / 2.0,
                              center.y() - iconSize * 0.58,
                              iconSize,
                              iconSize * 0.92);
        if (!icon.isNull()) {
            painter.drawPixmap(iconRect, icon, icon.rect());
        }

        painter.setPen(QColor(255, 247, 218));
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 8, QFont::Black));
        painter.drawText(QRectF(center.x() - 34.0, center.y() + halfH + 1.0, 68.0, 16.0),
                         Qt::AlignCenter,
                         QStringLiteral("BONUS"));
    } else if (!icon.isNull()) {
        const qreal iconSize = qMin<qreal>(58.0, qMax<qreal>(38.0, 50.0 * m_renderState.cameraZoom));
        const QRectF iconRect(center.x() - iconSize / 2.0,
                              center.y() - iconSize / 2.0,
                              iconSize,
                              iconSize);
        painter.drawPixmap(iconRect, icon, icon.rect());
    } else {
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", box.label.size() > 2 ? 9 : 13, QFont::Bold));
        painter.drawText(boxRect, Qt::AlignCenter, box.label);
    }

    painter.setPen(QColor(0, 0, 0, 180));
    painter.setFont(QFont("Arial", 9, QFont::Bold));
    const QRectF labelRect = balloonRushPowerup
        ? QRectF(center.x() - 72, center.y() + halfH + 16, 144, 16)
        : QRectF(center.x() - 46, center.y() + halfH + 4, 92, 16);
    painter.drawText(labelRect.translated(1, 1),
                     Qt::AlignCenter,
                     powerupType);
    painter.setPen(QColor(255, 255, 255, 210));
    painter.drawText(labelRect, Qt::AlignCenter, powerupType);
}

void GameViewWidget::drawWorldEffects(QPainter& painter)
{
    painter.save();

    for (int i = 0; i < m_oilPuddlePositions.size(); ++i) {
        const QPointF center = worldToScreen(m_oilPuddlePositions.at(i));
        const qreal radius = (i < m_oilPuddleRadii.size() ? m_oilPuddleRadii.at(i) : 64.0)
                             * m_renderState.cameraZoom;
        painter.setBrush(QColor(20, 20, 20, 150));
        painter.setPen(QPen(QColor(60, 60, 60), 2));
        painter.drawEllipse(center, radius, radius * 0.75);
    }

    for (const QVector2D& missilePos : m_missilePositions) {
        const QPointF center = worldToScreen(missilePos);
        const qreal radius = 8.0 * m_renderState.cameraZoom;
        painter.setBrush(QColor(231, 76, 60));
        painter.setPen(QPen(QColor(255, 200, 120), 2));
        painter.drawEllipse(center, radius, radius);
        painter.setBrush(QColor(255, 180, 60, 180));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(center, radius * 1.8, radius * 1.2);
    }

    painter.restore();
}

void GameViewWidget::drawCollisionImpacts(QPainter& painter)
{
    if (m_collisionImpactEffects.isEmpty()) {
        return;
    }

    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    const qreal zoomScale = qBound<qreal>(0.85, m_renderState.cameraZoom, 1.4);

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    for (int i = 0; i < m_collisionImpactEffects.size(); ++i) {
        const CollisionImpactEffect& effect = m_collisionImpactEffects.at(i);
        const qreal elapsedMs = qMax<qreal>(0.0, nowMs - effect.startMs);
        const qreal progress = qBound<qreal>(0.0, elapsedMs / qMax<qreal>(1.0, effect.durationMs), 1.0);
        const qreal fade = 1.0 - progress;
        const QPointF center = worldToScreen(effect.worldPosition);
        const qreal intensityScale = 0.85 + effect.intensity * 0.15;
        const qreal radius = (24.0 + 36.0 * progress) * zoomScale * intensityScale;
        const qreal innerRadius = radius * (0.32 + 0.2 * progress);

        QColor outerColor(QStringLiteral("#FF6A2A"));
        outerColor.setAlphaF(qBound<qreal>(0.0, 0.82 * fade, 0.82));
        QColor innerColor(QStringLiteral("#FF3366"));
        innerColor.setAlphaF(qBound<qreal>(0.0, 0.34 * fade, 0.34));

        QRadialGradient flash(center, radius);
        flash.setColorAt(0.0, innerColor);
        flash.setColorAt(0.35, QColor(255, 106, 42, qRound(52 * fade)));
        flash.setColorAt(1.0, QColor(255, 106, 42, 0));
        painter.setBrush(flash);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(center, radius, radius);

        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(outerColor, qMax<qreal>(2.0, 4.0 * fade * zoomScale)));
        painter.drawEllipse(center, radius, radius);

        innerColor.setAlphaF(qBound<qreal>(0.0, 0.62 * fade, 0.62));
        painter.setPen(QPen(innerColor, qMax<qreal>(1.5, 2.4 * fade * zoomScale)));
        painter.drawEllipse(center, innerRadius, innerRadius);
    }

    painter.restore();
}

void GameViewWidget::drawCoinChallengeOverlay(QPainter& painter)
{
    const CoinChallengeOverlayState& overlay = m_renderState.coinChallengeOverlay;
    if (!overlay.visible) {
        return;
    }

    const int goalCoins = qMax(1, overlay.goalCoins);
    const int displayedRunCoins = m_coinGoalDisplayInitialized
        ? qMax(0, m_coinChallengeDisplayedRunCoins)
        : qMax(0, overlay.runCoins);
    const qreal goalProgress = qBound<qreal>(0.0,
                                             static_cast<qreal>(qMin(displayedRunCoins, goalCoins))
                                                 / static_cast<qreal>(goalCoins),
                                             1.0);
    const qreal timeProgress = qBound<qreal>(0.0, overlay.timeProgress, 1.0);
    const bool displayGoalComplete = displayedRunCoins >= goalCoins;
    const bool lowTime = !overlay.countdownActive && overlay.remainingSeconds <= 10;
    const bool countdownGo = overlay.countdownActive && overlay.countdownStage == 0;
    const QString timeLabel = overlay.countdownActive
        ? (countdownGo ? QStringLiteral("GO") : QStringLiteral("%1").arg(qMax(1, overlay.countdownSeconds)))
        : QStringLiteral("%1s").arg(qMax(0, overlay.remainingSeconds));
    const QString statusLabel = overlay.countdownActive
        ? (countdownGo ? QStringLiteral("LAUNCH") : QStringLiteral("START SEQUENCE"))
        : (displayGoalComplete ? QStringLiteral("GOAL COMPLETE") : QStringLiteral("LIVE RUN"));
    const int integrity = qBound(0, overlay.integrityPercent, 100);
    const qreal integrityProgress = qBound<qreal>(0.0, integrity / 100.0, 1.0);
    const QColor integrityColor = coinChallengeIntegrityColor(integrity);

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    const QRectF leftRect = coinGoalOverlayRect();

    QLinearGradient leftBg(leftRect.topLeft(), leftRect.bottomRight());
    leftBg.setColorAt(0.0, QColor(6, 18, 42, 232));
    leftBg.setColorAt(1.0, QColor(12, 9, 28, 228));
    painter.setBrush(leftBg);
    painter.setPen(QPen(QColor(245, 196, 81, 210), 2.0));
    painter.drawRoundedRect(leftRect, 20.0, 20.0);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor(61, 241, 255, 76), 1.0));
    painter.drawRoundedRect(leftRect.adjusted(6.0, 6.0, -6.0, -6.0), 14.0, 14.0);

    const QRectF coinBadge(leftRect.center().x() - 18.0, leftRect.top() + 14.0, 36.0, 36.0);
    painter.setBrush(QColor(255, 214, 92));
    painter.setPen(QPen(QColor(255, 244, 194), 2.2));
    painter.drawEllipse(coinBadge);
    painter.setPen(QPen(QColor(148, 92, 10), 1.6));
    painter.setFont(QFont(QStringLiteral("Segoe UI"), 14, QFont::Black));
    painter.drawText(coinBadge, Qt::AlignCenter, QStringLiteral("C"));

    painter.setPen(QColor(247, 214, 96));
    painter.setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
    painter.drawText(QRectF(leftRect.left() + 12.0, leftRect.top() + 56.0, leftRect.width() - 24.0, 14.0),
                     Qt::AlignCenter,
                     QStringLiteral("COIN GOAL"));

    const QRectF trackRect = coinGoalTrackRect();
    painter.setBrush(QColor(255, 255, 255, 30));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(trackRect, 13.0, 13.0);

    const qreal fillHeight = trackRect.height() * goalProgress;
    const QRectF fillRect(trackRect.left(),
                          trackRect.bottom() - fillHeight,
                          trackRect.width(),
                          fillHeight);
    QLinearGradient goalFill(fillRect.topLeft(), fillRect.bottomLeft());
    goalFill.setColorAt(0.0, displayGoalComplete ? QColor(93, 255, 188) : QColor(255, 207, 72));
    goalFill.setColorAt(0.5, QColor(255, 137, 47));
    goalFill.setColorAt(1.0, displayGoalComplete ? QColor(125, 255, 198) : QColor(0, 228, 255));
    painter.setBrush(goalFill);
    painter.drawRoundedRect(fillRect, 13.0, 13.0);

    if (m_coinGoalFlashProgress > 0.0) {
        const qreal flash = qBound<qreal>(0.0, m_coinGoalFlashProgress, 1.0);
        const QPointF flashCenter(trackRect.center().x(),
                                  qBound(trackRect.top() + 18.0,
                                         fillRect.isValid() ? fillRect.top() + 10.0 : trackRect.bottom() - 18.0,
                                         trackRect.bottom() - 18.0));

        QRadialGradient flashGlow(flashCenter, qMax(trackRect.width() * 3.0, 44.0));
        flashGlow.setColorAt(0.0, QColor(255, 248, 205, qRound(200.0 * flash)));
        flashGlow.setColorAt(0.45, QColor(255, 204, 92, qRound(96.0 * flash)));
        flashGlow.setColorAt(1.0, QColor(255, 204, 92, 0));
        painter.setBrush(flashGlow);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(trackRect.adjusted(-20.0, -18.0, 20.0, 18.0));

        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(QColor(255, 244, 188, qRound(215.0 * flash)), 2.0 + flash * 2.2));
        painter.drawRoundedRect(trackRect.adjusted(-3.0, -3.0, 3.0, 3.0), 15.0, 15.0);
    }

    painter.setPen(QColor(229, 243, 255));
    painter.setFont(QFont(QStringLiteral("Segoe UI"), 12, QFont::Black));
    painter.drawText(QRectF(leftRect.left() + 8.0, leftRect.bottom() - 68.0, leftRect.width() - 16.0, 22.0),
                     Qt::AlignCenter,
                     QStringLiteral("%1 / %2").arg(displayedRunCoins).arg(goalCoins));
    painter.setPen(displayGoalComplete ? QColor(125, 255, 198) : QColor(153, 230, 255, 210));
    painter.setFont(QFont(QStringLiteral("Segoe UI"), 10, QFont::Bold));
    painter.drawText(QRectF(leftRect.left() + 8.0, leftRect.bottom() - 43.0, leftRect.width() - 16.0, 16.0),
                     Qt::AlignCenter,
                     QStringLiteral("%1%").arg(qRound(goalProgress * 100.0)));

    const QRectF integrityRect(leftRect.right() + 14.0, leftRect.top(), 94.0, leftRect.height());
    QLinearGradient integrityBg(integrityRect.topLeft(), integrityRect.bottomRight());
    integrityBg.setColorAt(0.0, QColor(10, 18, 34, 232));
    integrityBg.setColorAt(1.0, QColor(22, 10, 22, 228));
    painter.setBrush(integrityBg);
    painter.setPen(QPen(QColor(255, 122, 122, 176), 2.0));
    painter.drawRoundedRect(integrityRect, 20.0, 20.0);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor(255, 255, 255, 34), 1.0));
    painter.drawRoundedRect(integrityRect.adjusted(6.0, 6.0, -6.0, -6.0), 14.0, 14.0);

    const QRectF hpBadge(integrityRect.center().x() - 18.0, integrityRect.top() + 14.0, 36.0, 36.0);
    painter.setBrush(QColor(255, 92, 92));
    painter.setPen(QPen(QColor(255, 221, 221), 2.0));
    painter.drawEllipse(hpBadge);
    painter.setPen(QColor(255, 244, 244));
    painter.setFont(QFont(QStringLiteral("Segoe UI"), 13, QFont::Black));
    painter.drawText(hpBadge, Qt::AlignCenter, QStringLiteral("HP"));

    painter.setPen(QColor(255, 172, 172));
    painter.setFont(QFont(QStringLiteral("Segoe UI"), 8, QFont::Bold));
    painter.drawText(QRectF(integrityRect.left() + 10.0, integrityRect.top() + 56.0, integrityRect.width() - 20.0, 14.0),
                     Qt::AlignCenter,
                     QStringLiteral("INTEGRITY"));

    const QRectF integrityTrack(integrityRect.left() + 34.0,
                                integrityRect.top() + 82.0,
                                26.0,
                                integrityRect.height() - 158.0);
    painter.setBrush(QColor(255, 255, 255, 26));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(integrityTrack, 13.0, 13.0);

    const qreal integrityFillHeight = integrityTrack.height() * integrityProgress;
    const QRectF integrityFill(integrityTrack.left(),
                               integrityTrack.bottom() - integrityFillHeight,
                               integrityTrack.width(),
                               integrityFillHeight);
    QLinearGradient integrityFillGradient(integrityFill.topLeft(), integrityFill.bottomLeft());
    integrityFillGradient.setColorAt(0.0, integrityColor.lighter(120));
    integrityFillGradient.setColorAt(1.0, integrityColor.darker(120));
    painter.setBrush(integrityFillGradient);
    painter.drawRoundedRect(integrityFill, 13.0, 13.0);

    if (overlay.damageFlashProgress > 0.0 || overlay.lowIntegrityPulseProgress > 0.0) {
        const qreal flash = qMax(overlay.damageFlashProgress,
                                 overlay.lowIntegrityPulseProgress * 0.82);
        QRadialGradient hpGlow(integrityTrack.center(), qMax(integrityTrack.width() * 3.2, 46.0));
        hpGlow.setColorAt(0.0, QColor(255, 228, 228, qRound(185.0 * flash)));
        hpGlow.setColorAt(0.42, QColor(integrityColor.red(), integrityColor.green(), integrityColor.blue(), qRound(110.0 * flash)));
        hpGlow.setColorAt(1.0, QColor(integrityColor.red(), integrityColor.green(), integrityColor.blue(), 0));
        painter.setBrush(hpGlow);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(integrityTrack.adjusted(-22.0, -20.0, 22.0, 20.0));
    }

    painter.setPen(QColor(236, 244, 255));
    painter.setFont(QFont(QStringLiteral("Segoe UI"), 12, QFont::Black));
    painter.drawText(QRectF(integrityRect.left() + 8.0, integrityRect.bottom() - 68.0, integrityRect.width() - 16.0, 22.0),
                     Qt::AlignCenter,
                     QStringLiteral("%1%").arg(integrity));
    painter.setPen(integrityColor);
    painter.setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
    painter.drawText(QRectF(integrityRect.left() + 8.0, integrityRect.bottom() - 44.0, integrityRect.width() - 16.0, 18.0),
                     Qt::AlignCenter,
                     overlay.integrityStatusText);

    if (!overlay.damagePopupText.isEmpty()) {
        const qreal popupLift = 14.0 + 18.0 * qBound<qreal>(0.0, overlay.damageFlashProgress, 1.0);
        painter.setPen(QColor(255, 208, 208));
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 11, QFont::Black));
        painter.drawText(QRectF(integrityRect.left() - 16.0,
                                integrityRect.top() - popupLift,
                                integrityRect.width() + 32.0,
                                20.0),
                         Qt::AlignCenter,
                         overlay.damagePopupText);
        painter.setPen(QColor(255, 165, 165, 220));
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 8, QFont::Bold));
        painter.drawText(QRectF(integrityRect.left() - 20.0,
                                integrityRect.top() + 2.0 - popupLift,
                                integrityRect.width() + 40.0,
                                16.0),
                         Qt::AlignCenter,
                         overlay.damagePopupDetail);
    }

    const qreal panelWidth = qBound<qreal>(286.0, width() * 0.25, 324.0);
    const qreal panelHeight = qBound<qreal>(420.0, height() * 0.74, 540.0);
    const QRectF panelRect(width() - panelWidth - 20.0,
                           qMax<qreal>(24.0, height() * 0.05),
                           panelWidth,
                           panelHeight);

    QLinearGradient panelBg(panelRect.topLeft(), panelRect.bottomRight());
    panelBg.setColorAt(0.0, QColor(6, 12, 28, 242));
    panelBg.setColorAt(1.0, QColor(10, 22, 48, 236));
    painter.setBrush(panelBg);
    painter.setPen(QPen(QColor(0, 190, 255, 100), 2.0));
    painter.drawRoundedRect(panelRect, 16.0, 16.0);

    painter.setPen(QColor(255, 51, 102));
    painter.setFont(QFont(QStringLiteral("Segoe UI"), 10, QFont::Bold, true));
    painter.drawText(QRectF(panelRect.left() + 18.0, panelRect.top() + 14.0, panelRect.width() - 36.0, 16.0),
                     Qt::AlignCenter,
                     QStringLiteral("/  COIN CHALLENGE  /"));

    painter.setPen(QPen(QColor(0, 190, 255, 35), 1.0));
    painter.drawLine(QPointF(panelRect.left() + 16.0, panelRect.top() + 40.0),
                     QPointF(panelRect.right() - 16.0, panelRect.top() + 40.0));

    auto drawTopMetric = [&](const QRectF& rect, const QString& title, const QString& value, const QColor& color) {
        painter.setPen(QColor(150, 198, 230, 185));
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 8, QFont::Bold));
        painter.drawText(QRectF(rect.left(), rect.top(), rect.width(), 12.0),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         title);
        painter.setPen(color);
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 18, QFont::Black));
        painter.drawText(QRectF(rect.left(), rect.top() + 12.0, rect.width(), 24.0),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         value);
    };

    const QRectF topMetricRect(panelRect.left() + 18.0, panelRect.top() + 50.0, panelRect.width() - 36.0, 40.0);
    drawTopMetric(QRectF(topMetricRect.left(), topMetricRect.top(), 96.0, topMetricRect.height()),
                  QStringLiteral("TIME LEFT"),
                  timeLabel,
                  lowTime ? QColor(255, 109, 109) : QColor(246, 195, 67));
    drawTopMetric(QRectF(topMetricRect.center().x() - 34.0, topMetricRect.top(), 68.0, topMetricRect.height()),
                  QStringLiteral("GOAL"),
                  QStringLiteral("%1%").arg(qRound(goalProgress * 100.0)),
                  displayGoalComplete ? QColor(46, 242, 122) : QColor(0, 229, 255));
    drawTopMetric(QRectF(topMetricRect.right() - 96.0, topMetricRect.top(), 96.0, topMetricRect.height()),
                  QStringLiteral("ACTIVE"),
                  QString::number(overlay.activeCoins),
                  QColor(204, 102, 255));

    painter.setPen(lowTime ? QColor(255, 112, 112) : QColor(120, 237, 255));
    painter.setFont(QFont(QStringLiteral("Segoe UI"), 8, QFont::Bold));
    painter.drawText(QRectF(panelRect.left() + 18.0, panelRect.top() + 90.0, panelRect.width() - 36.0, 14.0),
                     Qt::AlignCenter,
                     statusLabel);

    painter.setPen(QColor(136, 232, 255, 220));
    painter.setFont(QFont(QStringLiteral("Segoe UI"), 8, QFont::DemiBold));
    painter.drawText(QRectF(panelRect.left() + 18.0, panelRect.top() + 104.0, panelRect.width() - 36.0, 14.0),
                     Qt::AlignCenter,
                     overlay.routeHintText.isEmpty()
                        ? QStringLiteral("FOLLOW THE COIN TRAIL")
                        : overlay.routeHintText);

    const QRectF gaugeRect(panelRect.left() + 46.0, panelRect.top() + 124.0, panelRect.width() - 92.0, 144.0);
    const QPointF gaugeCenter(gaugeRect.center().x(), gaugeRect.top() + 74.0);
    const qreal radius = qMin(gaugeRect.width(), gaugeRect.height()) * 0.47;
    const qreal speedFraction = qBound<qreal>(0.0, overlay.speedKmh / 120.0, 1.0);
    const qreal gaugeStartDeg = 225.0;
    const qreal gaugeSpanDeg = 270.0;

    {
        QColor glow = lowTime ? QColor(255, 82, 118, 42) : QColor(0, 190, 255, 42);
        painter.setPen(QPen(glow, 18, Qt::SolidLine, Qt::RoundCap));
        painter.setBrush(Qt::NoBrush);
        painter.drawArc(QRectF(gaugeCenter.x() - radius, gaugeCenter.y() - radius, radius * 2.0, radius * 2.0),
                        static_cast<int>(gaugeStartDeg * 16.0),
                        static_cast<int>(-gaugeSpanDeg * 16.0));
    }

    painter.setPen(QPen(QColor(15, 22, 45, 200), 8, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(QRectF(gaugeCenter.x() - radius, gaugeCenter.y() - radius, radius * 2.0, radius * 2.0),
                    static_cast<int>(gaugeStartDeg * 16.0),
                    static_cast<int>(-gaugeSpanDeg * 16.0));

    QColor speedState = overlay.speedKmh < 60
        ? QColor(QStringLiteral("#2EF27A"))
        : (overlay.speedKmh < 100 ? QColor(QStringLiteral("#F6C343")) : QColor(QStringLiteral("#FF3366")));
    painter.setPen(QPen(speedState, 8, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(QRectF(gaugeCenter.x() - radius, gaugeCenter.y() - radius, radius * 2.0, radius * 2.0),
                    static_cast<int>(gaugeStartDeg * 16.0),
                    static_cast<int>(-gaugeSpanDeg * speedFraction * 16.0));

    for (int v = 0; v <= 120; v += 10) {
        const bool major = (v % 30 == 0);
        const qreal angleDeg = gaugeStartDeg - (static_cast<qreal>(v) / 120.0) * gaugeSpanDeg;
        const qreal angleRad = qDegreesToRadians(angleDeg);
        const qreal r1 = radius * (major ? 0.80 : 0.86);
        const qreal r2 = radius * 0.94;
        painter.setPen(QPen(major ? QColor(200, 230, 255, 200) : QColor(100, 130, 180, 120), major ? 2.0 : 1.0));
        painter.drawLine(QPointF(gaugeCenter.x() + r1 * qCos(angleRad), gaugeCenter.y() - r1 * qSin(angleRad)),
                         QPointF(gaugeCenter.x() + r2 * qCos(angleRad), gaugeCenter.y() - r2 * qSin(angleRad)));
    }

    painter.setPen(speedState);
    painter.setFont(QFont(QStringLiteral("Segoe UI"), 36, QFont::Black));
    painter.drawText(QRectF(gaugeCenter.x() - 72.0, gaugeCenter.y() + 6.0, 144.0, 38.0),
                     Qt::AlignCenter,
                     overlay.countdownActive
                         ? (countdownGo ? QStringLiteral("GO") : QString::number(qMax(1, overlay.countdownSeconds)))
                                             : QString::number(overlay.speedKmh));
    painter.setPen(QColor(140, 170, 220));
    painter.setFont(QFont(QStringLiteral("Segoe UI"), 9));
    painter.drawText(QRectF(gaugeCenter.x() - 54.0, gaugeCenter.y() + 42.0, 108.0, 16.0),
                     Qt::AlignCenter,
                     overlay.countdownActive ? QStringLiteral("GO") : QStringLiteral("km/h"));
    painter.setPen(QColor(170, 220, 255, 190));
    painter.setFont(QFont(QStringLiteral("Segoe UI"), 8, QFont::Bold));
    painter.drawText(QRectF(gaugeCenter.x() - 68.0, gaugeCenter.y() - 50.0, 136.0, 14.0),
                     Qt::AlignCenter,
                     overlay.countdownActive ? QStringLiteral("STARTING IN") : QStringLiteral("CURRENT SPEED"));

    auto drawInfoCard = [&](const QRectF& rect,
                            const QString& title,
                            const QString& value,
                            const QColor& titleColor,
                            const QColor& valueColor) {
        painter.setBrush(QColor(8, 16, 40, 228));
        painter.setPen(QPen(QColor(0, 190, 255, 58), 1.0));
        painter.drawRoundedRect(rect, 10.0, 10.0);
        painter.setPen(titleColor);
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 8, QFont::Bold));
        painter.drawText(QRectF(rect.left() + 12.0, rect.top() + 8.0, rect.width() - 24.0, 12.0),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         title);
        painter.setPen(valueColor);
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 13, QFont::Black));
        painter.drawText(QRectF(rect.left() + 12.0, rect.top() + 21.0, rect.width() - 24.0, 18.0),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         value);
    };

    const qreal cardLeft = panelRect.left() + 18.0;
    const qreal cardTop = panelRect.top() + 280.0;
    const qreal cardGap = 8.0;
    const qreal cardW = (panelRect.width() - 46.0) / 2.0;
    const qreal cardH = 42.0;
    drawInfoCard(QRectF(cardLeft, cardTop, cardW, cardH),
                 QStringLiteral("RUN COINS"),
                 QStringLiteral("+%1").arg(displayedRunCoins),
                 QColor(125, 255, 184),
                 QColor(125, 255, 184));
    drawInfoCard(QRectF(cardLeft + cardW + cardGap, cardTop, cardW, cardH),
                 QStringLiteral("TOTAL COINS"),
                 QString::number(overlay.totalCoins),
                 QColor(255, 243, 204),
                 QColor(255, 243, 204));
    drawInfoCard(QRectF(cardLeft, cardTop + cardH + cardGap, cardW, cardH),
                 QStringLiteral("AVERAGE SPEED"),
                 QStringLiteral("%1 km/h").arg(overlay.averageSpeedKmh),
                 QColor(0, 229, 255),
                 QColor(0, 229, 255));
    drawInfoCard(QRectF(cardLeft + cardW + cardGap, cardTop + cardH + cardGap, cardW, cardH),
                 QStringLiteral("MAX SPEED"),
                 QStringLiteral("%1 km/h").arg(overlay.maxSpeedKmh),
                 QColor(255, 174, 64),
                 QColor(255, 174, 64));
    drawInfoCard(QRectF(cardLeft, cardTop + (cardH + cardGap) * 2.0, cardW, cardH),
                 QStringLiteral("EFFICIENCY"),
                 QStringLiteral("%1 coins/min").arg(overlay.efficiencyCoinsPerMinute),
                 QColor(204, 102, 255),
                 QColor(204, 102, 255));
    drawInfoCard(QRectF(cardLeft + cardW + cardGap, cardTop + (cardH + cardGap) * 2.0, cardW, cardH),
                 QStringLiteral("INTEGRITY"),
                 QStringLiteral("%1%%").arg(integrity),
                 integrityColor,
                 integrityColor);

    painter.setPen(overlay.powerupActive ? QColor(255, 198, 112) : QColor(122, 168, 214));
    painter.setFont(QFont(QStringLiteral("Segoe UI"), 8, QFont::Bold));
    painter.drawText(QRectF(cardLeft, panelRect.bottom() - 54.0, panelRect.width() - 36.0, 14.0),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     overlay.powerupActive ? overlay.powerupLabel : QStringLiteral("POWERUP: NONE"));

    const QRectF bottomBarRect(panelRect.left() + 18.0, panelRect.bottom() - 24.0, panelRect.width() - 36.0, 11.0);
    painter.setBrush(QColor(255, 255, 255, 24));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(bottomBarRect, 5.0, 5.0);

    QLinearGradient timeFill(bottomBarRect.topLeft(), bottomBarRect.topRight());
    timeFill.setColorAt(0.0, lowTime ? QColor(255, 78, 78) : QColor(0, 208, 255));
    timeFill.setColorAt(1.0, lowTime ? QColor(255, 186, 72) : QColor(124, 255, 197));
    painter.setBrush(timeFill);
    painter.drawRoundedRect(QRectF(bottomBarRect.left(),
                                   bottomBarRect.top(),
                                   bottomBarRect.width() * timeProgress,
                                   bottomBarRect.height()),
                            5.0,
                            5.0);

    if (overlay.tenSecondWarningVisible && !overlay.countdownActive) {
        const qreal progress = qBound<qreal>(0.0, overlay.tenSecondWarningProgress, 1.0);
        const qreal fadeIn = qMin<qreal>(1.0, progress / 0.18);
        const qreal fadeOut = progress > 0.78 ? qMax<qreal>(0.0, (1.0 - progress) / 0.22) : 1.0;
        const qreal envelope = qMin(fadeIn, fadeOut);
        const qreal scale = 0.9 + 0.12 * qSin(progress * 3.14159265358979323846);
        const qreal wobble = qSin(progress * 22.0) * 10.0 * envelope;
        const int countdownValue = qBound(1, overlay.remainingSeconds, 10);
        const QPointF warningCenter(width() * 0.50, height() * 0.26);

        painter.save();
        painter.translate(warningCenter);
        painter.scale(scale, scale);

        const QRectF warningRect(-205.0, -64.0, 410.0, 128.0);
        painter.setBrush(QColor(4, 10, 26, qRound(230.0 * envelope)));
        painter.setPen(QPen(QColor(255, 111, 111, qRound(230.0 * envelope)), 3.0));
        painter.drawRoundedRect(warningRect, 24.0, 24.0);

        painter.setPen(QPen(QColor(255, 204, 92, qRound(180.0 * envelope)), 1.0));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(warningRect.adjusted(8.0, 8.0, -8.0, -8.0), 18.0, 18.0);

        painter.save();
        painter.translate(-126.0, 0.0);
        painter.rotate(wobble);
        painter.setPen(QPen(QColor(255, 212, 116, qRound(235.0 * envelope)), 3.0));
        painter.setBrush(QColor(255, 128, 76, qRound(92.0 * envelope)));
        painter.drawEllipse(QRectF(-20.0, -20.0, 40.0, 40.0));
        painter.drawEllipse(QRectF(-22.0, -36.0, 12.0, 12.0));
        painter.drawEllipse(QRectF(10.0, -36.0, 12.0, 12.0));
        painter.drawLine(QPointF(-10.0, 22.0), QPointF(-15.0, 30.0));
        painter.drawLine(QPointF(10.0, 22.0), QPointF(15.0, 30.0));
        painter.drawLine(QPointF(0.0, 0.0), QPointF(0.0, -9.0));
        painter.drawLine(QPointF(0.0, 0.0), QPointF(9.0, 5.0));
        painter.restore();

        painter.setPen(QColor(255, 241, 207, qRound(245.0 * envelope)));
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 28, QFont::Black));
        painter.drawText(QRectF(-66.0, -22.0, 224.0, 38.0),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         QStringLiteral("%1s").arg(countdownValue));

        painter.setPen(QColor(255, 120, 120, qRound(220.0 * envelope)));
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 12, QFont::Bold));
        painter.drawText(QRectF(-66.0, 16.0, 230.0, 22.0),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         QStringLiteral("SECONDS LEFT"));
        painter.restore();
    }

    if (overlay.lowIntegrityPulseProgress > 0.0 && !overlay.destroyed) {
        const qreal elapsed = 1.0 - qBound<qreal>(0.0, overlay.lowIntegrityPulseProgress, 1.0);
        const qreal fadeIn = qMin<qreal>(1.0, elapsed / 0.14);
        const qreal fadeOut = elapsed > 0.58 ? qMax<qreal>(0.0, (1.0 - elapsed) / 0.42) : 1.0;
        const qreal envelope = qMin(fadeIn, fadeOut);
        const qreal pulse = 0.82 + 0.18 * qSin(elapsed * 16.0);
        const qreal intensity = envelope * pulse;
        const int alpha = qRound(135.0 * intensity);
        painter.fillRect(rect(), QColor(120, 12, 12, alpha));

        const QPointF warningCenter(width() * 0.5, height() * 0.22);
        const qreal scale = 0.96 + 0.06 * intensity;
        const qreal wobble = qSin(elapsed * 18.0) * 7.0 * envelope;
        const QRectF warningRect(-230.0, -62.0, 460.0, 124.0);

        painter.save();
        painter.translate(warningCenter.x() + wobble, warningCenter.y());
        painter.scale(scale, scale);
        painter.setBrush(QColor(14, 8, 12, qRound(232.0 * envelope)));
        painter.setPen(QPen(QColor(255, 112, 112, qRound(235.0 * envelope)), 3.0));
        painter.drawRoundedRect(warningRect, 24.0, 24.0);

        painter.setPen(QColor(255, 235, 235, qRound(245.0 * envelope)));
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 24, QFont::Black));
        painter.drawText(QRectF(warningRect.left() + 20.0, warningRect.top() + 18.0, warningRect.width() - 40.0, 30.0),
                         Qt::AlignCenter,
                         QStringLiteral("INTEGRITY CRITICAL"));

        painter.setPen(QColor(255, 176, 176, qRound(226.0 * envelope)));
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 11, QFont::Bold));
        painter.drawText(QRectF(warningRect.left() + 26.0, warningRect.top() + 54.0, warningRect.width() - 52.0, 24.0),
                         Qt::AlignCenter,
                         QStringLiteral("%1% REMAINING").arg(qBound(0, overlay.integrityPercent, 100)));
        painter.drawText(QRectF(warningRect.left() + 26.0, warningRect.top() + 78.0, warningRect.width() - 52.0, 20.0),
                         Qt::AlignCenter,
                         QStringLiteral("AVOID FURTHER IMPACT"));
        painter.restore();
    }

    if (overlay.forcedEndActive) {
        const qreal progress = qBound<qreal>(0.0, overlay.forcedEndProgress, 1.0);
        const QString towSceneAsset = QStringLiteral("ui/carbroken_tow.png");
        const QPixmap& towSceneArt = visualPixmap(towSceneAsset);
        painter.fillRect(rect(), QColor(2, 6, 14, qRound(120.0 + progress * 92.0)));

        const QRectF banner(width() * 0.5 - 240.0, height() * 0.12, 480.0, 126.0);
        painter.setBrush(QColor(8, 14, 28, 232));
        painter.setPen(QPen(QColor(255, 118, 118, 220), 3.0));
        painter.drawRoundedRect(banner, 22.0, 22.0);
        painter.setPen(QColor(255, 226, 226));
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 24, QFont::Black));
        painter.drawText(QRectF(banner.left() + 18.0, banner.top() + 18.0, banner.width() - 36.0, 34.0),
                         Qt::AlignCenter,
                         overlay.forcedEndHeadline.isEmpty() ? QStringLiteral("RECOVERY") : overlay.forcedEndHeadline);
        painter.setPen(QColor(255, 188, 188));
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 11, QFont::Bold));
        painter.drawText(QRectF(banner.left() + 24.0, banner.top() + 62.0, banner.width() - 48.0, 40.0),
                         Qt::AlignCenter | Qt::TextWordWrap,
                         overlay.forcedEndDetail.isEmpty()
                             ? QStringLiteral("Tow truck is securing the vehicle.")
                             : overlay.forcedEndDetail);

        const qreal roadY = height() * 0.76;

        painter.setPen(QPen(QColor(120, 160, 200, 90), 6.0, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(width() * 0.16, roadY + 32.0), QPointF(width() * 0.84, roadY + 32.0));

        if (!towSceneArt.isNull()) {
            const qreal artWidth = qBound<qreal>(360.0, width() * 0.48, 720.0);
            const qreal aspect = towSceneArt.height() > 0
                ? static_cast<qreal>(towSceneArt.width()) / static_cast<qreal>(towSceneArt.height())
                : 1.72;
            const qreal artHeight = artWidth / qMax<qreal>(0.1, aspect);
            const qreal artX = -artWidth * 0.92 + (width() * 0.74) * easeOutCubic(qMin<qreal>(1.0, progress * 1.08));
            const qreal artY = roadY - artHeight * 0.72 + qSin(progress * 6.28318530717958647692) * 3.0;
            const QRectF artRect(artX, artY, artWidth, artHeight);

            painter.save();
            painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

            for (int i = 0; i < 4; ++i) {
                const qreal puffProgress = qBound<qreal>(0.0, progress * 1.15 - i * 0.12, 1.0);
                if (puffProgress <= 0.01) {
                    continue;
                }
                const qreal smokeX = artRect.left() + artRect.width() * (0.12 - i * 0.02) - puffProgress * 68.0;
                const qreal smokeY = artRect.top() + artRect.height() * (0.56 - i * 0.03) - puffProgress * 20.0;
                const qreal smokeRadius = 14.0 + puffProgress * (24.0 + i * 5.0);
                painter.setBrush(QColor(132, 136, 145, qRound(88.0 * (1.0 - puffProgress))));
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(QPointF(smokeX, smokeY), smokeRadius, smokeRadius * 0.74);
            }

            painter.setBrush(QColor(0, 0, 0, 78));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(QRectF(artRect.left() + artRect.width() * 0.12,
                                       roadY + 6.0,
                                       artRect.width() * 0.62,
                                       artRect.height() * 0.22));
            painter.drawPixmap(artRect, towSceneArt, towSceneArt.rect());
            painter.restore();
        } else {
            static bool loggedTowSceneFallback = false;
            if (!loggedTowSceneFallback) {
                loggedTowSceneFallback = true;
                qWarning() << "[CoinChallenge] Failed to load tow scene art, falling back to procedural overlay:"
                           << towSceneAsset;
            }
            const qreal truckX = -180.0 + (width() * 0.56) * easeOutCubic(qMin<qreal>(1.0, progress * 1.1));
            const qreal carX = truckX + 126.0;

            painter.save();
            painter.translate(truckX, roadY);
            painter.setBrush(QColor(255, 178, 72));
            painter.setPen(QPen(QColor(255, 232, 186), 2.0));
            painter.drawRoundedRect(QRectF(0.0, -16.0, 84.0, 32.0), 8.0, 8.0);
            painter.drawRoundedRect(QRectF(56.0, -36.0, 38.0, 24.0), 6.0, 6.0);
            painter.setBrush(QColor(30, 32, 40));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(QPointF(20.0, 18.0), 12.0, 12.0);
            painter.drawEllipse(QPointF(66.0, 18.0), 12.0, 12.0);
            painter.setPen(QPen(QColor(255, 226, 148), 4.0));
            painter.drawLine(QPointF(84.0, -8.0), QPointF(126.0, 4.0));
            painter.restore();

            painter.save();
            painter.translate(carX, roadY + 2.0);
            painter.setBrush(QColor(54, 18, 18, 222));
            painter.setPen(QPen(QColor(255, 132, 132, 168), 2.0));
            painter.drawRoundedRect(QRectF(0.0, -14.0, 74.0, 28.0), 8.0, 8.0);
            painter.setBrush(QColor(24, 24, 28));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(QPointF(16.0, 16.0), 10.0, 10.0);
            painter.drawEllipse(QPointF(58.0, 16.0), 10.0, 10.0);
            painter.setBrush(QColor(90, 90, 94, 122));
            painter.drawEllipse(QPointF(20.0, -22.0), 18.0, 8.0);
            painter.drawEllipse(QPointF(34.0, -34.0), 14.0, 6.0);
            painter.restore();
        }

        const QRectF towProgress(width() * 0.5 - 180.0, roadY + 58.0, 360.0, 12.0);
        painter.setBrush(QColor(255, 255, 255, 24));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(towProgress, 6.0, 6.0);
        painter.setBrush(QColor(255, 130, 92));
        painter.drawRoundedRect(QRectF(towProgress.left(),
                                       towProgress.top(),
                                       towProgress.width() * progress,
                                       towProgress.height()),
                                6.0,
                                6.0);
    }

    drawCoinChallengeCountdownOverlay(painter, overlay);
    painter.restore();
}

void GameViewWidget::drawCoinChallengeCountdownOverlay(QPainter& painter,
                                                       const CoinChallengeOverlayState& overlay)
{
    const bool finalCountdownActive = !overlay.countdownActive
        && overlay.remainingSeconds > 0
        && overlay.remainingSeconds <= 3;
    if (!overlay.visible || (!overlay.countdownActive && !finalCountdownActive)) {
        return;
    }

    const int countdownStage = finalCountdownActive
        ? qBound(1, overlay.remainingSeconds, 3)
        : qMax(0, overlay.countdownStage);
    const qreal progress = finalCountdownActive
        ? qBound<qreal>(0.0,
                        static_cast<qreal>(QDateTime::currentMSecsSinceEpoch() % 1000) / 1000.0,
                        1.0)
        : qBound<qreal>(0.0, overlay.countdownStageProgress, 1.0);
    const qreal pulse = qSin(progress * 3.14159265358979323846);
    const qreal scale = finalCountdownActive
        ? 0.94 + 0.15 * pulse
        : (countdownStage == 0
        ? 1.0 + 0.1 * pulse
        : 0.92 + 0.12 * pulse);
    const qreal alpha = finalCountdownActive
        ? 0.78 + 0.18 * (1.0 - progress * 0.4)
        : (countdownStage == 0
        ? 0.92
        : (0.5 + 0.45 * (1.0 - progress * 0.35)));
    const QPointF center(width() * 0.5, finalCountdownActive ? height() * 0.34 : height() * 0.38);
    const QColor accent = finalCountdownActive
        ? QColor(255, 124, 96)
        : (countdownStage == 0
        ? QColor(125, 255, 184)
        : QColor(255, 201, 83));
    const QString countdownText = !finalCountdownActive && countdownStage == 0
        ? QStringLiteral("GO!")
        : QString::number(qMax(1, countdownStage));

    painter.save();
    painter.translate(center);
    painter.scale(scale, scale);

    QRadialGradient halo(QPointF(0.0, 0.0), 180.0);
    halo.setColorAt(0.0, QColor(accent.red(), accent.green(), accent.blue(), qRound(124.0 * alpha)));
    halo.setColorAt(0.45, QColor(255, 255, 255, qRound(36.0 * alpha)));
    halo.setColorAt(1.0, QColor(accent.red(), accent.green(), accent.blue(), 0));
    painter.setBrush(halo);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QRectF(-186.0, -146.0, 372.0, 292.0));

    painter.save();
    painter.translate(0.0, -64.0);
    painter.setPen(QPen(QColor(236, 246, 255, qRound(232.0 * alpha)), 4.0, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(QRectF(-24.0, -24.0, 48.0, 48.0));
    painter.drawLine(QPointF(0.0, 0.0), QPointF(0.0, -10.0));
    painter.drawLine(QPointF(0.0, 0.0), QPointF(11.0, 5.0));
    painter.drawEllipse(QRectF(-30.0, -35.0, 10.0, 10.0));
    painter.drawEllipse(QRectF(20.0, -35.0, 10.0, 10.0));
    painter.restore();

    painter.setPen(QColor(3, 8, 18, qRound(210.0 * alpha)));
    painter.setFont(QFont(QStringLiteral("Segoe UI"), countdownStage == 0 ? 62 : 74, QFont::Black));
    painter.drawText(QRectF(-202.0, -26.0, 404.0, 104.0),
                     Qt::AlignCenter,
                     countdownText);

    painter.setPen(QColor(accent.red(), accent.green(), accent.blue(), qRound(246.0 * alpha)));
    painter.drawText(QRectF(-206.0, -30.0, 404.0, 104.0),
                     Qt::AlignCenter,
                     countdownText);

    painter.setPen(QColor(242, 248, 255, qRound(220.0 * alpha)));
    painter.setFont(QFont(QStringLiteral("Segoe UI"), 13, QFont::Bold));
    painter.drawText(QRectF(-230.0, 66.0, 460.0, 24.0),
                     Qt::AlignCenter,
                     finalCountdownActive
                         ? QStringLiteral("FINAL COUNTDOWN")
                         : (countdownStage == 0
                                ? QStringLiteral("COIN CHALLENGE LIVE")
                                : QStringLiteral("COIN CHALLENGE START")));

    if (finalCountdownActive) {
        painter.setPen(QColor(255, 232, 198, qRound(205.0 * alpha)));
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 11, QFont::Bold));
        painter.drawText(QRectF(-220.0, 88.0, 440.0, 20.0),
                         Qt::AlignCenter,
                         QStringLiteral("GET THE LAST COINS"));
    }
    painter.restore();
}

void GameViewWidget::drawMilestoneCelebrations(QPainter& painter)
{
    if (m_milestoneCelebrations.isEmpty()) {
        return;
    }

    const MilestoneCelebration& celebration = m_milestoneCelebrations.first();
    const qreal progress = qBound<qreal>(0.0, celebration.progress, 1.0);
    const qreal enterEnd = 0.22;
    const qreal holdEnd = 0.74;
    const qreal panelWidth = celebration.compact
        ? qBound<qreal>(316.0, width() * 0.40, 430.0)
        : qBound<qreal>(360.0, width() * 0.48, 520.0);
    const qreal panelHeight = celebration.compact
        ? qBound<qreal>(102.0, height() * 0.14, 126.0)
        : qBound<qreal>(126.0, height() * 0.16, 160.0);
    const qreal centerY = celebration.compact
        ? qBound<qreal>(132.0, height() * 0.23, 246.0)
        : qMax<qreal>(124.0, height() * 0.20);
    const qreal startX = celebration.compact ? width() * 0.5 : -panelWidth * 0.7;
    const qreal centerX = width() * 0.5;
    const qreal endX = celebration.compact ? width() * 0.5 : width() + panelWidth * 0.7;

    qreal x = centerX;
    qreal y = centerY;
    if (progress < enterEnd) {
        const qreal t = progress / enterEnd;
        const qreal eased = 1.0 - qPow(1.0 - t, 3.0);
        x = startX + (centerX - startX) * eased;
        if (celebration.compact) {
            y = centerY + (1.0 - eased) * 34.0;
        }
    } else if (progress > holdEnd) {
        const qreal t = (progress - holdEnd) / (1.0 - holdEnd);
        const qreal eased = t * t * t;
        x = centerX + (endX - centerX) * eased;
        if (celebration.compact) {
            y = centerY - eased * 44.0;
        }
    }

    const qreal fadeIn = qMin<qreal>(1.0, progress / 0.12);
    const qreal fadeOut = progress > 0.82 ? qMax<qreal>(0.0, (1.0 - progress) / 0.18) : 1.0;
    const qreal alpha = qMin(fadeIn, fadeOut);
    const qreal pulse = progress >= enterEnd && progress <= holdEnd
        ? (celebration.compact ? (0.98 + 0.04 * qSin((progress - enterEnd) * 24.0))
                               : (0.94 + 0.06 * qSin((progress - enterEnd) * 20.0)))
        : 1.0;
    const QRectF panelRect(x - panelWidth * 0.5, y - panelHeight * 0.5, panelWidth, panelHeight);
    const QColor accent = celebration.accent.isValid() ? celebration.accent : QColor(0, 214, 255);

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.translate(panelRect.center());
    painter.scale(pulse, pulse);
    painter.translate(-panelRect.center());

    QRadialGradient glow(panelRect.center(), panelWidth * (celebration.compact ? 0.74 : 0.66));
    glow.setColorAt(0.0, QColor(accent.red(), accent.green(), accent.blue(), qRound((celebration.compact ? 116.0 : 96.0) * alpha)));
    glow.setColorAt(1.0, QColor(accent.red(), accent.green(), accent.blue(), 0));
    painter.setBrush(glow);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(panelRect.adjusted(-18.0, -14.0, 18.0, 14.0), celebration.compact ? 28.0 : 34.0, celebration.compact ? 28.0 : 34.0);

    QLinearGradient panelBg(panelRect.topLeft(), panelRect.bottomRight());
    panelBg.setColorAt(0.0, QColor(5, 14, 32, qRound(236.0 * alpha)));
    panelBg.setColorAt(1.0, QColor(17, 8, 28, qRound(230.0 * alpha)));
    painter.setBrush(panelBg);
    painter.setPen(QPen(QColor(accent.red(), accent.green(), accent.blue(), qRound(214.0 * alpha)), 2.2));
    painter.drawRoundedRect(panelRect, celebration.compact ? 20.0 : 24.0, celebration.compact ? 20.0 : 24.0);

    painter.setPen(QPen(QColor(255, 255, 255, qRound(76.0 * alpha)), 1.0));
    painter.drawRoundedRect(panelRect.adjusted(8.0, 8.0, -8.0, -8.0), celebration.compact ? 14.0 : 18.0, celebration.compact ? 14.0 : 18.0);
    painter.fillRect(QRectF(panelRect.left() + 18.0, panelRect.top() + 18.0, 6.0, panelRect.height() - 36.0),
                     QColor(accent.red(), accent.green(), accent.blue(), qRound(210.0 * alpha)));

    const QPointF carCenter(panelRect.left() + (celebration.compact ? 66.0 : 84.0),
                            panelRect.center().y() + (celebration.compact ? 5.0 : 8.0));
    painter.save();
    painter.translate(carCenter);
    painter.setBrush(QColor(accent.red(), accent.green(), accent.blue(), qRound((celebration.compact ? 72.0 : 54.0) * alpha)));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(0.0, 0.0), celebration.compact ? 34.0 : 42.0, celebration.compact ? 24.0 : 28.0);

    painter.setBrush(QColor(230, 236, 244, qRound(238.0 * alpha)));
    painter.setPen(QPen(QColor(accent.red(), accent.green(), accent.blue(), qRound(228.0 * alpha)), 2.0));
    QPainterPath carShape;
    carShape.moveTo(-34.0, 6.0);
    carShape.lineTo(-20.0, -12.0);
    carShape.lineTo(6.0, -18.0);
    carShape.lineTo(28.0, -6.0);
    carShape.lineTo(36.0, 8.0);
    carShape.lineTo(16.0, 18.0);
    carShape.lineTo(-18.0, 18.0);
    carShape.closeSubpath();
    painter.drawPath(carShape);

    painter.setBrush(QColor(32, 58, 82, qRound(214.0 * alpha)));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(QRectF(-10.0, -12.0, 18.0, 14.0), 4.0, 4.0);
    painter.setBrush(QColor(12, 14, 18, qRound(240.0 * alpha)));
    painter.drawEllipse(QPointF(-20.0, 18.0), 8.0, 8.0);
    painter.drawEllipse(QPointF(20.0, 18.0), 8.0, 8.0);
    painter.restore();

    painter.setPen(QColor(248, 205, 92, qRound(236.0 * alpha)));
    painter.setFont(QFont(QStringLiteral("Segoe UI"), celebration.compact ? 10 : 11, QFont::Bold));
    painter.drawText(QRectF(panelRect.left() + (celebration.compact ? 112.0 : 132.0),
                            panelRect.top() + (celebration.compact ? 16.0 : 24.0),
                            panelRect.width() - (celebration.compact ? 126.0 : 154.0),
                            18.0),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     celebration.channelLabel.isEmpty() ? QStringLiteral("COIN CHALLENGE") : celebration.channelLabel);

    painter.setPen(QColor(245, 251, 255, qRound(248.0 * alpha)));
    painter.setFont(QFont(QStringLiteral("Segoe UI"), celebration.compact ? 18 : 22, QFont::Black));
    painter.drawText(QRectF(panelRect.left() + (celebration.compact ? 112.0 : 132.0),
                            panelRect.top() + (celebration.compact ? 34.0 : 42.0),
                            panelRect.width() - (celebration.compact ? 124.0 : 152.0),
                            celebration.compact ? 26.0 : 30.0),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     celebration.headline);

    painter.setPen(QColor(accent.red(), accent.green(), accent.blue(), qRound(240.0 * alpha)));
    painter.setFont(QFont(QStringLiteral("Segoe UI"), celebration.compact ? 12 : 14, QFont::Bold));
    painter.drawText(QRectF(panelRect.left() + (celebration.compact ? 112.0 : 132.0),
                            panelRect.top() + (celebration.compact ? 60.0 : 78.0),
                            panelRect.width() - (celebration.compact ? 124.0 : 152.0),
                            22.0),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     celebration.detail);
    painter.restore();
}

void GameViewWidget::drawBalloonRushSceneOverlay(QPainter& painter)
{
    const BalloonRushSceneState& scene = m_renderState.balloonRushScene;
    if (!scene.active) {
        return;
    }

    const QPixmap& balloonCar = visualPixmap(QStringLiteral("carballoon.png"));
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    if (scene.bonusSceneVisible) {
        QLinearGradient sky(rect().topLeft(), rect().bottomLeft());
        sky.setColorAt(0.0, QColor(26, 86, 188));
        sky.setColorAt(0.28, QColor(89, 178, 248));
        sky.setColorAt(0.64, QColor(177, 231, 255));
        sky.setColorAt(1.0, QColor(255, 241, 191));
        painter.fillRect(rect(), sky);

        QLinearGradient haze(QPointF(0.0, height() * 0.42), QPointF(0.0, height()));
        haze.setColorAt(0.0, QColor(255, 255, 255, 0));
        haze.setColorAt(1.0, QColor(255, 234, 186, 78));
        painter.fillRect(rect(), haze);

        painter.setPen(Qt::NoPen);
        auto drawCloudCluster = [&](const QPointF& center, qreal scale, int alpha) {
            painter.setBrush(QColor(255, 255, 255, alpha));
            painter.drawEllipse(QRectF(center.x() - 70.0 * scale, center.y() - 18.0 * scale, 82.0 * scale, 42.0 * scale));
            painter.drawEllipse(QRectF(center.x() - 18.0 * scale, center.y() - 32.0 * scale, 92.0 * scale, 54.0 * scale));
            painter.drawEllipse(QRectF(center.x() + 42.0 * scale, center.y() - 16.0 * scale, 88.0 * scale, 40.0 * scale));
            painter.drawEllipse(QRectF(center.x() - 38.0 * scale, center.y() + 2.0 * scale, 152.0 * scale, 34.0 * scale));
        };

        drawCloudCluster(QPointF(width() * 0.14, height() * 0.12), 1.15, 106);
        drawCloudCluster(QPointF(width() * 0.78, height() * 0.16), 1.0, 116);
        drawCloudCluster(QPointF(width() * 0.64, height() * 0.26), 0.72, 88);
        drawCloudCluster(QPointF(width() * 0.28, height() * 0.24), 0.56, 74);

        for (int i = 0; i < 12; ++i) {
            const qreal t = (scene.roadScroll * 0.12) + i * 0.19;
            const qreal sparkleXOffset = 0.17 * i + 0.11 * qSin(t * 2.4);
            const qreal sparkleYOffset = 0.13 * i + t * 0.07;
            const qreal sparkleX = width() * (0.10 + (sparkleXOffset - qFloor(sparkleXOffset / 0.78) * 0.78));
            const qreal sparkleY = height() * (0.08 + (sparkleYOffset - qFloor(sparkleYOffset / 0.38) * 0.38));
            const qreal radius = 1.6 + (i % 3) * 1.3;
            const int alpha = 88 + ((i * 17) % 90);
            painter.setBrush(QColor(255, 247, 212, alpha));
            painter.drawEllipse(QPointF(sparkleX, sparkleY), radius, radius);
            painter.setPen(QPen(QColor(255, 255, 255, qMin(255, alpha + 40)), 1.0));
            painter.drawLine(QPointF(sparkleX - radius * 1.8, sparkleY),
                             QPointF(sparkleX + radius * 1.8, sparkleY));
            painter.drawLine(QPointF(sparkleX, sparkleY - radius * 1.8),
                             QPointF(sparkleX, sparkleY + radius * 1.8));
            painter.setPen(Qt::NoPen);
        }

        for (int i = 0; i < 4; ++i) {
            const qreal depth = 0.18 + i * 0.15;
            const qreal islandY = height() * depth;
            const qreal islandWidth = width() * (0.12 - i * 0.012);
            const qreal islandX = (i % 2 == 0 ? width() * 0.16 : width() * 0.72) + qSin(scene.roadScroll * 0.22 + i * 0.8) * 18.0;
            QRectF islandRect(islandX, islandY, islandWidth, height() * 0.04);
            painter.setBrush(QColor(91, 120, 150, 52 - i * 6));
            painter.drawRoundedRect(islandRect, 14.0, 14.0);
            painter.setBrush(QColor(255, 248, 214, 32 - i * 4));
            painter.drawRoundedRect(islandRect.adjusted(10.0, 6.0, -14.0, -islandRect.height() * 0.55), 12.0, 12.0);
        }

        const QRectF roadRect(width() * 0.22, height() * 0.14, width() * 0.56, height() * 0.86);
        QPainterPath shoulderPath;
        shoulderPath.moveTo(roadRect.left() - width() * 0.05, roadRect.bottom());
        shoulderPath.lineTo(roadRect.left() + roadRect.width() * 0.26, roadRect.top());
        shoulderPath.lineTo(roadRect.right() - roadRect.width() * 0.26, roadRect.top());
        shoulderPath.lineTo(roadRect.right() + width() * 0.05, roadRect.bottom());
        shoulderPath.closeSubpath();
        QLinearGradient meadow(roadRect.bottomLeft(), roadRect.topLeft());
        meadow.setColorAt(0.0, QColor(103, 176, 92));
        meadow.setColorAt(1.0, QColor(173, 225, 166));
        painter.setBrush(meadow);
        painter.drawPath(shoulderPath);

        QLinearGradient roadFill(QPointF(roadRect.center().x(), roadRect.top()),
                                 QPointF(roadRect.center().x(), roadRect.bottom()));
        roadFill.setColorAt(0.0, QColor(88, 98, 124));
        roadFill.setColorAt(0.22, QColor(59, 70, 96));
        roadFill.setColorAt(1.0, QColor(25, 28, 42));
        painter.setBrush(roadFill);
        painter.setPen(QPen(QColor(233, 243, 255, 54), 2.0));
        painter.drawPath(shoulderPath);

        QLinearGradient roadCore(QPointF(roadRect.center().x(), roadRect.top()),
                                 QPointF(roadRect.center().x(), roadRect.bottom()));
        roadCore.setColorAt(0.0, QColor(255, 255, 255, 18));
        roadCore.setColorAt(0.4, QColor(255, 255, 255, 0));
        roadCore.setColorAt(0.8, QColor(255, 224, 166, 14));
        roadCore.setColorAt(1.0, QColor(255, 208, 118, 28));
        painter.setBrush(roadCore);
        painter.setPen(Qt::NoPen);
        painter.drawPath(shoulderPath);

        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(QColor(110, 236, 255, 116), 4.0));
        painter.drawLine(QPointF(roadRect.left() - width() * 0.035, roadRect.bottom()),
                         QPointF(roadRect.left() + roadRect.width() * 0.26, roadRect.top()));
        painter.drawLine(QPointF(roadRect.right() + width() * 0.035, roadRect.bottom()),
                         QPointF(roadRect.right() - roadRect.width() * 0.26, roadRect.top()));
        painter.setPen(QPen(QColor(255, 247, 216, 126), 1.8));
        painter.drawLine(QPointF(roadRect.left() - width() * 0.028, roadRect.bottom()),
                         QPointF(roadRect.left() + roadRect.width() * 0.255, roadRect.top()));
        painter.drawLine(QPointF(roadRect.right() + width() * 0.028, roadRect.bottom()),
                         QPointF(roadRect.right() - roadRect.width() * 0.255, roadRect.top()));

        const qreal dashBase = scene.roadScroll - qFloor(scene.roadScroll);
        for (int i = -1; i < 10; ++i) {
            const qreal depth = dashBase + i * 0.18;
            if (depth < 0.0 || depth > 1.2) {
                continue;
            }
            const qreal t = qBound<qreal>(0.0, depth, 1.0);
            const qreal y = roadRect.top() + roadRect.height() * t;
            const qreal widthScale = 0.24 + t * 0.76;
            const qreal dashHeight = 14.0 + 44.0 * t;
            const qreal halfGap = roadRect.width() * (0.10 + (1.0 - t) * 0.06);
            painter.setPen(QPen(QColor(255, 244, 199, 224), qMax<qreal>(2.0, 2.0 + 6.0 * t), Qt::SolidLine, Qt::RoundCap));
            painter.drawLine(QPointF(roadRect.center().x() - halfGap, y),
                             QPointF(roadRect.center().x() - halfGap, y + dashHeight * widthScale));
            painter.drawLine(QPointF(roadRect.center().x() + halfGap, y),
                             QPointF(roadRect.center().x() + halfGap, y + dashHeight * widthScale));
            painter.setPen(QPen(QColor(83, 226, 255, 62), qMax<qreal>(1.0, 1.0 + 4.0 * t), Qt::SolidLine, Qt::RoundCap));
            painter.drawLine(QPointF(roadRect.center().x(), y - 6.0 * (1.0 - t)),
                             QPointF(roadRect.center().x(), y + dashHeight * widthScale * 0.8));
        }

        auto laneCenterX = [&](qreal laneValue, qreal depth) {
            const qreal t = qBound<qreal>(0.0, depth, 1.0);
            const qreal horizonSpread = roadRect.width() * 0.12;
            const qreal nearSpread = roadRect.width() * 0.33;
            const qreal spread = horizonSpread + (nearSpread - horizonSpread) * t;
            return roadRect.center().x() + (laneValue - 1.0) * spread;
        };

        if (scene.bonusSceneVisible) {
            for (const QPointF& coin : scene.coinLayout) {
                const qreal depth = qBound<qreal>(0.0, coin.y(), 1.0);
                const qreal y = roadRect.top() + roadRect.height() * depth;
                const qreal x = laneCenterX(coin.x(), depth);
                const qreal radiusY = 9.5 + 25.5 * depth;
                const qreal spinPhase = scene.roadScroll * 2.75 + depth * 3.1 + coin.x() * 0.52;
                const qreal spin = qSin(spinPhase);
                const qreal widthFactor = 0.26 + 0.74 * qAbs(spin);
                const qreal radiusX = qMax<qreal>(3.2, radiusY * widthFactor);
                const bool edgeView = widthFactor < 0.34;

                painter.setBrush(QColor(106, 72, 12, qRound(44.0 + depth * 30.0)));
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(QPointF(x, y + radiusY * 0.16), radiusX * 0.88, radiusY * 0.20);

                QRadialGradient glow(QPointF(x, y), radiusY * 1.18);
                glow.setColorAt(0.0, QColor(255, 244, 198, 118));
                glow.setColorAt(0.48, QColor(255, 208, 78, 74));
                glow.setColorAt(1.0, QColor(255, 208, 78, 0));
                painter.setBrush(glow);
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(QPointF(x, y), radiusY * 1.04, radiusY * 1.04);

                if (edgeView) {
                    QLinearGradient edgeFill(QPointF(x - radiusX, y), QPointF(x + radiusX, y));
                    edgeFill.setColorAt(0.0, QColor(176, 110, 18));
                    edgeFill.setColorAt(0.5, QColor(255, 234, 148));
                    edgeFill.setColorAt(1.0, QColor(176, 110, 18));
                    painter.setBrush(edgeFill);
                    painter.setPen(QPen(QColor(255, 245, 210), qMax<qreal>(1.0, 1.0 + depth * 1.45)));
                    painter.drawRoundedRect(QRectF(x - radiusX, y - radiusY * 0.92, radiusX * 2.0, radiusY * 1.84),
                                            radiusX * 0.72,
                                            radiusX * 0.72);
                } else {
                    painter.setBrush(QColor(158, 92, 14, qRound(122.0 + depth * 44.0)));
                    painter.setPen(Qt::NoPen);
                    painter.drawEllipse(QPointF(x, y + radiusY * 0.10), radiusX * 0.96, radiusY * 0.96);

                    QLinearGradient coinFill(QPointF(x - radiusX, y - radiusY), QPointF(x + radiusX, y + radiusY));
                    coinFill.setColorAt(0.0, QColor(255, 247, 196));
                    coinFill.setColorAt(0.24, QColor(255, 221, 108));
                    coinFill.setColorAt(0.58, QColor(242, 176, 42));
                    coinFill.setColorAt(1.0, QColor(182, 110, 20));
                    painter.setBrush(coinFill);
                    painter.setPen(QPen(QColor(255, 246, 218), qMax<qreal>(1.0, 1.15 + depth * 1.85)));
                    painter.drawEllipse(QPointF(x, y), radiusX, radiusY);

                    painter.setBrush(Qt::NoBrush);
                    painter.setPen(QPen(QColor(165, 96, 16), qMax<qreal>(0.9, 1.0 + depth * 1.35)));
                    painter.drawEllipse(QPointF(x, y), radiusX * 0.76, radiusY * 0.78);

                    const qreal highlightOffset = (spin >= 0.0 ? -1.0 : 1.0) * radiusX * 0.32;
                    painter.setBrush(QColor(255, 254, 228, qRound(168.0 + depth * 34.0)));
                    painter.setPen(Qt::NoPen);
                    painter.drawEllipse(QPointF(x + highlightOffset, y - radiusY * 0.34),
                                        radiusX * 0.22,
                                        radiusY * 0.16);
                    painter.setBrush(QColor(255, 243, 186, 80));
                    painter.drawEllipse(QPointF(x - highlightOffset * 0.32, y + radiusY * 0.10),
                                        radiusX * 0.10,
                                        radiusY * 0.08);
                    painter.setBrush(QColor(255, 255, 255, qRound(42.0 + depth * 22.0)));
                    painter.drawEllipse(QPointF(x - radiusX * 0.10, y - radiusY * 0.18),
                                        radiusX * 0.10,
                                        radiusY * 0.10);

                    painter.setPen(QPen(QColor(132, 74, 14), qMax<qreal>(0.8, 0.95 + depth * 1.2)));
                    painter.setFont(QFont(QStringLiteral("Segoe UI"), qMax(8, qRound(8.0 + 17.0 * depth)), QFont::Black));
                    painter.drawText(QRectF(x - radiusX * 0.88,
                                            y - radiusY * 0.82,
                                            radiusX * 1.76,
                                            radiusY * 1.64),
                                     Qt::AlignCenter,
                                     QStringLiteral("C"));
                }
            }
        }

        if (scene.bonusSceneVisible) {
            const qreal carDepth = 0.9;
            const QPointF carCenter(laneCenterX(scene.laneVisual, carDepth), roadRect.top() + roadRect.height() * carDepth);
            const QRectF shadowRect(carCenter.x() - 68.0, carCenter.y() + 36.0, 136.0, 26.0);
            painter.setBrush(QColor(8, 14, 18, 76));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(shadowRect);

            if (!balloonCar.isNull()) {
                const qreal scale = 0.30;
                const QSizeF badgeSize(balloonCar.width() * scale, balloonCar.height() * scale);
                const QRectF badgeRect(carCenter.x() - badgeSize.width() * 0.5,
                                       carCenter.y() - badgeSize.height() * 0.78,
                                       badgeSize.width(),
                                       badgeSize.height());
                painter.drawPixmap(badgeRect.toRect(), balloonCar);
            } else {
                painter.setBrush(QColor(238, 70, 70));
                painter.setPen(QPen(QColor(255, 241, 241), 2.4));
                painter.drawRoundedRect(QRectF(carCenter.x() - 54.0, carCenter.y() - 36.0, 108.0, 50.0), 18.0, 18.0);
            }

            const QRectF leftHud(18.0, 18.0, qMin<qreal>(318.0, width() * 0.36), 144.0);
            painter.setBrush(QColor(4, 18, 42, 198));
            painter.setPen(QPen(QColor(255, 232, 153, 172), 2.0));
            painter.drawRoundedRect(leftHud, 20.0, 20.0);
            painter.setPen(QColor(255, 241, 194));
            painter.setFont(QFont(QStringLiteral("Segoe UI"), 10, QFont::Bold));
            painter.drawText(leftHud.adjusted(16.0, 12.0, -16.0, -108.0), Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("BALLOON RUSH GAIN"));
            painter.setFont(QFont(QStringLiteral("Segoe UI"), 28, QFont::Black));
            painter.setPen(QColor(255, 248, 216));
            painter.drawText(leftHud.adjusted(16.0, 28.0, -16.0, -70.0), Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("+%1").arg(scene.collectedCoins));
            painter.setPen(QColor(208, 240, 255, 220));
            painter.setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::DemiBold));
            painter.drawText(leftHud.adjusted(16.0, 64.0, -16.0, -50.0), Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("Catch the lane and surge the goal"));

            if (scene.gainFlashProgress > 0.0 && scene.recentGainValue > 0) {
                const qreal flash = qBound<qreal>(0.0, scene.gainFlashProgress, 1.0);
                const QRectF gainChip(leftHud.right() - 122.0, leftHud.top() + 24.0, 96.0, 34.0);
                painter.setBrush(QColor(255, 212, 90, qRound(160.0 * flash)));
                painter.setPen(QPen(QColor(255, 245, 220, qRound(220.0 * flash)), 1.6));
                painter.drawRoundedRect(gainChip, 16.0, 16.0);
                painter.setPen(QColor(74, 36, 4, qRound(240.0 * flash)));
                painter.setFont(QFont(QStringLiteral("Segoe UI"), 14, QFont::Black));
                painter.drawText(gainChip, Qt::AlignCenter, QStringLiteral("+%1").arg(scene.recentGainValue));
            }

            const QRectF goalLabelRect(leftHud.left() + 16.0, leftHud.top() + 92.0, leftHud.width() - 32.0, 16.0);
            painter.setPen(QColor(255, 233, 164));
            painter.setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
            painter.drawText(goalLabelRect, Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("COIN GOAL"));
            painter.setPen(QColor(226, 245, 255));
            painter.drawText(goalLabelRect, Qt::AlignRight | Qt::AlignVCenter, QStringLiteral("%1 / %2").arg(scene.runCoins).arg(qMax(1, scene.goalCoins)));

            const QRectF progressTrack(leftHud.left() + 16.0, leftHud.top() + 114.0, leftHud.width() - 32.0, 14.0);
            painter.setBrush(QColor(255, 255, 255, 28));
            painter.setPen(Qt::NoPen);
            painter.drawRoundedRect(progressTrack, 7.0, 7.0);
            QLinearGradient progressFill(progressTrack.topLeft(), progressTrack.topRight());
            progressFill.setColorAt(0.0, QColor(255, 206, 78));
            progressFill.setColorAt(0.55, QColor(255, 148, 58));
            progressFill.setColorAt(1.0, QColor(112, 255, 194));
            painter.setBrush(progressFill);
            painter.drawRoundedRect(QRectF(progressTrack.left(),
                                           progressTrack.top(),
                                           progressTrack.width() * qBound<qreal>(0.0, scene.goalProgress, 1.0),
                                           progressTrack.height()),
                                    7.0,
                                    7.0);
            if (scene.gainFlashProgress > 0.0) {
                painter.setBrush(Qt::NoBrush);
                painter.setPen(QPen(QColor(255, 250, 226, qRound(156.0 * scene.gainFlashProgress)), 2.0));
                painter.drawRoundedRect(progressTrack.adjusted(-2.0, -2.0, 2.0, 2.0), 8.0, 8.0);
            }

            const QRectF rightHud(width() - qMin<qreal>(220.0, width() * 0.28) - 18.0, 18.0, qMin<qreal>(220.0, width() * 0.28), 94.0);
            painter.setBrush(QColor(4, 18, 42, 190));
            painter.setPen(QPen(QColor(118, 228, 255, 156), 2.0));
            painter.drawRoundedRect(rightHud, 18.0, 18.0);
            painter.setPen(QColor(170, 238, 255));
            painter.setFont(QFont(QStringLiteral("Segoe UI"), 10, QFont::Bold));
            painter.drawText(rightHud.adjusted(16.0, 12.0, -16.0, -56.0), Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("RETURN"));
            painter.setPen(QColor(255, 247, 206));
            painter.setFont(QFont(QStringLiteral("Segoe UI"), 22, QFont::Black));
            painter.drawText(rightHud.adjusted(16.0, 34.0, -16.0, -24.0), Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("%1s").arg(qMax(0, scene.remainingSeconds)));
            painter.setPen(QColor(208, 240, 255, 220));
            painter.setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::DemiBold));
            painter.drawText(rightHud.adjusted(16.0, 64.0, -16.0, -10.0), Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("Main timer paused"));

            if (scene.introCountdownVisible && !scene.introCountdownLabel.isEmpty()) {
                const qreal pulse = qSin(scene.introCountdownProgress * 3.14159265358979323846);
                const qreal scale = 0.94 + 0.16 * pulse;
                const qreal alpha = scene.introCountdownProgress > 0.80
                    ? qMax<qreal>(0.0, (1.0 - scene.introCountdownProgress) / 0.20)
                    : 1.0;
                const QPointF countdownCenter(width() * 0.5, height() * 0.39);

                painter.save();
                painter.translate(countdownCenter);
                painter.scale(scale, scale);

                QRadialGradient halo(QPointF(0.0, 0.0), 178.0);
                halo.setColorAt(0.0, QColor(255, 236, 176, qRound(126.0 * alpha)));
                halo.setColorAt(0.42, QColor(255, 170, 62, qRound(82.0 * alpha)));
                halo.setColorAt(1.0, QColor(255, 170, 62, 0));
                painter.setBrush(halo);
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(QRectF(-188.0, -156.0, 376.0, 312.0));

                painter.setPen(QColor(14, 20, 32, qRound(228.0 * alpha)));
                painter.setFont(QFont(QStringLiteral("Segoe UI"), 96, QFont::Black));
                painter.drawText(QRectF(-168.0, -98.0, 336.0, 148.0),
                                 Qt::AlignCenter,
                                 scene.introCountdownLabel);
                painter.setPen(QColor(255, 247, 216, qRound(248.0 * alpha)));
                painter.drawText(QRectF(-172.0, -102.0, 336.0, 148.0),
                                 Qt::AlignCenter,
                                 scene.introCountdownLabel);
                painter.setPen(QColor(255, 205, 116, qRound(230.0 * alpha)));
                painter.setFont(QFont(QStringLiteral("Segoe UI"), 12, QFont::Bold));
                painter.drawText(QRectF(-156.0, 54.0, 312.0, 28.0),
                                 Qt::AlignCenter,
                                 QStringLiteral("BALLOON RUSH"));
                painter.restore();
            }

            if (scene.milestonePopupVisible && !scene.milestoneHeadline.isEmpty()) {
                const qreal progress = qBound<qreal>(0.0, scene.milestonePopupProgress, 1.0);
                const qreal fadeIn = qMin<qreal>(1.0, progress / 0.16);
                const qreal fadeOut = progress > 0.80 ? qMax<qreal>(0.0, (1.0 - progress) / 0.20) : 1.0;
                const qreal alpha = qMin(fadeIn, fadeOut);
                const qreal floatOffset = progress < 0.24
                    ? (1.0 - progress / 0.24) * 34.0
                    : (progress > 0.72 ? -(progress - 0.72) / 0.28 * 36.0 : 0.0);
                const QRectF panelRect(width() * 0.5 - qBound<qreal>(332.0, width() * 0.36, 420.0) * 0.5,
                                       height() * 0.24 + floatOffset,
                                       qBound<qreal>(332.0, width() * 0.36, 420.0),
                                       106.0);
                const QColor accent = scene.milestoneAccent.isValid() ? scene.milestoneAccent : QColor(255, 194, 88);

                QRadialGradient glow(panelRect.center(), panelRect.width() * 0.72);
                glow.setColorAt(0.0, QColor(accent.red(), accent.green(), accent.blue(), qRound(110.0 * alpha)));
                glow.setColorAt(1.0, QColor(accent.red(), accent.green(), accent.blue(), 0));
                painter.setBrush(glow);
                painter.setPen(Qt::NoPen);
                painter.drawRoundedRect(panelRect.adjusted(-16.0, -12.0, 16.0, 12.0), 28.0, 28.0);

                QLinearGradient bg(panelRect.topLeft(), panelRect.bottomRight());
                bg.setColorAt(0.0, QColor(5, 16, 34, qRound(234.0 * alpha)));
                bg.setColorAt(1.0, QColor(20, 10, 32, qRound(226.0 * alpha)));
                painter.setBrush(bg);
                painter.setPen(QPen(QColor(accent.red(), accent.green(), accent.blue(), qRound(218.0 * alpha)), 2.0));
                painter.drawRoundedRect(panelRect, 20.0, 20.0);

                painter.fillRect(QRectF(panelRect.left() + 18.0, panelRect.top() + 16.0, 6.0, panelRect.height() - 32.0),
                                 QColor(accent.red(), accent.green(), accent.blue(), qRound(214.0 * alpha)));
                painter.setPen(QColor(248, 210, 102, qRound(236.0 * alpha)));
                painter.setFont(QFont(QStringLiteral("Segoe UI"), 10, QFont::Bold));
                painter.drawText(QRectF(panelRect.left() + 38.0, panelRect.top() + 14.0, panelRect.width() - 56.0, 18.0),
                                 Qt::AlignLeft | Qt::AlignVCenter,
                                 QStringLiteral("BALLOON RUSH"));
                painter.setPen(QColor(246, 252, 255, qRound(246.0 * alpha)));
                painter.setFont(QFont(QStringLiteral("Segoe UI"), 22, QFont::Black));
                painter.drawText(QRectF(panelRect.left() + 38.0, panelRect.top() + 32.0, panelRect.width() - 56.0, 30.0),
                                 Qt::AlignLeft | Qt::AlignVCenter,
                                 scene.milestoneHeadline);
                painter.setPen(QColor(accent.red(), accent.green(), accent.blue(), qRound(238.0 * alpha)));
                painter.setFont(QFont(QStringLiteral("Segoe UI"), 13, QFont::Bold));
                painter.drawText(QRectF(panelRect.left() + 38.0, panelRect.top() + 64.0, panelRect.width() - 56.0, 22.0),
                                 Qt::AlignLeft | Qt::AlignVCenter,
                                 scene.milestoneDetail);
            }
        }

    }

    if (scene.returnTransitionVisible) {
        const qreal progress = qBound<qreal>(0.0, scene.returnTransitionProgress, 1.0);
        const qreal descendProgress = qMin<qreal>(1.0, progress / 0.74);
        const qreal settleProgress = progress > 0.74
            ? qBound<qreal>(0.0, (progress - 0.74) / 0.26, 1.0)
            : 0.0;
        const qreal descendEased = 1.0 - qPow(1.0 - descendProgress, 3.0);
        const qreal settleLift = qSin(settleProgress * 3.14159265358979323846) * 10.0;
        const qreal fade = settleProgress > 0.18
            ? qMax<qreal>(0.0, 1.0 - (settleProgress - 0.18) / 0.82)
            : 1.0;
        const QPointF returnCenter(width() * 0.5,
                                   -height() * 0.10
                                       + descendEased * height() * 0.54
                                       + settleProgress * height() * 0.03
                                       - settleLift);
        const qreal scale = 0.92 - settleProgress * 0.10;

        painter.fillRect(rect(), QColor(8, 18, 36, qRound(84.0 * fade)));
        painter.save();
        painter.translate(returnCenter);
        painter.scale(scale, scale);

        QRadialGradient halo(QPointF(0.0, 0.0), 210.0);
        halo.setColorAt(0.0, QColor(255, 240, 194, qRound(132.0 * fade)));
        halo.setColorAt(0.50, QColor(112, 214, 255, qRound(78.0 * fade)));
        halo.setColorAt(1.0, QColor(112, 214, 255, 0));
        painter.setBrush(halo);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QRectF(-210.0, -176.0, 420.0, 352.0));

        if (!balloonCar.isNull()) {
            const QSizeF imageSize(balloonCar.width() * 0.34, balloonCar.height() * 0.34);
            const QRectF imageRect(-imageSize.width() * 0.5,
                                   -imageSize.height() * 0.60,
                                   imageSize.width(),
                                   imageSize.height());
            painter.setOpacity(fade);
            painter.drawPixmap(imageRect.toRect(), balloonCar);
            painter.setOpacity(1.0);
        }

        painter.restore();
        painter.setPen(QColor(255, 247, 218, qRound(244.0 * fade)));
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 26, QFont::Black));
        painter.drawText(QRectF(0.0, height() * 0.18, width(), 36.0),
                         Qt::AlignCenter,
                         scene.headline.isEmpty() ? QStringLiteral("RETURNING TO TRACK") : scene.headline);
        painter.setPen(QColor(198, 236, 255, qRound(226.0 * fade)));
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 12, QFont::Bold));
        painter.drawText(QRectF(0.0, height() * 0.23, width(), 24.0),
                         Qt::AlignCenter,
                         scene.detail.isEmpty() ? QStringLiteral("Dropping back from Balloon Rush") : scene.detail);
    }

    if (scene.triggerVisible) {
        painter.fillRect(rect(), QColor(4, 12, 28, qRound(148.0 * scene.badgeOpacity)));
        const QPointF center(width() * 0.5, height() * 0.48 + scene.badgeYOffset);
        if (!balloonCar.isNull()) {
            const qreal scale = qBound<qreal>(0.35, scene.badgeScale, 1.45);
            const QSizeF imageSize(balloonCar.width() * scale, balloonCar.height() * scale);
            const QRectF imageRect(center.x() - imageSize.width() * 0.5,
                                   center.y() - imageSize.height() * 0.5,
                                   imageSize.width(),
                                   imageSize.height());
            painter.setOpacity(scene.badgeOpacity);
            painter.drawPixmap(imageRect.toRect(), balloonCar);
            painter.setOpacity(1.0);
        }

        painter.setPen(QColor(255, 246, 210, qRound(255.0 * scene.badgeOpacity)));
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 28, QFont::Black));
        painter.drawText(QRectF(0.0, height() * 0.12, width(), 44.0), Qt::AlignCenter, scene.headline);
        painter.setPen(QColor(206, 242, 255, qRound(228.0 * scene.badgeOpacity)));
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 13, QFont::Bold));
        painter.drawText(QRectF(0.0, height() * 0.18, width(), 26.0), Qt::AlignCenter, scene.detail);
    }

    painter.restore();
}

void GameViewWidget::drawCoinGoalAnimations(QPainter& painter)
{
    if (m_coinFlyAnimations.isEmpty()) {
        return;
    }

    auto lerpPoint = [](const QPointF& a, const QPointF& b, qreal t) {
        return QPointF(a.x() + (b.x() - a.x()) * t,
                       a.y() + (b.y() - a.y()) * t);
    };

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    for (const CoinFlyAnim& anim : m_coinFlyAnimations) {
        const qreal t = qBound<qreal>(0.0, anim.t, 1.0);
        const qreal eased = 1.0 - qPow(1.0 - t, 3.0);
        const QPointF ab = lerpPoint(anim.start, anim.control, eased);
        const QPointF bc = lerpPoint(anim.control, anim.end, eased);
        const QPointF position = lerpPoint(ab, bc, eased);
        const qreal popT = qMin<qreal>(1.0, t / 0.22);
        const qreal popScale = 1.0
            + 0.28 * qSin(popT * 3.14159265358979323846) * (1.0 - qMin<qreal>(1.0, t * 0.9));
        const qreal scale = (anim.startScale + (anim.endScale - anim.startScale) * eased) * popScale;
        const qreal rotation = anim.rotation + anim.rotationSpeed * eased;
        const qreal trailAlpha = qMax<qreal>(0.0, 1.0 - t * 0.78);

        painter.save();
        painter.translate(position);
        painter.rotate(rotation);
        painter.scale(scale, scale);

        QRadialGradient glow(QPointF(0.0, 0.0), 24.0);
        glow.setColorAt(0.0, QColor(255, 248, 196, qRound(190.0 * trailAlpha)));
        glow.setColorAt(0.45, QColor(255, 208, 72, qRound(102.0 * trailAlpha)));
        glow.setColorAt(1.0, QColor(255, 208, 72, 0));
        painter.setBrush(glow);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPointF(0.0, 0.0), 24.0, 24.0);

        painter.setBrush(QColor(214, 148, 20));
        painter.setPen(QPen(QColor(255, 236, 165), 2.1));
        painter.drawEllipse(QPointF(0.0, 0.0), 12.0, 12.0);

        painter.setBrush(QColor(255, 214, 92));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPointF(0.0, 0.0), 8.1, 8.1);

        painter.setBrush(QColor(255, 252, 208, qRound(175.0 * trailAlpha)));
        painter.drawEllipse(QPointF(-3.4, -3.6), 2.7, 2.1);

        painter.setPen(QPen(QColor(160, 98, 10), 1.5));
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 12, QFont::Black));
        painter.drawText(QRectF(-8.8, -9.8, 17.6, 19.6), Qt::AlignCenter, QStringLiteral("C"));

        painter.restore();
    }

    painter.restore();
}

void GameViewWidget::advanceCoinGoalAnimations()
{
    bool needsUpdate = false;

    for (int i = m_coinFlyAnimations.size() - 1; i >= 0; --i) {
        CoinFlyAnim& anim = m_coinFlyAnimations[i];
        anim.t += 0.016 / qMax<qreal>(0.1, anim.duration);
        needsUpdate = true;

        if (anim.t >= 1.0) {
            m_coinChallengeDisplayedRunCoins = qMin(m_coinChallengeTargetRunCoins,
                                                    m_coinChallengeDisplayedRunCoins + qMax(1, anim.value));
            m_coinGoalFlashProgress = qMax(m_coinGoalFlashProgress, 1.0);
            m_coinFlyAnimations.removeAt(i);
        }
    }

    if (m_coinGoalFlashProgress > 0.0) {
        m_coinGoalFlashProgress = qMax<qreal>(0.0, m_coinGoalFlashProgress - 0.085);
        needsUpdate = true;
    }

    if (m_coinFlyAnimations.isEmpty() && m_coinGoalFlashProgress <= 0.0) {
        if (m_coinChallengeDisplayedRunCoins != m_coinChallengeTargetRunCoins) {
            m_coinChallengeDisplayedRunCoins = m_coinChallengeTargetRunCoins;
            needsUpdate = true;
        }
        m_coinFlyAnimTimer->stop();
    }

    if (needsUpdate) {
        update();
    }
}

void GameViewWidget::advanceMilestoneCelebrations()
{
    if (m_milestoneCelebrations.isEmpty()) {
        if (m_milestoneAnimTimer->isActive()) {
            m_milestoneAnimTimer->stop();
        }
        return;
    }

    MilestoneCelebration& celebration = m_milestoneCelebrations.first();
    celebration.progress += 0.016 / qMax<qreal>(0.15, celebration.duration);
    if (celebration.progress >= 1.0) {
        m_milestoneCelebrations.removeFirst();
        if (m_milestoneCelebrations.isEmpty()) {
            m_milestoneAnimTimer->stop();
        }
    }
    update();
}

QRectF GameViewWidget::coinGoalOverlayRect() const
{
    const qreal leftWidth = 88.0;
    const qreal leftHeight = qBound<qreal>(260.0, height() * 0.54, 360.0);
    return QRectF(18.0, qMax<qreal>(90.0, height() * 0.16), leftWidth, leftHeight);
}

QRectF GameViewWidget::coinGoalTrackRect() const
{
    const QRectF leftRect = coinGoalOverlayRect();
    return QRectF(leftRect.left() + 31.0,
                  leftRect.top() + 82.0,
                  26.0,
                  leftRect.height() - 158.0);
}

QPointF GameViewWidget::coinGoalAnimationTarget(int value) const
{
    const QRectF trackRect = coinGoalTrackRect();
    const int goalCoins = qMax(1, m_renderState.coinChallengeOverlay.goalCoins);
    const int projectedCoins = qBound(0,
                                      m_coinChallengeDisplayedRunCoins + qMax(1, value),
                                      goalCoins);
    const qreal progress = qBound<qreal>(0.0,
                                         static_cast<qreal>(projectedCoins) / static_cast<qreal>(goalCoins),
                                         1.0);
    const qreal verticalPadding = 12.0;
    const qreal targetY = trackRect.bottom() - verticalPadding
        - (trackRect.height() - verticalPadding * 2.0) * progress;
    return QPointF(trackRect.center().x(),
                   qBound(trackRect.top() + verticalPadding,
                          targetY,
                          trackRect.bottom() - verticalPadding));
}

void GameViewWidget::drawCoin(QPainter& painter, const GameRenderObject& coin)
{
    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    const qreal elapsedSeconds = static_cast<qreal>(nowMs) / 1000.0;
    const qreal phase = coin.extraData.value(QStringLiteral("phaseSeed"), 0).toInt() * 0.0174532925;
    const qreal bobOffset = qSin(elapsedSeconds * 2.8 + phase) * qMax<qreal>(1.5, coin.size.height() * 0.08);
    const QPointF center = worldToScreen(coin.position) + QPointF(0.0, bobOffset);
    const qreal baseRadius = coin.extraData.value(QStringLiteral("radius"), coin.size.width() * 0.5).toReal();
    const qreal radius = qMax<qreal>(6.0, baseRadius * m_renderState.cameraZoom);
    const qreal scaleX = 0.45 + 0.55 * qAbs(qSin(elapsedSeconds * 5.2 + phase));

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRadialGradient outerGlow(center, radius * 1.7);
    outerGlow.setColorAt(0.0, QColor(255, 243, 176, 180));
    outerGlow.setColorAt(0.55, QColor(255, 210, 72, 90));
    outerGlow.setColorAt(1.0, QColor(255, 210, 72, 0));
    painter.setBrush(outerGlow);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(center, radius * 1.7, radius * 1.7);

    painter.translate(center);
    painter.scale(scaleX, 1.0);

    painter.setBrush(QColor(214, 148, 20));
    painter.setPen(QPen(QColor(255, 236, 165), qMax<qreal>(1.5, radius * 0.15)));
    painter.drawEllipse(QPointF(0.0, 0.0), radius, radius);

    painter.setBrush(QColor(255, 214, 92));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(0.0, 0.0), radius * 0.68, radius * 0.68);

    painter.setBrush(QColor(255, 252, 208, 170));
    painter.drawEllipse(QPointF(-radius * 0.28, -radius * 0.3),
                        radius * 0.2,
                        radius * 0.16);

    if (scaleX > 0.7) {
        painter.setPen(QPen(QColor(160, 98, 10), qMax<qreal>(1.2, radius * 0.1)));
        painter.setFont(QFont(QStringLiteral("Segoe UI"), qMax(8, qRound(radius * 0.95)), QFont::Black));
        painter.drawText(QRectF(-radius * 0.62, -radius * 0.68, radius * 1.24, radius * 1.36),
                         Qt::AlignCenter,
                         QStringLiteral("C"));
    } else {
        painter.setPen(QPen(QColor(255, 238, 178), qMax<qreal>(1.1, radius * 0.1)));
        painter.drawLine(QPointF(0.0, -radius * 0.78), QPointF(0.0, radius * 0.78));
    }

    painter.restore();
}

void GameViewWidget::drawChallengeObstacle(QPainter& painter, const GameRenderObject& obstacle)
{
    const QPointF center = worldToScreen(obstacle.position);
    const qreal width = obstacle.size.width() * m_renderState.cameraZoom;
    const qreal height = obstacle.size.height() * m_renderState.cameraZoom;
    const int obstacleType = obstacle.extraData.value(QStringLiteral("obstacleType")).toInt();
    const bool circular = obstacle.extraData.value(QStringLiteral("circular")).toBool();
    const qreal radius = obstacle.extraData.value(QStringLiteral("radius")).toDouble() * m_renderState.cameraZoom;

    painter.save();
    painter.translate(center);
    painter.rotate(obstacle.rotation);

    if (circular && radius > 0.0) {
        switch (obstacleType) {
        case static_cast<int>(ChallengeObstacleType::Tree): {
            painter.setBrush(QColor(18, 32, 16, 52));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(QRectF(-radius * 0.92, radius * 0.58, radius * 1.84, radius * 0.42));

            painter.setBrush(QColor(92, 64, 28, 220));
            painter.setPen(Qt::NoPen);
            painter.drawRoundedRect(QRectF(-radius * 0.14, radius * 0.08, radius * 0.28, radius * 0.64), 3.0, 3.0);

            auto drawCanopyLayer = [&](const QPointF& centerPoint,
                                       qreal rx,
                                       qreal ry,
                                       const QColor& inner,
                                       const QColor& outer) {
                QRadialGradient canopy(centerPoint + QPointF(-rx * 0.18, -ry * 0.22), qMax(rx, ry) * 1.12);
                canopy.setColorAt(0.0, inner);
                canopy.setColorAt(1.0, outer);
                painter.setBrush(canopy);
                painter.setPen(QPen(QColor(214, 255, 194, 120), 1.1));
                painter.drawEllipse(QRectF(centerPoint.x() - rx,
                                           centerPoint.y() - ry,
                                           rx * 2.0,
                                           ry * 2.0));
            };

            drawCanopyLayer(QPointF(-radius * 0.34, -radius * 0.12),
                            radius * 0.52,
                            radius * 0.44,
                            QColor(182, 246, 146),
                            QColor(38, 114, 48));
            drawCanopyLayer(QPointF(radius * 0.34, -radius * 0.10),
                            radius * 0.50,
                            radius * 0.42,
                            QColor(170, 238, 134),
                            QColor(32, 106, 44));
            drawCanopyLayer(QPointF(0.0, -radius * 0.44),
                            radius * 0.60,
                            radius * 0.48,
                            QColor(194, 255, 162),
                            QColor(46, 128, 54));
            drawCanopyLayer(QPointF(0.0, radius * 0.02),
                            radius * 0.46,
                            radius * 0.36,
                            QColor(118, 210, 104),
                            QColor(24, 96, 42));

            painter.setBrush(QColor(232, 255, 214, 62));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(QRectF(-radius * 0.42, -radius * 0.64, radius * 0.48, radius * 0.22));
            break;
        }
        case static_cast<int>(ChallengeObstacleType::Rock): {
            QPainterPath rockPath;
            rockPath.moveTo(-radius * 0.85, radius * 0.28);
            rockPath.cubicTo(-radius * 1.08, -radius * 0.18,
                             -radius * 0.32, -radius * 0.90,
                             radius * 0.10, -radius * 0.78);
            rockPath.cubicTo(radius * 0.92, -radius * 0.62,
                             radius * 1.08, radius * 0.02,
                             radius * 0.70, radius * 0.52);
            rockPath.cubicTo(radius * 0.26, radius * 0.92,
                             -radius * 0.42, radius * 0.90,
                             -radius * 0.85, radius * 0.28);
            QLinearGradient rockFill(QPointF(-radius, -radius), QPointF(radius, radius));
            rockFill.setColorAt(0.0, QColor(152, 160, 180));
            rockFill.setColorAt(0.55, QColor(92, 102, 124));
            rockFill.setColorAt(1.0, QColor(48, 58, 76));
            painter.setBrush(rockFill);
            painter.setPen(QPen(QColor(196, 208, 228, 110), 1.4));
            painter.drawPath(rockPath);
            break;
        }
        default: {
            QLinearGradient barrelFill(QPointF(0.0, -radius), QPointF(0.0, radius));
            barrelFill.setColorAt(0.0, QColor(255, 196, 72));
            barrelFill.setColorAt(0.45, QColor(255, 128, 48));
            barrelFill.setColorAt(1.0, QColor(182, 68, 24));
            painter.setBrush(barrelFill);
            painter.setPen(QPen(QColor(255, 235, 180, 190), 1.8));
            painter.drawEllipse(QRectF(-radius, -radius, radius * 2.0, radius * 2.0));
            painter.setPen(QPen(QColor(96, 44, 18), 2.2));
            painter.drawLine(QPointF(-radius * 0.72, -radius * 0.18), QPointF(radius * 0.72, -radius * 0.18));
            painter.drawLine(QPointF(-radius * 0.72, radius * 0.26), QPointF(radius * 0.72, radius * 0.26));
            break;
        }
        }
    } else {
        const QRectF body(-width * 0.5, -height * 0.5, width, height);
        painter.setBrush(QColor(255, 160, 62));
        painter.setPen(QPen(QColor(255, 232, 178, 190), 1.8));
        painter.drawRoundedRect(body, 6.0, 6.0);
        painter.setPen(QPen(QColor(52, 24, 12), 2.8));
        const qreal stripeGap = qMax<qreal>(8.0, width / 4.8);
        for (qreal x = body.left() - height; x < body.right() + height; x += stripeGap) {
            painter.drawLine(QPointF(x, body.top()), QPointF(x + height, body.bottom()));
        }
        painter.setBrush(QColor(88, 88, 92));
        painter.setPen(Qt::NoPen);
        painter.drawRect(QRectF(body.left() + 4.0, body.bottom() - 5.0, 9.0, 7.0));
        painter.drawRect(QRectF(body.right() - 13.0, body.bottom() - 5.0, 9.0, 7.0));
    }

    painter.restore();
}

void GameViewWidget::drawBlockerVehicle(QPainter& painter, const GameRenderObject& vehicle)
{
    GameRenderObject renderVehicle = vehicle;
    renderVehicle.label.clear();
    renderVehicle.extraData[QStringLiteral("shieldActive")] = false;
    renderVehicle.extraData[QStringLiteral("boostActive")] = false;
    renderVehicle.extraData[QStringLiteral("invisibleActive")] = false;
    renderVehicle.extraData[QStringLiteral("magnetActive")] = false;
    drawRaceCar(painter, renderVehicle, false);
}

void GameViewWidget::drawPausedOverlay(QPainter& painter)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.fillRect(rect(), QColor(2, 8, 20, 105));

    const QSize boxSize(qMin(360, qMax(240, width() / 4)), 96);
    const QRect boxRect((width() - boxSize.width()) / 2,
                        qMax(80, (height() - boxSize.height()) / 2 - 40),
                        boxSize.width(),
                        boxSize.height());

    QLinearGradient bg(boxRect.topLeft(), boxRect.bottomRight());
    bg.setColorAt(0.0, QColor(5, 18, 38, 238));
    bg.setColorAt(1.0, QColor(18, 10, 42, 238));
    painter.setBrush(bg);
    painter.setPen(QPen(QColor(0, 230, 255), 2));
    painter.drawRoundedRect(boxRect, 14, 14);

    painter.setPen(QPen(QColor(255, 64, 150, 150), 1));
    painter.drawRoundedRect(boxRect.adjusted(5, 5, -5, -5), 10, 10);

    QFont titleFont(QStringLiteral("Segoe UI"), 28, QFont::Bold);
    titleFont.setLetterSpacing(QFont::AbsoluteSpacing, 4);
    painter.setFont(titleFont);
    painter.setPen(QColor(238, 252, 255));
    painter.drawText(boxRect.adjusted(0, 8, 0, -24), Qt::AlignCenter, QStringLiteral("PAUSED"));

    QFont subFont(QStringLiteral("Segoe UI"), 10, QFont::DemiBold);
    painter.setFont(subFont);
    painter.setPen(QColor(120, 235, 255, 220));
    painter.drawText(boxRect.adjusted(0, 58, 0, -8),
                     Qt::AlignCenter,
                     QStringLiteral("Press Resume to continue"));

    painter.restore();
}

void GameViewWidget::drawTrafficLight(QPainter& painter, const GameRenderObject& light)
{
    QPointF center = worldToScreen(light.position);
    qreal halfW = light.size.width() * m_renderState.cameraZoom * kTrafficLightVisualScale / 2;
    qreal halfH = light.size.height() * m_renderState.cameraZoom * kTrafficLightVisualScale / 2;

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRectF poleRect(center.x() - halfW * 0.18, center.y() - halfH * 0.25, halfW * 0.36, halfH * 1.85);
    painter.fillRect(poleRect, QColor(43, 52, 61));
    painter.fillRect(poleRect.adjusted(poleRect.width() * 0.55, 0, 0, 0), QColor(28, 35, 43, 180));

    QRectF lightRect(center.x() - halfW * 0.82, center.y() - halfH * 1.52, halfW * 1.64, halfH * 1.18);
    painter.setBrush(QColor(18, 30, 44, 245));
    painter.setPen(QPen(QColor(145, 168, 186), 1.4));
    painter.drawRoundedRect(lightRect, 5, 5);

    painter.setBrush(QColor(6, 14, 24, 160));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(lightRect.adjusted(3, 3, -3, -3), 4, 4);

    QString state = light.extraData["state"].toString();
    QColor lightColor;
    if (state == "red") {
        lightColor = QColor(231, 76, 60);
    } else if (state == "yellow") {
        lightColor = QColor(241, 196, 15);
    } else {
        lightColor = QColor(46, 204, 113);
    }

    qreal lightRadius = halfW * 0.58;
    const QPointF bulbCenter(center.x(), center.y() - halfH * 0.93);
    QColor glowColor = lightColor;
    glowColor.setAlpha(110);
    painter.setBrush(glowColor);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(bulbCenter, lightRadius * 1.55, lightRadius * 1.55);

    QRadialGradient bulbGradient(bulbCenter, lightRadius);
    bulbGradient.setColorAt(0.0, QColor(255, 255, 255, 230));
    bulbGradient.setColorAt(0.28, lightColor.lighter(130));
    bulbGradient.setColorAt(1.0, lightColor.darker(120));
    painter.setBrush(QBrush(bulbGradient));
    painter.setPen(QPen(QColor(255, 255, 255, 180), 1.2));
    painter.drawEllipse(bulbCenter, lightRadius, lightRadius);

    painter.setBrush(QColor(255, 255, 255, 110));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(bulbCenter.x() - lightRadius * 0.3, bulbCenter.y() - lightRadius * 0.32),
                        lightRadius * 0.22,
                        lightRadius * 0.18);

    QRectF baseRect(center.x() - halfW * 0.42, center.y() + halfH * 1.48, halfW * 0.84, halfH * 0.13);
    painter.setBrush(QColor(34, 43, 52));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(baseRect, 2, 2);

    painter.setBrush(QBrush(lightColor));
    painter.setPen(QColor(235, 245, 255));
    painter.setFont(QFont("Arial", 8, QFont::Bold));
    painter.drawText(QRectF(center.x() - halfW, center.y() + halfH, halfW * 2, 12),
                     Qt::AlignCenter, state);
    painter.restore();
}

void GameViewWidget::drawSpeedLimitSign(QPainter& painter, const GameRenderObject& sign)
{
    QPointF center = worldToScreen(sign.position);
    const QPointF offset = worldToScreen(sign.position + roadsideVisualOffset(m_currentTrack, sign.position, m_tileSize)) - center;
    const QPointF visualCenter = center + offset;
    qreal halfW = sign.size.width() * m_renderState.cameraZoom * 2.18 / 2;
    qreal halfH = sign.size.height() * m_renderState.cameraZoom * 2.18 / 2;

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRectF poleRect(visualCenter.x() - 2.5, visualCenter.y(), 5, halfH * 1.35);
    painter.fillRect(poleRect, QColor(88, 103, 114));
    painter.fillRect(poleRect.adjusted(2.5, 0, 0, 0), QColor(45, 55, 65, 160));

    QRectF signRect(visualCenter.x() - halfW, visualCenter.y() - halfH * 2, halfW * 2, halfH * 2);
    const int limit = sign.extraData.value(QStringLiteral("limit"), sign.label).toInt();
    const int displayLimit = nearestSpeedLimitSignValue(limit);
    const QPixmap& signImage = visualPixmap(speedLimitImagePath(displayLimit));
    if (!signImage.isNull()) {
        painter.drawPixmap(signRect, signImage, signImage.rect());
    } else {
        painter.setBrush(QBrush(QColor(255, 255, 255)));
        painter.setPen(QPen(QColor(231, 76, 60), 3));
        painter.drawEllipse(signRect);
    }

    painter.setPen(QColor(44, 62, 80));
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.drawText(signRect, Qt::AlignCenter, QString::number(displayLimit));
    painter.restore();
}

void GameViewWidget::drawPedestrianCrossing(QPainter& painter, const GameRenderObject& crossing)
{
    QPointF center = worldToScreen(crossing.position);
    qreal halfW = crossing.size.width() * m_renderState.cameraZoom / 2;
    qreal halfH = crossing.size.height() * m_renderState.cameraZoom / 2;

    QRectF crossRect(center.x() - halfW, center.y() - halfH, halfW * 2, halfH * 2);
    painter.fillRect(crossRect, QColor(255, 255, 255, 28));

    painter.setPen(QPen(QColor(255, 255, 255, 120), 2, Qt::DashLine));
    painter.drawRect(crossRect);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 215));
    const bool verticalRoad = roadRunsVerticallyAt(m_currentTrack, crossing.position, crossing.size);
    const qreal roadVisualWidth = qMin(crossRect.width() * 0.72,
                                       m_tileSize * m_renderState.cameraZoom * 2.35);
    const qreal roadVisualHeight = qMin(crossRect.height() * 0.72,
                                        m_tileSize * m_renderState.cameraZoom * 2.35);
    if (verticalRoad) {
        const QRectF stripeArea(center.x() - roadVisualWidth / 2.0,
                                crossRect.top() + crossRect.height() * 0.09,
                                roadVisualWidth,
                                crossRect.height() * 0.82);
        const int stripeCount = 6;
        const qreal stripeH = stripeArea.height() / (stripeCount * 1.85);
        const qreal gap = (stripeArea.height() - stripeH * stripeCount) / (stripeCount + 1);
        for (int i = 0; i < stripeCount; ++i) {
            const qreal y = stripeArea.top() + gap + i * (stripeH + gap);
            const QRectF stripe(stripeArea.left(),
                                y,
                                stripeArea.width(),
                                stripeH);
            painter.fillRect(stripe, QColor(255, 255, 255, 220));
        }
    } else {
        const QRectF stripeArea(crossRect.left() + crossRect.width() * 0.09,
                                center.y() - roadVisualHeight / 2.0,
                                crossRect.width() * 0.82,
                                roadVisualHeight);
        const int stripeCount = 6;
        const qreal stripeW = stripeArea.width() / (stripeCount * 1.85);
        const qreal gap = (stripeArea.width() - stripeW * stripeCount) / (stripeCount + 1);
        for (int i = 0; i < stripeCount; ++i) {
            const qreal x = stripeArea.left() + gap + i * (stripeW + gap);
            const QRectF stripe(x,
                                stripeArea.top(),
                                stripeW,
                                stripeArea.height());
            painter.fillRect(stripe, QColor(255, 255, 255, 220));
        }
    }

    painter.setPen(QColor(255, 255, 255));
    painter.setFont(QFont("Arial", 9));
    painter.drawText(QRectF(center.x() - halfW, center.y() + halfH + 2, halfW * 2, 12),
                     Qt::AlignCenter, crossing.label);

    const QVariant pedestrianValue = crossing.extraData.value(QStringLiteral("pedestrianCount"));
    const int pedestrianCount = qMax(1, pedestrianValue.isValid() ? pedestrianValue.toInt() : 3);
    const QPixmap& pedestrianImage = visualPixmap(QStringLiteral("traffic/pd_pedestrian_walk.png"));
    for (int i = 0; i < pedestrianCount; ++i) {
        const qreal t = (i + 1.0) / (pedestrianCount + 1.0);
        const QPointF p(center.x() - halfW + t * halfW * 2.0,
                        center.y() - halfH * 0.55 + (i % 2) * halfH * 0.75);
        if (!pedestrianImage.isNull()) {
            const qreal personW = qBound<qreal>(34.0, halfW * 0.42, 58.0);
            const qreal personH = personW * 1.45;
            const QRectF personRect(p.x() - personW / 2.0,
                                    p.y() - personH * 0.65,
                                    personW,
                                    personH);
            painter.drawPixmap(personRect, pedestrianImage, pedestrianImage.rect());
            continue;
        }

        painter.setPen(QPen(QColor(44, 62, 80), 2));
        painter.setBrush(QColor(241, 196, 15));
        painter.drawEllipse(QPointF(p.x(), p.y() - 7), 5, 5);
        painter.drawLine(QPointF(p.x(), p.y() - 2), QPointF(p.x(), p.y() + 13));
        painter.drawLine(QPointF(p.x() - 8, p.y() + 4), QPointF(p.x() + 8, p.y() + 4));
        painter.drawLine(QPointF(p.x(), p.y() + 13), QPointF(p.x() - 7, p.y() + 23));
        painter.drawLine(QPointF(p.x(), p.y() + 13), QPointF(p.x() + 7, p.y() + 23));
    }
}

QPointF GameViewWidget::worldToScreen(const QVector2D& worldPos) const
{
    QPointF center(width() / 2.0, height() / 2.0);
    qreal zoom = m_renderState.cameraZoom;

    return QPointF(
        center.x() + (worldPos.x() - m_renderState.cameraPosition.x()) * zoom,
        center.y() - (worldPos.y() - m_renderState.cameraPosition.y()) * zoom
    );
}

QVector2D GameViewWidget::screenToWorld(const QPointF& screenPos) const
{
    QPointF center(width() / 2.0, height() / 2.0);
    qreal zoom = m_renderState.cameraZoom;

    return QVector2D(
        (screenPos.x() - center.x()) / zoom + m_renderState.cameraPosition.x(),
        -(screenPos.y() - center.y()) / zoom + m_renderState.cameraPosition.y()
    );
}

QRectF GameViewWidget::getViewportWorldBounds() const
{
    QVector2D topLeft = screenToWorld(QPointF(0, 0));
    QVector2D bottomRight = screenToWorld(QPointF(width(), height()));
    return QRectF(topLeft.toPointF(), bottomRight.toPointF()).normalized();
}

void GameViewWidget::updateRenderState()
{
    m_renderState.viewportBounds = getViewportWorldBounds();
    emit renderStateChanged(m_renderState);
}

void GameViewWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_W || event->key() == Qt::Key_Up ||
        event->key() == Qt::Key_S || event->key() == Qt::Key_Down ||
        event->key() == Qt::Key_A || event->key() == Qt::Key_Left ||
        event->key() == Qt::Key_D || event->key() == Qt::Key_Right ||
        event->key() == Qt::Key_Space) {
        emit keyInputReceived(event);
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void GameViewWidget::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_W || event->key() == Qt::Key_Up ||
        event->key() == Qt::Key_S || event->key() == Qt::Key_Down ||
        event->key() == Qt::Key_A || event->key() == Qt::Key_Left ||
        event->key() == Qt::Key_D || event->key() == Qt::Key_Right ||
        event->key() == Qt::Key_Space) {
        emit keyReleased(event);
        event->accept();
        return;
    }
    QWidget::keyReleaseEvent(event);
}

void GameViewWidget::focusInEvent(QFocusEvent* event)
{
    QWidget::focusInEvent(event);
}

void GameViewWidget::focusOutEvent(QFocusEvent* event)
{
    QWidget::focusOutEvent(event);
}

}

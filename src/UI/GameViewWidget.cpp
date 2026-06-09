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
#include <QFont>
#include <QFontMetrics>
#include <QRadialGradient>
#include <QTimer>
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
    return QColor(26, 241, 255);
}

QString normalizedPlayerId(const QString& playerId)
{
    const QString trimmed = playerId.trimmed();
    return trimmed.isEmpty() ? QStringLiteral("P1") : trimmed;
}

constexpr int kCollisionImpactDurationMs = 320;
constexpr int kCollisionImpactCooldownMs = 200;
constexpr int kMaxCollisionImpacts = 5;

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
    , m_collisionImpactTimer(new QTimer(this))
    , m_lastCollisionImpactMs(-kCollisionImpactCooldownMs)
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
                    QColor(26, 241, 255),
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
                                     bool magnetActive)
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

    for (GameRenderObject& existing : m_renderState.playerCars) {
        if (existing.extraData.value(QStringLiteral("playerId")).toString() == id) {
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
    if (normalizedType.contains(QStringLiteral("boost"))) {
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

void GameViewWidget::clearAll()
{
    m_renderState.clear();
    m_currentTrack = nullptr;
    m_collisionImpactEffects.clear();
    m_collisionImpactTimer->stop();
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

    painter.fillRect(rect(), m_backgroundColor);

    drawTrack(painter);
    drawStartFinishMarkers(painter);
    drawCheckpoints(painter);

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

        painter.save();
        painter.setPen(QPen(QColor(255, 216, 80, 80), 8));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(screenRect.adjusted(-2, -2, 2, 2), 8, 8);
        painter.setPen(QPen(QColor(255, 216, 80), 3));
        painter.setBrush(QColor(255, 216, 80, 55));
        painter.drawRoundedRect(screenRect, 6, 6);

        const QRectF labelRect(screenRect.center().x() - 24,
                               screenRect.center().y() - 13,
                               48,
                               26);
        painter.setBrush(QColor(8, 18, 32, 230));
        painter.setPen(QPen(QColor(255, 216, 80), 2));
        painter.drawRoundedRect(labelRect, 7, 7);
        painter.setPen(Qt::white);
        painter.setFont(QFont(QStringLiteral("Arial"), 10, QFont::Bold));
        painter.drawText(labelRect, Qt::AlignCenter, QStringLiteral("CP%1").arg(i + 1));
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
            painter.setBrush(QColor(255, 70, 110, 55));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(center, 30.0, 30.0);
            const QRectF flag(center.x() - 34.0, center.y() - 15.0, 68.0, 30.0);
            painter.setBrush(QColor(255, 255, 255, 235));
            painter.setPen(QPen(QColor(255, 66, 104), 3));
            painter.drawRoundedRect(flag, 4, 4);
            painter.setPen(QColor(231, 76, 60));
            painter.setFont(QFont(QStringLiteral("Arial"), 8, QFont::Bold));
            painter.drawText(flag, Qt::AlignCenter, QStringLiteral("FINISH"));
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
    const QPointF center = worldToScreen(car.position);
    const qreal halfW = car.size.width() * m_renderState.cameraZoom / 2.0;
    const qreal halfH = car.size.height() * m_renderState.cameraZoom / 2.0;
    const QColor bodyColor = car.color.isValid() ? car.color : (playerCar ? QColor(26, 241, 255) : QColor(73, 153, 255));
    const QColor glowColor(bodyColor.red(), bodyColor.green(), bodyColor.blue(), playerCar ? 96 : 56);

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

    painter.setPen(QPen(QColor(255, 255, 255, 210), qMax<qreal>(1.0, 1.2 * m_renderState.cameraZoom)));
    painter.drawLine(QPointF(0, -halfH * 0.82), QPointF(0, halfH * 0.78));
    painter.setPen(QPen(bodyColor.lighter(165), qMax<qreal>(1.5, 2.0 * m_renderState.cameraZoom)));
    painter.drawLine(QPointF(-halfW * 0.68, -halfH * 0.12), QPointF(-halfW * 0.48, halfH * 0.54));
    painter.drawLine(QPointF(halfW * 0.68, -halfH * 0.12), QPointF(halfW * 0.48, halfH * 0.54));

    QPainterPath cockpit;
    cockpit.addRoundedRect(QRectF(-halfW * 0.45, -halfH * 0.42, halfW * 0.9, halfH * 0.52), 4, 4);
    QLinearGradient glass(0, -halfH * 0.42, 0, halfH * 0.1);
    glass.setColorAt(0.0, QColor(225, 255, 255, 220));
    glass.setColorAt(1.0, QColor(30, 64, 92, 210));
    painter.setBrush(glass);
    painter.setPen(QPen(QColor(170, 250, 255, 190), 1));
    painter.drawPath(cockpit);

    painter.setBrush(QColor(255, 255, 255, 230));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(-halfW * 0.32, -halfH * 0.76), halfW * 0.12, halfW * 0.12);
    painter.drawEllipse(QPointF(halfW * 0.32, -halfH * 0.76), halfW * 0.12, halfW * 0.12);

    painter.setOpacity(1.0);
    painter.restore();

    painter.save();
    const QString id = car.extraData.value(QStringLiteral("playerId")).toString();
    const QString labelText = playerCar && !id.isEmpty() ? id : car.label;
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

    painter.setBrush(QBrush(box.color));
    painter.setPen(QPen(box.color.lighter(140), 3));
    painter.drawRoundedRect(boxRect, 4, 4);

    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", box.label.size() > 2 ? 9 : 13, QFont::Bold));
    painter.drawText(boxRect, Qt::AlignCenter, box.label);

    painter.setPen(QColor(255, 255, 255, 210));
    painter.setFont(QFont("Arial", 8));
    painter.drawText(QRectF(center.x() - 38, center.y() + halfH + 4, 76, 14),
                     Qt::AlignCenter,
                     box.extraData.value(QStringLiteral("powerupType")).toString());
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

void GameViewWidget::drawTrafficLight(QPainter& painter, const GameRenderObject& light)
{
    QPointF center = worldToScreen(light.position);
    qreal halfW = light.size.width() * m_renderState.cameraZoom / 2;
    qreal halfH = light.size.height() * m_renderState.cameraZoom / 2;

    QRectF poleRect(center.x() - halfW * 0.3, center.y() - halfH, halfW * 0.6, halfH * 2);
    painter.fillRect(poleRect, QColor(52, 58, 64));

    QRectF lightRect(center.x() - halfW, center.y() - halfH * 1.5, halfW * 2, halfH);
    painter.setBrush(QBrush(QColor(44, 62, 80)));
    painter.setPen(QPen(QColor(149, 165, 166), 1));
    painter.drawRoundedRect(lightRect, 3, 3);

    QString state = light.extraData["state"].toString();
    QColor lightColor;
    if (state == "red") {
        lightColor = QColor(231, 76, 60);
    } else if (state == "yellow") {
        lightColor = QColor(241, 196, 15);
    } else {
        lightColor = QColor(46, 204, 113);
    }

    qreal lightRadius = halfW * 0.6;
    painter.setBrush(QBrush(lightColor));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(center.x(), center.y() - halfH * 0.5), lightRadius, lightRadius);

    painter.setPen(QColor(255, 255, 255));
    painter.setFont(QFont("Arial", 8));
    painter.drawText(QRectF(center.x() - halfW, center.y() + halfH, halfW * 2, 12),
                     Qt::AlignCenter, state);
}

void GameViewWidget::drawSpeedLimitSign(QPainter& painter, const GameRenderObject& sign)
{
    QPointF center = worldToScreen(sign.position);
    qreal halfW = sign.size.width() * m_renderState.cameraZoom / 2;
    qreal halfH = sign.size.height() * m_renderState.cameraZoom / 2;

    QRectF poleRect(center.x() - 2, center.y(), 4, halfH);
    painter.fillRect(poleRect, QColor(127, 140, 141));

    QRectF signRect(center.x() - halfW, center.y() - halfH * 2, halfW * 2, halfH * 2);
    painter.setBrush(QBrush(QColor(255, 255, 255)));
    painter.setPen(QPen(QColor(231, 76, 60), 3));
    painter.drawEllipse(signRect);

    painter.setPen(QColor(44, 62, 80));
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    painter.drawText(signRect, Qt::AlignCenter, sign.label);
}

void GameViewWidget::drawPedestrianCrossing(QPainter& painter, const GameRenderObject& crossing)
{
    QPointF center = worldToScreen(crossing.position);
    qreal halfW = crossing.size.width() * m_renderState.cameraZoom / 2;
    qreal halfH = crossing.size.height() * m_renderState.cameraZoom / 2;

    QRectF crossRect(center.x() - halfW, center.y() - halfH, halfW * 2, halfH * 2);
    painter.fillRect(crossRect, QColor(255, 255, 255, 80));

    painter.setPen(QPen(QColor(255, 255, 255), 2, Qt::DashLine));
    painter.drawRect(crossRect);

    int stripeCount = 5;
    qreal stripeWidth = halfW * 2 / stripeCount;
    for (int i = 0; i < stripeCount; ++i) {
        if (i % 2 == 0) {
            QRectF stripe(center.x() - halfW + i * stripeWidth, center.y() - halfH,
                         stripeWidth, halfH * 2);
            painter.fillRect(stripe, QColor(255, 255, 255, 150));
        }
    }

    painter.setPen(QColor(255, 255, 255));
    painter.setFont(QFont("Arial", 9));
    painter.drawText(QRectF(center.x() - halfW, center.y() + halfH + 2, halfW * 2, 12),
                     Qt::AlignCenter, crossing.label);

    const QVariant pedestrianValue = crossing.extraData.value(QStringLiteral("pedestrianCount"));
    const int pedestrianCount = qMax(1, pedestrianValue.isValid() ? pedestrianValue.toInt() : 3);
    for (int i = 0; i < pedestrianCount; ++i) {
        const qreal t = (i + 1.0) / (pedestrianCount + 1.0);
        const QPointF p(center.x() - halfW + t * halfW * 2.0,
                        center.y() - halfH * 0.55 + (i % 2) * halfH * 0.75);
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

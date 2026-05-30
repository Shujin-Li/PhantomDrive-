#include "UI/GameViewWidget.h"
#include "track/TrackTile.h"
#include "track/Checkpoint.h"

#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QFontMetrics>
#include <QtMath>
#include <QDebug>

namespace PhantomDrive {

namespace {

void dumpTrackLayoutForDebug(const TrackData* track, const QString& label)
{
    if (!track) {
        qDebug() << "[CustomTrackDebug]" << label << "track=null";
        return;
    }

    int roadCount = 0;
    int grassCount = 0;
    int wallCount = 0;
    QPoint finishTile(-1, -1);
    for (int row = 0; row < track->getRowCount(); ++row) {
        for (int col = 0; col < track->getColCount(); ++col) {
            const TrackTile* tile = track->getTileAt(row, col);
            if (!tile) {
                continue;
            }
            switch (tile->getType()) {
            case TileType::Road:
            case TileType::Asphalt:
                ++roadCount;
                break;
            case TileType::Grass:
                ++grassCount;
                break;
            case TileType::Wall:
            case TileType::Barrier:
                ++wallCount;
                break;
            case TileType::FinishLine:
            case TileType::StartLine:
                finishTile = QPoint(col, row);
                ++roadCount;
                break;
            default:
                break;
            }
        }
    }

    const QList<QVector2D> starts = track->getStartPositions();
    const QPoint startTile = starts.isEmpty() ? QPoint(-1, -1) : TrackData::worldToTile(starts.first());
    qDebug().noquote()
        << QStringLiteral("[CustomTrackDebug] %1 rows=%2 cols=%3 start(row=%4,col=%5) finish(row=%6,col=%7) road=%8 grass=%9 wall=%10")
               .arg(label)
               .arg(track->getRowCount())
               .arg(track->getColCount())
               .arg(startTile.y())
               .arg(startTile.x())
               .arg(finishTile.y())
               .arg(finishTile.x())
               .arg(roadCount)
               .arg(grassCount)
               .arg(wallCount);

    const QList<Checkpoint*> checkpoints = track->getCheckpointsInOrder();
    for (int i = 0; i < checkpoints.size(); ++i) {
        const Checkpoint* cp = checkpoints.at(i);
        if (!cp) {
            continue;
        }
        const QPoint cpTile = TrackData::worldToTile(cp->getPosition());
        qDebug().noquote()
            << QStringLiteral("[CustomTrackDebug] %1 CP%2 row=%3 col=%4")
                   .arg(label)
                   .arg(i + 1)
                   .arg(cpTile.y())
                   .arg(cpTile.x());
    }

    const QList<QVector2D> items = track->getItemBoxPositions();
    for (int i = 0; i < items.size(); ++i) {
        const QPoint itemTile = TrackData::worldToTile(items.at(i));
        qDebug().noquote()
            << QStringLiteral("[CustomTrackDebug] %1 Item%2 row=%3 col=%4")
                   .arg(label)
                   .arg(i + 1)
                   .arg(itemTile.y())
                   .arg(itemTile.x());
    }
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
{
    setMinimumSize(400, 300);
    setMouseTracking(true);
    setBackgroundRole(QPalette::Dark);
    setAutoFillBackground(true);
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
    setAttribute(Qt::WA_InputMethodEnabled, true);
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
    GameRenderObject car;
    car.type = RenderObjectType::PlayerCar;
    car.position = position;
    car.rotation = rotation;
    car.size = QSizeF(24, 40);
    car.color = QColor(231, 76, 60);
    car.label = QString("Player (%1 km/h)").arg(qRound(speed));
    car.extraData["speed"] = speed;
    car.extraData["boostActive"] = m_playerBoostActive;
    car.extraData["shieldActive"] = m_playerShieldActive;
    car.extraData["invisibleActive"] = m_playerInvisibleActive;
    car.extraData["magnetActive"] = m_playerMagnetActive;

    m_renderState.playerCars.clear();
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
    update();
}

void GameViewWidget::setCameraZoom(qreal zoom)
{
    m_renderState.cameraZoom = qMax(0.25, qMin(3.0, zoom));
    update();
}

void GameViewWidget::resetCamera()
{
    m_renderState.cameraPosition = QVector2D(0, 0);
    m_renderState.cameraZoom = 1.0;
    update();
}

void GameViewWidget::clearAll()
{
    m_renderState.clear();
    m_currentTrack = nullptr;
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
        painter.setPen(QColor(149, 165, 166));
        painter.setFont(QFont("Arial", 14));
        painter.drawText(rect(), Qt::AlignCenter, "No Track Loaded");
        return;
    }

    int rows = m_currentTrack->getRowCount();
    int cols = m_currentTrack->getColCount();

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            TrackTile* tile = m_currentTrack->getTileAt(row, col);
            if (!tile) continue;

            const QPointF cornerA = worldToScreen(QVector2D(col * m_tileSize, row * m_tileSize));
            const QPointF cornerB = worldToScreen(QVector2D((col + 1) * m_tileSize, (row + 1) * m_tileSize));
            const QRectF tileRect = QRectF(cornerA, cornerB).normalized();

            if (!rect().toRectF().intersects(tileRect)) continue;

            QColor tileColor;
            switch (tile->getType()) {
                case TileType::Road:
                    tileColor = QColor(52, 58, 64);
                    break;
                case TileType::Grass:
                    tileColor = QColor(39, 174, 96);
                    break;
                case TileType::Sand:
                    tileColor = QColor(243, 156, 18);
                    break;
                case TileType::Asphalt:
                    tileColor = QColor(44, 62, 80);
                    break;
                case TileType::StartLine:
                    tileColor = QColor(46, 204, 113);
                    break;
                case TileType::FinishLine:
                    tileColor = QColor(231, 76, 60);
                    break;
                case TileType::Wall:
                case TileType::Barrier:
                    tileColor = QColor(127, 140, 141);
                    break;
                default:
                    tileColor = QColor(189, 195, 199);
                    break;
            }

            painter.fillRect(tileRect, tileColor);

            if (m_showGrid) {
                painter.setPen(QPen(QColor(255, 255, 255, 30), 1));
                painter.drawRect(tileRect);
            }
        }
    }
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
    QPointF center = worldToScreen(car.position);
    qreal halfW = car.size.width() * m_renderState.cameraZoom / 2;
    qreal halfH = car.size.height() * m_renderState.cameraZoom / 2;

    painter.save();
    painter.translate(center);
    painter.rotate(car.rotation);

    if (car.extraData.value(QStringLiteral("shieldActive")).toBool()) {
        painter.setBrush(QColor(52, 152, 219, 55));
        painter.setPen(QPen(QColor(93, 173, 226), 3));
        painter.drawEllipse(QPointF(0, 0), halfH * 1.35, halfH * 1.35);
    }

    if (car.extraData.value(QStringLiteral("magnetActive")).toBool()) {
        painter.setBrush(QColor(230, 126, 34, 45));
        painter.setPen(QPen(QColor(243, 156, 18), 2, Qt::DashLine));
        painter.drawEllipse(QPointF(0, 0), halfH * 1.55, halfH * 1.55);
    }

    if (car.extraData.value(QStringLiteral("boostActive")).toBool()) {
        painter.setBrush(QColor(46, 204, 113, 130));
        painter.setPen(Qt::NoPen);
        QPointF flame[] = {
            QPointF(-halfW * 0.45, halfH * 0.75),
            QPointF(0, halfH * 1.75),
            QPointF(halfW * 0.45, halfH * 0.75)
        };
        painter.drawPolygon(flame, 3);
    }

    painter.setBrush(QBrush(car.color));
    painter.setPen(QPen(QColor(255, 255, 255), 2));

    if (car.extraData.value(QStringLiteral("invisibleActive")).toBool()) {
        painter.setOpacity(0.35);
    }

    QPointF body[] = {
        QPointF(0, -halfH),
        QPointF(halfW, halfH * 0.6),
        QPointF(halfW * 0.5, halfH),
        QPointF(-halfW * 0.5, halfH),
        QPointF(-halfW, halfH * 0.6)
    };
    painter.drawPolygon(body, 5);

    painter.setBrush(QBrush(QColor(52, 152, 219)));
    painter.drawRect(-halfW * 0.4, -halfH * 0.3, halfW * 0.8, halfH * 0.4);

    painter.setOpacity(1.0);
    painter.restore();

    painter.setPen(QColor(255, 255, 255));
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    QFontMetrics fm(painter.font());
    QRect textRect = fm.boundingRect(car.label);
    textRect.moveCenter(QPoint(center.x(), center.y() - halfH - 10));
    painter.fillRect(textRect, QColor(0, 0, 0, 180));
    painter.drawText(textRect, Qt::AlignCenter, car.label);
}

void GameViewWidget::drawAICar(QPainter& painter, const GameRenderObject& car)
{
    QPointF center = worldToScreen(car.position);
    qreal halfW = car.size.width() * m_renderState.cameraZoom / 2;
    qreal halfH = car.size.height() * m_renderState.cameraZoom / 2;

    painter.save();
    painter.translate(center);
    painter.rotate(car.rotation);

    painter.setBrush(QBrush(car.color));
    painter.setPen(QPen(QColor(255, 255, 255), 1));

    QPointF body[] = {
        QPointF(0, -halfH),
        QPointF(halfW, halfH * 0.6),
        QPointF(halfW * 0.5, halfH),
        QPointF(-halfW * 0.5, halfH),
        QPointF(-halfW, halfH * 0.6)
    };
    painter.drawPolygon(body, 5);

    painter.setBrush(QBrush(QColor(149, 165, 166)));
    painter.drawRect(-halfW * 0.3, -halfH * 0.2, halfW * 0.6, halfH * 0.3);

    painter.restore();

    painter.setPen(QColor(200, 200, 200));
    painter.setFont(QFont("Arial", 9));
    QFontMetrics fm(painter.font());
    QRect textRect = fm.boundingRect(car.label);
    textRect.moveCenter(QPoint(center.x(), center.y() - halfH - 8));
    painter.fillRect(textRect, QColor(0, 0, 0, 150));
    painter.drawText(textRect, Qt::AlignCenter, car.label);
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
    qDebug() << "GameViewWidget gained focus - keyboard control enabled";
}

void GameViewWidget::focusOutEvent(QFocusEvent* event)
{
    QWidget::focusOutEvent(event);
    qDebug() << "GameViewWidget lost focus - keyboard control disabled";
}

}

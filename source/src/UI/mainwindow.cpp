#include "UI/mainwindow.h"
#include "ui_mainwindow.h"

#include "UI/DrivingReportWidget.h"
#include "UI/CustomTrackEditorWidget.h"
#include "UI/ThemeManager.h"
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

#include <QApplication>
#include <QComboBox>
#include <QDateTime>
#include <QDebug>
#include <QDialog>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QHash>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLayoutItem>
#include <QMessageBox>
#include <QPointer>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
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
    , m_btnGuide(nullptr)
    , m_btnPlayCustomTrack(nullptr)
    , m_btnSaveCustomTrack(nullptr)
    , m_btnLoadCustomTrackForEdit(nullptr)
    , m_btnExportCustomTrackJson(nullptr)
    , m_customTrackEditor(nullptr)
    , m_customTrackMode(new CustomTrackMode(this))
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
    , m_selectedTrackId(QStringLiteral("neon_loop"))
    , m_selectedAIDifficulty(QStringLiteral("medium"))
    , m_twoPlayerMode(false)
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
    , m_lapsCompleted(0)
    , m_totalLaps(3)
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
            m_twoPlayerMode = true;
            if (m_playerCountCombo) {
                const int twoPlayerIndex = m_playerCountCombo->findData(2);
                m_playerCountCombo->setCurrentIndex(twoPlayerIndex >= 0 ? twoPlayerIndex : 1);
            }
            startBuiltInTrackSession(QStringLiteral("Arcade"));
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
    if (!ui->stackedWidget) {
        startBuiltInTrackSession(QStringLiteral("Arcade"));
        return;
    }

    QWidget* setupPage = ui->stackedWidget->findChild<QWidget*>(QStringLiteral("pageArcadeSetup"));
    if (!setupPage) {
        setupPage = new QWidget(ui->stackedWidget);
        setupPage->setObjectName(QStringLiteral("pageArcadeSetup"));
        setupPage->setStyleSheet(QStringLiteral(
            "QWidget#pageArcadeSetup{background:#06101F;color:#EAFBFF;}"
            "QLabel#label_ArcadeSetupTitle,QLabel.arcadeSetupSection{"
            "color:#38F6FF;font-size:20px;font-weight:800;letter-spacing:7px;}"
            "QLabel#label_TrackDifficulty{color:#FFD84D;font-size:16px;font-weight:800;letter-spacing:2px;}"
            "QLabel#label_TrackDescription{color:#BCEEFF;font-size:15px;line-height:130%;}"
            "QComboBox{background:#071126;color:#F3FBFF;border:2px solid #00CFE8;"
            "border-radius:10px;padding:0 28px;font-size:26px;font-weight:700;min-height:78px;}"
            "QComboBox::drop-down{width:74px;border-left:1px solid #008FB0;}"
            "QPushButton{background:#071126;color:#F3FBFF;border:2px solid #00CFE8;"
            "border-radius:10px;min-height:64px;font-size:22px;font-weight:800;}"
            "QPushButton:hover{background:#0A1C38;border-color:#38F6FF;}"
            "QPushButton#btn_StartArcade{background:#0B2734;border-color:#35F6FF;color:#FFFFFF;}"));

        auto* outer = new QVBoxLayout(setupPage);
        outer->setContentsMargins(18, 22, 18, 22);
        outer->setSpacing(18);

        auto* title = new QLabel(QStringLiteral("Race Setup"), setupPage);
        title->setObjectName(QStringLiteral("label_ArcadeSetupTitle"));
        title->setAlignment(Qt::AlignCenter);
        outer->addWidget(title);

        m_trackSelectCombo = new QComboBox(setupPage);
        m_trackSelectCombo->setObjectName(QStringLiteral("combo_BuiltInTrack"));
        for (const BuiltInTrackInfo& info : BuiltInTrackFactory::tracks()) {
            m_trackSelectCombo->addItem(info.name, info.id);
        }
        outer->addWidget(m_trackSelectCombo);

        m_trackDifficultyLabel = new QLabel(setupPage);
        m_trackDifficultyLabel->setObjectName(QStringLiteral("label_TrackDifficulty"));
        m_trackDifficultyLabel->setAlignment(Qt::AlignCenter);
        outer->addWidget(m_trackDifficultyLabel);

        m_trackDescriptionLabel = new QLabel(setupPage);
        m_trackDescriptionLabel->setObjectName(QStringLiteral("label_TrackDescription"));
        m_trackDescriptionLabel->setAlignment(Qt::AlignCenter);
        m_trackDescriptionLabel->setWordWrap(true);
        m_trackDescriptionLabel->setMinimumHeight(44);
        outer->addWidget(m_trackDescriptionLabel);

        m_playerCountCombo = new QComboBox(setupPage);
        m_playerCountCombo->setObjectName(QStringLiteral("combo_PlayerCount"));
        m_playerCountCombo->addItem(QStringLiteral("1 Player + AI"), 1);
        m_playerCountCombo->addItem(QStringLiteral("2 Players + AI"), 2);
        outer->addWidget(m_playerCountCombo);

        auto* difficultyLabel = new QLabel(QStringLiteral("AI Difficulty"), setupPage);
        difficultyLabel->setProperty("class", QStringLiteral("arcadeSetupSection"));
        difficultyLabel->setAlignment(Qt::AlignCenter);
        outer->addWidget(difficultyLabel);

        m_aiDifficultyCombo = new QComboBox(setupPage);
        m_aiDifficultyCombo->setObjectName(QStringLiteral("combo_AIDifficulty"));
        m_aiDifficultyCombo->addItem(QStringLiteral("Easy"), QStringLiteral("easy"));
        m_aiDifficultyCombo->addItem(QStringLiteral("Medium"), QStringLiteral("medium"));
        m_aiDifficultyCombo->addItem(QStringLiteral("Hard"), QStringLiteral("hard"));
        m_aiDifficultyCombo->addItem(QStringLiteral("Adaptive AI"), QStringLiteral("adaptive"));
        m_aiDifficultyCombo->setCurrentIndex(1);
        outer->addWidget(m_aiDifficultyCombo);

        auto* buttonRow = new QHBoxLayout();
        buttonRow->setContentsMargins(0, 8, 0, 0);
        buttonRow->setSpacing(14);

        auto* backButton = new QPushButton(QStringLiteral("Back"), setupPage);
        backButton->setObjectName(QStringLiteral("btn_BackArcadeSetup"));
        auto* startButton = new QPushButton(QStringLiteral("Start Game"), setupPage);
        startButton->setObjectName(QStringLiteral("btn_StartArcade"));
        buttonRow->addWidget(backButton);
        buttonRow->addWidget(startButton);
        outer->addLayout(buttonRow);
        outer->addStretch(1);

        connect(backButton, &QPushButton::clicked, this, [this]() {
            if (ui->stackedWidget) {
                ui->stackedWidget->setCurrentIndex(0);
            }
        });
        connect(startButton, &QPushButton::clicked, this, [this]() {
            if (m_trackSelectCombo) {
                m_selectedTrackId = m_trackSelectCombo->currentData().toString();
            }
            m_selectedAIDifficulty = selectedAIDifficulty();
            m_twoPlayerMode = selectedPlayerCount() == 2;
            startBuiltInTrackSession(QStringLiteral("Arcade"));
        });

        connect(m_trackSelectCombo, &QComboBox::currentIndexChanged,
                this, [this](int) { updateArcadeSetupTrackInfo(); });
        connect(m_aiDifficultyCombo, &QComboBox::currentIndexChanged,
                this, [this](int) { m_selectedAIDifficulty = selectedAIDifficulty(); });

        ui->stackedWidget->addWidget(setupPage);
    }

    if (m_trackSelectCombo) {
        const int trackIndex = m_trackSelectCombo->findData(m_selectedTrackId);
        m_trackSelectCombo->setCurrentIndex(trackIndex >= 0 ? trackIndex : 0);
    }
    if (m_playerCountCombo) {
        m_playerCountCombo->setCurrentIndex(m_twoPlayerMode ? 1 : 0);
    }
    if (m_aiDifficultyCombo) {
        const int difficultyIndex = m_aiDifficultyCombo->findData(m_selectedAIDifficulty);
        m_aiDifficultyCombo->setCurrentIndex(difficultyIndex >= 0 ? difficultyIndex : 1);
    }
    updateArcadeSetupTrackInfo();
    ui->stackedWidget->setCurrentWidget(setupPage);
    statusBar()->clearMessage();
}

void MainWindow::styleMainMenu()
{
    if (ui->pageMenu) {
        QWidget* background = ui->pageMenu->findChild<QWidget*>(QStringLiteral("mainMenuVisualBackground"));
        if (!background) {
            background = new MenuBackgroundLayer(ui->pageMenu);
            background->setObjectName(QStringLiteral("mainMenuVisualBackground"));
        }
        background->setGeometry(ui->pageMenu->rect());
        background->lower();
        background->show();
        ui->pageMenu->installEventFilter(background);
        ui->pageMenu->setStyleSheet(QStringLiteral("QWidget#pageMenu{background:transparent;}"));
    }

    if (ui->label) {
        ui->label->setObjectName(QStringLiteral("label_MainLogo"));
        ui->label->clear();
        ui->label->setVisible(false);
    }

    if (ui->menuGridLayout) {
        ui->menuGridLayout->setContentsMargins(96, 74, 96, 74);
        ui->menuGridLayout->setVerticalSpacing(20);
        ui->menuGridLayout->setRowMinimumHeight(0, 300);
        ui->menuGridLayout->setRowStretch(0, 0);
        ui->menuGridLayout->setRowStretch(1, 1);
    }

    if (ui->verticalLayout) {
        ui->verticalLayout->setSpacing(22);
        ui->verticalLayout->setContentsMargins(0, 0, 0, 0);
        ui->verticalLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

        if (!ui->pageMenu->property("mainMenuLayoutRebuilt").toBool()) {
            while (QLayoutItem* item = ui->verticalLayout->takeAt(0)) {
                delete item;
            }

            auto addCenteredRow = [this](const QList<QPushButton*>& buttons, int spacing) {
                auto* row = new QHBoxLayout();
                row->setSpacing(spacing);
                row->setContentsMargins(0, 0, 0, 0);
                row->addStretch(1);
                for (QPushButton* button : buttons) {
                    if (button) {
                        row->addWidget(button);
                    }
                }
                row->addStretch(1);
                ui->verticalLayout->addLayout(row);
            };

            if (m_btnGuide) {
                m_btnGuide->setParent(ui->pageMenu);
                m_btnGuide->raise();
            }

            addCenteredRow({ ui->btn_Arcade, m_btnCustomTrackMode, ui->btn_Learn }, 28);
            addCenteredRow({ m_btnTwoPlayerRace, m_btnAdaptiveDemo }, 28);

            auto* secondaryColumn = new QVBoxLayout();
            secondaryColumn->setSpacing(12);
            secondaryColumn->setContentsMargins(0, 24, 0, 0);
            secondaryColumn->setAlignment(Qt::AlignHCenter);
            for (QPushButton* button : { m_btnLoadCustomTrack, ui->btn_History, ui->btn_Exit }) {
                if (button) {
                    secondaryColumn->addWidget(button, 0, Qt::AlignHCenter);
                }
            }
            ui->verticalLayout->addLayout(secondaryColumn);
            ui->pageMenu->setProperty("mainMenuLayoutRebuilt", true);
        }
    }

    const QList<QPushButton*> menuButtons = {
        ui->btn_Arcade,
        m_btnTwoPlayerRace,
        m_btnAdaptiveDemo,
        ui->btn_Learn,
        m_btnCustomTrackMode,
        ui->btn_History,
        m_btnLoadCustomTrack,
        m_btnGuide,
        ui->btn_Exit
    };
    for (QPushButton* button : menuButtons) {
        if (!button) {
            continue;
        }
        button->setCursor(Qt::PointingHandCursor);
        button->setMinimumSize(0, 0);
        button->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    }

    for (QPushButton* button : { ui->btn_Arcade, m_btnCustomTrackMode, ui->btn_Learn }) {
        if (button) {
            button->setFixedSize(310, 82);
        }
    }
    for (QPushButton* button : { m_btnTwoPlayerRace, m_btnAdaptiveDemo }) {
        if (button) {
            button->setFixedSize(330, 66);
        }
    }
    for (QPushButton* button : { m_btnLoadCustomTrack, ui->btn_History, ui->btn_Exit }) {
        if (button) {
            button->setFixedSize(520, 58);
        }
    }
    if (m_btnGuide && ui->pageMenu) {
        m_btnGuide->setFixedSize(380, 52);
        m_btnGuide->setMinimumWidth(380);
        m_btnGuide->setMaximumWidth(380);
        m_btnGuide->move(qMax(24, ui->pageMenu->width() - m_btnGuide->width() - 64), 52);
        m_btnGuide->raise();
    }

    if (m_aiDifficultyCombo) {
        m_aiDifficultyCombo->setMinimumSize(620, 56);
        m_aiDifficultyCombo->setMaximumSize(860, 62);
        m_aiDifficultyCombo->setCursor(Qt::PointingHandCursor);
    }
    if (m_trackSelectCombo) {
        m_trackSelectCombo->setMinimumSize(620, 56);
        m_trackSelectCombo->setMaximumSize(860, 62);
        m_trackSelectCombo->setCursor(Qt::PointingHandCursor);
    }
    if (m_playerCountCombo) {
        m_playerCountCombo->setMinimumSize(620, 56);
        m_playerCountCombo->setMaximumSize(860, 62);
        m_playerCountCombo->setCursor(Qt::PointingHandCursor);
    }
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

    if (m_btnGuide && ui->pageMenu) {
        m_btnGuide->setFixedSize(380, 52);
        m_btnGuide->move(qMax(24, ui->pageMenu->width() - m_btnGuide->width() - 64), 52);
        m_btnGuide->raise();
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
        for (Checkpoint* cp : m_defaultRaceTrack->getCheckpointsInOrder()) {
            if (cp) {
                waypoints.append(cp->getPosition());
            }
        }
        waypoints.append(m_defaultRaceTrack->getStartPosition());
        trackMgr->setWaypoints(waypoints);
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
                    if (!m_gamePaused && m_vehiclePhysics) {
                        m_vehiclePhysics->handleKeyPress(event);
                    }
                });
        connect(m_gameView, &GameViewWidget::keyReleased,
                this, [this](QKeyEvent* event) {
                    if (m_vehiclePhysics) {
                        m_vehiclePhysics->handleKeyRelease(event);
                    }
                });
        connect(m_gameView, &GameViewWidget::keyInputReceived,
                this, [this](QKeyEvent* event) {
                    if (!m_gamePaused && m_player2Physics) {
                        m_player2Physics->handleKeyPress(event);
                    }
                });
        connect(m_gameView, &GameViewWidget::keyReleased,
                this, [this](QKeyEvent* event) {
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
                                                    QColor(255, 48, 118),
                                                    m_vehiclePhysics->isSpeedBoostActive(),
                                                    m_vehiclePhysics->isShieldActive(),
                                                    m_vehiclePhysics->isInvisible(),
                                                    m_vehiclePhysics->isMagnetActive());
                        updateTwoPlayerCamera();
                    } else {
                        m_gameView->updatePlayerCar(m_playerPosition, m_playerRotation, displaySpeedKmh());
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
                                                    QColor(255, 48, 118),
                                                    m_vehiclePhysics->isSpeedBoostActive(),
                                                    m_vehiclePhysics->isShieldActive(),
                                                    m_vehiclePhysics->isInvisible(),
                                                    m_vehiclePhysics->isMagnetActive());
                        updateTwoPlayerCamera();
                    } else {
                        m_gameView->updatePlayerCar(m_playerPosition, m_vehiclePhysics->getRotation(), displaySpeedKmh());
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
        const QVector2D spawnPos = aiWaypoints.first().position + QVector2D(35.0f * i, 42.0f * i);
        ai->setPosition(spawnPos);
        if (aiWaypoints.size() > 1) {
            const QVector2D toNext = aiWaypoints.at(1).position - aiWaypoints.first().position;
            ai->setRotation(qRadiansToDegrees(std::atan2(toNext.x(), toNext.y())));
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

    if (m_vehiclePhysics) {
        m_vehiclePhysics->setPosition(spawnPos);
        m_vehiclePhysics->setRotation(spawnRotation);
    }

    if (m_gameView) {
        m_gameView->updatePlayerCar(m_playerPosition, m_playerRotation, displaySpeedKmh());
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

        QList<QVector2D> waypoints;
        auto appendWaypointIfNew = [&waypoints](const QVector2D& point) {
            if (waypoints.isEmpty() || (waypoints.last() - point).lengthSquared() > 1.0f) {
                waypoints.append(point);
            }
        };

        appendWaypointIfNew(track->getStartPosition());

        for (Checkpoint* cp : track->getCheckpointsInOrder()) {
            if (cp) {
                appendWaypointIfNew(cp->getPosition());
            }
        }

        QVector2D finishPos = track->getStartPosition();
        bool finishFound = false;
        for (int row = 0; row < track->getRowCount(); ++row) {
            for (int col = 0; col < track->getColCount(); ++col) {

                TrackTile* tile = track->getTileAt(row, col);

                if (tile &&
                    tile->getType() == TileType::FinishLine)
                {
                    finishPos = TrackData::tileToWorldCenter(row, col);
                    finishFound = true;
                    break;
                }
            }

            if (finishFound)
                break;
        }

        appendWaypointIfNew(finishFound ? finishPos : track->getStartPosition());

        trackMgr->setWaypoints(waypoints);
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
    const float collectRadius = (m_vehiclePhysics && m_vehiclePhysics->isMagnetActive()) ? 96.0f : 54.0f;
    const float player2CollectRadius = (m_player2Physics && m_player2Physics->isMagnetActive()) ? 96.0f : 54.0f;

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
                                                displaySpeedKmh(), QColor(255, 48, 118));
                    updateTwoPlayerCamera();
                } else {
                    m_gameView->updatePlayerCar(m_playerPosition, m_playerRotation, displaySpeedKmh());
                    m_gameView->setCameraPosition(m_playerPosition);
                }
            }
            notifyArcadeHUD(QStringLiteral("Teleport"), 1200);
        } else {
            notifyArcadeHUD(QStringLiteral("Teleport"), 0);
        }
    } else if (effectiveType == PowerupType::Magnet && targetPhysics) {
        targetPhysics->activateMagnet(6000);
        notifyArcadeHUD(QStringLiteral("Magnet"), 6000);
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
                                                QColor(255, 48, 118),
                                                m_vehiclePhysics->isSpeedBoostActive(),
                                                m_vehiclePhysics->isShieldActive(),
                                                m_vehiclePhysics->isInvisible(),
                                                m_vehiclePhysics->isMagnetActive());
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
    if (m_gameView) {
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
    if (mode == QStringLiteral("Custom Track") && m_runtimeCustomTrack) {
        startCustomTrackSession(m_runtimeCustomTrack);
    } else {
        startDrivingSession(mode);
    }
}

void MainWindow::updateTrafficAndHud(int tick)
{
    // LearningHUD is fully deprecated; all HUD data goes to ArcadeHUD.
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
                                        QColor(255, 48, 118),
                                        m_vehiclePhysics ? m_vehiclePhysics->isSpeedBoostActive() : false,
                                        m_vehiclePhysics ? m_vehiclePhysics->isShieldActive() : false,
                                        m_vehiclePhysics ? m_vehiclePhysics->isInvisible() : false,
                                        m_vehiclePhysics ? m_vehiclePhysics->isMagnetActive() : false);
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
            m_gameView->updatePlayerCar(m_playerPosition, m_playerRotation, displaySpeedKmh());
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
                                    QColor(255, 48, 118),
                                    m_vehiclePhysics ? m_vehiclePhysics->isSpeedBoostActive() : false,
                                    m_vehiclePhysics ? m_vehiclePhysics->isShieldActive() : false,
                                    m_vehiclePhysics ? m_vehiclePhysics->isInvisible() : false,
                                    m_vehiclePhysics ? m_vehiclePhysics->isMagnetActive() : false);
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
        m_gameView->updatePlayerCar(data.position, data.rotation, data.speed);
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
    if (m_arcadeHUD) {
        m_arcadeHUD->show();
        updateRaceHud();
    }

    PhantomDrive::InteractiveFeedback::instance(this).showCountdown(3);

    // Voice countdown + race start sound are handled inside InteractiveFeedback::showCountdown
    // and in onRaceStart respectively.
    PhantomDrive::SoundManager::instance(this).play(PhantomDrive::SoundEffect::RaceStart);

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

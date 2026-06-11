#include "track/BuiltInTrackFactory.h"

#include "track/Checkpoint.h"
#include "track/TrackData.h"
#include "track/TrackTile.h"

#include <QtGlobal>
#include <QVector>

namespace PhantomDrive {
namespace {

constexpr int TrackRows = 18;
constexpr int TrackCols = 24;
constexpr int RoadRadius = 1;

struct TileCoord {
    int row;
    int col;
};

QVector<TileCoord> builtInAIDrivingRoute(const QString& id)
{
    if (id == QStringLiteral("neon_loop")) {
        return {
            {4, 11},
            {4, 19},
            {13, 19},
            {13, 4},
            {4, 4}
        };
    }
    if (id == QStringLiteral("blade_s")) {
        return {
            {3, 5},
            {3, 19},
            {8, 19},
            {8, 4},
            {15, 4},
            {15, 19},
            {3, 19}
        };
    }
    if (id == QStringLiteral("overdrive_sprint")) {
        return {
            {12, 12},
            {12, 3},
            {5, 3},
            {5, 20},
            {12, 20}
        };
    }
    if (id == QStringLiteral("circuit_breaker")) {
        return {
            {14, 6},
            {14, 9},
            {10, 9},
            {10, 15},
            {14, 15},
            {14, 20},
            {4, 20},
            {4, 13},
            {7, 13},
            {7, 7},
            {4, 7},
            {4, 4},
            {14, 4}
        };
    }
    return {};
}

enum class GateAxis {
    Horizontal,
    Vertical
};

QVector<BuiltInTrackInfo> builtInTrackSpecs()
{
    return {
        {QStringLiteral("neon_loop"),
         QStringLiteral("Neon Loop"),
         QStringLiteral("A balanced night-city loop with wide corners and a center barrier."),
         QStringLiteral("Easy")},
        {QStringLiteral("blade_s"),
         QStringLiteral("Blade S"),
         QStringLiteral("A sharp S-course with alternating sweepers and short recovery lanes."),
         QStringLiteral("Medium")},
        {QStringLiteral("overdrive_sprint"),
         QStringLiteral("Overdrive Sprint"),
         QStringLiteral("A fast two-straight circuit built for clean throttle control."),
         QStringLiteral("Easy")},
        {QStringLiteral("circuit_breaker"),
         QStringLiteral("Circuit Breaker"),
         QStringLiteral("An angular technical route with chicanes and guarded apexes."),
         QStringLiteral("Hard")}
    };
}

QVector2D tileCenter(int row, int col)
{
    return TrackData::tileToWorldCenter(row, col);
}

bool inBounds(int row, int col)
{
    return row >= 0 && row < TrackRows && col >= 0 && col < TrackCols;
}

void setTile(TrackData* track, int row, int col, TileType type, TileDirection direction = TileDirection::North)
{
    if (!track || !inBounds(row, col)) {
        return;
    }

    TrackTile* tile = track->getTileAt(row, col);
    if (!tile) {
        return;
    }

    tile->setType(type);
    tile->setDirection(direction);
}

void paintRoadTile(TrackData* track, int row, int col, TileType roadType = TileType::Road)
{
    for (int dr = -RoadRadius; dr <= RoadRadius; ++dr) {
        for (int dc = -RoadRadius; dc <= RoadRadius; ++dc) {
            setTile(track, row + dr, col + dc, roadType);
        }
    }
}

void paintHorizontalRoad(TrackData* track, int row, int colA, int colB, TileType roadType = TileType::Road)
{
    const int start = qMin(colA, colB);
    const int end = qMax(colA, colB);
    for (int col = start; col <= end; ++col) {
        paintRoadTile(track, row, col, roadType);
    }
}

void paintVerticalRoad(TrackData* track, int col, int rowA, int rowB, TileType roadType = TileType::Road)
{
    const int start = qMin(rowA, rowB);
    const int end = qMax(rowA, rowB);
    for (int row = start; row <= end; ++row) {
        paintRoadTile(track, row, col, roadType);
    }
}

void paintSegment(TrackData* track, TileCoord from, TileCoord to, TileType roadType = TileType::Road)
{
    if (from.row == to.row) {
        paintHorizontalRoad(track, from.row, from.col, to.col, roadType);
        return;
    }

    if (from.col == to.col) {
        paintVerticalRoad(track, from.col, from.row, to.row, roadType);
        return;
    }

    paintHorizontalRoad(track, from.row, from.col, to.col, roadType);
    paintVerticalRoad(track, to.col, from.row, to.row, roadType);
}

void paintRoute(TrackData* track, const QVector<TileCoord>& route, TileType roadType = TileType::Road)
{
    if (route.size() < 2) {
        return;
    }

    for (int i = 0; i + 1 < route.size(); ++i) {
        paintSegment(track, route.at(i), route.at(i + 1), roadType);
    }
}

void addOuterWalls(TrackData* track)
{
    for (int col = 0; col < TrackCols; ++col) {
        setTile(track, 0, col, TileType::Wall);
        setTile(track, TrackRows - 1, col, TileType::Wall);
    }

    for (int row = 0; row < TrackRows; ++row) {
        setTile(track, row, 0, TileType::Wall);
        setTile(track, row, TrackCols - 1, TileType::Wall);
    }
}

void paintBarrierLine(TrackData* track, TileCoord from, TileCoord to)
{
    if (from.row == to.row) {
        const int start = qMin(from.col, to.col);
        const int end = qMax(from.col, to.col);
        for (int col = start; col <= end; ++col) {
            setTile(track, from.row, col, TileType::Barrier);
        }
        return;
    }

    if (from.col == to.col) {
        const int start = qMin(from.row, to.row);
        const int end = qMax(from.row, to.row);
        for (int row = start; row <= end; ++row) {
            setTile(track, row, from.col, TileType::Barrier);
        }
    }
}

void addCheckpoint(TrackData* track, int id, int routeIndex, TileCoord tile, GateAxis axis)
{
    auto* checkpoint = new Checkpoint(id, tileCenter(tile.row, tile.col), track);
    checkpoint->setIndexInRoute(routeIndex);
    checkpoint->setMandatory(true);
    checkpoint->setActive(true);
    checkpoint->setRequiredLap(1);

    if (axis == GateAxis::Horizontal) {
        checkpoint->setWidth(192.0);
        checkpoint->setHeight(64.0);
    } else {
        checkpoint->setWidth(64.0);
        checkpoint->setHeight(192.0);
    }

    track->addCheckpoint(checkpoint);
}

void addItemBoxes(TrackData* track, const QVector<TileCoord>& tiles)
{
    for (const TileCoord& tile : tiles) {
        track->addItemBoxPosition(tileCenter(tile.row, tile.col));
    }
}

QList<QVector2D> denseWaypointsFromRoute(const QVector<TileCoord>& route, qreal spacingPx = 64.0)
{
    QList<QVector2D> waypoints;
    if (route.isEmpty()) {
        return waypoints;
    }

    auto appendUnique = [&waypoints](const QVector2D& point) {
        if (waypoints.isEmpty() || (waypoints.last() - point).lengthSquared() > 0.5) {
            waypoints.append(point);
        }
    };

    const QVector2D first = tileCenter(route.first().row, route.first().col);
    appendUnique(first);

    auto sampleSegment = [&](const QVector2D& from, const QVector2D& to, bool appendEnd) {
        const QVector2D delta = to - from;
        const qreal length = delta.length();
        if (length < 1.0) {
            if (appendEnd) {
                appendUnique(to);
            }
            return;
        }

        for (qreal distance = spacingPx; distance < length; distance += spacingPx) {
            appendUnique(from + delta * (distance / length));
        }

        if (appendEnd) {
            appendUnique(to);
        }
    };

    for (int i = 1; i < route.size(); ++i) {
        sampleSegment(tileCenter(route.at(i - 1).row, route.at(i - 1).col),
                      tileCenter(route.at(i).row, route.at(i).col),
                      true);
    }

    sampleSegment(tileCenter(route.last().row, route.last().col), first, false);
    return waypoints;
}

TrackData* createBaseTrack(const BuiltInTrackInfo& info, QObject* parent)
{
    auto* track = new TrackData(info.name, parent);
    track->setId(info.id);
    track->setAuthor(QStringLiteral("PhantomDrive"));
    track->setDescription(info.description);
    track->setDifficulty(info.difficulty);
    track->setSize(TrackRows, TrackCols);
    track->setMaxLaps(3);
    addOuterWalls(track);
    return track;
}

TrackData* createNeonLoop(QObject* parent)
{
    const QVector<BuiltInTrackInfo> specs = builtInTrackSpecs();
    TrackData* track = createBaseTrack(specs.at(0), parent);
    paintHorizontalRoad(track, 4, 4, 19, TileType::Asphalt);
    paintVerticalRoad(track, 19, 4, 13, TileType::Asphalt);
    paintHorizontalRoad(track, 13, 4, 19, TileType::Asphalt);
    paintVerticalRoad(track, 4, 4, 13, TileType::Asphalt);

    setTile(track, 4, 11, TileType::StartLine, TileDirection::East);
    setTile(track, 5, 11, TileType::FinishLine, TileDirection::East);
    track->setStartPosition(tileCenter(4, 11));
    track->addStartPosition(tileCenter(4, 11));
    track->setStartRotation(90.0);
    track->setTrackLength(3450.0);
    track->setEstimatedLapTime(42.0);

    addCheckpoint(track, 1, 0, {4, 17}, GateAxis::Vertical);
    addCheckpoint(track, 2, 1, {9, 19}, GateAxis::Horizontal);
    addCheckpoint(track, 3, 2, {13, 9}, GateAxis::Vertical);
    addCheckpoint(track, 4, 3, {8, 4}, GateAxis::Horizontal);
    addItemBoxes(track, {{4, 7}, {9, 19}, {13, 16}, {8, 4}});
    paintBarrierLine(track, {8, 10}, {8, 13});
    paintBarrierLine(track, {9, 10}, {9, 13});
    return track;
}

TrackData* createBladeS(QObject* parent)
{
    const QVector<BuiltInTrackInfo> specs = builtInTrackSpecs();
    TrackData* track = createBaseTrack(specs.at(1), parent);
    paintRoute(track, {
        {3, 4}, {3, 19}, {8, 19}, {8, 4}, {15, 4}, {15, 19}, {3, 19}
    });

    setTile(track, 3, 5, TileType::StartLine, TileDirection::East);
    track->setStartPosition(tileCenter(3, 5));
    track->addStartPosition(tileCenter(3, 5));
    track->setStartRotation(90.0);
    track->setTrackLength(3900.0);
    track->setEstimatedLapTime(48.0);

    addCheckpoint(track, 1, 0, {3, 17}, GateAxis::Vertical);
    addCheckpoint(track, 2, 1, {8, 6}, GateAxis::Vertical);
    addCheckpoint(track, 3, 2, {15, 10}, GateAxis::Vertical);
    addCheckpoint(track, 4, 3, {10, 19}, GateAxis::Horizontal);
    addItemBoxes(track, {{3, 11}, {8, 14}, {15, 7}, {14, 19}});
    paintBarrierLine(track, {5, 11}, {6, 11});
    paintBarrierLine(track, {10, 12}, {12, 12});
    return track;
}

TrackData* createOverdriveSprint(QObject* parent)
{
    const QVector<BuiltInTrackInfo> specs = builtInTrackSpecs();
    TrackData* track = createBaseTrack(specs.at(2), parent);
    paintHorizontalRoad(track, 5, 3, 20, TileType::Asphalt);
    paintVerticalRoad(track, 20, 5, 12, TileType::Asphalt);
    paintHorizontalRoad(track, 12, 3, 20, TileType::Asphalt);
    paintVerticalRoad(track, 3, 5, 12, TileType::Asphalt);

    setTile(track, 12, 12, TileType::StartLine, TileDirection::West);
    track->setStartPosition(tileCenter(12, 12));
    track->addStartPosition(tileCenter(12, 12));
    track->setStartRotation(270.0);
    track->setTrackLength(3200.0);
    track->setEstimatedLapTime(35.0);

    addCheckpoint(track, 1, 0, {12, 5}, GateAxis::Vertical);
    addCheckpoint(track, 2, 1, {8, 3}, GateAxis::Horizontal);
    addCheckpoint(track, 3, 2, {5, 12}, GateAxis::Vertical);
    addCheckpoint(track, 4, 3, {9, 20}, GateAxis::Horizontal);
    addItemBoxes(track, {{12, 8}, {5, 8}, {5, 16}, {12, 17}});
    paintBarrierLine(track, {8, 8}, {8, 15});
    paintBarrierLine(track, {9, 8}, {9, 15});
    return track;
}

TrackData* createCircuitBreaker(QObject* parent)
{
    const QVector<BuiltInTrackInfo> specs = builtInTrackSpecs();
    TrackData* track = createBaseTrack(specs.at(3), parent);
    paintRoute(track, {
        {14, 5}, {14, 9}, {10, 9}, {10, 15}, {14, 15}, {14, 20},
        {4, 20}, {4, 13}, {7, 13}, {7, 7}, {4, 7}, {4, 4}, {14, 4}, {14, 5}
    });

    setTile(track, 14, 6, TileType::StartLine, TileDirection::East);
    track->setStartPosition(tileCenter(14, 6));
    track->addStartPosition(tileCenter(14, 6));
    track->setStartRotation(90.0);
    track->setTrackLength(4100.0);
    track->setEstimatedLapTime(55.0);
    track->setMaxLaps(4);

    addCheckpoint(track, 1, 0, {10, 9}, GateAxis::Horizontal);
    addCheckpoint(track, 2, 1, {14, 18}, GateAxis::Vertical);
    addCheckpoint(track, 3, 2, {4, 18}, GateAxis::Vertical);
    addCheckpoint(track, 4, 3, {7, 9}, GateAxis::Vertical);
    addCheckpoint(track, 5, 4, {9, 4}, GateAxis::Horizontal);
    addItemBoxes(track, {{14, 11}, {10, 13}, {6, 20}, {7, 11}, {5, 4}});
    paintBarrierLine(track, {12, 12}, {13, 12});
    paintBarrierLine(track, {5, 15}, {6, 15});
    paintBarrierLine(track, {9, 6}, {12, 6});
    return track;
}

} // namespace

QList<BuiltInTrackInfo> BuiltInTrackFactory::tracks()
{
    QList<BuiltInTrackInfo> result;
    for (const BuiltInTrackInfo& info : builtInTrackSpecs()) {
        result.append(info);
    }
    return result;
}

QStringList BuiltInTrackFactory::trackIds()
{
    QStringList ids;
    for (const BuiltInTrackInfo& info : tracks()) {
        ids.append(info.id);
    }
    return ids;
}

QString BuiltInTrackFactory::trackName(const QString& id)
{
    for (const BuiltInTrackInfo& info : tracks()) {
        if (info.id == id) {
            return info.name;
        }
    }
    return QString();
}

bool BuiltInTrackFactory::isBuiltInTrackId(const QString& id)
{
    return !trackName(id).isEmpty();
}

TrackData* BuiltInTrackFactory::createTrack(const QString& id, QObject* parent)
{
    if (id == QStringLiteral("neon_loop")) {
        return createNeonLoop(parent);
    }
    if (id == QStringLiteral("blade_s")) {
        return createBladeS(parent);
    }
    if (id == QStringLiteral("overdrive_sprint")) {
        return createOverdriveSprint(parent);
    }
    if (id == QStringLiteral("circuit_breaker")) {
        return createCircuitBreaker(parent);
    }
    return nullptr;
}

QList<QVector2D> BuiltInTrackFactory::getAIDrivingWaypoints(const QString& id)
{
    return denseWaypointsFromRoute(builtInAIDrivingRoute(id));
}

} // namespace PhantomDrive

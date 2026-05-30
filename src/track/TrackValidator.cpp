#include "track/TrackValidator.h"

#include "track/Checkpoint.h"
#include "track/TrackTile.h"

namespace PhantomDrive {

namespace {

constexpr qreal TileSize = 64.0;
constexpr int MinimumDrivableTiles = 12;

int tileRowForPoint(const QVector2D& point)
{
    return static_cast<int>(point.y() / TileSize);
}

int tileColForPoint(const QVector2D& point)
{
    return static_cast<int>(point.x() / TileSize);
}

} // namespace

TrackValidationResult TrackValidator::validateCustomTrack(const TrackData& track)
{
    TrackValidationResult result;

    if (track.getRowCount() != 18 || track.getColCount() != 24) {
        result.errors.append(QStringLiteral("Custom Track Mode requires a 24 x 18 tile map."));
    }

    const QList<QVector2D> startPositions = track.getStartPositions();
    if (startPositions.size() != 1) {
        result.errors.append(QStringLiteral("Track must have exactly one player Start."));
    } else if (!isPointOnDrivableTile(track, startPositions.first())) {
        result.errors.append(QStringLiteral("Start must be placed on a drivable tile."));
    }

    int finishTileCount = 0;
    QVector2D finishCenter;
    int drivableTileCount = 0;
    for (int row = 0; row < track.getRowCount(); ++row) {
        for (int col = 0; col < track.getColCount(); ++col) {
            const TrackTile* tile = track.getTileAt(row, col);
            if (!tile) {
                continue;
            }

            const TileType type = tile->getType();
            if (isDrivableType(type)) {
                ++drivableTileCount;
            }
            if (type == TileType::FinishLine || type == TileType::StartLine) {
                ++finishTileCount;
                finishCenter = tile->getCenter();
            }
        }
    }

    if (finishTileCount != 1) {
        result.errors.append(QStringLiteral("Track must have exactly one Finish / StartLine tile."));
    } else if (!isPointOnDrivableTile(track, finishCenter)) {
        result.errors.append(QStringLiteral("Finish / StartLine must be on a drivable tile."));
    }

    if (drivableTileCount < MinimumDrivableTiles) {
        result.errors.append(QStringLiteral("Track must contain at least 12 drivable Road / Asphalt / line tiles."));
    }

    const QList<Checkpoint*> checkpoints = track.getCheckpointsInOrder();
    if (checkpoints.size() < 2) {
        result.errors.append(QStringLiteral("Track must have at least 2 checkpoints."));
    }

    for (int i = 0; i < checkpoints.size(); ++i) {
        const Checkpoint* cp = checkpoints.at(i);
        if (!cp) {
            result.errors.append(QStringLiteral("Checkpoint %1 is invalid.").arg(i + 1));
            continue;
        }
        if (!isPointOnDrivableTile(track, cp->getPosition())) {
            result.errors.append(QStringLiteral("Checkpoint %1 must be placed on a drivable tile.").arg(i + 1));
        }
    }

    const QList<QVector2D> itemBoxes = track.getItemBoxPositions();
    for (int i = 0; i < itemBoxes.size(); ++i) {
        if (!isPointNearDrivableTile(track, itemBoxes.at(i))) {
            result.errors.append(QStringLiteral("Item Box %1 must be on or next to a drivable tile.").arg(i + 1));
        }
    }

    result.warnings.append(QStringLiteral("Road connectivity is not checked in this first version."));
    result.ok = result.errors.isEmpty();
    return result;
}

bool TrackValidator::isDrivableType(TileType type)
{
    return type == TileType::Road
        || type == TileType::Asphalt
        || type == TileType::StartLine
        || type == TileType::FinishLine;
}

bool TrackValidator::isPointOnDrivableTile(const TrackData& track, const QVector2D& point)
{
    const int row = tileRowForPoint(point);
    const int col = tileColForPoint(point);
    const TrackTile* tile = track.getTileAt(row, col);
    return tile && isDrivableType(tile->getType());
}

bool TrackValidator::isPointNearDrivableTile(const TrackData& track, const QVector2D& point)
{
    const int centerRow = tileRowForPoint(point);
    const int centerCol = tileColForPoint(point);

    for (int row = centerRow - 1; row <= centerRow + 1; ++row) {
        for (int col = centerCol - 1; col <= centerCol + 1; ++col) {
            const TrackTile* tile = track.getTileAt(row, col);
            if (tile && isDrivableType(tile->getType())) {
                return true;
            }
        }
    }

    return false;
}

} // namespace PhantomDrive

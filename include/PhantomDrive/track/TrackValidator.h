#pragma once

#include "PhantomDrive_global.h"
#include "TrackData.h"

#include <QStringList>

namespace PhantomDrive {

struct PHANTOMDRIVE_EXPORT TrackValidationResult {
    bool ok = false;
    QStringList errors;
    QStringList warnings;
};

class PHANTOMDRIVE_EXPORT TrackValidator
{
public:
    static TrackValidationResult validateCustomTrack(const TrackData& track);

private:
    static bool isDrivableType(TileType type);
    static bool isPointOnDrivableTile(const TrackData& track, const QVector2D& point);
    static bool isPointNearDrivableTile(const TrackData& track, const QVector2D& point);
};

} // namespace PhantomDrive

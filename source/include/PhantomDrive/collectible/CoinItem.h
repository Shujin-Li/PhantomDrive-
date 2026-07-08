#pragma once

#include "PhantomDrive_global.h"

#include <QPointF>

namespace PhantomDrive {

struct PHANTOMDRIVE_EXPORT CoinItem
{
    QPointF position;
    qreal radius = 10.0;
    bool active = false;
    bool collected = false;
    qreal respawnTimer = 0.0;
    int value = 1;
    int phaseSeed = 0;
};

} // namespace PhantomDrive

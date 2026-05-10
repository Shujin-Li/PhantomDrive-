#ifndef WAYPOINT_H
#define WAYPOINT_H

#include <QPointF>

namespace PhantomDrive {

struct Waypoint
{
    QPointF position;
    float targetSpeed = 0.0f;
};

}

#endif // WAYPOINT_H
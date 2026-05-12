#ifndef AIOPPONENT_H
#define AIOPPONENT_H

#include <QObject>
#include <QList>

#include "AIDecision.h"
#include "Waypoint.h"

namespace PhantomDrive {

class AIOpponent : public QObject
{
    //Q_OBJECT

public:
    explicit AIOpponent(QObject* parent = nullptr);
    virtual ~AIOpponent();

    virtual void update(float deltaTime) = 0;

    virtual AIDecision makeDecision() = 0;

    void setWaypoints(const QList<Waypoint>& waypoints);

protected:
    QList<Waypoint> m_waypoints;
};

}

#endif // AIOPPONENT_H

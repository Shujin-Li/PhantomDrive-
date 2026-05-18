
#include <QCoreApplication>
#include <QDebug>

#include <PhantomDrive/ai/StrategicAI.h>

using namespace PhantomDrive;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    StrategicAI ai;
    QList<Waypoint> points;

    Waypoint wp1;
    wp1.x = 100;
    wp1.y = 50;

    Waypoint wp2;
    wp2.x = -100;
    wp2.y = 20;

    points.append(wp1);
    points.append(wp2);

    ai.setWaypoints(points);

    for (int i = 0; i < 30; i++)
    {
        ai.update(0.016f);

        AIDecision decision = ai.makeDecision();

        qDebug() << "Step:" << i;
        qDebug() << "Throttle:" << decision.throttle;
        qDebug() << "Brake:" << decision.brake;
        qDebug() << "Steering:" << decision.steering;
    }

    AIDecision decision = ai.makeDecision();

    qDebug() << "Throttle:" << decision.throttle;
    qDebug() << "Brake:" << decision.brake;
    qDebug() << "Steering:" << decision.steering;

    return 0;
}
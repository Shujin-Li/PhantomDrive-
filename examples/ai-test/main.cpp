#include <QCoreApplication>
#include <QDebug>

#include <PhantomDrive/ai/AIOpponentFSM.h>

using namespace PhantomDrive;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    AIOpponentFSM ai;

    ai.update(0.016f);

    AIDecision decision = ai.makeDecision();

    qDebug() << "Throttle:" << decision.throttle;
    qDebug() << "Brake:" << decision.brake;
    qDebug() << "Steering:" << decision.steering;

    return 0;
}
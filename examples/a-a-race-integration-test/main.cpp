#include <QCoreApplication>
#include <QDebug>
#include <QJsonDocument>

#include "PhantomDrive/gamemode/AIOpponentManager.h"

using namespace PhantomDrive;

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    AIOpponentManager manager;
    manager.setRaceTotalLaps(1);
    manager.setTrackBounds(QRectF(0, 0, 1000, 1000));
    manager.setPlayerPosition(QVector2D(180, 180));
    manager.setPlayerRaceProgress(0, 0, 5.0);

    QList<Waypoint> waypoints;
    waypoints << Waypoint(QVector2D(180, 180), 90, false, 0, 0)
              << Waypoint(QVector2D(820, 180), 115, false, 0, 1)
              << Waypoint(QVector2D(820, 820), 70, true, 2, 2)
              << Waypoint(QVector2D(180, 820), 95, true, 1, 3);

    AIOpponent* hard = manager.createOpponent(QStringLiteral("ai_hard"), AIStyle::Aggressive);
    AIOpponent* easy = manager.createOpponent(QStringLiteral("ai_easy"), AIStyle::Conservative);
    if (!hard || !easy) {
        qCritical() << "Failed to create AI opponents";
        return 1;
    }

    manager.setWaypointsForAll(waypoints);
    hard->setPosition(QVector2D(180, 180));
    easy->setPosition(QVector2D(220, 220));
    hard->setState(AIState::Racing);
    easy->setState(AIState::Racing);

    for (int step = 0; step < 900; ++step) {
        const qreal playerProgress = qMin(100.0, step / 9.0);
        manager.setPlayerRaceProgress(0, static_cast<int>(playerProgress), playerProgress);
        manager.update(50);
    }

    const QStringList order = manager.getRaceOrder();
    qDebug() << "A-A race order:" << order;
    qDebug().noquote() << QJsonDocument(manager.raceResultsToJson()).toJson(QJsonDocument::Compact);

    if (order.size() < 3 || manager.getOpponentPosition(QStringLiteral("ai_hard")) <= 0) {
        qCritical() << "Ranking integration failed";
        return 2;
    }

    return 0;
}

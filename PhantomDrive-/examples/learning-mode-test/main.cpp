#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QDateTime>
#include <QtMath>

#include "PhantomDrive/gamemode/TrafficObject.h"
#include "PhantomDrive/gamemode/TrafficLightObject.h"
#include "PhantomDrive/gamemode/SpeedLimitSignObject.h"
#include "PhantomDrive/gamemode/PedestrianCrossingObject.h"
#include "PhantomDrive/gamemode/TrafficObjectManager.h"

using namespace PhantomDrive;

void testTrafficLight()
{
    qDebug() << "\n========== TrafficLightObject Test ==========";

    TrafficLightObject light("traffic_light_1");

    qDebug() << "Initial state:";
    qDebug() << "  ID:" << light.getId();
    qDebug() << "  Current color:" << (light.isRed() ? "Red" : (light.isGreen() ? "Green" : "Yellow"));
    qDebug() << "  Cycle duration:" << light.getTotalCycleDurationMs() << "ms";

    light.setRedDurationMs(3000);
    light.setYellowDurationMs(1500);
    light.setGreenDurationMs(3000);
    qDebug() << "  Updated cycle duration:" << light.getTotalCycleDurationMs() << "ms";

    light.start();
    qDebug() << "  Light started";

    for (int i = 0; i < 5; ++i) {
        QThread::msleep(500);
        light.update(500);
        QString color = light.isRed() ? "RED" : (light.isYellow() ? "YELLOW" : "GREEN");
        qDebug() << "  [" << i << "] Color:" << color << "| Remaining:" << light.getRemainingTimeInCurrentState() << "ms";
    }

    light.stop();
    qDebug() << "  Light stopped";
    qDebug() << "  Total cycles:" << light.getTotalCycleDurationMs();

    QRectF testBounds(0.0, 0.0, 10.0, 10.0);
    light.setBounds(testBounds);
    QVector2D testPos(5.0, 5.0);
    qDebug() << "  Violation check at center:" << light.checkRedLightViolation(testPos);
}

void testSpeedLimitSign()
{
    qDebug() << "\n========== SpeedLimitSignObject Test ==========";

    SpeedLimitSignObject sign("speed_limit_1");
    sign.setSpeedLimit(60.0);
    sign.setPosition(QVector2D(100.0, 100.0));
    sign.setDetectionRadius(50.0);

    qDebug() << "Initial state:";
    qDebug() << "  ID:" << sign.getId();
    qDebug() << "  Speed limit:" << sign.getSpeedLimit();
    qDebug() << "  Position:" << sign.getPosition().x() << "," << sign.getPosition().y();
    qDebug() << "  Detection radius:" << sign.getDetectionRadius();

    qDebug() << "\nSpeed violation tests:";
    qDebug() << "  Speed 50 (limit 60): violation =" << sign.checkSpeedViolation(50.0);
    qDebug() << "  Speed 70 (limit 60): violation =" << sign.checkSpeedViolation(70.0);
    qDebug() << "  Overspeed 70 vs 60:" << sign.getOverspeedPercentage(70.0) << "%";

    qDebug() << "\nZone detection tests:";
    QVector2D insidePos(105.0, 105.0);
    QVector2D outsidePos(200.0, 200.0);
    qDebug() << "  Position (105, 105) in zone:" << sign.isVehicleInZone(insidePos);
    qDebug() << "  Position (200, 200) in zone:" << sign.isVehicleInZone(outsidePos);

    sign.onVehiclePositionChanged(insidePos);
    sign.onVehicleSpeedChanged(80.0);
    qDebug() << "  Max speed recorded:" << sign.getMaxSpeedRecorded();
    qDebug() << "  Violation count:" << sign.getViolationCount();

    sign.reset();
    qDebug() << "\nAfter reset:";
    qDebug() << "  Violation count:" << sign.getViolationCount();
    qDebug() << "  Max speed:" << sign.getMaxSpeedRecorded();
}

void testPedestrianCrossing()
{
    qDebug() << "\n========== PedestrianCrossingObject Test ==========";

    PedestrianCrossingObject crossing("crossing_1");
    QRectF bounds(0.0, 0.0, 20.0, 5.0);
    crossing.setBounds(bounds);
    crossing.setPosition(QVector2D(10.0, 2.5));

    qDebug() << "Initial state:";
    qDebug() << "  ID:" << crossing.getId();
    qDebug() << "  Bounds:" << crossing.getBounds().x() << "," << crossing.getBounds().y()
             << "-" << crossing.getBounds().width() << "x" << crossing.getBounds().height();
    qDebug() << "  Pedestrian count:" << crossing.getPedestrianCount();

    qDebug() << "\nAdding pedestrians:";
    crossing.addPedestrian("ped_1");
    crossing.addPedestrian("ped_2");
    qDebug() << "  After adding 2:" << crossing.getPedestrianCount() << "pedestrians";

    qDebug() << "\nZone detection tests:";
    QVector2D insidePos(10.0, 2.5);
    QVector2D outsidePos(50.0, 50.0);
    qDebug() << "  Position (10, 2.5) in zone:" << crossing.isVehicleInZone(insidePos);
    qDebug() << "  Position (50, 50) in zone:" << crossing.isVehicleInZone(outsidePos);

    qDebug() << "\nSimulation update (10s):";
    for (int i = 0; i < 20; ++i) {
        crossing.update(500);
        if (i % 5 == 0) {
            qDebug() << "  [" << i << "] Pedestrians:" << crossing.getPedestrianCount();
        }
    }

    qDebug() << "\nViolation test:";
    crossing.addPedestrian("ped_test");
    bool hasViolation = crossing.checkPedestrianViolation(insidePos);
    qDebug() << "  Violation when pedestrian crossing:" << hasViolation;

    crossing.reset();
    qDebug() << "\nAfter reset:";
    qDebug() << "  Pedestrians:" << crossing.getPedestrianCount();
    qDebug() << "  Violations:" << crossing.getViolationCount();
}

void testTrafficObjectManager()
{
    qDebug() << "\n========== TrafficObjectManager Test ==========";

    TrafficObjectManager manager;

    qDebug() << "Initial state:";
    qDebug() << "  Total objects:" << manager.getTotalObjectCount();
    qDebug() << "  Traffic lights:" << manager.getTrafficLights().size();
    qDebug() << "  Speed limit signs:" << manager.getSpeedLimitSigns().size();
    qDebug() << "  Pedestrian crossings:" << manager.getPedestrianCrossings().size();

    TrafficLightObject* light = new TrafficLightObject("tl_main");
    light->setPosition(QVector2D(0.0, 0.0));
    light->setBounds(QRectF(-5.0, -5.0, 10.0, 10.0));
    manager.registerTrafficObject(light);

    SpeedLimitSignObject* sign = new SpeedLimitSignObject("sl_school");
    sign->setPosition(QVector2D(100.0, 100.0));
    sign->setSpeedLimit(30.0);
    sign->setDetectionRadius(30.0);
    manager.registerTrafficObject(sign);

    PedestrianCrossingObject* crossing = new PedestrianCrossingObject("pc_crosswalk");
    crossing->setPosition(QVector2D(50.0, 50.0));
    crossing->setBounds(QRectF(45.0, 45.0, 10.0, 10.0));
    manager.registerTrafficObject(crossing);

    qDebug() << "\nAfter registration:";
    qDebug() << "  Total objects:" << manager.getTotalObjectCount();
    qDebug() << "  Traffic lights:" << manager.getTrafficLights().size();
    qDebug() << "  Speed limit signs:" << manager.getSpeedLimitSigns().size();
    qDebug() << "  Pedestrian crossings:" << manager.getPedestrianCrossings().size();

    qDebug() << "\nQuery by position:";
    QList<TrafficObject*> nearLight = manager.getObjectsInRange(QVector2D(0.0, 0.0), 20.0);
    qDebug() << "  Objects within 20 units of (0,0):" << nearLight.size();

    QList<TrafficObject*> nearCrossing = manager.getObjectsInRange(QVector2D(50.0, 50.0), 20.0);
    qDebug() << "  Objects within 20 units of (50,50):" << nearCrossing.size();

    qDebug() << "\nSpeed limit check:";
    qDebug() << "  Current limit at (100, 100):" << manager.getCurrentSpeedLimit(QVector2D(100.0, 100.0));
    qDebug() << "  Current limit at (0, 0):" << manager.getCurrentSpeedLimit(QVector2D(0.0, 0.0));

    manager.startAll();
    qDebug() << "\nAll traffic lights started";

    manager.onVehiclePositionChanged(QVector2D(100.0, 100.0));
    manager.onVehicleSpeedChanged(50.0);

    manager.update(1000);
    qDebug() << "\nManager updated for 1000ms";
    qDebug() << "  Total violations:" << manager.getTotalViolationCount();

    manager.stopAll();
    qDebug() << "\nAll traffic lights stopped";

    manager.clear();
    qDebug() << "\nAfter clear:";
    qDebug() << "  Total objects:" << manager.getTotalObjectCount();
}

void testLearningModeIntegration()
{
    qDebug() << "\n========== Learning Mode Integration Test ==========";

    TrafficObjectManager manager;

    TrafficLightObject* light1 = new TrafficLightObject("light_intersection_1");
    light1->setPosition(QVector2D(0.0, 0.0));
    light1->setBounds(QRectF(-5.0, -5.0, 10.0, 10.0));
    light1->setRedDurationMs(5000);
    light1->setGreenDurationMs(5000);
    light1->setYellowDurationMs(1500);
    manager.registerTrafficObject(light1);

    SpeedLimitSignObject* signSchool = new SpeedLimitSignObject("sign_school_zone");
    signSchool->setPosition(QVector2D(50.0, 0.0));
    signSchool->setSpeedLimit(30.0);
    signSchool->setDetectionRadius(40.0);
    signSchool->setZoneId("school_zone");
    manager.registerTrafficObject(signSchool);

    SpeedLimitSignObject* signHighway = new SpeedLimitSignObject("sign_highway");
    signHighway->setPosition(QVector2D(200.0, 0.0));
    signHighway->setSpeedLimit(120.0);
    signHighway->setDetectionRadius(50.0);
    signHighway->setZoneId("highway");
    manager.registerTrafficObject(signHighway);

    PedestrianCrossingObject* crossing = new PedestrianCrossingObject("crossing_main");
    crossing->setPosition(QVector2D(100.0, 0.0));
    crossing->setBounds(QRectF(95.0, -5.0, 10.0, 10.0));
    manager.registerTrafficObject(crossing);

    qDebug() << "Test scenario: Vehicle drives through traffic elements";
    qDebug() << "  Route: (0,0) -> (50,0) -> (100,0) -> (200,0)";

    manager.startAll();

    QVector2D positions[] = {
        QVector2D(0.0, 0.0),
        QVector2D(25.0, 0.0),
        QVector2D(50.0, 0.0),
        QVector2D(75.0, 0.0),
        QVector2D(100.0, 0.0),
        QVector2D(150.0, 0.0),
        QVector2D(200.0, 0.0)
    };

    qreal speeds[] = {30.0, 35.0, 40.0, 35.0, 30.0, 80.0, 120.0};

    for (int i = 0; i < 7; ++i) {
        qDebug() << "\n  Step" << i << ": Position" << positions[i].x() << "," << positions[i].y()
                 << "Speed:" << speeds[i];

        manager.onVehiclePositionChanged(positions[i]);
        manager.onVehicleSpeedChanged(speeds[i]);
        manager.update(1000);

        qDebug() << "    Speed limit:" << manager.getCurrentSpeedLimit(positions[i]);

        if (manager.getTotalViolationCount() > 0) {
            qDebug() << "    VIOLATION! Total:" << manager.getTotalViolationCount();
        }
    }

    qDebug() << "\nFinal Statistics:";
    qDebug() << "  Total violations:" << manager.getTotalViolationCount();
    qDebug() << "  Red light violations:" << manager.getRedLightViolationCount();
    qDebug() << "  Speed violations:" << manager.getSpeedViolationCount();
    qDebug() << "  Pedestrian violations:" << manager.getPedestrianViolationCount();

    manager.stopAll();
    manager.clear();
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "========================================";
    qDebug() << "  PhantomDrive Learning Mode Test Suite";
    qDebug() << "  Traffic Object System Tests";
    qDebug() << "========================================";

    testTrafficLight();

    testSpeedLimitSign();

    testPedestrianCrossing();

    testTrafficObjectManager();

    testLearningModeIntegration();

    qDebug() << "\n========================================";
    qDebug() << "  All Tests Completed Successfully!";
    qDebug() << "========================================";

    return 0;
}

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QDateTime>

#include "PhantomDrive/gamemode/DrivingDataCollector.h"
#include "PhantomDrive/gamemode/VehicleSensor.h"
#include "PhantomDrive/gamemode/CollisionDetector.h"
#include "PhantomDrive/gamemode/DrivingDataStorage.h"

using namespace PhantomDrive;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "=== DrivingDataCollector 测试 ===" << endl;

    // 创建采集器
    DrivingDataCollector collector;
    collector.setVehicleId("test_vehicle_01");
    collector.setSamplingInterval(100); // 10Hz

    // 配置碰撞检测
    collector.setCollisionDetectionEnabled(true);
    collector.registerCollidableObject("obstacle_1", QVector2D(10.0f, 20.0f), 1.0f);
    collector.registerCollidableObject("obstacle_2", QVector2D(15.0f, 25.0f), 1.5f);

    // 连接信号
    QObject::connect(&collector, &DrivingDataCollector::dataCollected,
                     [](const DrivingData& data) {
                         qDebug() << "[数据] 时间:" << data.timestamp
                                  << "位置:" << data.position.x() << "," << data.position.y()
                                  << "速度:" << data.speed
                                  << "转向角:" << data.steeringAngle;
                     });

    QObject::connect(&collector, &DrivingDataCollector::violationDetected,
                     [](const ViolationEvent& violation) {
                         qDebug() << "[违规] 类型:" << static_cast<int>(violation.type)
                                  << "描述:" << violation.description
                                  << "罚分:" << violation.penaltyPoints;
                     });

    QObject::connect(collector.collisionDetector(), &CollisionDetector::collisionDetected,
                     [](const QString& objectId, const QVector2D& position, qreal impactForce) {
                         qDebug() << "[碰撞] 物体:" << objectId
                                  << "位置:" << position.x() << "," << position.y()
                                  << "冲击力:" << impactForce;
                     });

    QObject::connect(collector.vehicleSensor(), &VehicleSensor::speedLimitExceeded,
                     [](qreal currentSpeed, qreal limit) {
                         qDebug() << "[超速] 当前速度:" << currentSpeed
                                  << "限制:" << limit;
                     });

    // 开始采集
    collector.startCollection();
    qDebug() << "\n采集已开始..." << endl;

    // 模拟车辆数据更新
    QTimer updateTimer;
    int frameCount = 0;
    qreal simulatedTime = 0.0;

    QObject::connect(&updateTimer, &QTimer::timeout,
                     [&]() {
                         frameCount++;
                         simulatedTime += 0.1f; // 100ms

                         VehicleSensor* sensor = collector.vehicleSensor();

                         // 模拟车辆运动
                         qreal x = qCos(simulatedTime) * 10.0f;
                         qreal y = qSin(simulatedTime) * 10.0f;
                         QVector2D position(x, y);

                         qreal vx = -qSin(simulatedTime) * 10.0f;
                         qreal vy = qCos(simulatedTime) * 10.0f;
                         QVector2D velocity(vx, vy);

                         qreal rotation = qAtan2(vy, vx) * 180.0 / 3.14159;
                         qreal steeringAngle = qSin(simulatedTime * 2.0) * 30.0;
                         qreal acceleration = qCos(simulatedTime * 3.0) * 2.0;

                         // 更新传感器数据
                         sensor->updatePosition(position);
                         sensor->updateVelocity(velocity);
                         sensor->updateRotation(rotation);
                         sensor->updateSteeringAngle(steeringAngle);
                         sensor->updateAcceleration(acceleration);
                         sensor->updateBrakeState(frameCount % 100 < 20); // 每 10 帧刹车 2 帧
                         sensor->updateAcceleratorState(frameCount % 100 >= 20);

                         // 更新碰撞物位置（模拟移动障碍物）
                         CollisionDetector* detector = collector.collisionDetector();
                         qreal obsX = 5.0f + qCos(simulatedTime * 0.5f) * 3.0f;
                         qreal obsY = 5.0f + qSin(simulatedTime * 0.5f) * 3.0f;
                         detector->updateCollidableObject("obstacle_1", QVector2D(obsX, obsY));

                         // 每 50 帧设置一次速度限制
                         if (frameCount % 50 == 0) {
                             collector.setCurrentSpeedLimit(15.0f, "test_zone");
                         }

                         // 运行 200 帧后停止
                         if (frameCount >= 200) {
                             updateTimer.stop();

                             qDebug() << "\n=== 采集统计 ===" << endl;
                             qDebug() << "采集时长:" << collector.getCollectionDuration() << "ms";
                             qDebug() << "数据点总数:" << collector.getTotalDataPoints();
                             qDebug() << "违规总数:" << collector.getTotalViolations();

                             // 访问存储数据
                             DrivingDataStorage* storage = collector.dataStorage();
                             qDebug() << "平均速度:" << storage->getAverageSpeed();
                             qDebug() << "最大速度:" << storage->getMaxSpeed();
                             qDebug() << "碰撞次数:" << storage->getTotalCollisions();

                             // 导出数据
                             QString fileName = QString("test_data_%1.json")
                                 .arg(QDateTime::currentMSecsSinceEpoch());
                             if (collector.exportData(fileName)) {
                                 qDebug() << "\n数据已导出到:" << fileName << endl;
                             }

                             // 退出程序
                             QCoreApplication::quit();
                         }
                     });

    updateTimer.start(100); // 100ms

    qDebug() << "运行模拟..." << endl;

    return app.exec();
}

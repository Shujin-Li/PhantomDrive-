#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QtMath>

#include "PhantomDrive/gamemode/VehicleSensor.h"
#include "PhantomDrive/gamemode/CollisionDetector.h"

using namespace PhantomDrive;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "=== PhantomDrive 核心功能测试 ===";

    // 1. 测试 VehicleSensor
    qDebug() << "\n[1] 测试 VehicleSensor...";
    VehicleSensor sensor;
    sensor.setVehicleId("test_car_01");
    sensor.setSamplingInterval(100);

    sensor.startSensing();
    qDebug() << "  传感器已启动";

    // 2. 测试 CollisionDetector
    qDebug() << "\n[2] 测试 CollisionDetector...";
    CollisionDetector detector;
    detector.setVehicleId("test_car_01");
    detector.setDetectionRadius(2.0f);
    detector.enableCollisionDetection(true);

    detector.registerCollidableObject("obstacle_1", QVector2D(5.0f, 5.0f), 1.0f);
    detector.registerCollidableObject("obstacle_2", QVector2D(10.0f, 10.0f), 1.5f);
    qDebug() << "  已注册 2 个碰撞物体";

    // 3. 模拟数据更新
    qDebug() << "\n[3] 开始模拟数据更新...";
    QTimer updateTimer;
    int frameCount = 0;

    QObject::connect(&updateTimer, &QTimer::timeout,
                     [&]() {
                         frameCount++;
                         qint64 timestamp = QDateTime::currentMSecsSinceEpoch();

                         // 模拟车辆运动 - 圆形轨道
                         qreal angle = frameCount * 0.1;
                         qreal x = qCos(angle) * 10.0f;
                         qreal y = qSin(angle) * 10.0f;
                         QVector2D position(x, y);

                         // 计算速度向量
                         qreal vx = -qSin(angle) * 10.0f;
                         qreal vy = qCos(angle) * 10.0f;
                         QVector2D velocity(vx, vy);

                         // 计算旋转角度
                         qreal rotation = qAtan2(vy, vx) * 180.0 / 3.14159;
                         qreal steeringAngle = qSin(angle * 2.0) * 30.0;
                         qreal acceleration = qCos(angle * 3.0) * 2.0;

                         // 更新传感器
                         sensor.updatePosition(position);
                         sensor.updateVelocity(velocity);
                         sensor.updateRotation(rotation);
                         sensor.updateSteeringAngle(steeringAngle);
                         sensor.updateAcceleration(acceleration);
                         sensor.updateBrakeState(frameCount % 10 == 0);
                         sensor.updateAcceleratorState(frameCount % 10 != 0);

                         // 更新碰撞检测器
                         detector.updateVehiclePosition(position);
                         detector.updateVehicleRotation(rotation);
                         detector.checkCollisions();

                         // 更新障碍物位置（模拟移动）
                         qreal obsX = 5.0f + qCos(frameCount * 0.05) * 3.0f;
                         detector.updateCollidableObject("obstacle_1", QVector2D(obsX, 5.0f));

                         // 获取当前传感器读数
                         DrivingData currentData = sensor.getCurrentReading();
                         currentData.timestamp = timestamp;

                         // 每 5 帧输出一次状态
                         if (frameCount % 5 == 0) {
                             qDebug() << "  帧" << frameCount 
                                      << "- 位置:" << QString("(%1, %2)").arg(x, 6, 'f', 2).arg(y, 6, 'f', 2)
                                      << "速度:" << QString("%1").arg(currentData.speed, 5, 'f', 2)
                                      << "转向:" << QString("%1").arg(currentData.steeringAngle, 5, 'f', 1);
                         }

                         // 运行 15 帧后停止
                         if (frameCount >= 15) {
                             updateTimer.stop();
                             sensor.stopSensing();

                             qDebug() << "\n=== 测试统计 ===";
                             qDebug() << "总帧数:" << frameCount;
                             qDebug() << "传感器历史数据:" << sensor.getReadingsHistory().size() << "条";
                             DrivingData lastData = sensor.getCurrentReading();
                             qDebug() << "当前速度:" << QString("%1").arg(lastData.speed, 0, 'f', 2);
                             QList<QString> collisions = detector.getCurrentCollisions();
                             qDebug() << "当前碰撞数:" << collisions.count();
                             qDebug() << "\n=== 测试完成 ===";

                             QCoreApplication::quit();
                         }
                     });

    updateTimer.start(100);
    qDebug() << "运行模拟 (1.5 秒)...";

    return app.exec();
}

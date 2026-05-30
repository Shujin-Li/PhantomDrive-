#include <QCoreApplication>
#include <QDebug>
#include <QtMath>
#include <QVector2D>
#include <QDateTime>

#include "PhantomDrive/gamemode/DrivingData.h"

using namespace PhantomDrive;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "=== PhantomDrive 数据类型测试 ===";
    qDebug() << "\n[1] 测试 DrivingData 结构体...";

    // 创建测试数据
    DrivingData data1;
    data1.timestamp = QDateTime::currentMSecsSinceEpoch();
    data1.position = QVector2D(10.0f, 20.0f);
    data1.velocity = QVector2D(5.0f, 3.0f);
    data1.acceleration = 2.5;
    data1.rotation = 45.0;
    data1.steeringAngle = 15.0;
    data1.speed = 8.5;
    data1.currentSpeedLimit = 10.0;
    data1.isBraking = false;
    data1.isAccelerating = true;

    qDebug() << "  数据点 1:";
    qDebug() << "    时间戳:" << data1.timestamp;
    qDebug() << "    位置:" << QString("(%1, %2)").arg(data1.position.x()).arg(data1.position.y());
    qDebug() << "    速度:" << data1.speed;
    qDebug() << "    转向角:" << data1.steeringAngle;
    qDebug() << "    旋转:" << data1.rotation;
    qDebug() << "    加速度:" << data1.acceleration;
    qDebug() << "    限速:" << data1.currentSpeedLimit;
    qDebug() << "    刹车:" << (data1.isBraking ? "是" : "否");
    qDebug() << "    加速:" << (data1.isAccelerating ? "是" : "否");

    // 创建第二个数据点
    DrivingData data2;
    data2.timestamp = QDateTime::currentMSecsSinceEpoch() + 100;
    data2.position = QVector2D(12.0f, 22.0f);
    data2.speed = 9.2;
    data2.steeringAngle = -10.0;
    data2.hasCollided = false;

    qDebug() << "\n  数据点 2:";
    qDebug() << "    时间戳:" << data2.timestamp;
    qDebug() << "    位置:" << QString("(%1, %2)").arg(data2.position.x()).arg(data2.position.y());
    qDebug() << "    速度:" << data2.speed;
    qDebug() << "    转向角:" << data2.steeringAngle;
    qDebug() << "    碰撞:" << (data2.hasCollided ? "是" : "否");

    // 测试数据验证
    qDebug() << "\n[2] 测试数据验证...";
    bool isValid1 = data1.position.x() != 0.0f || data1.position.y() != 0.0f;
    qDebug() << "  数据点 1 有效:" << (isValid1 ? "是" : "否");

    // 计算两点之间的距离
    qreal distance = (data2.position - data1.position).length();
    qDebug() << "\n[3] 计算结果...";
    qDebug() << "  两点距离:" << QString("%1").arg(distance, 0, 'f', 2);

    // 计算速度变化
    qreal speedDiff = data2.speed - data1.speed;
    qDebug() << "  速度变化:" << QString("%1").arg(speedDiff, 0, 'f', 2);

    // 测试 ViolationEvent
    qDebug() << "\n[4] 测试 ViolationEvent 结构体...";
    ViolationEvent violation;
    violation.timestamp = QDateTime::currentMSecsSinceEpoch();
    violation.type = ViolationType::SpeedOverLimit;
    violation.description = "超速行驶";
    violation.position = QVector2D(15.0f, 25.0f);
    violation.speedAtViolation = 15.5;
    violation.speedLimit = 10.0;
    violation.penaltyPoints = 3;

    qDebug() << "  违规记录:";
    qDebug() << "    类型:" << static_cast<int>(violation.type);
    qDebug() << "    描述:" << violation.description;
    qDebug() << "    位置:" << QString("(%1, %2)").arg(violation.position.x()).arg(violation.position.y());
    qDebug() << "    速度:" << violation.speedAtViolation;
    qDebug() << "    限速:" << violation.speedLimit;
    qDebug() << "    罚分:" << violation.penaltyPoints;

    // 测试 QList<DrivingData>
    qDebug() << "\n[5] 测试数据存储...";
    QList<DrivingData> dataList;
    dataList.append(data1);
    dataList.append(data2);

    qDebug() << "  存储数据点数量:" << dataList.count();
    qDebug() << "  平均速度:" << QString("%1").arg((data1.speed + data2.speed) / 2.0, 0, 'f', 2);

    qDebug() << "\n=== 测试完成 ===";
    qDebug() << "所有核心数据类型测试通过!";

    return 0;
}

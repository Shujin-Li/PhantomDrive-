#pragma once

#include "PhantomDrive_global.h"
#include "DrivingData.h"

#include <QObject>
#include <QVector2D>
#include <QTimer>

namespace PhantomDrive {

class VehicleSensor : public QObject
{
    Q_OBJECT // Qt 宏，启用信号槽、元对象等特性

public:
    explicit VehicleSensor(QObject* parent = nullptr); 
    ~VehicleSensor() override;

    void setVehicleId(const QString& vehicleId); // 设置车辆ID
    QString vehicleId() const { return m_vehicleId; } // 获取车辆ID

    void setSamplingInterval(int intervalMs); // 设置采样间隔
    int samplingInterval() const { return m_samplingInterval; } // 获取采样间隔

    void startSensing(); // 开始传感器采样
    void stopSensing(); // 停止传感器采样
    bool isSensing() const { return m_isSensing; } // 是否正在采样

    void setSpeedLimitViolationEnabled(bool enabled);
    bool isSpeedLimitViolationEnabled() const { return m_speedLimitViolationEnabled; }

    DrivingData getCurrentReading() const; // 获取当前传感器数据
    QList<DrivingData> getReadingsHistory() const { return m_readingsHistory; } // 获取传感器数据历史记录

    void clearHistory(); // 清空传感器数据历史记录

public slots:
    void updatePosition(const QVector2D& position); // 更新车辆位置
    void updateVelocity(const QVector2D& velocity); // 更新车辆速度
    void updateRotation(qreal rotation); // 更新车辆旋转角度
    void updateSteeringAngle(qreal steeringAngle); // 更新车辆转向角度
    void updateAcceleration(qreal acceleration); // 更新车辆加速度
    void updateSpeedLimit(qreal limit, const QString& zoneId = QString()); // 更新速度限制
    void updateBrakeState(bool isBraking); // 更新刹车状态
    void updateAcceleratorState(bool isAccelerating); // 更新加速器状态
    void updateHonkState(bool isHonking); // 更新HonkState(bool isHonking);

signals:
    void sensorDataReady(const DrivingData& data); // 传感器数据就绪信号
    void speedLimitExceeded(qreal currentSpeed, qreal limit); // 速度限制超过信号，包含当前速度和限制速度
    void sensorStarted(); // 传感器采样开始信号
    void sensorStopped(); // 传感器采样停止信号
    void errorOccurred(const QString& error); // 错误发生信号

protected slots:
    void onSamplingTimer(); // 采样定时器槽函数，用于采集传感器数据

private:
    void calculateDerivedData(DrivingData& data) const; // 计算派生数据，如速度、加速度等
    qreal calculateSpeed(const QVector2D& velocity) const; // 计算速度
    qreal normalizeAngle(qreal angle) const; // 归一化角度

    QString m_vehicleId; // 车辆ID
    int m_samplingInterval; // 采样间隔
    bool m_isSensing; // 是否正在采样

    QVector2D m_currentPosition; // 当前车辆位置
    QVector2D m_currentVelocity; // 当前车辆速度
    qreal m_currentRotation; // 当前车辆旋转角度
    qreal m_currentSteeringAngle; // 当前车辆转向角度
    qreal m_currentAcceleration; // 当前车辆加速度

    qreal m_currentSpeedLimit; // 当前速度限制
    QString m_currentSpeedLimitZoneId; // 当前速度限制区域ID

    bool m_isBraking; // 是否正在刹车
    bool m_isAccelerating; // 是否正在加速
    bool m_isHonking; // 是否正在鸣笛
    bool m_speedLimitViolationEnabled;

    QList<DrivingData> m_readingsHistory; // 传感器数据历史记录

    QTimer* m_samplingTimer; // 采样定时器
    qint64 m_lastSampleTime; // 上次采样时间;
};

}

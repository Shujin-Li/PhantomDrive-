#include "VehicleSensor.h"

#include <QDateTime>
#include <QtMath>
#include <QDebug>

namespace PhantomDrive {

VehicleSensor::VehicleSensor(QObject* parent)
    : QObject(parent)
    , m_samplingInterval(100) // 默认采样间隔100ms
    , m_isSensing(false) // 是否正在采集数据
    , m_currentRotation(0.0) // 当前车辆旋转角度
    , m_currentSteeringAngle(0.0) // 当前车辆转向角度
    , m_currentAcceleration(0.0) // 当前加速度
    , m_currentSpeedLimit(0.0) // 当前速度限制
    , m_isBraking(false) // 是否正在刹车
    , m_isAccelerating(false) // 是否正在加速
    , m_isHonking(false) // 是否正在鸣笛
    , m_samplingTimer(new QTimer(this)) // 采样定时器
    , m_lastSampleTime(0) // 上次采样时间
{
    connect(m_samplingTimer, &QTimer::timeout,
            this, &VehicleSensor::onSamplingTimer);// 连接采样定时器超时信号到 onSamplingTimer槽函数

    m_samplingTimer->setSingleShot(false); // 设置为非单次模式，确保定时器在每次采样后继续触发
} // 采样定时器构造函数



VehicleSensor::~VehicleSensor()
{
    stopSensing();
}

void VehicleSensor::setVehicleId(const QString& vehicleId)
{
    m_vehicleId = vehicleId;
}// 设置车辆ID

void VehicleSensor::setSamplingInterval(int intervalMs)
{
    if (intervalMs <= 0) {
        emit errorOccurred("Sampling interval must be positive");
        return;
    }

    m_samplingInterval = intervalMs;

    if (m_isSensing) {
        m_samplingTimer->setInterval(m_samplingInterval);
    }// 如果正在采集数据，更新定时器间隔
}

void VehicleSensor::startSensing()
{
    if (m_isSensing) {
        return;
    }

    m_samplingTimer->setInterval(m_samplingInterval);
    m_samplingTimer->start();
    m_isSensing = true;
    m_lastSampleTime = QDateTime::currentMSecsSinceEpoch();

    emit sensorStarted();
}

void VehicleSensor::stopSensing()
{
    if (!m_isSensing) {
        return;
    }

    m_samplingTimer->stop();
    m_isSensing = false;

    emit sensorStopped();
}

DrivingData VehicleSensor::getCurrentReading() const
{
    DrivingData data;
    data.timestamp = QDateTime::currentMSecsSinceEpoch();
    data.position = m_currentPosition;
    data.velocity = m_currentVelocity;
    data.rotation = m_currentRotation;
    data.steeringAngle = m_currentSteeringAngle;
    data.acceleration = m_currentAcceleration;
    data.speed = calculateSpeed(m_currentVelocity);
    data.isBraking = m_isBraking;
    data.isAccelerating = m_isAccelerating;
    data.isHonking = m_isHonking;
    data.currentSpeedLimit = m_currentSpeedLimit;
    data.isInSpeedLimitZone = !m_currentSpeedLimitZoneId.isEmpty();
    data.currentZoneId = m_currentSpeedLimitZoneId;

    calculateDerivedData(data);

    return data;
}

void VehicleSensor::clearHistory()
{
    m_readingsHistory.clear();
}

void VehicleSensor::updatePosition(const QVector2D& position)
{
    m_currentPosition = position;
}

void VehicleSensor::updateVelocity(const QVector2D& velocity)
{
    m_currentVelocity = velocity;
}

void VehicleSensor::updateRotation(qreal rotation)
{
    m_currentRotation = normalizeAngle(rotation);
}

void VehicleSensor::updateSteeringAngle(qreal steeringAngle)
{
    m_currentSteeringAngle = qBound(-45.0, steeringAngle, 45.0);
}

void VehicleSensor::updateAcceleration(qreal acceleration)
{
    m_currentAcceleration = acceleration;
}

void VehicleSensor::updateSpeedLimit(qreal limit, const QString& zoneId)
{
    m_currentSpeedLimit = limit;
    m_currentSpeedLimitZoneId = zoneId;

    DrivingData currentData = getCurrentReading();
    qreal currentSpeed = calculateSpeed(m_currentVelocity);

    if (limit > 0.0 && currentSpeed > limit) {
        emit speedLimitExceeded(currentSpeed, limit);
    }
}

void VehicleSensor::updateBrakeState(bool isBraking)
{
    m_isBraking = isBraking;
}

void VehicleSensor::updateAcceleratorState(bool isAccelerating)
{
    m_isAccelerating = isAccelerating;
}

void VehicleSensor::updateHonkState(bool isHonking)
{
    m_isHonking = isHonking;
}

void VehicleSensor::onSamplingTimer()
{
    DrivingData data = getCurrentReading();
    m_readingsHistory.append(data);

    emit sensorDataReady(data);
}

void VehicleSensor::calculateDerivedData(DrivingData& data) const
{
    data.speed = calculateSpeed(data.velocity);

    if (data.currentSpeedLimit > 0.0 && data.speed > data.currentSpeedLimit) {
        data.isInSpeedLimitZone = true;
    }
}

qreal VehicleSensor::calculateSpeed(const QVector2D& velocity) const
{
    return velocity.length();
}

qreal VehicleSensor::normalizeAngle(qreal angle) const
{
    while (angle < 0.0) {
        angle += 360.0;
    }
    while (angle >= 360.0) {
        angle -= 360.0;
    }
    return angle;
}

}

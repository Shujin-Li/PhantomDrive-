#include "DrivingDataStorage.h"

#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QDebug>
#include <algorithm>

namespace PhantomDrive {

DrivingDataStorage::DrivingDataStorage(QObject* parent)
    : QObject(parent)
    , m_maxStorageSize(10000)
    , m_isCircularBuffer(true)
    , m_firstDataTime(0)
    , m_lastDataTime(0)
{
}

DrivingDataStorage::~DrivingDataStorage()
{
}

void DrivingDataStorage::setMaxStorageSize(int maxSize)
{
    if (maxSize <= 0) {
        emit errorOccurred("Max storage size must be positive");
        return;
    }

    m_maxStorageSize = maxSize;

    if (m_dataBuffer.size() > m_maxStorageSize) {
        enforceMaxSize();
    }
}

void DrivingDataStorage::clear()
{
    m_dataBuffer.clear();
    m_violations.clear();
    m_firstDataTime = 0;
    m_lastDataTime = 0;

    emit bufferCleared();
}

void DrivingDataStorage::addData(const DrivingData& data)
{
    if (!validateData(data)) {
        return;
    }

    if (m_dataBuffer.isEmpty()) {
        m_firstDataTime = data.timestamp;
    }

    m_lastDataTime = data.timestamp;
    m_dataBuffer.append(data);

    emit dataAdded(data);

    if (m_dataBuffer.size() >= m_maxStorageSize) {
        if (m_isCircularBuffer) {
            m_dataBuffer.removeFirst();
        }
        emit bufferFull();
    }
}

void DrivingDataStorage::addDataBatch(const QList<DrivingData>& dataBatch)
{
    if (dataBatch.isEmpty()) {
        return;
    }

    for (const auto& data : dataBatch) {
        addData(data);
    }

    emit dataBatchAdded(dataBatch.size());
}

QList<DrivingData> DrivingDataStorage::getData(qint64 startTime, qint64 endTime) const
{
    QList<DrivingData> result;

    for (const auto& data : m_dataBuffer) {
        if (data.timestamp >= startTime && data.timestamp <= endTime) {
            result.append(data);
        }
    }

    return result;
}

QList<DrivingData> DrivingDataStorage::getData(int startIndex, int endIndex) const
{
    if (startIndex < 0 || endIndex >= m_dataBuffer.size() || startIndex > endIndex) {
        return QList<DrivingData>();
    }

    return m_dataBuffer.mid(startIndex, endIndex - startIndex + 1);
}

void DrivingDataStorage::addViolation(const ViolationEvent& violation)
{
    m_violations.append(violation);
    emit violationAdded(violation);
}

void DrivingDataStorage::clearViolations()
{
    m_violations.clear();
}

DrivingData DrivingDataStorage::getLatestData() const
{
    if (m_dataBuffer.isEmpty()) {
        return DrivingData();
    }
    return m_dataBuffer.last();
}

DrivingData DrivingDataStorage::getDataAt(qint64 timestamp) const
{
    if (m_dataBuffer.isEmpty()) {
        return DrivingData();
    }

    auto it = std::lower_bound(m_dataBuffer.begin(), m_dataBuffer.end(), timestamp,
                               [](const DrivingData& data, qint64 ts) {
                                   return data.timestamp < ts;
                               });

    if (it != m_dataBuffer.end()) {
        return *it;
    }

    return m_dataBuffer.last();
}

QJsonArray DrivingDataStorage::toJsonArray() const
{
    QJsonArray array;

    for (const auto& data : m_dataBuffer) {
        QJsonObject obj;
        obj["timestamp"] = QString::number(data.timestamp);
        obj["positionX"] = data.position.x();
        obj["positionY"] = data.position.y();
        obj["velocityX"] = data.velocity.x();
        obj["velocityY"] = data.velocity.y();
        obj["rotation"] = data.rotation;
        obj["speed"] = data.speed;
        obj["steeringAngle"] = data.steeringAngle;
        obj["acceleration"] = data.acceleration;
        obj["isBraking"] = data.isBraking;
        obj["isAccelerating"] = data.isAccelerating;
        obj["isHonking"] = data.isHonking;
        obj["currentSpeedLimit"] = data.currentSpeedLimit;
        obj["isInSpeedLimitZone"] = data.isInSpeedLimitZone;
        obj["currentZoneId"] = data.currentZoneId;
        obj["hasCollided"] = data.hasCollided;
        obj["collisionObjectId"] = data.collisionObjectId;
        obj["lapTime"] = data.lapTime;
        obj["currentLap"] = data.currentLap;
        obj["checkpointsPassed"] = data.checkpointsPassed;

        array.append(obj);
    }

    return array;
}

QString DrivingDataStorage::toJsonString() const
{
    QJsonDocument doc(toJsonArray());
    return QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
}

bool DrivingDataStorage::exportToFile(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file for writing:" << filePath;
        return false;
    }

    QJsonObject root;
    root["version"] = "1.0";
    root["exportTime"] = QDateTime::currentMSecsSinceEpoch();
    root["vehicleId"] = "";
    root["dataPoints"] = toJsonArray();

    QJsonArray violationsArray;
    for (const auto& violation : m_violations) {
        QJsonObject vObj;
        vObj["timestamp"] = QString::number(violation.timestamp);
        vObj["type"] = static_cast<int>(violation.type);
        vObj["description"] = violation.description;
        vObj["positionX"] = violation.position.x();
        vObj["positionY"] = violation.position.y();
        vObj["speedAtViolation"] = violation.speedAtViolation;
        vObj["speedLimit"] = violation.speedLimit;
        vObj["penaltyPoints"] = violation.penaltyPoints;
        violationsArray.append(vObj);
    }
    root["violations"] = violationsArray;

    QJsonDocument doc(root);
    QTextStream out(&file);
    out << doc.toJson(QJsonDocument::Indented);
    file.close();

    return true;
}

bool DrivingDataStorage::importFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit errorOccurred(QString("Cannot open file for reading: %1").arg(filePath));
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit errorOccurred(QString("JSON parse error: %1").arg(parseError.errorString()));
        return false;
    }

    if (!doc.isObject()) {
        emit errorOccurred("Invalid file format: root is not an object");
        return false;
    }

    QJsonObject root = doc.object();
    QJsonArray dataArray = root["dataPoints"].toArray();

    clear();

    for (const QJsonValue& val : dataArray) {
        QJsonObject obj = val.toObject();

        DrivingData data;
        data.timestamp = obj["timestamp"].toString().toLongLong();
        data.position = QVector2D(obj["positionX"].toDouble(), obj["positionY"].toDouble());
        data.velocity = QVector2D(obj["velocityX"].toDouble(), obj["velocityY"].toDouble());
        data.rotation = obj["rotation"].toDouble();
        data.speed = obj["speed"].toDouble();
        data.steeringAngle = obj["steeringAngle"].toDouble();
        data.acceleration = obj["acceleration"].toDouble();
        data.isBraking = obj["isBraking"].toBool();
        data.isAccelerating = obj["isAccelerating"].toBool();
        data.isHonking = obj["isHonking"].toBool();
        data.currentSpeedLimit = obj["currentSpeedLimit"].toDouble();
        data.isInSpeedLimitZone = obj["isInSpeedLimitZone"].toBool();
        data.currentZoneId = obj["currentZoneId"].toString();
        data.hasCollided = obj["hasCollided"].toBool();
        data.collisionObjectId = obj["collisionObjectId"].toString();
        data.lapTime = obj["lapTime"].toDouble();
        data.currentLap = obj["currentLap"].toInt();
        data.checkpointsPassed = obj["checkpointsPassed"].toInt();

        m_dataBuffer.append(data);
    }

    QJsonArray violationsArray = root["violations"].toArray();
    for (const QJsonValue& val : violationsArray) {
        QJsonObject obj = val.toObject();

        ViolationEvent violation;
        violation.timestamp = obj["timestamp"].toString().toLongLong();
        violation.type = static_cast<ViolationType>(obj["type"].toInt());
        violation.description = obj["description"].toString();
        violation.position = QVector2D(obj["positionX"].toDouble(), obj["positionY"].toDouble());
        violation.speedAtViolation = obj["speedAtViolation"].toDouble();
        violation.speedLimit = obj["speedLimit"].toDouble();
        violation.penaltyPoints = obj["penaltyPoints"].toInt();

        m_violations.append(violation);
    }

    if (!m_dataBuffer.isEmpty()) {
        m_firstDataTime = m_dataBuffer.first().timestamp;
        m_lastDataTime = m_dataBuffer.last().timestamp;
    }

    emit importCompleted(filePath);
    return true;
}

QList<DrivingData> DrivingDataStorage::filterBySpeed(qreal minSpeed, qreal maxSpeed) const
{
    QList<DrivingData> result;

    for (const auto& data : m_dataBuffer) {
        if (data.speed >= minSpeed && data.speed <= maxSpeed) {
            result.append(data);
        }
    }

    return result;
}

QList<DrivingData> DrivingDataStorage::filterByCollision(bool hasCollision) const
{
    QList<DrivingData> result;

    for (const auto& data : m_dataBuffer) {
        if (data.hasCollided == hasCollision) {
            result.append(data);
        }
    }

    return result;
}

QList<DrivingData> DrivingDataStorage::filterByTimeRange(qint64 startMs, qint64 endMs) const
{
    return getData(startMs, endMs);
}

qint64 DrivingDataStorage::getRecordingDuration() const
{
    if (m_dataBuffer.isEmpty()) {
        return 0;
    }
    return m_lastDataTime - m_firstDataTime;
}

qreal DrivingDataStorage::getAverageSpeed() const
{
    if (m_dataBuffer.isEmpty()) {
        return 0.0;
    }

    qreal totalSpeed = 0.0;
    for (const auto& data : m_dataBuffer) {
        totalSpeed += data.speed;
    }

    return totalSpeed / m_dataBuffer.size();
}

qreal DrivingDataStorage::getMaxSpeed() const
{
    qreal maxSpeed = 0.0;

    for (const auto& data : m_dataBuffer) {
        if (data.speed > maxSpeed) {
            maxSpeed = data.speed;
        }
    }

    return maxSpeed;
}

int DrivingDataStorage::getTotalCollisions() const
{
    int count = 0;

    for (const auto& data : m_dataBuffer) {
        if (data.hasCollided) {
            count++;
        }
    }

    return count;
}

void DrivingDataStorage::enforceMaxSize()
{
    while (m_dataBuffer.size() > m_maxStorageSize) {
        m_dataBuffer.removeFirst();
    }

    if (!m_dataBuffer.isEmpty()) {
        m_firstDataTime = m_dataBuffer.first().timestamp;
    }
}

bool DrivingDataStorage::validateData(const DrivingData& data) const
{
    if (data.timestamp <= 0) {
        return false;
    }

    return true;
}

}

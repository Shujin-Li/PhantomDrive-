#include "PedestrianCrossingObject.h"

#include <QDebug>
#include <QDateTime>
#include <QtMath>

namespace PhantomDrive {

PedestrianCrossingObject::PedestrianCrossingObject(const QString& id, QObject* parent)
    : TrafficObject(id, TrafficObjectType::PedestrianCrossing, parent)
    , m_bounds(0.0, 0.0, 20.0, 5.0)
    , m_isActive(true)
    , m_violationPenalty(15)
    , m_violationCount(0)
    , m_lastPedestrianTime(0)
    , m_lastUpdateTime(0)
    , m_pedestrianSpawnInterval(5000)
    , m_pedestrianSpeed(1.0)
    , m_maxPedestrians(5)
    , m_sessionStartTime(0)
{
}

PedestrianCrossingObject::~PedestrianCrossingObject()
{
}

void PedestrianCrossingObject::setBounds(const QRectF& bounds)
{
    m_bounds = bounds;
}

void PedestrianCrossingObject::setActive(bool active)
{
    m_isActive = active;
}

void PedestrianCrossingObject::addPedestrian(const QString& pedestrianId)
{
    if (m_pedestrians.size() >= m_maxPedestrians) {
        return;
    }

    Pedestrian ped;
    ped.id = pedestrianId;
    ped.position = QVector2D(m_bounds.left(), m_bounds.center().y());
    ped.progress = 0.0;
    ped.isCrossing = false;
    ped.speed = m_pedestrianSpeed;

    m_pedestrians.append(ped);
    emit pedestrianSpawned(getId(), pedestrianId);
}

void PedestrianCrossingObject::removePedestrian(const QString& pedestrianId)
{
    for (int i = 0; i < m_pedestrians.size(); ++i) {
        if (m_pedestrians[i].id == pedestrianId) {
            emit pedestrianRemoved(getId(), pedestrianId);
            m_pedestrians.removeAt(i);
            return;
        }
    }
}

void PedestrianCrossingObject::clearPedestrians()
{
    m_pedestrians.clear();
}

bool PedestrianCrossingObject::isVehicleInZone(const QVector2D& vehiclePosition) const
{
    return m_bounds.contains(vehiclePosition.toPointF());
}

bool PedestrianCrossingObject::checkPedestrianViolation(const QVector2D& vehiclePosition) const
{
    if (!m_isActive) {
        return false;
    }

    if (!m_bounds.contains(vehiclePosition.toPointF())) {
        return false;
    }

    for (const auto& ped : m_pedestrians) {
        if (ped.isCrossing) {
            return true;
        }
    }

    return false;
}

bool PedestrianCrossingObject::isPedestrianInDangerZone() const
{
    for (const auto& ped : m_pedestrians) {
        if (ped.isCrossing && ped.progress > 0.2 && ped.progress < 0.8) {
            return true;
        }
    }
    return false;
}

void PedestrianCrossingObject::incrementViolationCount()
{
    m_violationCount++;
    emit pedestrianViolation(getId());
}

void PedestrianCrossingObject::spawnPedestrian()
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    if (currentTime - m_lastPedestrianTime < static_cast<qint64>(m_pedestrianSpawnInterval)) {
        return;
    }

    if (m_pedestrians.size() >= m_maxPedestrians) {
        return;
    }

    QString pedestrianId = QString("ped_%1_%2").arg(getId()).arg(currentTime);
    addPedestrian(pedestrianId);

    if (m_pedestrians.size() > 0) {
        m_pedestrians.last().isCrossing = true;
    }

    m_lastPedestrianTime = currentTime;
}

void PedestrianCrossingObject::update(qint64 elapsedMs)
{
    if (!m_isActive) {
        return;
    }

    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    if (m_lastUpdateTime == 0) {
        m_lastUpdateTime = currentTime;
        m_sessionStartTime = currentTime;
        return;
    }

    qint64 deltaTime = currentTime - m_lastUpdateTime;
    m_lastUpdateTime = currentTime;

    updatePedestrians(deltaTime);
}

void PedestrianCrossingObject::onVehicleApproaching(const QVector2D& vehiclePosition)
{
    Q_UNUSED(vehiclePosition)
}

void PedestrianCrossingObject::onVehicleInZone(const QVector2D& vehiclePosition)
{
    if (isVehicleInZone(vehiclePosition)) {
        emit zoneEntered(getId());

        if (isPedestrianInDangerZone()) {
            incrementViolationCount();
        }
    }
}

void PedestrianCrossingObject::reset()
{
    m_pedestrians.clear();
    m_violationCount = 0;
    m_lastPedestrianTime = 0;
    m_lastUpdateTime = 0;
    m_isActive = true;
}

void PedestrianCrossingObject::updatePedestrians(qint64 elapsedMs)
{
    qreal deltaSeconds = elapsedMs / 1000.0;

    for (int i = m_pedestrians.size() - 1; i >= 0; --i) {
        Pedestrian& ped = m_pedestrians[i];

        if (ped.isCrossing) {
            ped.progress += (ped.speed * deltaSeconds) / m_bounds.width();

            ped.position.setX(m_bounds.left() + ped.progress * m_bounds.width());

            if (ped.progress >= 1.0) {
                emit pedestrianCrossed(getId(), ped.id);
                m_pedestrians.removeAt(i);
            }
        }
    }
}

PedestrianCrossingObject::Pedestrian* PedestrianCrossingObject::findPedestrian(const QString& pedestrianId)
{
    for (auto& ped : m_pedestrians) {
        if (ped.id == pedestrianId) {
            return &ped;
        }
    }
    return nullptr;
}

}

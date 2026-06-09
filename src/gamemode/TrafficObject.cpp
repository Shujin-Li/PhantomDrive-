#include "TrafficObject.h"

#include <QJsonObject>

namespace PhantomDrive {

TrafficObject::TrafficObject(const QString& id, TrafficObjectType type, QObject* parent)
    : QObject(parent)
    , m_id(id)
    , m_type(type)
    , m_state(TrafficObjectState::Active)
    , m_position(0.0, 0.0)
    , m_bounds(0.0, 0.0, 10.0, 10.0)
    , m_enabled(true)
    , m_lastUpdateTime(0)
{
}

TrafficObject::~TrafficObject()
{
}

void TrafficObject::setPosition(const QVector2D& position)
{
    if (m_position != position) {
        m_position = position;
        emit positionChanged(m_position);
    }
}

void TrafficObject::setBounds(const QRectF& bounds)
{
    if (m_bounds != bounds) {
        m_bounds = bounds;
        emit boundsChanged(m_bounds);
    }
}

void TrafficObject::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        emit enabledChanged(m_enabled);
    }
}

bool TrafficObject::checkViolation(const QVector2D& vehiclePosition, qreal vehicleSpeed) const
{
    Q_UNUSED(vehiclePosition)
    Q_UNUSED(vehicleSpeed)
    return false;
}

void TrafficObject::update(qint64 elapsedMs)
{
    Q_UNUSED(elapsedMs)
}

void TrafficObject::reset()
{
}

void TrafficObject::resetViolationCount()
{
}

QJsonObject TrafficObject::toJson() const
{
    QJsonObject json;
    json["id"] = m_id;
    json["type"] = static_cast<int>(m_type);
    json["state"] = static_cast<int>(m_state);
    json["positionX"] = m_position.x();
    json["positionY"] = m_position.y();
    json["boundsX"] = m_bounds.x();
    json["boundsY"] = m_bounds.y();
    json["boundsWidth"] = m_bounds.width();
    json["boundsHeight"] = m_bounds.height();
    json["enabled"] = m_enabled;
    return json;
}

void TrafficObject::fromJson(const QJsonObject& json)
{
    m_id = json["id"].toString();
    m_type = static_cast<TrafficObjectType>(json["type"].toInt());
    m_state = static_cast<TrafficObjectState>(json["state"].toInt());
    m_position.setX(json["positionX"].toDouble());
    m_position.setY(json["positionY"].toDouble());
    m_bounds.setRect(
        json["boundsX"].toDouble(),
        json["boundsY"].toDouble(),
        json["boundsWidth"].toDouble(),
        json["boundsHeight"].toDouble()
    );
    m_enabled = json["enabled"].toBool();
}

QString TrafficObject::getDisplayText() const
{
    switch (m_type) {
        case TrafficObjectType::TrafficLight:
            return QStringLiteral("Traffic Light");
        case TrafficObjectType::SpeedLimitSign:
            return QStringLiteral("Speed Limit");
        case TrafficObjectType::PedestrianCrossing:
            return QStringLiteral("Pedestrian Crossing");
        case TrafficObjectType::StopSign:
            return QStringLiteral("Stop Sign");
        default:
            return QStringLiteral("Unknown");
    }
}

QString TrafficObject::getStateDescription() const
{
    switch (m_state) {
        case TrafficObjectState::Inactive:
            return QStringLiteral("Inactive");
        case TrafficObjectState::Active:
            return QStringLiteral("Active");
        case TrafficObjectState::Warning:
            return QStringLiteral("Warning");
        case TrafficObjectState::Disabled:
            return QStringLiteral("Disabled");
        default:
            return QStringLiteral("Unknown");
    }
}

void TrafficObject::setState(TrafficObjectState newState)
{
    if (m_state != newState) {
        TrafficObjectState oldState = m_state;
        m_state = newState;
        emit stateChanged(oldState, m_state);
    }
}

}

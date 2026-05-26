#include "TrafficLightObject.h"

#include <QDebug>
#include <QDateTime>

namespace PhantomDrive {

TrafficLightObject::TrafficLightObject(const QString& id, QObject* parent)
    : TrafficObject(id, TrafficObjectType::TrafficLight, parent)
    , m_currentColor(LightColor::Red)
    , m_previousColor(LightColor::Red)
    , m_redDurationMs(5000)
    , m_yellowDurationMs(2000)
    , m_greenDurationMs(5000)
    , m_lastChangeTime(0)
    , m_isRunning(false)
    , m_wasRedWhenApproaching(false)
    , m_violationPenalty(10)
    , m_redLightViolationCount(0)
    , m_cycleCount(0)
    , m_sessionStartTime(0)
{
}

TrafficLightObject::~TrafficLightObject()
{
}

void TrafficLightObject::setCurrentColor(LightColor color)
{
    if (m_currentColor != color) {
        m_previousColor = m_currentColor;
        m_currentColor = color;
        m_lastChangeTime = QDateTime::currentMSecsSinceEpoch();
        emit colorChanged(m_previousColor, m_currentColor);
    }
}

void TrafficLightObject::setRedDurationMs(int durationMs)
{
    if (durationMs >= 0) {
        m_redDurationMs = durationMs;
    }
}

void TrafficLightObject::setYellowDurationMs(int durationMs)
{
    if (durationMs >= 0) {
        m_yellowDurationMs = durationMs;
    }
}

void TrafficLightObject::setGreenDurationMs(int durationMs)
{
    if (durationMs >= 0) {
        m_greenDurationMs = durationMs;
    }
}

int TrafficLightObject::getRemainingTimeInCurrentState() const
{
    qint64 elapsed = getTimeSinceLastChange();
    int duration = 0;

    switch (m_currentColor) {
        case LightColor::Red:
            duration = m_redDurationMs;
            break;
        case LightColor::Yellow:
            duration = m_yellowDurationMs;
            break;
        case LightColor::Green:
            duration = m_greenDurationMs;
            break;
        default:
            break;
    }

    return qMax(0, duration - static_cast<int>(elapsed));
}

void TrafficLightObject::start()
{
    if (!m_isRunning) {
        m_isRunning = true;
        m_sessionStartTime = QDateTime::currentMSecsSinceEpoch();
        m_lastChangeTime = m_sessionStartTime;
        m_cycleCount = 0;
        qDebug() << "TrafficLightObject:" << getId() << "started";
    }
}

void TrafficLightObject::stop()
{
    if (m_isRunning) {
        m_isRunning = false;
        qDebug() << "TrafficLightObject:" << getId() << "stopped";
    }
}

void TrafficLightObject::update(qint64 elapsedMs)
{
    Q_UNUSED(elapsedMs)

    if (!m_isRunning) {
        return;
    }

    qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - m_lastChangeTime;
    int duration = 0;

    switch (m_currentColor) {
        case LightColor::Red:
            duration = m_redDurationMs;
            break;
        case LightColor::Yellow:
            duration = m_yellowDurationMs;
            break;
        case LightColor::Green:
            duration = m_greenDurationMs;
            break;
        default:
            break;
    }

    if (elapsed >= duration) {
        advanceToNextColor();
    }
}

bool TrafficLightObject::checkRedLightViolation(const QVector2D& vehiclePosition) const
{
    if (!isRed()) {
        return false;
    }

    if (getBounds().contains(vehiclePosition.toPointF())) {
        if (m_wasRedWhenApproaching) {
            return true;
        }
    }

    return false;
}

int TrafficLightObject::getTotalCycleDurationMs() const
{
    return m_redDurationMs + m_yellowDurationMs + m_greenDurationMs;
}

void TrafficLightObject::onVehicleApproaching(const QVector2D& vehiclePosition)
{
    Q_UNUSED(vehiclePosition)

    if (isRed()) {
        m_wasRedWhenApproaching = true;
    }
}

void TrafficLightObject::reset()
{
    m_currentColor = LightColor::Red;
    m_previousColor = LightColor::Red;
    m_lastChangeTime = QDateTime::currentMSecsSinceEpoch();
    m_wasRedWhenApproaching = false;
    m_redLightViolationCount = 0;
    m_cycleCount = 0;
    m_isRunning = false;
}

void TrafficLightObject::advanceToNextColor()
{
    LightColor nextColor;

    switch (m_currentColor) {
        case LightColor::Red:
            nextColor = LightColor::Green;
            break;
        case LightColor::Yellow:
            nextColor = LightColor::Red;
            break;
        case LightColor::Green:
            nextColor = m_yellowDurationMs > 0 ? LightColor::Yellow : LightColor::Red;
            break;
        default:
            nextColor = LightColor::Red;
            break;
    }

    setCurrentColor(nextColor);

    if (nextColor == LightColor::Red) {
        m_cycleCount++;
        if (m_wasRedWhenApproaching) {
            m_redLightViolationCount++;
            emit redLightViolation(getId());
        }
        m_wasRedWhenApproaching = false;
        emit cycleCompleted(m_cycleCount);
    }
}

qint64 TrafficLightObject::getTimeSinceLastChange() const
{
    return QDateTime::currentMSecsSinceEpoch() - m_lastChangeTime;
}

}

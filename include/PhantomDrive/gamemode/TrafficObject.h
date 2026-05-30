#pragma once

#include <QObject>
#include <QString>
#include <QVector2D>
#include <QRectF>
#include <QJsonObject>
#include <QJsonArray>

namespace PhantomDrive {

enum class TrafficObjectType {
    Unknown = -1,
    TrafficLight = 0,
    SpeedLimitSign = 1,
    PedestrianCrossing = 2,
    StopSign = 3,
    Count
};

enum class TrafficObjectState {
    Inactive = 0,
    Active = 1,
    Warning = 2,
    Disabled = 3
};

class TrafficObject : public QObject
{
    Q_OBJECT

public:
    explicit TrafficObject(const QString& id, TrafficObjectType type, QObject* parent = nullptr);
    ~TrafficObject() override;

    QString getId() const { return m_id; }
    TrafficObjectType getType() const { return m_type; }
    TrafficObjectState getState() const { return m_state; }

    QVector2D getPosition() const { return m_position; }
    void setPosition(const QVector2D& position);

    QRectF getBounds() const { return m_bounds; }
    void setBounds(const QRectF& bounds);

    bool isActive() const { return m_state == TrafficObjectState::Active; }
    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enabled);

    virtual bool checkViolation(const QVector2D& vehiclePosition, qreal vehicleSpeed = 0.0) const;
    virtual void update(qint64 elapsedMs);
    virtual void reset();
    virtual void resetViolationCount();

    virtual QJsonObject toJson() const;
    virtual void fromJson(const QJsonObject& json);

    virtual QString getDisplayText() const;
    virtual QString getStateDescription() const;

signals:
    void stateChanged(TrafficObjectState oldState, TrafficObjectState newState);
    void positionChanged(const QVector2D& newPosition);
    void boundsChanged(const QRectF& newBounds);
    void enabledChanged(bool enabled);
    void violationTriggered(const QString& objectId, const QVector2D& position);
    void objectUpdated(const QString& objectId);

protected:
    void setState(TrafficObjectState newState);

    QString m_id;
    TrafficObjectType m_type;
    TrafficObjectState m_state;
    QVector2D m_position;
    QRectF m_bounds;
    bool m_enabled;

    qint64 m_lastUpdateTime;
};

}

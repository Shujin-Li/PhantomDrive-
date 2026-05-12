#pragma once

#include "PhantomDrive_global.h"

#include <QObject>
#include <QString>
#include <QVector2D>
#include <QRectF>
#include <QVariantMap>

namespace PhantomDrive {

class PHANTOMDRIVE_EXPORT Checkpoint : public QObject
{
    Q_OBJECT

public:
    explicit Checkpoint(QObject* parent = nullptr);
    Checkpoint(int id, const QVector2D& position, QObject* parent = nullptr);
    ~Checkpoint() override;

    int getId() const { return m_id; }
    void setId(int id) { m_id = id; }

    QVector2D getPosition() const { return m_position; }
    void setPosition(const QVector2D& pos) { m_position = pos; }

    qreal getWidth() const { return m_width; }
    void setWidth(qreal width) { m_width = width; }

    qreal getHeight() const { return m_height; }
    void setHeight(qreal height) { m_height = height; }

    QRectF getBounds() const;

    bool isActive() const { return m_isActive; }
    void setActive(bool active) { m_isActive = active; }

    bool isMandatory() const { return m_isMandatory; }
    void setMandatory(bool mandatory) { m_isMandatory = mandatory; }

    int getRequiredLap() const { return m_requiredLap; }
    void setRequiredLap(int lap) { m_requiredLap = lap; }

    int getIndexInRoute() const { return m_indexInRoute; }
    void setIndexInRoute(int index) { m_indexInRoute = index; }

    bool containsPoint(const QVector2D& point) const;

    QVariantMap toVariantMap() const;
    void fromVariantMap(const QVariantMap& data);

signals:
    void checkpointActivated(int checkpointId);
    void checkpointDeactivated(int checkpointId);

private:
    int m_id;
    QVector2D m_position;
    qreal m_width;
    qreal m_height;
    bool m_isActive;
    bool m_isMandatory;
    int m_requiredLap;
    int m_indexInRoute;
};

}

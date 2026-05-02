#include "Checkpoint.h"

namespace PhantomDrive {

Checkpoint::Checkpoint(QObject* parent)
    : QObject(parent)
    , m_id(0)
    , m_width(64.0)
    , m_height(10.0)
    , m_isActive(true)
    , m_isMandatory(true)
    , m_requiredLap(1)
    , m_indexInRoute(0)
{
}

Checkpoint::Checkpoint(int id, const QVector2D& position, QObject* parent)
    : QObject(parent)
    , m_id(id)
    , m_position(position)
    , m_width(64.0)
    , m_height(10.0)
    , m_isActive(true)
    , m_isMandatory(true)
    , m_requiredLap(1)
    , m_indexInRoute(0)
{
}

Checkpoint::~Checkpoint()
{
}

QRectF Checkpoint::getBounds() const
{
    qreal halfWidth = m_width / 2.0;
    qreal halfHeight = m_height / 2.0;
    return QRectF(
        m_position.x() - halfWidth,
        m_position.y() - halfHeight,
        m_width,
        m_height
    );
}

bool Checkpoint::containsPoint(const QVector2D& point) const
{
    return getBounds().contains(point.toPointF());
}

QVariantMap Checkpoint::toVariantMap() const
{
    QVariantMap map;
    map["id"] = m_id;
    map["positionX"] = m_position.x();
    map["positionY"] = m_position.y();
    map["width"] = m_width;
    map["height"] = m_height;
    map["active"] = m_isActive;
    map["mandatory"] = m_isMandatory;
    map["requiredLap"] = m_requiredLap;
    map["indexInRoute"] = m_indexInRoute;
    return map;
}

void Checkpoint::fromVariantMap(const QVariantMap& data)
{
    m_id = data.value("id").toInt();
    m_position.setX(data.value("positionX").toReal());
    m_position.setY(data.value("positionY").toReal());
    m_width = data.value("width").toDouble();
    m_height = data.value("height").toDouble();
    m_isActive = data.value("active").toBool();
    m_isMandatory = data.value("mandatory").toBool();
    m_requiredLap = data.value("requiredLap").toInt();
    m_indexInRoute = data.value("indexInRoute").toInt();
}

}

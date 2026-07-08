#pragma once

#include "PhantomDrive_global.h"

#include <QRectF>
#include <QSizeF>
#include <QString>
#include <QVector2D>

namespace PhantomDrive {

struct BlockerVehicle {
    QString id;
    QVector2D position;
    QVector2D pointA;
    QVector2D pointB;
    QSizeF size = QSizeF(22.0, 38.0);
    QSizeF collisionSize;
    qreal rotation = 0.0;
    qreal speed = 0.0;
    qreal progress = 0.0;
    qreal duration = 3.0;
    bool forward = true;
    int stageRequired = 0;
    int turnaroundCount = 0;

    QRectF bounds() const
    {
        const QSizeF effectiveSize = (collisionSize.width() > 0.0 && collisionSize.height() > 0.0)
            ? collisionSize
            : size;
        return QRectF(position.x() - effectiveSize.width() * 0.5,
                      position.y() - effectiveSize.height() * 0.5,
                      effectiveSize.width(),
                      effectiveSize.height());
    }
};

} // namespace PhantomDrive

#pragma once

#include "PhantomDrive_global.h"

#include <QRectF>
#include <QSizeF>
#include <QString>
#include <QVector2D>

namespace PhantomDrive {

enum class ChallengeObstacleType {
    ConstructionWall,
    ConeBarrier,
    Tree,
    Rock,
    ConstructionBarrel
};

struct PHANTOMDRIVE_EXPORT ChallengeObstacle
{
    QString id;
    ChallengeObstacleType type = ChallengeObstacleType::ConstructionWall;
    QVector2D position;
    QSizeF size = QSizeF(40.0, 40.0);
    QSizeF collisionSize;
    qreal rotation = 0.0;
    bool circular = false;
    qreal radius = 20.0;
    qreal collisionRadius = 0.0;
    int stageRequired = 0;

    QRectF bounds() const
    {
        if (circular) {
            const qreal effectiveRadius = collisionRadius > 0.0 ? collisionRadius : radius;
            return QRectF(position.x() - effectiveRadius,
                          position.y() - effectiveRadius,
                          effectiveRadius * 2.0,
                          effectiveRadius * 2.0);
        }
        const QSizeF effectiveSize = (collisionSize.width() > 0.0 && collisionSize.height() > 0.0)
            ? collisionSize
            : size;
        return QRectF(position.x() - effectiveSize.width() * 0.5f,
                      position.y() - effectiveSize.height() * 0.5f,
                      effectiveSize.width(),
                      effectiveSize.height());
    }
};

} // namespace PhantomDrive

#pragma once

#include "PhantomDrive_global.h"

#include <QObject>
#include <QVector2D>
#include <QString>
#include <QList>
#include <QColor>
#include <QVariantMap>
#include <QRectF>

namespace PhantomDrive {

enum class RenderObjectType {
    PlayerCar,
    AICar,
    PowerupBox,
    TrafficLight,
    SpeedLimitSign,
    PedestrianCrossing,
    TrackTile
};

struct GameRenderObject {
    RenderObjectType type;
    QVector2D position;
    qreal rotation;
    QSizeF size;
    QString label;
    QColor color;
    QVariantMap extraData;

    GameRenderObject()
        : type(RenderObjectType::TrackTile)
        , position(0, 0)
        , rotation(0)
        , size(32, 32)
        , color(Qt::white)
    {}
};

struct RenderState {
    QList<GameRenderObject> trackTiles;
    QList<GameRenderObject> playerCars;
    QList<GameRenderObject> aiCars;
    QList<GameRenderObject> powerupBoxes;
    QList<GameRenderObject> trafficLights;
    QList<GameRenderObject> speedLimitSigns;
    QList<GameRenderObject> pedestrianCrossings;

    QVector2D cameraPosition;
    qreal cameraZoom;
    QRectF viewportBounds;

    RenderState()
        : cameraPosition(0, 0)
        , cameraZoom(1.0)
    {}

    void clear() {
        trackTiles.clear();
        playerCars.clear();
        aiCars.clear();
        powerupBoxes.clear();
        trafficLights.clear();
        speedLimitSigns.clear();
        pedestrianCrossings.clear();
    }
};

}

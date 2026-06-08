#pragma once

#include "PhantomDrive_global.h"

#include <QString>
#include <QList>
#include <QStringList>
#include <QVector2D>

class QObject;

namespace PhantomDrive {

class TrackData;

struct BuiltInTrackInfo {
    QString id;
    QString name;
    QString description;
    QString difficulty;
};

class PHANTOMDRIVE_EXPORT BuiltInTrackFactory
{
public:
    static QList<BuiltInTrackInfo> tracks();
    static QStringList trackIds();
    static QString trackName(const QString& id);
    static bool isBuiltInTrackId(const QString& id);
    static TrackData* createTrack(const QString& id, QObject* parent = nullptr);
    static QList<QVector2D> getAIDrivingWaypoints(const QString& id);
};

} // namespace PhantomDrive

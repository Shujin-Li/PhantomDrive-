#pragma once

#include "PhantomDrive_global.h"
#include "TrackData.h"

#include <QObject>
#include <QString>
#include <QJsonObject>

namespace PhantomDrive {

class PHANTOMDRIVE_EXPORT TrackIO : public QObject
{
    Q_OBJECT

public:
    explicit TrackIO(QObject* parent = nullptr);
    ~TrackIO() override;

    bool saveTrack(TrackData* track, const QString& filePath) const;
    TrackData* loadTrack(const QString& filePath) const;

    bool saveTrackToJson(TrackData* track, QJsonObject& jsonObject) const;
    bool loadTrackFromJson(TrackData* track, const QJsonObject& jsonObject) const;

    QString getLastError() const { return m_lastError; }
    void clearError() { m_lastError.clear(); }

    static QStringList getSupportedExtensions();
    static bool isValidTrackFile(const QString& filePath);

signals:
    void saveStarted(const QString& filePath);
    void saveCompleted(const QString& filePath);
    void loadStarted(const QString& filePath);
    void loadCompleted(const QString& filePath);
    void errorOccurred(const QString& error);

protected:
    void setError(const QString& error);

private:
    QString m_lastError;
    static const QStringList SUPPORTED_EXTENSIONS;
};

}

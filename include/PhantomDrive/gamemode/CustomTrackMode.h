#pragma once

#include "GameMode.h"
#include "PhantomDrive_global.h"

#include <QPointer>

namespace PhantomDrive {

class TrackData;

enum class CustomTrackModeState {
    Editing,
    Playing
};

class PHANTOMDRIVE_EXPORT CustomTrackMode : public GameMode
{
    Q_OBJECT

public:
    explicit CustomTrackMode(QObject* parent = nullptr);
    ~CustomTrackMode() override;

    ModeType getType() const override { return ModeType::CustomTrack; }
    QString getName() const override { return QStringLiteral("Custom Track Mode"); }
    QString getDescription() const override { return QStringLiteral("Build and race custom tile-based tracks"); }

    void onEnter() override;
    void onExit() override;
    void update(qint64 elapsedMs) override;
    void render() override;

    void setTrack(TrackData* track);
    TrackData* track() const { return m_track.data(); }

    void setCustomState(CustomTrackModeState state);
    CustomTrackModeState customState() const { return m_customState; }

signals:
    void customStateChanged(PhantomDrive::CustomTrackModeState state);
    void customTrackChanged(PhantomDrive::TrackData* track);

private:
    QPointer<TrackData> m_track;
    CustomTrackModeState m_customState;
};

} // namespace PhantomDrive

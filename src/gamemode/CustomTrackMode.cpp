#include "gamemode/CustomTrackMode.h"

#include "track/TrackData.h"

namespace PhantomDrive {

CustomTrackMode::CustomTrackMode(QObject* parent)
    : GameMode(parent)
    , m_track(nullptr)
    , m_customState(CustomTrackModeState::Editing)
{
}

CustomTrackMode::~CustomTrackMode()
{
}

void CustomTrackMode::onEnter()
{
    GameMode::onEnter();
    setCustomState(CustomTrackModeState::Editing);
}

void CustomTrackMode::onExit()
{
    setCustomState(CustomTrackModeState::Editing);
    GameMode::onExit();
}

void CustomTrackMode::update(qint64 elapsedMs)
{
    if (!isActive()) {
        return;
    }

    emit modeUpdated(elapsedMs);
}

void CustomTrackMode::render()
{
}

void CustomTrackMode::setTrack(TrackData* track)
{
    if (m_track == track) {
        return;
    }

    m_track = track;
    emit customTrackChanged(track);
}

void CustomTrackMode::setCustomState(CustomTrackModeState state)
{
    if (m_customState == state) {
        return;
    }

    m_customState = state;
    emit customStateChanged(state);
}

} // namespace PhantomDrive

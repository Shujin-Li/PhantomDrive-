#include "GameMode.h"

namespace PhantomDrive {

GameMode::GameMode(QObject* parent)
    : QObject(parent)
    , m_state(ModeState::Inactive)
    , m_transitionDurationMs(500)
{
}

GameMode::~GameMode()
{
}

QString GameMode::getName() const
{
    return QString();
}

QString GameMode::getDescription() const
{
    return QString();
}

void GameMode::onEnter()
{
    setState(ModeState::Active);
}

void GameMode::onExit()
{
    setState(ModeState::Inactive);
}

void GameMode::pause()
{
    if (m_state == ModeState::Active) {
        setState(ModeState::Paused);
        emit modePaused();
    }
}

void GameMode::resume()
{
    if (m_state == ModeState::Paused) {
        setState(ModeState::Active);
        emit modeResumed();
    }
}

void GameMode::handleInput(const QVariantMap& input)
{
    Q_UNUSED(input)
}

QVariantMap GameMode::getStateData() const
{
    QVariantMap data;
    data["modeType"] = static_cast<int>(getType());
    data["modeName"] = getName();
    data["state"] = static_cast<int>(m_state);
    return data;
}

void GameMode::loadStateData(const QVariantMap& data)
{
    Q_UNUSED(data)
}

void GameMode::setTransitionDuration(int ms)
{
    m_transitionDurationMs = qMax(0, ms);
}

int GameMode::getTransitionDuration() const
{
    return m_transitionDurationMs;
}

QList<QString> GameMode::getRequiredAssets() const
{
    return QList<QString>();
}

void GameMode::onVehicleCollision(const QString& objectId)
{
    Q_UNUSED(objectId)
}

void GameMode::onSpeedLimitChanged(qreal newLimit)
{
    Q_UNUSED(newLimit)
}

void GameMode::onTrafficLightChanged(bool isRed)
{
    Q_UNUSED(isRed)
}

void GameMode::onCheckpointReached(int checkpointId)
{
    Q_UNUSED(checkpointId)
}

void GameMode::setState(ModeState newState)
{
    if (m_state != newState) {
        ModeState oldState = m_state;
        m_state = newState;
        emit stateChanged(oldState, newState);
    }
}

void GameMode::requestTransition(ModeType targetMode)
{
    emit transitionRequired(targetMode);
}

}

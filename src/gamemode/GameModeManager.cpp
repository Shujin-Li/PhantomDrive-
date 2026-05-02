#include "GameModeManager.h"

#include <QDebug>
#include <QCoreApplication>

namespace PhantomDrive {

GameModeManager* GameModeManager::s_instance = nullptr;

GameModeManager::GameModeManager(QObject* parent)
    : QObject(parent)
    , m_currentMode(nullptr)
    , m_currentModeType(ModeType::Unknown)
    , m_targetModeType(ModeType::Unknown)
    , m_transition(new ModeTransition(this))
    , m_isTransitioning(false)
    , m_switchPending(false)
    , m_lastUpdateTime(0)
{
    connect(m_transition, &ModeTransition::finished,
            this, &GameModeManager::onTransitionFinished);
    connect(m_transition, &ModeTransition::cancelled,
            this, &GameModeManager::onTransitionCancelled);
}

GameModeManager::~GameModeManager()
{
}

GameModeManager* GameModeManager::instance(QObject* parent)
{
    if (!s_instance) {
        s_instance = new GameModeManager(parent ? parent : qApp);
    }
    return s_instance;
}

void GameModeManager::registerMode(GameMode* mode, ModeType type)
{
    if (!mode) {
        emit errorOccurred("Cannot register null mode");
        return;
    }

    if (m_registeredModes.contains(type)) {
        qWarning() << "Mode type" << static_cast<int>(type) << "is already registered. Unregistering old mode.";
        m_registeredModes.remove(type);
    }

    m_registeredModes[type] = mode;
    connect(mode, &GameMode::transitionRequired,
            this, [this](ModeType target) {
                switchTo(target);
            });

    emit modeRegistered(type);
}

void GameModeManager::unregisterMode(ModeType type)
{
    if (m_registeredModes.contains(type)) {
        GameMode* mode = m_registeredModes[type];
        if (mode == m_currentMode) {
            mode->onExit();
            m_currentMode = nullptr;
            m_currentModeType = ModeType::Unknown;
        }
        m_registeredModes.remove(type);
        emit modeUnregistered(type);
    }
}

bool GameModeManager::isModeRegistered(ModeType type) const
{
    return m_registeredModes.contains(type);
}

bool GameModeManager::switchTo(ModeType targetMode, TransitionType transition)
{
    if (m_isTransitioning) {
        qWarning() << "Cannot switch mode: transition already in progress";
        return false;
    }

    if (targetMode == m_currentModeType) {
        qWarning() << "Cannot switch to the same mode";
        return false;
    }

    if (!m_registeredModes.contains(targetMode)) {
        emit errorOccurred(QString("Mode type %1 is not registered").arg(static_cast<int>(targetMode)));
        return false;
    }

    m_targetModeType = targetMode;
    m_transition->setTransitionType(transition);
    m_isTransitioning = true;

    if (m_currentMode) {
        m_history.push(m_currentModeType);
        executeExitSequence(m_currentMode);
    }

    emit modeSwitchStarted(m_currentModeType, targetMode);
    m_transition->start();

    return true;
}

bool GameModeManager::switchTo(const QString& modeName, TransitionType transition)
{
    for (auto it = m_registeredModes.begin(); it != m_registeredModes.end(); ++it) {
        if (it.value()->getName() == modeName) {
            return switchTo(it.key(), transition);
        }
    }
    emit errorOccurred(QString("Mode with name '%1' not found").arg(modeName));
    return false;
}

GameMode* GameModeManager::getCurrentMode() const
{
    return m_currentMode;
}

ModeType GameModeManager::getCurrentModeType() const
{
    return m_currentModeType;
}

GameMode* GameModeManager::getMode(ModeType type) const
{
    return m_registeredModes.value(type, nullptr);
}

qreal GameModeManager::getTransitionProgress() const
{
    return m_transition->getProgress();
}

bool GameModeManager::isTransitioning() const
{
    return m_isTransitioning;
}

void GameModeManager::cancelTransition()
{
    if (m_isTransitioning) {
        m_transition->cancel();
    }
}

void GameModeManager::pauseCurrentMode()
{
    if (m_currentMode && !m_currentMode->isPaused()) {
        m_currentMode->pause();
        emit currentModeUpdated(m_lastUpdateTime);
    }
}

void GameModeManager::resumeCurrentMode()
{
    if (m_currentMode && m_currentMode->isPaused()) {
        m_currentMode->resume();
    }
}

QList<ModeType> GameModeManager::getTransitionHistory() const
{
    return m_history.values();
}

bool GameModeManager::canGoBack() const
{
    return !m_history.isEmpty();
}

bool GameModeManager::goBack(TransitionType transition)
{
    if (!canGoBack()) {
        return false;
    }
    ModeType previousMode = m_history.pop();
    return switchTo(previousMode, transition);
}

void GameModeManager::update(qint64 elapsedMs)
{
    m_lastUpdateTime = elapsedMs;

    if (m_isTransitioning) {
        return;
    }

    if (m_currentMode && m_currentMode->isActive()) {
        m_currentMode->update(elapsedMs);
        emit currentModeUpdated(elapsedMs);
    }
}

void GameModeManager::render()
{
    if (m_currentMode) {
        m_currentMode->render();
    }
}

void GameModeManager::onTransitionFinished()
{
    performSwitch(m_targetModeType);
    m_isTransitioning = false;
    m_transitionHistory.append(qMakePair(m_currentModeType, m_targetModeType));
    emit modeSwitchCompleted(m_targetModeType);
}

void GameModeManager::onTransitionCancelled()
{
    m_isTransitioning = false;

    if (!m_history.isEmpty()) {
        m_history.pop();
    }

    emit modeSwitchCancelled();
}

void GameModeManager::performSwitch(ModeType targetMode)
{
    GameMode* newMode = m_registeredModes.value(targetMode, nullptr);
    if (!newMode) {
        emit errorOccurred(QString("Target mode %1 not found").arg(static_cast<int>(targetMode)));
        return;
    }

    m_currentMode = newMode;
    m_currentModeType = targetMode;
    m_targetModeType = ModeType::Unknown;

    executeEnterSequence(newMode);
}

void GameModeManager::executeExitSequence(GameMode* mode)
{
    if (mode) {
        mode->setState(ModeState::Exiting);
        emit mode->modeAboutToExit();
        mode->onExit();
        mode->setState(ModeState::Inactive);
    }
}

void GameModeManager::executeEnterSequence(GameMode* mode)
{
    if (mode) {
        mode->setState(ModeState::Entering);
        mode->onEnter();
        mode->setState(ModeState::Active);
        emit mode->modeEntered();
    }
}

}

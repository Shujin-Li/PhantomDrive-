#pragma once

#include "PhantomDrive_global.h"
#include "GameMode.h"
#include "ModeTransition.h"

#include <QObject>
#include <QMap>
#include <QStack>
#include <QList>
#include <QPair>

namespace PhantomDrive {

class PHANTOMDRIVE_EXPORT GameModeManager : public QObject
{
    Q_OBJECT

public:
    PHANTOMDRIVE_EXPORT static GameModeManager* instance(QObject* parent = nullptr);

    void registerMode(GameMode* mode, ModeType type);
    void unregisterMode(ModeType type);
    bool isModeRegistered(ModeType type) const;

    bool switchTo(ModeType targetMode, TransitionType transition = TransitionType::Fade);
    bool switchTo(const QString& modeName, TransitionType transition = TransitionType::Fade);

    GameMode* getCurrentMode() const;
    ModeType getCurrentModeType() const;
    GameMode* getMode(ModeType type) const;

    qreal getTransitionProgress() const;
    bool isTransitioning() const;
    void cancelTransition();

    void pauseCurrentMode();
    void resumeCurrentMode();

    QList<ModeType> getTransitionHistory() const;
    bool canGoBack() const;
    bool goBack(TransitionType transition = TransitionType::SlideRight);

public slots:
    void update(qint64 elapsedMs);
    void render();

signals:
    void modeSwitchStarted(ModeType from, ModeType to);
    void modeSwitchCompleted(ModeType newMode);
    void modeSwitchCancelled();
    void transitionProgressChanged(qreal progress);
    void currentModeUpdated(qint64 elapsedMs);
    void modeRegistered(ModeType type);
    void modeUnregistered(ModeType type);
    void errorOccurred(const QString& error);

protected:
    explicit GameModeManager(QObject* parent = nullptr);
    ~GameModeManager();
    GameModeManager(const GameModeManager&) = delete;
    GameModeManager& operator=(const GameModeManager&) = delete;

private slots:
    void onTransitionFinished();
    void onTransitionCancelled();

private:
    void performSwitch(ModeType targetMode);
    void executeExitSequence(GameMode* mode);
    void executeEnterSequence(GameMode* mode);

    static GameModeManager* s_instance;

    QMap<ModeType, GameMode*> m_registeredModes;
    GameMode* m_currentMode;
    ModeType m_currentModeType;
    ModeType m_targetModeType;

    ModeTransition* m_transition;

    bool m_isTransitioning;
    bool m_switchPending;

    QStack<ModeType> m_history;
    QList<QPair<ModeType, ModeType>> m_transitionHistory;

    qint64 m_lastUpdateTime;
};

}

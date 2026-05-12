#pragma once

#include "PhantomDrive_global.h"

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QTimer>

namespace PhantomDrive {

enum class ModeType {
    Unknown = -1,
    Arcade = 0,
    Learning = 1,
    Count
};

enum class ModeState {
    Inactive,
    Entering,
    Active,
    Paused,
    Exiting
};

class ModeTransition;

class PHANTOMDRIVE_EXPORT GameMode : public QObject
{
    Q_OBJECT

public:
    explicit GameMode(QObject* parent = nullptr);
    virtual ~GameMode();

    virtual ModeType getType() const = 0;
    virtual QString getName() const;
    virtual QString getDescription() const;

    ModeState getState() const { return m_state; }
    bool isActive() const { return m_state == ModeState::Active; }
    bool isPaused() const { return m_state == ModeState::Paused; }

    virtual void onEnter();
    virtual void onExit();

    virtual void update(qint64 elapsedMs) = 0;
    virtual void render() = 0;

    virtual void pause();
    virtual void resume();
    virtual void handleInput(const QVariantMap& input);

    virtual QVariantMap getStateData() const;
    virtual void loadStateData(const QVariantMap& data);

    void setTransitionDuration(int ms);
    int getTransitionDuration() const;

    virtual QList<QString> getRequiredAssets() const;

public slots:
    virtual void onVehicleCollision(const QString& objectId);
    virtual void onSpeedLimitChanged(qreal newLimit);
    virtual void onTrafficLightChanged(bool isRed);
    virtual void onCheckpointReached(int checkpointId);

signals:
    void modeEntered();
    void modeExited();
    void modeUpdated(qint64 elapsedMs);
    void modePaused();
    void modeResumed();
    void modeAboutToExit();
    void transitionRequired(ModeType from, ModeType to);
    void stateChanged(ModeState oldState, ModeState newState);
    void errorOccurred(const QString& errorMessage);

protected:
    friend class GameModeManager;
    void setState(ModeState newState);
    void requestTransition(ModeType targetMode);

    int m_transitionDurationMs;

private:
    ModeState m_state;
};

}

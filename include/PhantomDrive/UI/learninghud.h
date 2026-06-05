#ifndef LEARNINGHUD_H
#define LEARNINGHUD_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QMap>

#include "PhantomDrive_global.h"

class PHANTOMDRIVE_EXPORT LearningHUD : public QWidget
{
    Q_OBJECT
public:
    explicit LearningHUD(QWidget *parent = nullptr);
    ~LearningHUD();

public slots:
    // Speed
    void updateCurrentSpeed(qreal speed);
    void updateSpeedLimit(int limit);
    void updateSpeedStatus(bool isOverLimit);

    // Traffic light
    void updateTrafficLight(const QString& state, int remainingSeconds = 0);

    // Floating messages (now delegated via signals, not drawn inside HUD)
    void showPenaltyMessage(const QString& message, int points);
    void showViolationWarning(const QString& violationType);

    // Powerup
    void updatePowerupState(const QString& powerupId, const QString& name, bool active);

    // Mode / lap
    void updateGameMode(const QString& mode);
    void updateLapInfo(int currentLap, int totalLaps);
    void updateLapTime(const QString& time);

    // Control
    void setVisible(bool visible) override;
    void clearAllWarnings();

signals:
    void speedLimitChanged(int limit);
    void violationTriggered(const QString& type);
    void powerupActivated(const QString& powerupId, const QString& name);
    void powerupDeactivated(const QString& powerupId);
    void penaltyMessageRequested(const QString& message, int points);
    void violationWarningRequested(const QString& violationType);

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void onRedLightBlink();

private:
    void setupUI();
    void updateSpeedColor();

    // ---- Top bar ----
    QLabel *m_modeLabel;
    QLabel *m_trafficDot;
    QLabel *m_stopLabel;

    // ---- Speed ----
    QLabel *m_speedLabel;
    QLabel *m_speedUnitLabel;
    QLabel *m_speedLimitLabel;

    // ---- Traffic light text ----
    QLabel *m_trafficLightLabel;

    // ---- Lap / time ----
    QLabel *m_lapLabel;
    QLabel *m_lapTimeLabel;

    // ---- Powerup ----
    QLabel *m_powerupLabel;

    // ---- Blink timer for red light ----
    QTimer *m_blinkTimer;
    bool    m_blinkOn = false;

    // ---- State ----
    qreal   m_currentSpeed = 0.0;
    int     m_speedLimit   = 60;
    bool    m_isOverLimit  = false;
    QString m_trafficState = "green";
    QString m_currentMode;

    // Legacy: floating message list (kept for clearAllWarnings)
    QList<QLabel*> m_floatingMessages;
    QMap<QString, QLabel*> m_powerupIcons;
};

#endif // LEARNINGHUD_H

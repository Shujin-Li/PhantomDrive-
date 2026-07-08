#ifndef ARCADE_HUD_H
#define ARCADE_HUD_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QPainter>
#include <QPaintEvent>
#include <QColor>

namespace PhantomDrive {

#include "PhantomDrive_global.h"

// Full-circle speedometer gauge (0-120 km/h) — compact 200x200.
class SpeedometerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SpeedometerWidget(QWidget* parent = nullptr);
    void setSpeed(qreal speed);
    void setMaxSpeed(qreal maxSpeed);
    void setRedLight(bool red);
    void setSpeedLimit(qreal limit);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    qreal m_speed     = 0.0;
    qreal m_maxSpeed  = 120.0;
    qreal m_speedLimit = 0.0;
    bool  m_redLight  = false;
};

class PHANTOMDRIVE_EXPORT ArcadeHUD : public QWidget
{
    Q_OBJECT
public:
    explicit ArcadeHUD(QWidget* parent = nullptr);
    ~ArcadeHUD();

    // "Arcade", "Learning", "Custom Track"
    void setGameMode(const QString& mode);
    void updateSpeed(qreal speed);
    void updateLap(int lapsCompleted, int totalLaps);
    void updateRouteProgress(int checkpointsPassed, int totalCheckpoints, const QString& nextTarget);
    void updateLapTime(const QString& time);
    void updateTotalTime(const QString& time);
    void updatePosition(int position, int totalRacers);
    void updateBoost(qreal boostPercent);
    void updateTrafficLight(const QString& state);
    void updateAISpeed1(qreal speedKmh, bool available);
    void updateAISpeed2(qreal speedKmh, bool available);
    void updateSpeedLimit(qreal limitKmh);
    void setCustomTrackVisualMode(bool enabled);
    void setTwoPlayerMode(bool enabled);
    void updatePlayer1Status(qreal speedKmh,
                             qreal limitKmh,
                             const QString& lightState,
                             int lapCurrent,
                             int lapTotal,
                             int position,
                             int totalRacers);
    void updatePlayer2Status(qreal speedKmh,
                             qreal limitKmh,
                             const QString& lightState,
                             int lapCurrent,
                             int lapTotal,
                             int position,
                             int totalRacers);
    void updatePlayerPowerup(int playerIndex, const QString& type, int remainingSecs);

    // Powerup state: type string ("Boost"/"Shield"/"EMP"/"Repair"/"Missile"/"Oil"/"Invis"/"Magnet"), remaining seconds
    void updatePowerupState(const QString& type, int remainingSecs);
    void clearPowerupState();

    // Current race objective: "Next: CP1", "Next: FINISH", etc.
    void updateCurrentObjective(const QString& objective);
    void setPaused(bool paused);

    void showRaceBanner(const QString& message);
    void showLapCompleted(int lapNumber);
    void showRaceFinished(int finalPosition, const QString& totalTime);

    void reset();

signals:
    void centerNotificationRequested(const QString& text, int durationMs);

public slots:
    void setVisible(bool visible) override;

protected:
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onRedLightBlink();
    void onPowerupTimerTick();

private:
    void setupUI();
    void applyTheme();
    void setFloatingStyle();
    void setCustomTrackStyle();
    void updateSpeedDisplay();
    void setupTwoPlayerOverlay();
    void layoutTwoPlayerOverlay();

    struct PlayerHudWidgets {
        QWidget* panel = nullptr;
        QLabel* titleLabel = nullptr;
        QLabel* badgeLabel = nullptr;
        QLabel* speedLabel = nullptr;
        QLabel* limitLabel = nullptr;
        QLabel* lightDot = nullptr;
        QLabel* lightLabel = nullptr;
        SpeedometerWidget* speedo = nullptr;
        QLabel* lapLabel = nullptr;
        QLabel* positionLabel = nullptr;
        QLabel* positionTotalLabel = nullptr;
        QLabel* powerupLabel = nullptr;
        QLabel* powerupTimerLabel = nullptr;
    };

    QWidget* createPlayerPanel(PlayerHudWidgets& widgets,
                               const QString& title,
                               const QString& badge,
                               const QColor& accent);
    void updatePlayerStatus(PlayerHudWidgets& widgets,
                            qreal speedKmh,
                            qreal limitKmh,
                            const QString& lightState,
                            int lapCurrent,
                            int lapTotal,
                            int position,
                            int totalRacers);
    void updatePlayerPowerupLabel(PlayerHudWidgets& widgets,
                                  const QString& type,
                                  int remainingSecs);

    // ---- Mode title ----
    QLabel*      m_modeLabel;

    // ---- Speed row ----
    QLabel*      m_speedBigLabel;
    QLabel*      m_speedUnitLabel;
    QLabel*      m_speedLimitLabel;
    QLabel*      m_trafficDot;
    QLabel*      m_trafficStateLabel;

    // ---- Full-circle speedometer ----
    SpeedometerWidget* m_speedo;

    // ---- Info labels ----
    QLabel*      m_lapLabel;
    QLabel*      m_lapTimeLabel;
    QLabel*      m_totalTimeLabel;
    QLabel*      m_positionLabel;
    QLabel*      m_positionTotalLabel;
    QLabel*      m_ai1SpeedLabel;
    QLabel*      m_ai2SpeedLabel;
    QProgressBar* m_boostBar;

    // ---- Powerup state ----
    QLabel*      m_powerupSlot1;
    QLabel*      m_powerupTimer1;
    QLabel*      m_powerupSlot2;
    QLabel*      m_powerupTimer2;
    QLabel*      m_objectiveLabel;

    // ---- Overlays ----
    QLabel*      m_stopLabel;

    // ---- Two-player HUD overlay ----
    QWidget*     m_twoPlayerOverlay = nullptr;
    PlayerHudWidgets m_player1Hud;
    PlayerHudWidgets m_player2Hud;
    QLabel*      m_twoAi1SpeedLabel = nullptr;
    QLabel*      m_twoAi2SpeedLabel = nullptr;

    // ---- Timers ----
    QTimer*      m_blinkTimer;
    QTimer*      m_powerupTimer;

    // ---- State ----
    int     m_currentLap       = 1;
    int     m_totalLaps        = 3;
    int     m_totalRacers      = 1;
    qreal   m_currentSpeed     = 0.0;
    qreal   m_currentSpeedLimit = 0.0;
    qreal   m_boostPercent    = 0.0;
    QString m_trafficState     = "green";
    bool    m_blinkOn          = false;
    bool    m_twoPlayerMode    = false;
    bool    m_paused           = false;
    qint64  m_pauseStartedMs   = 0;
    QString m_gameMode         = "Arcade";
    // Powerup timers (msecs when each slot expires)
    qint64  m_powerup1ExpiryMs = 0;
    qint64  m_powerup2ExpiryMs = 0;
    QString m_powerup1Type;
    QString m_powerup2Type;
};

} // namespace PhantomDrive

#endif // ARCADE_HUD_H

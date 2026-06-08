#ifndef ARCADE_HUD_H
#define ARCADE_HUD_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QPainter>
#include <QPaintEvent>

namespace PhantomDrive {

#include "PhantomDrive_global.h"

// Full-circle speedometer gauge (0-240 km/h) — compact 200x200.
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
    qreal m_maxSpeed  = 240.0;
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
    void updateBestLapTime(const QString& time);
    void updateBoost(qreal boostPercent);
    void updateTrafficLight(const QString& state);
    void updateAISpeed(const QString& aiId, qreal speed);
    void updateSpeedLimit(qreal limitKmh);
    void setCustomTrackVisualMode(bool enabled);

    void showCountdown(int seconds);
    void showGo();
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
    void onCountdownTick();
    void onRedLightBlink();

private:
    void setupUI();
    void applyTheme();
    void setFloatingStyle();
    void setCustomTrackStyle();
    void showFloatingMessage(const QString&, const QString&);
    void updateSpeedDisplay();
    void repositionCountdown();

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
    QLabel*      m_bestLapLabel;
    QLabel*      m_aiSpeedLabel;
    QProgressBar* m_boostBar;

    // ---- Overlays ----
    QLabel*      m_countdownLabel;
    QLabel*      m_stopLabel;

    // ---- Timers ----
    QTimer*      m_countdownTimer;
    QTimer*      m_blinkTimer;

    // ---- State ----
    int     m_countdownValue   = 0;
    int     m_currentLap       = 1;
    int     m_totalLaps        = 3;
    qreal   m_currentSpeed     = 0.0;
    qreal   m_currentSpeedLimit = 0.0;
    qreal   m_boostPercent    = 0.0;
    QString m_trafficState     = "green";
    bool    m_blinkOn          = false;
    QString m_gameMode         = "Arcade";

    static const int COUNTDOWN_START = 3;
};

} // namespace PhantomDrive

#endif // ARCADE_HUD_H

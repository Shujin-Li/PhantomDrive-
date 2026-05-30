#ifndef ARCADE_HUD_H
#define ARCADE_HUD_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QMap>

namespace PhantomDrive {

#include "PhantomDrive_global.h"

class PHANTOMDRIVE_EXPORT ArcadeHUD : public QWidget
{
    Q_OBJECT
public:
    explicit ArcadeHUD(QWidget* parent = nullptr);
    ~ArcadeHUD();

    void updateSpeed(qreal speed);
    void updateLap(int currentLap, int totalLaps);
    void updateRouteProgress(int checkpointsPassed, int totalCheckpoints, const QString& nextTarget);
    void updateLapTime(const QString& time);
    void updateTotalTime(const QString& time);
    void updatePosition(int position, int totalRacers);
    void updateBestLapTime(const QString& time);
    void setCustomTrackVisualMode(bool enabled);

    void showCountdown(int seconds);
    void showGo();
    void showRaceBanner(const QString& message);
    void showLapCompleted(int lapNumber);
    void showRaceFinished(int finalPosition, const QString& totalTime);

    void reset();

public slots:
    void setVisible(bool visible) override;

protected:
    void showEvent(QShowEvent* event) override;

private slots:
    void onCountdownTick();
    void onMessageTimeout();

private:
    void setupUI();
    void setFloatingStyle();
    void setCustomTrackStyle();
    void showFloatingMessage(const QString& message, const QString& style);

    QLabel* m_titleLabel;
    QLabel* m_speedLabel;
    QLabel* m_speedUnitLabel;
    QLabel* m_lapLabel;
    QLabel* m_lapTimeLabel;
    QLabel* m_totalTimeLabel;
    QLabel* m_positionLabel;
    QLabel* m_bestLapLabel;
    QLabel* m_countdownLabel;

    QWidget* m_centerMessageWidget;
    QLabel* m_centerMessageLabel;

    QTimer* m_countdownTimer;
    int m_countdownValue;
    int m_currentLap;
    int m_totalLaps;
    qreal m_currentSpeed;
    bool m_customTrackVisualMode;

    static const int COUNTDOWN_START = 3;
};

} // namespace PhantomDrive

#endif // ARCADE_HUD_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QApplication>
#include <QVector2D>
#include <QList>
#include <QSet>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include "PhantomDrive_global.h"
#include "learninghud.h"
#include "UI/ArcadeHUD.h"
#include "UI/GameViewWidget.h"
#include "UI/DrivingReportWidget.h"
#include "UI/InteractiveFeedback.h"
#include "UI/SoundManager.h"
#include "gamemode/DrivingDataCollector.h"
#include "scoring/ScoreManager.h"
#include "core/datamodels.h"
#include "gamemode/AIOpponentManager.h"
#include "gamemode/AIOpponent.h"
#include "core/VehiclePhysics.h"

class QPushButton;
class QComboBox;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

namespace PhantomDrive {
class CustomTrackEditorWidget;
class CustomTrackMode;
class TrackData;
class PowerupBox;
class TrafficObjectManager;
class PowerupWorldRuntime;
enum class PowerupType;
}

class PHANTOMDRIVE_EXPORT MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateHUD(int speed, const QString &status);
    void setDrivingDataCollector(PhantomDrive::DrivingDataCollector* collector);

    PhantomDrive::GameViewWidget* getGameView() { return m_gameView; }

    void showInteractiveFeedback(const QString& message, PhantomDrive::FeedbackType type);
    void playSound(PhantomDrive::SoundEffect effect);

    void showCountdown();
    void onRaceStart();
    void onLapCompleted(int lapNumber);
    void onCheckpointReached(int checkpointNumber);
    void onCollision();
    void onPowerupCollected(const QString& powerupType);

private:
    Ui::MainWindow *ui;
    LearningHUD *m_learningHUD;
    PhantomDrive::ArcadeHUD* m_arcadeHUD;
    PhantomDrive::GameViewWidget *m_gameView;
    PhantomDrive::VehiclePhysics *m_vehiclePhysics;
    PhantomDrive::DrivingDataCollector *m_drivingDataCollector;
    PhantomDrive::ScoreManager *m_scoreManager;
    PhantomDrive::AIOpponentManager *m_aiManager;
    PhantomDrive::TrafficObjectManager *m_trafficObjectManager;
    QList<PhantomDrive::PowerupBox*> m_powerupBoxes;
    PhantomDrive::PowerupWorldRuntime* m_powerupWorld;
    PhantomDrive::DrivingReportWidget *m_reportWidget;
    QWidget *m_reportPage;   // cached pointer to the report page in stackedWidget
    QPushButton *m_btnFinishDrive;
    QComboBox *m_aiDifficultyCombo;
    QPushButton *m_btnLoadCustomTrack;
    QPushButton *m_btnCustomTrackMode;
    QPushButton *m_btnPlayCustomTrack;
    QPushButton *m_btnSaveCustomTrack;
    QPushButton *m_btnLoadCustomTrackForEdit;
    QPushButton *m_btnExportCustomTrackJson;
    PhantomDrive::CustomTrackEditorWidget *m_customTrackEditor;
    PhantomDrive::CustomTrackMode *m_customTrackMode;
    PhantomDrive::TrackData *m_defaultRaceTrack;
    PhantomDrive::TrackData *m_runtimeCustomTrack;
    QTimer *m_simTimer;
    QTimer *m_learningSessionTimer;   // auto-ends Learning Mode after max duration
    QString m_currentMode;
    QString m_customTrackPath;
    int m_currentSpeedLimit;
    QString m_currentTrafficLightState;
    bool m_driveActive;
    bool m_countdownActive;
    bool m_arcadeRaceFinished;
    bool m_customTrackPlaying;
    int m_lapsCompleted;
    int m_totalLaps;
    int m_simTick;
    qint64 m_sessionElapsedMs;
    qint64 m_currentLapStartMs;
    qint64 m_bestLapMs;
    QVector2D m_playerPosition;
    QVector2D m_previousPlayerPosition;
    qreal m_playerRotation;
    qreal m_playerSpeed;
    bool m_arcadeRaceLogicActive;
    int m_nextCheckpointIndex;
    int m_raceCheckpointTotal;
    bool m_hasLeftNorthSector;
    bool m_wasOnStartLine;
    bool m_wasInNorthGate;
    bool m_blockCheckpointsUntilLeaveNorth;
    bool m_wasInsideNextGate;
    QSet<QString> m_playerVehicleContacts;

    void setupGameView();
    void setupVehiclePhysics();
    void setupDataBindings();
    void setupDemoControls();
    void setupCustomTrackControls();
    void styleMainMenu();
    void setGameHeaderVisible(bool visible);
    void initializeAIOpponents();
    void applyAIDifficultySelection();
    void loadCustomTrack();
    void showCustomTrackEditor();
    void hideCustomTrackEditor();
    void playCurrentCustomTrack();
    void startCustomTrackSession(PhantomDrive::TrackData* track);
    void saveCurrentCustomTrack();
    void loadCustomTrackIntoEditor();
    void exportCurrentCustomTrackJson();
    void restoreDefaultRaceTrack();
    void focusGameViewForDriving();
    void startDrivingSession(const QString& mode);
    void finishDrivingSession();
    void onGameFinished();          // unified end-of-game entry point (shows report)
    void silentFinishSession();     // end session without showing report (used when starting a new game)
    void showReportWindow(const PhantomDrive::ScoreReport* report = nullptr);
    void fadeInReportWindow();
    void onReportBackToMenu();
    void onReportNewDrive();
    void updateGameViewFromData(const PhantomDrive::DrivingData& data);
    void updateTrafficAndHud(int tick);
    void updateRaceHud();
    int speedToDisplayKmh(qreal physicsSpeed) const;
    int displaySpeedKmh() const;
    QString powerupTypeToString(PhantomDrive::PowerupType type) const;
    QString formatRaceTime(qint64 milliseconds) const;
    qreal estimatePlayerProgress() const;
    void simulateGameLoop();
    void setupEBRuntimeObjects();
    void setupCustomTrackRuntimeObjects(PhantomDrive::TrackData* track);
    void clearEBRuntimeObjects();
    void updateEBRuntime(qreal deltaSeconds);
    void handlePowerupCollected(PhantomDrive::PowerupType type);
    void handleTrafficViolation(const PhantomDrive::ViolationEvent& violation);
    void applyPlayerSpawnAtStartLine();
    void syncRaceTrackToManager();
    void resetArcadeRaceProgress();
    void updateArcadeRaceProgress(const QVector2D& positionBefore);
    void finishCustomTrackRoute();
    void resolvePlayerAiVehicleContact(PhantomDrive::AIOpponent* ai);

private slots:
    void onDrivingDataCollected(const PhantomDrive::DrivingData& data);
    void onViolationDetected(const PhantomDrive::ViolationEvent& violation);
    void onScoreReady(const PhantomDrive::ScoreReport& report);
    void onCoachReportReady(const QString& markdown);
    void on_btn_History_clicked();
};

#endif // MAINWINDOW_H

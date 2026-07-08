#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/qtmetamacros.h>
#include <QObject>
#include <QMainWindow>
#include <QTimer>
#include <QApplication>
#include <QVector2D>
#include <QPointF>
#include <QList>
#include <QSet>
#include <QHash>
#include <QColor>
#include <QRectF>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <memory>
#include "PhantomDrive_global.h"
#include "learninghud.h"
#include "UI/ArcadeHUD.h"
#include "UI/GameViewWidget.h"
#include "UI/DrivingReportWidget.h"
#include "UI/InteractiveFeedback.h"
#include "UI/SoundManager.h"
#include "gamemode/DrivingDataCollector.h"
#include "scoring/ScoreManager.h"
#include "scoring/ScoreReport.h"
#include "core/datamodels.h"
#include "gamemode/AIOpponentManager.h"
#include "gamemode/AIOpponent.h"
#include "core/VehiclePhysics.h"

class QPushButton;
class QComboBox;
class QLabel;
class QResizeEvent;
class QWidget;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

namespace PhantomDrive {
class CustomTrackEditorWidget;
class CustomTrackMode;
class CollectibleManager;
class GarageWidget;
class MainMenuWidget;
class TrackData;
class PowerupBox;
class TrafficObjectManager;
class PowerupWorldRuntime;
class ChallengeObstacleManager;
class BlockerVehicleManager;
class PlayerProfileStore;
class CurrencyManager;
class SkinManager;
enum class PowerupType;

enum class CoinChallengeBalloonRushPhase {
    Inactive,
    Trigger,
    BonusScene,
    Return
};

enum class CoinChallengeEndReason {
    None,
    Timeout,
    GoalComplete,
    ForcedFinish,
    VehicleDestroyed,
    FatalCrash
};
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

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    Ui::MainWindow *ui;
    LearningHUD *m_learningHUD;
    PhantomDrive::ArcadeHUD* m_arcadeHUD;
    PhantomDrive::GameViewWidget *m_gameView;
    PhantomDrive::VehiclePhysics *m_vehiclePhysics;
    PhantomDrive::VehiclePhysics *m_player2Physics;
    PhantomDrive::DrivingDataCollector *m_drivingDataCollector;
    PhantomDrive::DrivingDataCollector *m_player2DataCollector;
    PhantomDrive::ScoreManager *m_scoreManager;
    PhantomDrive::ScoreManager *m_player2ScoreManager;
    PhantomDrive::AIOpponentManager *m_aiManager;
    PhantomDrive::TrafficObjectManager *m_trafficObjectManager;
    QList<PhantomDrive::PowerupBox*> m_powerupBoxes;
    PhantomDrive::PowerupWorldRuntime* m_powerupWorld;
    PhantomDrive::DrivingReportWidget *m_reportWidget;
    QWidget *m_reportPage;   // cached pointer to the report page in stackedWidget
    QPushButton *m_btnFinishDrive;
    QPushButton *m_btnPause;
    QComboBox *m_aiDifficultyCombo;
    QPushButton *m_btnLoadCustomTrack;
    QPushButton *m_btnCustomTrackMode;
    QPushButton *m_btnTwoPlayerRace;
    QPushButton *m_btnAdaptiveDemo;
    QPushButton *m_btnCoinChallenge;
    QPushButton *m_btnGarage;
    QPushButton *m_btnGuide;
    QPushButton *m_btnPlayCustomTrack;
    QPushButton *m_btnSaveCustomTrack;
    QPushButton *m_btnLoadCustomTrackForEdit;
    QPushButton *m_btnExportCustomTrackJson;
    PhantomDrive::CustomTrackEditorWidget *m_customTrackEditor;
    PhantomDrive::CustomTrackMode *m_customTrackMode;
    PhantomDrive::CollectibleManager *m_collectibleManager;
    std::unique_ptr<PhantomDrive::ChallengeObstacleManager> m_challengeObstacleManager;
    std::unique_ptr<PhantomDrive::BlockerVehicleManager> m_blockerVehicleManager;
    PhantomDrive::GarageWidget *m_garageWidget;
    PhantomDrive::MainMenuWidget *m_mainMenuWidget;
    PhantomDrive::PlayerProfileStore *m_playerProfileStore;
    PhantomDrive::CurrencyManager *m_currencyManager;
    PhantomDrive::SkinManager *m_skinManager;
    PhantomDrive::TrackData *m_defaultRaceTrack;
    PhantomDrive::TrackData *m_selectedBuiltInTrack;
    PhantomDrive::TrackData *m_runtimeCustomTrack;
    QTimer *m_simTimer;
    QTimer *m_learningSessionTimer;   // auto-ends Learning Mode after max duration
    QTimer *m_countdownFinishTimer;
    QString m_currentMode;
    QString m_customTrackPath;
    QComboBox* m_trackSelectCombo;
    QComboBox* m_playerCountCombo;
    QLabel* m_trackDifficultyLabel;
    QLabel* m_trackDescriptionLabel;
    QLabel* m_coinChallengeHudLabel;
    QWidget* m_coinChallengeSummaryOverlay;
    QLabel* m_coinChallengeSummaryTitleLabel;
    QLabel* m_coinChallengeSummaryFlavorLabel;
    QLabel* m_coinChallengeSummaryRewardLabel;
    QLabel* m_coinChallengeSummaryCoinBagLabel;
    QLabel* m_coinChallengeSummaryDeltaLabel;
    QLabel* m_coinChallengeSummaryStatsLabel;
    QPushButton* m_coinChallengePlayAgainButton;
    QPushButton* m_coinChallengeExitButton;
    QTimer* m_coinChallengeSummaryAnimTimer;
    QString m_selectedTrackId;
    QString m_arcadeTrackId;
    QString m_twoPlayerTrackId;
    QString m_selectedAIDifficulty;
    bool m_twoPlayerMode;
    bool m_twoPlayerSetupActive;
    int m_currentSpeedLimit;
    QString m_currentTrafficLightState;
    bool m_driveActive;
    bool m_countdownActive;
    bool m_gamePaused;
    int m_sessionGeneration;
    int m_countdownRemainingMs;
    qint64 m_countdownTimerStartedMs;
    int m_countdownSessionGeneration;
    int m_learningTimerRemainingMs;
    qint64 m_learningTimerStartedMs;
    bool m_arcadeRaceFinished;
    bool m_customTrackPlaying;
    bool m_coinChallengeActive;
    bool m_coinChallengeSummaryVisible;
    bool m_coinChallengeTenSecondWarningShown;
    int m_coinChallengeCountdownAnnouncedStage;
    int m_lapsCompleted;
    int m_totalLaps;
    int m_coinChallengeDurationMs;
    int m_coinChallengeRemainingMs;
    int m_coinChallengeGoalCoins;
    int m_coinChallengeMaxSpeedKmh;
    int m_coinChallengeLastAverageSpeedKmh;
    int m_coinChallengeLastEfficiencyCpm;
    int m_coinChallengeLastRunCoins;
    qint64 m_coinChallengeWarningStartedMs;
    qint64 m_coinChallengeSpeedSampleSumKmh;
    qint64 m_coinChallengeSpeedSampleCount;
    qint64 m_coinChallengeLastElapsedMs;
    int m_coinChallengeMagnetRemainingMs;
    int m_coinChallengeBalloonRushRemainingMs;
    int m_coinChallengeBalloonRushTriggerRemainingMs;
    int m_coinChallengeBalloonRushReturnRemainingMs;
    int m_coinChallengeBalloonRushIntroCountdownRemainingMs;
    int m_coinChallengeBalloonRushIntroCountdownAnnounced;
    int m_coinChallengeBalloonRushCollectedCoins;
    int m_coinChallengeBalloonRushRecentGainValue;
    int m_coinChallengeBalloonRushGainFlashRemainingMs;
    int m_coinChallengeBalloonRushMilestoneRemainingMs;
    int m_coinChallengeBalloonRushMilestoneDurationMs;
    int m_coinChallengePowerupsUsed;
    int m_coinChallengeLoopsCompleted;
    int m_coinChallengeLoopCheckpointIndex;
    int m_coinChallengeMagnetSpawnStage;
    int m_coinChallengeFinalCountdownAnnouncedSecond;
    int m_coinChallengeSummaryAnimatedRunCoins;
    int m_coinChallengeSummaryAnimatedTotalCoins;
    int m_coinChallengeSummaryTargetRunCoins;
    int m_coinChallengeSummaryTargetTotalCoins;
    int m_coinChallengeSummaryDepositSoundStage;
    int m_coinChallengeVehicleIntegrity = 100;
    int m_coinChallengeObstacleHits = 0;
    int m_coinChallengeAICollisionCount = 0;
    int m_coinChallengeDamageTaken = 0;
    int m_coinChallengeRecentDamageAmount = 0;
    int m_coinChallengeDamageFlashRemainingMs = 0;
    int m_coinChallengeDamagePopupRemainingMs = 0;
    int m_coinChallengeLowIntegrityWarningRemainingMs = 0;
    qreal m_coinChallengeSummaryVisualProgress;
    qint64 m_coinChallengeLastHazardSoundMs;
    qint64 m_coinChallengeLastDamageMs = -1000;
    bool m_coinChallengeLeftStartZone;
    bool m_coinChallengeWasOnStartZone;
    bool m_coinChallengeBalloonRushSpawned;
    bool m_coinChallengeMagnetLoopPlaying;
    bool m_coinChallengeForcedEndActive = false;
    int m_coinChallengeForcedEndRemainingMs = 0;
    PhantomDrive::CoinChallengeEndReason m_coinChallengeEndReason = PhantomDrive::CoinChallengeEndReason::None;
    QString m_coinChallengeDamagePopupText;
    QString m_coinChallengeDamagePopupDetail;
    QString m_coinChallengeEndTitle;
    QString m_coinChallengeEndDetail;
    QSet<QString> m_coinChallengeBlockerDamageContacts;
    QSet<QString> m_coinChallengeObstacleDamageContacts;
    QVector2D m_coinChallengeLastSafePlayerPosition;
    qreal m_coinChallengeLastSafePlayerRotation = 0.0;
    bool m_coinChallengeHasSafePlayerPosition = false;
    QList<QVector2D> m_coinChallengeMagnetSpawnPool;
    QList<QVector2D> m_coinChallengeBalloonRushSpawnPool;
    PhantomDrive::CoinChallengeBalloonRushPhase m_coinChallengeBalloonRushPhase;
    QVector2D m_coinChallengeBalloonRushSavedPosition;
    qreal m_coinChallengeBalloonRushSavedRotation;
    qreal m_coinChallengeBalloonRushSavedSpeed;
    int m_coinChallengeBalloonRushLaneIndex;
    qreal m_coinChallengeBalloonRushLaneVisual;
    qreal m_coinChallengeBalloonRushRoadScroll;
    qreal m_coinChallengeBalloonRushSpawnAccumulatorMs;
    int m_coinChallengeBalloonRushPatternLane;
    int m_coinChallengeBalloonRushPatternBatchCount;
    QList<int> m_coinChallengeBalloonRushRecentSegmentLanes;
    QSet<int> m_coinChallengeBalloonRushVisitedLanes;
    QList<QPointF> m_coinChallengeBalloonRushCoinLayout;
    QString m_coinChallengeBalloonRushMilestoneHeadline;
    QString m_coinChallengeBalloonRushMilestoneDetail;
    QColor m_coinChallengeBalloonRushMilestoneAccent;
    QSet<int> m_coinChallengeTriggeredMilestones;
    int m_simTick;
    qint64 m_sessionElapsedMs;
    qint64 m_currentLapStartMs;
    qint64 m_bestLapMs;
    QVector2D m_playerPosition;
    QVector2D m_previousPlayerPosition;
    qreal m_playerRotation;
    qreal m_playerSpeed;
    QVector2D m_player2Position;
    QVector2D m_previousPlayer2Position;
    qreal m_player2Rotation;
    qreal m_player2Speed;
    int m_player2LapsCompleted;
    int m_player2NextCheckpointIndex;
    bool m_player2WasInsideNextGate;
    bool m_player1Finished;
    bool m_player2Finished;
    bool m_twoPlayerFinishHandled;
    PhantomDrive::ScoreReport m_player1PendingReport;
    PhantomDrive::ScoreReport m_player2PendingReport;
    QList<PhantomDrive::DrivingData> m_player1PendingSamples;
    QList<PhantomDrive::DrivingData> m_player2PendingSamples;
    bool m_arcadeRaceLogicActive;
    int m_nextCheckpointIndex;
    int m_raceCheckpointTotal;
    bool m_hasLeftNorthSector;
    bool m_wasOnStartLine;
    bool m_wasInNorthGate;
    bool m_blockCheckpointsUntilLeaveNorth;
    bool m_wasInsideNextGate;
    QSet<QString> m_playerVehicleContacts;
    QHash<QString, qreal> m_hudRaceProgressById;
    QString m_hudRaceRouteSignature;

    void setupGameView();
    void setupVehiclePhysics();
    void setupDataBindings();
    void setupDemoControls();
    void setupGaragePage();
    void setupCustomTrackControls();
    void setupRaceSetupControls();
    void styleMainMenu();
    void setGameHeaderVisible(bool visible);
    void clearTransientDrivingFeedback();
    void toggleGamePaused();
    void setGamePaused(bool paused);
    void updatePauseButtonState();
    void startCountdownFinishTimer(int remainingMs);
    void returnToMainMenuFromGame(bool finishWithReport);
    void exitApplicationFromGame();
    void initializeAIOpponents();
    void applyAIDifficultySelection();
    QString selectedAIDifficulty() const;
    void updateArcadeSetupTrackInfo();
    void loadCustomTrack();
    void showCustomTrackEditor();
    void hideCustomTrackEditor();
    void playCurrentCustomTrack();
    void startCustomTrackSession(PhantomDrive::TrackData* track);
    void showArcadeSetupDialog();
    void showTwoPlayerSetupDialog();
    void showCoinChallengeTrackDialog();
    void showGaragePage();
    void startCoinChallengeMode();
    void finishCoinChallengeMode();
    void saveCurrentCustomTrack();
    void loadCustomTrackIntoEditor();
    void exportCurrentCustomTrackJson();
    void restoreDefaultRaceTrack();
    void focusGameViewForDriving();
    void startDrivingSession(const QString& mode);
    void startBuiltInTrackSession(const QString& mode);
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
    bool isCoinChallengeModeActive() const;
    QRectF playerCoinCollectRect() const;
    void resetCoinChallengeStats();
    void updateCoinChallengeStats(int currentSpeedKmh);
    int coinChallengeAverageSpeedKmh() const;
    int coinChallengeMaxSpeedDisplayKmh() const;
    int coinChallengeEfficiencyCpm() const;
    qreal coinChallengeGoalProgress() const;
    int coinChallengeDesiredActiveCoins() const;
    int coinChallengeCountdownStage() const;
    qreal coinChallengeCountdownStageProgress() const;
    void updateCoinChallengeHud();
    void layoutCoinChallengeHud();
    void ensureCoinChallengeSummaryOverlay();
    void showCoinChallengeSummaryOverlay();
    void hideCoinChallengeSummaryOverlay();
    void exitCoinChallengeToMenu();
    void setupCoinChallengePowerupBoxes();
    bool isCoinChallengePowerupSpawnPointValid(const QVector2D& position, qreal minDistanceFromPlayers = 140.0f) const;
    bool tryTakeCoinChallengePowerupSpawnPoint(QList<QVector2D>& spawnPool, QVector2D& outPosition) const;
    void updateCoinChallengeLoopProgress(const QVector2D& positionBefore);
    void maybeSpawnCoinChallengeMagnet();
    void maybeSpawnCoinChallengeBalloonRush();
    void spawnCoinChallengeMagnetBox(const QVector2D& position);
    void spawnCoinChallengeBalloonRushBox(const QVector2D& position);
    void activateCoinChallengeBalloonRush();
    void updateCoinChallengeBalloonRush(int deltaMs);
    void enterCoinChallengeBalloonRushBonusScene();
    void finishCoinChallengeBalloonRush(bool completeSequence = true);
    void refreshCoinChallengeBalloonRushScene();
    void moveCoinChallengeBalloonRushLane(int direction);
    bool isCoinChallengeBalloonRushSequenceActive() const;
    bool isCoinChallengeBalloonRushSceneActive() const;
    void syncCoinChallengeMagnetLoop();
    void refreshCoinChallengeHazardVisuals();
    QList<QRectF> currentCoinChallengeBlockedAreas() const;
    QRectF playerHazardCollisionRect(const QVector2D& position) const;
    bool isCoinChallengePlayerPositionSafe(const QVector2D& position) const;
    void updateCoinChallengeLastSafePosition();
    void applyCoinChallengeHazardCollision(const QVector2D& positionBeforeUpdate);
    void updateCoinChallengeForcedEndSequence(int deltaMs);
    void applyCoinChallengeIntegrityDamage(int amount,
                                           const QString& popupText,
                                           const QString& popupDetail = QString());
    void triggerCoinChallengeForcedEnd(PhantomDrive::CoinChallengeEndReason reason,
                                       const QString& title,
                                       const QString& detail);
    int coinChallengeIntegrityStage() const;
    void refreshCoinChallengeSummaryVisuals();
    void animateCoinChallengeSummaryStep();
    void handleCoinChallengeMilestones(int runCoinsBeforeUpdate, int runCoinsAfterUpdate);
    void triggerCoinChallengeMilestone(int milestoneCoins);
    QString currentPlayerSkinId() const;
    QColor currentPlayerSkinColor() const;
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
    void handlePowerupCollectedForPlayer(PhantomDrive::PowerupType type, int playerIndex);
    void handleTrafficViolation(const PhantomDrive::ViolationEvent& violation);
    void applyPlayerSpawnAtStartLine();
    void syncRaceTrackToManager();
    void resetArcadeRaceProgress();
    void updateArcadeRaceProgress(const QVector2D& positionBefore);
    void updatePlayer2RaceProgress(const QVector2D& positionBefore);
    void finishCustomTrackRoute();
    void markTwoPlayerFinished(int playerIndex);
    void finishTwoPlayerRace();
    void resolvePlayerAiVehicleContact(PhantomDrive::AIOpponent* ai);
    void applyPlayer2SpawnAtStartLine();
    void updateTwoPlayerCamera();
    int selectedPlayerCount() const;
    bool isTwoPlayerSelected() const;
    void preparePlayerReportSystems();
    void showGuideDialog();
    void loadPlayerProfile();
    bool savePlayerProfile();
    void refreshGaragePage();
    void handleGaragePurchase(const QString& skinId);
    void handleGarageSelect(const QString& skinId);

private slots:
    void onDrivingDataCollected(const PhantomDrive::DrivingData& data);
    void onViolationDetected(const PhantomDrive::ViolationEvent& violation);
    void onScoreReady(const PhantomDrive::ScoreReport& report);
    void onCoachReportReady(const QString& markdown);
    void on_btn_History_clicked();
};

#endif // MAINWINDOW_H

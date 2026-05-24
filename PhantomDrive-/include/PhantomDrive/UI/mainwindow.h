#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QApplication>
#include "PhantomDrive_global.h"
#include "learninghud.h"
#include "UI/GameViewWidget.h"
#include "UI/DrivingReportWidget.h"
#include "gamemode/DrivingDataCollector.h"
#include "scoring/ScoreManager.h"
#include "core/datamodels.h"
#include "gamemode/AIOpponentManager.h"

class QPushButton;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class PHANTOMDRIVE_EXPORT MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateHUD(int speed, const QString &status);
    void setDrivingDataCollector(PhantomDrive::DrivingDataCollector* collector);

    PhantomDrive::GameViewWidget* getGameView() { return m_gameView; }

private:
    Ui::MainWindow *ui;
    LearningHUD *m_learningHUD;
    PhantomDrive::GameViewWidget *m_gameView;
    PhantomDrive::DrivingDataCollector *m_drivingDataCollector;
    PhantomDrive::ScoreManager *m_scoreManager;
    PhantomDrive::AIOpponentManager *m_aiManager;
    PhantomDrive::DrivingReportWidget *m_reportWidget;
    QPushButton *m_btnFinishDrive;
    QTimer *m_simTimer;
    QString m_currentMode;
    int m_currentSpeedLimit;
    QString m_currentTrafficLightState;
    bool m_driveActive;

    void setupGameView();
    void setupDataBindings();
    void setupDemoControls();
    void startDrivingSession(const QString& mode);
    void finishDrivingSession();
    void showReportWindow(const PhantomDrive::ScoreReport* report = nullptr);
    void updateGameViewFromData(const PhantomDrive::DrivingData& data);
    void updateTrafficAndHud(int tick);
    void simulateGameLoop();

private slots:
    void onDrivingDataCollected(const PhantomDrive::DrivingData& data);
    void onViolationDetected(const PhantomDrive::ViolationEvent& violation);
    void onScoreReady(const PhantomDrive::ScoreReport& report);
    void onCoachReportReady(const QString& markdown);
    void on_btn_History_clicked();
};

#endif // MAINWINDOW_H

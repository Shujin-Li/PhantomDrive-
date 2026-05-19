#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QApplication>
#include "PhantomDrive_global.h"
#include "learninghud.h"
#include "UI/GameViewWidget.h"
#include "gamemode/DrivingDataCollector.h"
#include "gamemode/SimpleAIOpponent.h"

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
    QList<PhantomDrive::SimpleAIOpponent*> m_aiOpponents;
    Ui::MainWindow *ui;
    LearningHUD *m_learningHUD;
    PhantomDrive::GameViewWidget *m_gameView;
    PhantomDrive::DrivingDataCollector *m_drivingDataCollector;
    QTimer *m_simTimer;
    QString m_currentMode;
    int m_currentSpeedLimit;
    QString m_currentTrafficLightState;
    void generateDemoWaypoints();

    void setupGameView();
    void setupDataBindings();
    void updateGameViewFromData(const PhantomDrive::DrivingData& data);
    void updateTrafficAndHud(int tick);
    void simulateGameLoop();

private slots:
    void onDrivingDataCollected(const PhantomDrive::DrivingData& data);
    void onViolationDetected(const PhantomDrive::ViolationEvent& violation);
    void on_btn_History_clicked();
};

#endif // MAINWINDOW_H

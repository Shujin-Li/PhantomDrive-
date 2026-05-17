#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QApplication>
#include "PhantomDrive_global.h"
#include "learninghud.h"
#include "UI/GameViewWidget.h"

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

    PhantomDrive::GameViewWidget* getGameView() { return m_gameView; }

private:
    Ui::MainWindow *ui;
    LearningHUD *m_learningHUD;
    PhantomDrive::GameViewWidget *m_gameView;

    void setupGameView();
    void simulateGameLoop();

private slots:
    void on_btn_History_clicked();
};

#endif // MAINWINDOW_H
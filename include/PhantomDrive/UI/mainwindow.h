#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QApplication>
#include "learninghud.h"  // 新增：包含学习模式HUD

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // [TO 引擎组] W3 数据对接核心接口
    void updateHUD(int speed, const QString &status);

private:
    Ui::MainWindow *ui;

    // 新增：学习模式HUD对象
    LearningHUD *m_learningHUD;

private slots:
    void on_btn_History_clicked();
};

#endif // MAINWINDOW_H
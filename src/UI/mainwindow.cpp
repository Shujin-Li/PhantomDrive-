#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QMessageBox>
#include "DrivingReportWidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_learningHUD(nullptr)  // 新增：初始化HUD指针
{
    ui->setupUi(this);

    // ========================================================
    // [W1 任务] 前端 UI 路由与页面切换控制 (已完成)
    // ========================================================

    // 1. 进入游戏模式 (切至 HUD 界面)
    if (ui->btn_Arcade) {
        connect(ui->btn_Arcade, &QPushButton::clicked, this, [this](){ ui->stackedWidget->setCurrentIndex(1); });
    }
    if (ui->btn_Learn) {
        connect(ui->btn_Learn, &QPushButton::clicked, this, [this](){
            ui->stackedWidget->setCurrentIndex(1);
            // 新增：学习模式时显示学习HUD
            if (m_learningHUD) m_learningHUD->show();
        });
    }

    // 2. [W2 任务预留] 驾驶历史数据面板
    // TO 负责数据库的同学：后续在这里接入 MySQL 查询逻辑，把 QMessageBox 换成新页面的跳转
    if (ui->btn_History) {
        connect(ui->btn_History, &QPushButton::clicked, this, [this](){
            QMessageBox::information(this, "系统提示", "W2阶段：历史数据看板（数据库联调）开发中...");
        });
    }

    // 3. 退出系统
    if (ui->btn_Exit) {
        connect(ui->btn_Exit, &QPushButton::clicked, this, &MainWindow::close);
    }

    // 4. 模拟完成，返回主菜单
    if (ui->btn_Back) {
        connect(ui->btn_Back, &QPushButton::clicked, this, [this](){ ui->stackedWidget->setCurrentIndex(0); });
    }
    // ========================================================
    // 新增：创建学习模式HUD（但不立即显示，等进入驾驶界面再显示）
    // ========================================================
    m_learningHUD = new LearningHUD();
    // 初始时隐藏，只有进入驾驶界面（街机/学习模式）才显示
    m_learningHUD->hide();

    // ========================================================
    // [W3 任务模拟] 引擎数据流预演
    // --------------------------------------------------------
    // TO 引擎组：这是一个用来假装是引擎的定时器。
    // 等你们把 DustRacing2D (或你们的底层引擎) 接进来后，
    // 请直接把这串 QTimer 代码删掉，换成你们引擎发出的 SIGNAL！
    // ========================================================
    QTimer *simTimer = new QTimer(this);
    connect(simTimer, &QTimer::timeout, this, [this](){
        static int fakeSpeed = 0;
        fakeSpeed = (fakeSpeed + 2) % 180; // 模拟 0-180 的加速

        // 只有在驾驶界面(page_2)才更新UI，节省系统开销
        if(ui->stackedWidget->currentIndex() == 1) {
            this->updateHUD(fakeSpeed, "行驶中");
        }
    });
    simTimer->start(100);
}

MainWindow::~MainWindow()
{
    // 新增：清理HUD对象
    if (m_learningHUD) {
        delete m_learningHUD;
        m_learningHUD = nullptr;}
    delete ui;
}

// ========================================================
// [前端与 AI 逻辑融合区] HUD 界面实时更新
// ========================================================
void MainWindow::updateHUD(int speed, const QString &status) {
    if (ui->label_Speed) {
        ui->label_Speed->setText(QString("速度: %1 km/h").arg(speed));

        // ----------------------------------------------------
        // [TO AI 组 / 评分组]
        // 这里的阈值(80)目前是写死的。后续请接入你们的惩罚模型、
        // 轨迹评价算法或联邦学习预测结果，在这里根据扣分情况改变 UI 颜色。
        // ----------------------------------------------------
        if (speed > 80) {
            ui->label_Speed->setStyleSheet("color: red; font-size: 18px; font-weight: bold;");
            // ui->label_Signal->setText("警告：已超速！"); // 预留红绿灯/警告标签
        } else {
            ui->label_Speed->setStyleSheet("color: #27ae60; font-size: 16px;"); // 柔和的绿色
            // ui->label_Signal->setText(status);
        }
    }
}
// ========================================================
// 新增：将引擎数据传递给学习HUD（用于后续扩展）
// 目前引擎组只提供了速度和状态，暂时用不到
// 等引擎组提供限速、红绿灯、扣分信号后，这里可以删除
// ========================================================
// if (m_learningHUD && m_learningHUD->isVisible()) {
//     // 预留：未来可以根据速度判断超速扣分
//     if (speed > 80) {
//         // m_learningHUD->showPenaltyMessage("超速行驶", 5);
//     }
// }

void MainWindow::on_btn_History_clicked()
{
    // 创建报告窗口实例
    // 注意：这里不设 parent，让它作为一个独立的弹出窗口
    DrivingReportWidget *reportWindow = new DrivingReportWidget();

    // 设置窗口标题（会显示在弹出窗口的边框上）
    reportWindow->setWindowTitle("Phantom Drive - 历史行驶数据分析");

    // 设置窗口大小
    reportWindow->resize(1000, 700);

    // 关键：设置窗口属性，确保关闭时能释放内存，避免程序运行久了占用过多资源
    reportWindow->setAttribute(Qt::WA_DeleteOnClose);

    // 如果你想让它弹出时带有那种“置顶”的科技感，可以取消下面这行的注释
    reportWindow->setWindowModality(Qt::ApplicationModal);

    // 显示窗口
    reportWindow->show();
}

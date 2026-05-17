#include "UI/mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QLabel>
#include "UI/DrivingReportWidget.h"
#include "track/TrackManager.h"
#include "gamemode/TrafficObjectManager.h"
#include "gamemode/PowerupManager.h"

using namespace PhantomDrive;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_learningHUD(nullptr)
    , m_gameView(nullptr)
{
    ui->setupUi(this);

    setupGameView();

    if (ui->btn_Arcade) {
        connect(ui->btn_Arcade, &QPushButton::clicked, this, [this](){
            ui->stackedWidget->setCurrentIndex(1);
            if (m_gameView) m_gameView->show();
            if (m_learningHUD) m_learningHUD->show();
        });
    }
    if (ui->btn_Learn) {
        connect(ui->btn_Learn, &QPushButton::clicked, this, [this](){
            ui->stackedWidget->setCurrentIndex(1);
            if (m_gameView) m_gameView->show();
            if (m_learningHUD) m_learningHUD->show();
        });
    }

    if (ui->btn_History) {
        connect(ui->btn_History, &QPushButton::clicked, this, [this](){
            QMessageBox::information(this, "系统提示", "W2阶段：历史数据看板（数据库联调）开发中...");
        });
    }

    if (ui->btn_Exit) {
        connect(ui->btn_Exit, &QPushButton::clicked, this, &MainWindow::close);
    }

    if (ui->btn_Back) {
        connect(ui->btn_Back, &QPushButton::clicked, this, [this](){
            ui->stackedWidget->setCurrentIndex(0);
            if (m_gameView) m_gameView->hide();
            if (m_learningHUD) m_learningHUD->hide();
        });
    }

    m_learningHUD = new LearningHUD();
    m_learningHUD->hide();

    simulateGameLoop();
}

MainWindow::~MainWindow()
{
    if (m_learningHUD) {
        delete m_learningHUD;
        m_learningHUD = nullptr;
    }
    delete ui;
}

void MainWindow::setupGameView()
{
    m_gameView = new GameViewWidget(this);
    m_gameView->hide();

    if (ui->stackedWidget && ui->stackedWidget->count() > 1) {
        QWidget* gamePage = ui->stackedWidget->widget(1);
        if (gamePage) {
            QVBoxLayout* layout = new QVBoxLayout(gamePage);
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(m_gameView);
            gamePage->setLayout(layout);
        }
    }

    TrackManager* trackMgr = TrackManager::instance(this);
    if (trackMgr && trackMgr->hasCurrentTrack()) {
        m_gameView->setTrackData(trackMgr->getCurrentTrack());
    } else {
        TrackData* testTrack = new TrackData();
        testTrack->setId("test_track");
        testTrack->setName("Test Track");
        testTrack->setSize(30, 20);

        for (int row = 0; row < 30; ++row) {
            for (int col = 0; col < 20; ++col) {
                TrackTile* tile = new TrackTile();
                tile->setPosition(row, col);

                bool isRoad = false;
                if (col >= 8 && col <= 11) isRoad = true;
                if (row >= 5 && row <= 8 && col >= 6 && col <= 13) isRoad = true;
                if (row >= 15 && row <= 18 && col >= 6 && col <= 13) isRoad = true;
                if (row >= 22 && row <= 25 && col >= 8 && col <= 11) isRoad = true;

                if (row == 5 && col >= 8 && col <= 11) {
                    tile->setType(TileType::StartLine);
                } else if (row == 25 && col >= 8 && col <= 11) {
                    tile->setType(TileType::FinishLine);
                } else if (isRoad) {
                    tile->setType(TileType::Road);
                } else {
                    tile->setType(TileType::Grass);
                }

                testTrack->setTileAt(row, col, tile);
            }
        }

        m_gameView->setTrackData(testTrack);
    }
}

void MainWindow::simulateGameLoop()
{
    QTimer *simTimer = new QTimer(this);
    connect(simTimer, &QTimer::timeout, this, [this](){
        static int fakeSpeed = 0;
        static qreal carX = 0;
        static qreal carY = 0;
        static qreal carRotation = 0;
        static int tick = 0;

        fakeSpeed = (fakeSpeed + 2) % 180;
        tick++;

        carX += qCos(carRotation * M_PI / 180.0) * 2.0;
        carY += qSin(carRotation * M_PI / 180.0) * 2.0;

        if (tick % 60 == 0) {
            carRotation += 15.0;
        }

        if (tick == 1) {
            m_gameView->addPowerupBox("powerup_1", QVector2D(200, 150), "Shield");
            m_gameView->addPowerupBox("powerup_2", QVector2D(400, 300), "Boost");
            m_gameView->addTrafficLight("light_1", QVector2D(300, 200), "green");
            m_gameView->addSpeedLimitSign("sign_1", QVector2D(250, 100), 60);
            m_gameView->addPedestrianCrossing("crossing_1", QVector2D(350, 250), QSizeF(80, 40));
            m_gameView->updateAICar("ai_1", QVector2D(150, 200), 45, 80);
        }

        if (tick % 120 == 0) {
            QString lightState = (tick % 240 == 0) ? "red" : "green";
            m_gameView->updateTrafficLight("light_1", lightState);
        }

        if (m_gameView) {
            m_gameView->updatePlayerCar(QVector2D(carX, carY), carRotation, fakeSpeed);
            m_gameView->updateAICar("ai_1", QVector2D(carX + 100, carY + 50), carRotation + 10, 80);
            m_gameView->setCameraPosition(QVector2D(carX, carY));
        }

        if(ui->stackedWidget->currentIndex() == 1) {
            this->updateHUD(fakeSpeed, "行驶中");
        }
    });
    simTimer->start(50);
}

void MainWindow::updateHUD(int speed, const QString &status) {
    if (ui->label_Speed) {
        ui->label_Speed->setText(QString("速度: %1 km/h").arg(speed));

        if (speed > 80) {
            ui->label_Speed->setStyleSheet("color: red; font-size: 18px; font-weight: bold;");
        } else {
            ui->label_Speed->setStyleSheet("color: #27ae60; font-size: 16px;");
        }
    }
}

void MainWindow::on_btn_History_clicked()
{
    DrivingReportWidget *reportWindow = new DrivingReportWidget();
    reportWindow->setWindowTitle("Phantom Drive - 历史行驶数据分析");
    reportWindow->resize(1000, 700);
    reportWindow->setAttribute(Qt::WA_DeleteOnClose);
    reportWindow->setWindowModality(Qt::ApplicationModal);
    reportWindow->show();
}

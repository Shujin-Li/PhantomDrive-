#include "UI/mainwindow.h"
#include "ui_mainwindow.h"

#include "UI/DrivingReportWidget.h"
#include "gamemode/VehicleSensor.h"
#include "track/TrackData.h"
#include "track/TrackManager.h"
#include "track/TrackTile.h"

#include <QDateTime>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QtMath>

using namespace PhantomDrive;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_learningHUD(nullptr)
    , m_gameView(nullptr)
    , m_drivingDataCollector(new DrivingDataCollector(this))
    , m_simTimer(nullptr)
    , m_currentMode("Arcade")
    , m_currentSpeedLimit(60)
    , m_currentTrafficLightState("green")
{
    ui->setupUi(this);

    setupGameView();
    setupDataBindings();

    SimpleAIOpponent* ai1 =
        new SimpleAIOpponent("ai_1", this);

    SimpleAIOpponent* ai2 =
        new SimpleAIOpponent("ai_2", this);

    SimpleAIOpponent* ai3 =
        new SimpleAIOpponent("ai_3", this);


    AIConfig aggressiveConfig;
    aggressiveConfig.style = AIStyle::Aggressive;
    aggressiveConfig.maxSpeed = 220.0;

    AIConfig conservativeConfig;
    conservativeConfig.style = AIStyle::Conservative;
    conservativeConfig.maxSpeed = 140.0;

    AIConfig normalConfig;
    normalConfig.style = AIStyle::Normal;
    normalConfig.maxSpeed = 180.0;

    ai1->setConfig(aggressiveConfig);
    ai2->setConfig(conservativeConfig);
    ai3->setConfig(normalConfig);


    ai1->setPosition(QVector2D(200, 200));
    ai2->setPosition(QVector2D(170, 220));
    ai3->setPosition(QVector2D(230, 180));

    m_aiOpponents.append(ai1);
    m_aiOpponents.append(ai2);
    m_aiOpponents.append(ai3);

    generateDemoWaypoints();

    m_learningHUD = new LearningHUD(this);
    m_learningHUD->hide();

    if (ui->btn_Arcade) {
        connect(ui->btn_Arcade, &QPushButton::clicked, this, [this]() {
            m_currentMode = "Arcade";
            ui->stackedWidget->setCurrentIndex(1);
            if (m_gameView) {
                m_gameView->show();
            }
            if (m_learningHUD) {
                m_learningHUD->hide();
            }
            statusBar()->showMessage("Arcade mode running");
        });
    }

    if (ui->btn_Learn) {
        connect(ui->btn_Learn, &QPushButton::clicked, this, [this]() {
            m_currentMode = "Learning";
            ui->stackedWidget->setCurrentIndex(1);
            if (m_gameView) {
                m_gameView->show();
            }
            if (m_learningHUD) {
                m_learningHUD->show();
            }
            statusBar()->showMessage("Learning mode running");
        });
    }

    if (ui->btn_History) {
        connect(ui->btn_History, &QPushButton::clicked, this, &MainWindow::on_btn_History_clicked);
    }

    if (ui->btn_Exit) {
        connect(ui->btn_Exit, &QPushButton::clicked, this, &MainWindow::close);
    }

    if (ui->btn_Back) {
        connect(ui->btn_Back, &QPushButton::clicked, this, [this]() {
            ui->stackedWidget->setCurrentIndex(0);
            if (m_gameView) {
                m_gameView->hide();
            }
            if (m_learningHUD) {
                m_learningHUD->hide();
            }
            statusBar()->clearMessage();
        });
    }

    m_drivingDataCollector->setVehicleId("player");
    m_drivingDataCollector->setSamplingInterval(50);
    m_drivingDataCollector->setCurrentSpeedLimit(m_currentSpeedLimit, "demo_limit_zone");
    m_drivingDataCollector->startCollection();

    simulateGameLoop();
}

MainWindow::~MainWindow()
{
    if (m_drivingDataCollector) {
        m_drivingDataCollector->stopCollection();
    }
    delete ui;
}

void MainWindow::setDrivingDataCollector(DrivingDataCollector* collector)
{
    if (!collector || collector == m_drivingDataCollector) {
        return;
    }

    if (m_drivingDataCollector && m_drivingDataCollector->parent() == this) {
        m_drivingDataCollector->stopCollection();
        m_drivingDataCollector->deleteLater();
    }

    m_drivingDataCollector = collector;
    setupDataBindings();
}

void MainWindow::setupDataBindings()
{
    if (!m_drivingDataCollector) {
        return;
    }

    connect(m_drivingDataCollector, &IDrivingDataCollector::dataCollected,
            this, &MainWindow::onDrivingDataCollected, Qt::UniqueConnection);
    connect(m_drivingDataCollector, &IDrivingDataCollector::violationDetected,
            this, &MainWindow::onViolationDetected, Qt::UniqueConnection);
}

void MainWindow::setupGameView()
{
    m_gameView = new GameViewWidget(this);
    m_gameView->hide();

    if (ui->stackedWidget && ui->stackedWidget->count() > 1) {
        QWidget* gamePage = ui->stackedWidget->widget(1);
        if (gamePage) {
            QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(gamePage->layout());
            if (!layout) {
                layout = new QVBoxLayout(gamePage);
                layout->setContentsMargins(0, 32, 0, 0);
                layout->setSpacing(0);
                gamePage->setLayout(layout);
            }
            layout->addWidget(m_gameView);
        }
    }

    TrackManager* trackMgr = TrackManager::instance(this);
    if (trackMgr && trackMgr->hasCurrentTrack()) {
        m_gameView->setTrackData(trackMgr->getCurrentTrack());
        return;
    }

    TrackData* testTrack = new TrackData();
    testTrack->setId("ua_demo_track");
    testTrack->setName("UA Demo Track");
    testTrack->setSize(30, 20);

    for (int row = 0; row < 30; ++row) {
        for (int col = 0; col < 20; ++col) {
            TrackTile* tile = new TrackTile();
            tile->setPosition(row, col);

            const bool isRoad = (col >= 8 && col <= 11)
                || (row >= 5 && row <= 8 && col >= 6 && col <= 13)
                || (row >= 15 && row <= 18 && col >= 6 && col <= 13)
                || (row >= 22 && row <= 25 && col >= 8 && col <= 11);

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
    m_gameView->addPowerupBox("powerup_1", QVector2D(200, 150), "Shield");
    m_gameView->addPowerupBox("powerup_2", QVector2D(400, 300), "Boost");
    m_gameView->addTrafficLight("light_1", QVector2D(300, 200), m_currentTrafficLightState);
    m_gameView->addSpeedLimitSign("sign_1", QVector2D(250, 100), m_currentSpeedLimit);
    m_gameView->addPedestrianCrossing("crossing_1", QVector2D(350, 250), QSizeF(80, 40));
}

void MainWindow::generateDemoWaypoints()
{
    QList<PhantomDrive::Waypoint> waypoints;

    PhantomDrive::Waypoint wp1;
    wp1.position = QVector2D(200, 200);

    PhantomDrive::Waypoint wp2;
    wp2.position = QVector2D(400, 250);
    wp2.isCorner = true;
    wp2.cornerSeverity = 2;
    wp2.preferredSpeed = 120;

    PhantomDrive::Waypoint wp3;
    wp3.position = QVector2D(450, 450);

    PhantomDrive::Waypoint wp4;
    wp4.position = QVector2D(250, 500);
    wp4.isCorner = true;
    wp4.cornerSeverity = 3;
    wp4.preferredSpeed = 80;

    waypoints.append(wp1);
    waypoints.append(wp2);
    waypoints.append(wp3);
    waypoints.append(wp4);

    for (int i = 0; i < m_aiOpponents.size(); ++i)
    {
        m_aiOpponents[i]->setWaypoints(waypoints);
    }
}

void MainWindow::simulateGameLoop()
{
    m_simTimer = new QTimer(this);

    connect(m_simTimer, &QTimer::timeout, this, [this]() {
        static qreal carX = 320.0;
        static qreal carY = 320.0;
        static qreal carRotation = -90.0;
        static qreal speed = 0.0;
        static int tick = 0;

        ++tick;
        speed = 45.0 + 35.0 * qSin(tick / 18.0);
        if (tick % 220 > 160) {
            speed += 35.0;
        }

        if (tick % 70 == 0) {
            carRotation += 22.5;
        }

        const qreal radians = qDegreesToRadians(carRotation);
        const QVector2D velocity(qCos(radians) * speed, qSin(radians) * speed);
        carX += velocity.x() * 0.018;
        carY += velocity.y() * 0.018;

        if (carX < 80 || carX > 560 || carY < 80 || carY > 760) {
            carRotation += 120.0;
            carX = qBound(90.0, carX, 550.0);
            carY = qBound(90.0, carY, 750.0);
        }

        updateTrafficAndHud(tick);
        for (int i = 0; i < m_aiOpponents.size(); ++i)
        {
            SimpleAIOpponent* ai = m_aiOpponents[i];

            ai->update(50);

            m_gameView->updateAICar(
                QString("ai_%1").arg(i + 1),
                ai->getPosition(),
                ai->getRotation(),
                ai->getSpeed()
                );

            qDebug()
                << QString("AI_%1").arg(i + 1)
                << "Style:" << static_cast<int>(ai->getStyle())
                << "Speed:" << ai->getSpeed()
                << "State:" << static_cast<int>(ai->getState())
                << "WP:" << ai->getCurrentWaypointIndex();


        }


        if (m_drivingDataCollector && m_drivingDataCollector->vehicleSensor()) {
            VehicleSensor* sensor = m_drivingDataCollector->vehicleSensor();
            sensor->updatePosition(QVector2D(carX, carY));
            sensor->updateVelocity(velocity);
            sensor->updateRotation(carRotation);
            sensor->updateSpeedLimit(m_currentSpeedLimit, "demo_limit_zone");
            sensor->updateAcceleratorState(speed >= m_drivingDataCollector->getCurrentData().speed);
            sensor->updateBrakeState(speed < m_drivingDataCollector->getCurrentData().speed);
        }
    });

    m_simTimer->start(50);
}

void MainWindow::updateTrafficAndHud(int tick)
{
    if (tick % 180 == 0) {
        m_currentTrafficLightState = (m_currentTrafficLightState == "green") ? "yellow"
            : (m_currentTrafficLightState == "yellow") ? "red" : "green";
        if (m_gameView) {
            m_gameView->updateTrafficLight("light_1", m_currentTrafficLightState);
        }
    }

    if (tick % 300 == 0) {
        m_currentSpeedLimit = (m_currentSpeedLimit == 60) ? 80 : 60;
        if (m_gameView) {
            m_gameView->updateSpeedLimitSign("sign_1", m_currentSpeedLimit);
        }
    }

    const int remainingSeconds = 9 - ((tick / 20) % 10);
    if (m_learningHUD) {
        m_learningHUD->updateSpeedLimit(m_currentSpeedLimit);
        m_learningHUD->updateTrafficLight(m_currentTrafficLightState, remainingSeconds);
    }
}

void MainWindow::onDrivingDataCollected(const DrivingData& data)
{
    updateGameViewFromData(data);
    updateHUD(qRound(data.speed), data.isBraking ? "Braking" : "Driving");
}

void MainWindow::onViolationDetected(const ViolationEvent& violation)
{
    if (m_learningHUD && ui->stackedWidget->currentIndex() == 1 && m_currentMode == "Learning") {
        m_learningHUD->showPenaltyMessage(violation.description, violation.penaltyPoints);
    }
    statusBar()->showMessage(violation.description, 3000);
}

void MainWindow::updateGameViewFromData(const DrivingData& data)
{
    if (!m_gameView) {
        return;
    }

    m_gameView->updatePlayerCar(
        data.position,
        data.rotation,
        data.speed
        );


}

void MainWindow::updateHUD(int speed, const QString &status)
{
    if (ui->label_Speed) {
        ui->label_Speed->setText(QString("Speed: %1 km/h").arg(speed));
        ui->label_Speed->setStyleSheet(speed > m_currentSpeedLimit
            ? "color: #e74c3c; font-size: 16px; font-weight: bold;"
            : "color: #27ae60; font-size: 16px; font-weight: bold;");
    }

    if (ui->label_Limit) {
        ui->label_Limit->setText(QString("Limit: %1 km/h").arg(m_currentSpeedLimit));
    }

    if (ui->label_) {
        ui->label_->setText(QString("Light: %1").arg(m_currentTrafficLightState));
    }


    statusBar()->showMessage(QString("%1 | %2 mode").arg(status, m_currentMode));
}

void MainWindow::on_btn_History_clicked()
{
    DrivingReportWidget *reportWindow = new DrivingReportWidget();
    reportWindow->setWindowTitle("PhantomDrive - Driving Data Report");
    reportWindow->resize(1000, 700);
    reportWindow->setAttribute(Qt::WA_DeleteOnClose);
    reportWindow->setWindowModality(Qt::ApplicationModal);
    reportWindow->show();
}

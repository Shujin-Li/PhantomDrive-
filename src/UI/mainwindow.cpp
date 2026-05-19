#include "UI/mainwindow.h"
#include "ui_mainwindow.h"

#include "UI/DrivingReportWidget.h"
#include "UI/ThemeManager.h"
#include "gamemode/VehicleSensor.h"
#include "track/TrackData.h"
#include "track/TrackManager.h"
#include "track/TrackTile.h"
#include "scoring/ScoreReport.h"
#include "core/saveloadmanager.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayoutItem>
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
    , m_scoreManager(new ScoreManager(this))
    , m_reportWidget(new DrivingReportWidget(this))
    , m_btnFinishDrive(nullptr)
    , m_simTimer(nullptr)
    , m_currentMode("Arcade")
    , m_currentSpeedLimit(60)
    , m_currentTrafficLightState("green")
    , m_driveActive(false)
{
    ui->setupUi(this);

    setupGameView();
    setupDemoControls();
    setupDataBindings();

    m_learningHUD = new LearningHUD(this);
    m_learningHUD->hide();

    m_reportWidget->setMockDataEnabled(false);
    m_reportWidget->hide();
    m_scoreManager->setVehicleId("player");

    if (ui->btn_Arcade) {
        connect(ui->btn_Arcade, &QPushButton::clicked, this, [this]() {
            startDrivingSession("Arcade");
        });
    }

    if (ui->btn_Learn) {
        connect(ui->btn_Learn, &QPushButton::clicked, this, [this]() {
            startDrivingSession("Learning");
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
            if (m_driveActive) {
                finishDrivingSession();
            }
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
    m_drivingDataCollector->setCurrentSpeedLimit(m_currentSpeedLimit, "main_route_speed_zone");

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
    connect(m_drivingDataCollector, &IDrivingDataCollector::violationDetected,
            m_scoreManager, &ScoreManager::onViolationDetected, Qt::UniqueConnection);

    if (m_scoreManager) {
        connect(m_scoreManager, &ScoreManager::scoreReady,
                this, &MainWindow::onScoreReady, Qt::UniqueConnection);
        connect(m_scoreManager, &ScoreManager::coachReportReady,
                this, &MainWindow::onCoachReportReady, Qt::UniqueConnection);
        connect(m_scoreManager, &ScoreManager::scoringFailed,
                this, [this](const QString& reason) {
                    statusBar()->showMessage(QStringLiteral("Scoring failed: %1").arg(reason), 5000);
                });
    }
}

void MainWindow::setupDemoControls()
{
    if (ui->btn_History) {
        ui->btn_History->setText(QStringLiteral("Driving Report / History"));
    }

    m_btnFinishDrive = new QPushButton(QStringLiteral("Finish Drive"), this);
    m_btnFinishDrive->setObjectName(QStringLiteral("btn_FinishDrive"));
    m_btnFinishDrive->setEnabled(false);

    QWidget* gamePage = ui->stackedWidget ? ui->stackedWidget->widget(1) : nullptr;
    QVBoxLayout* pageLayout = gamePage ? qobject_cast<QVBoxLayout*>(gamePage->layout()) : nullptr;
    if (pageLayout && pageLayout->count() > 0) {
        QLayoutItem* firstItem = pageLayout->itemAt(0);
        QHBoxLayout* hudLayout = firstItem ? qobject_cast<QHBoxLayout*>(firstItem->layout()) : nullptr;
        if (hudLayout) {
            hudLayout->addWidget(m_btnFinishDrive);
        }
    }

    connect(m_btnFinishDrive, &QPushButton::clicked,
            this, &MainWindow::finishDrivingSession);
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
    testTrack->setId("main_city_training_route");
    testTrack->setName("Main City Training Route");
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

void MainWindow::simulateGameLoop()
{
    m_simTimer = new QTimer(this);

    connect(m_simTimer, &QTimer::timeout, this, [this]() {
        if (!m_driveActive) {
            return;
        }

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

        if (m_drivingDataCollector && m_drivingDataCollector->vehicleSensor()) {
            VehicleSensor* sensor = m_drivingDataCollector->vehicleSensor();
            sensor->updatePosition(QVector2D(carX, carY));
            sensor->updateVelocity(velocity);
            sensor->updateRotation(carRotation);
            sensor->updateSpeedLimit(m_currentSpeedLimit, "main_route_speed_zone");
            sensor->updateAcceleratorState(speed >= m_drivingDataCollector->getCurrentData().speed);
            sensor->updateBrakeState(speed < m_drivingDataCollector->getCurrentData().speed);
        }

        if (m_reportWidget && m_reportWidget->isVisible()) {
            m_reportWidget->addSpeedData(speed, tick);
        }
    });

    m_simTimer->start(50);
}

void MainWindow::startDrivingSession(const QString& mode)
{
    m_currentMode = mode;
    m_driveActive = true;

    if (m_drivingDataCollector) {
        m_drivingDataCollector->stopCollection();
        m_drivingDataCollector->clearData();
        m_drivingDataCollector->setCurrentSpeedLimit(m_currentSpeedLimit, "main_route_speed_zone");
        m_drivingDataCollector->startCollection();
    }

    ui->stackedWidget->setCurrentIndex(1);
    if (m_gameView) {
        m_gameView->show();
    }
    if (m_learningHUD) {
        if (m_currentMode == "Learning") {
            m_learningHUD->show();
        } else {
            m_learningHUD->hide();
        }
        m_learningHUD->updateGameMode(m_currentMode);
    }
    if (m_btnFinishDrive) {
        m_btnFinishDrive->setEnabled(true);
    }

    statusBar()->showMessage(QStringLiteral("%1 mode running").arg(m_currentMode));
}

void MainWindow::finishDrivingSession()
{
    if (!m_driveActive || !m_scoreManager || !m_drivingDataCollector) {
        return;
    }

    m_driveActive = false;
    m_drivingDataCollector->stopCollection();
    if (m_btnFinishDrive) {
        m_btnFinishDrive->setEnabled(false);
    }

    statusBar()->showMessage(QStringLiteral("Scoring current driving session..."));
    m_scoreManager->evaluateFromCollector(m_drivingDataCollector);
}

void MainWindow::showReportWindow(const ScoreReport* report)
{
    if (!m_reportWidget || !m_reportWidget->isVisible()) {
        if (m_reportWidget && m_reportWidget->parent() == nullptr) {
            m_reportWidget->deleteLater();
        }
        m_reportWidget = new DrivingReportWidget(nullptr);
        m_reportWidget->setAttribute(Qt::WA_DeleteOnClose);
        m_reportWidget->setMockDataEnabled(false);
        m_reportWidget->setMinimumSize(980, 760);
        m_reportWidget->resize(980, 760);
        connect(m_reportWidget, &QObject::destroyed, this, [this]() {
            m_reportWidget = nullptr;
        });
    }

    m_reportWidget->loadHistoryFromSaveLoadManager();
    if (report) {
        m_reportWidget->setCurrentReport(*report);
    }
    m_reportWidget->show();
    m_reportWidget->raise();
    m_reportWidget->activateWindow();
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
        const qreal currentSpeed = m_drivingDataCollector
            ? m_drivingDataCollector->getCurrentData().speed
            : 0.0;

        m_learningHUD->updateCurrentSpeed(currentSpeed);
        m_learningHUD->updateSpeedLimit(m_currentSpeedLimit);
        m_learningHUD->updateSpeedStatus(currentSpeed > m_currentSpeedLimit);
        m_learningHUD->updateTrafficLight(m_currentTrafficLightState, remainingSeconds);
        m_learningHUD->updateGameMode(m_currentMode);
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

void MainWindow::onScoreReady(const ScoreReport& report)
{
    SaveLoadManager::instance().saveReport(report);
    showReportWindow(&report);
    if (m_scoreManager) {
        m_scoreManager->generateCoachReport(report);
    }
    statusBar()->showMessage(QStringLiteral("Driving report ready: %1 (%2)")
                                 .arg(report.totalScore, 0, 'f', 1)
                                 .arg(report.grade), 5000);
}

void MainWindow::onCoachReportReady(const QString& markdown)
{
    if (m_reportWidget) {
        m_reportWidget->setCoachReportMarkdown(markdown);
    }
}

void MainWindow::updateGameViewFromData(const DrivingData& data)
{
    if (!m_gameView) {
        return;
    }

    m_gameView->updatePlayerCar(data.position, data.rotation, data.speed);
    m_gameView->updateAICar("ai_1",
                            data.position + QVector2D(120, 56),
                            data.rotation + 8.0,
                            qMax(35.0, data.speed * 0.88));
    m_gameView->setCameraPosition(data.position);
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
    showReportWindow();
}



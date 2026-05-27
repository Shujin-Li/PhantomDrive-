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

#include <QApplication>
#include <QComboBox>
#include <QDateTime>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QMessageBox>
#include <QPushButton>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QtMath>

#include "UI/SoundGenerator.h"

using namespace PhantomDrive;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_learningHUD(nullptr)
    , m_arcadeHUD(nullptr)
    , m_gameView(nullptr)
    , m_drivingDataCollector(new DrivingDataCollector(this))
    , m_scoreManager(new ScoreManager(this))
    , m_aiManager(new AIOpponentManager(this))
    , m_reportWidget(new DrivingReportWidget(this))
    , m_btnFinishDrive(nullptr)
    , m_aiDifficultyCombo(nullptr)
    , m_btnLoadCustomTrack(nullptr)
    , m_simTimer(nullptr)
    , m_currentMode("Arcade")
    , m_customTrackPath()
    , m_currentSpeedLimit(60)
    , m_currentTrafficLightState("green")
    , m_driveActive(false)
    , m_arcadeRaceFinished(false)
    , m_currentLap(1)
    , m_totalLaps(3)
    , m_simTick(0)
    , m_sessionElapsedMs(0)
    , m_currentLapStartMs(0)
    , m_bestLapMs(0)
    , m_playerPosition(320.0, 320.0)
    , m_playerRotation(-90.0)
    , m_playerSpeed(0.0)
{
    ui->setupUi(this);

    setupGameView();
    setupDemoControls();
    setupDataBindings();
    connect(m_scoreManager,
            &ScoreManager::qLearningFeedbackReady,
            m_aiManager,
            &AIOpponentManager::onQLearningFeedbackReady);

    connect(m_scoreManager, &ScoreManager::feedbackReady,
            this, [this](const QString& text, int, const QString& severity) {
                const FeedbackType type = severity == QStringLiteral("positive")
                    ? FeedbackType::Positive
                    : severity == QStringLiteral("danger")
                        ? FeedbackType::Critical
                        : FeedbackType::Warning;
                showInteractiveFeedback(text, type);
            });

    connect(m_aiManager, &AIOpponentManager::rankingsUpdated,
            this, [this](const QList<QString>&) {
                updateRaceHud();
            });

    connect(m_aiManager, &AIOpponentManager::opponentFinished,
            this, [this](const QString& opponentId, int finalPosition) {
                showInteractiveFeedback(QString("%1 finished P%2").arg(opponentId).arg(finalPosition),
                                        FeedbackType::Milestone);
            });

    qDebug() << "=== QLearning Connected ===";

    qApp->setStyleSheet(ThemeManager::getStyleSheet("dark"));

    PhantomDrive::InteractiveFeedback::instance(this);
    PhantomDrive::SoundManager::instance(this);
    PhantomDrive::SoundGenerator::instance().generateAllSounds();

    m_arcadeHUD = new PhantomDrive::ArcadeHUD(this);
    m_arcadeHUD->hide();

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

    initializeAIOpponents();
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

    if (!m_aiDifficultyCombo && ui->verticalLayout) {
        QLabel* difficultyLabel = new QLabel(QStringLiteral("AI Difficulty"), this);
        difficultyLabel->setObjectName(QStringLiteral("label_AIDifficulty"));
        difficultyLabel->setAlignment(Qt::AlignCenter);

        m_aiDifficultyCombo = new QComboBox(this);
        m_aiDifficultyCombo->setObjectName(QStringLiteral("combo_AIDifficulty"));
        m_aiDifficultyCombo->addItem(QStringLiteral("Easy"), QStringLiteral("easy"));
        m_aiDifficultyCombo->addItem(QStringLiteral("Medium"), QStringLiteral("medium"));
        m_aiDifficultyCombo->addItem(QStringLiteral("Hard"), QStringLiteral("hard"));
        m_aiDifficultyCombo->addItem(QStringLiteral("Adaptive"), QStringLiteral("adaptive"));
        m_aiDifficultyCombo->setCurrentIndex(1);

        m_btnLoadCustomTrack = new QPushButton(QStringLiteral("Load Custom Track"), this);
        m_btnLoadCustomTrack->setObjectName(QStringLiteral("btn_LoadCustomTrack"));

        ui->verticalLayout->insertWidget(0, difficultyLabel);
        ui->verticalLayout->insertWidget(1, m_aiDifficultyCombo);
        ui->verticalLayout->insertWidget(2, m_btnLoadCustomTrack);

        connect(m_aiDifficultyCombo, qOverload<int>(&QComboBox::currentIndexChanged),
                this, [this]() {
                    applyAIDifficultySelection();
                    statusBar()->showMessage(QStringLiteral("AI difficulty: %1")
                                                 .arg(m_aiDifficultyCombo->currentText()), 2500);
                });
        connect(m_btnLoadCustomTrack, &QPushButton::clicked,
                this, &MainWindow::loadCustomTrack);
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

void MainWindow::initializeAIOpponents()
{
    if (!m_aiManager) {
        return;
    }

    m_aiManager->destroyAllOpponents();
    m_aiManager->setRaceTotalLaps(m_totalLaps);
    m_aiManager->setPlayerPosition(m_playerPosition);
    m_aiManager->setPlayerRaceProgress(0, 0, 0.0, false, 0.0);
    m_aiManager->setTrackBounds(QRectF(0.0, 0.0, 1280.0, 1920.0));

    TrackManager* trackMgr = TrackManager::instance(this);
    if (trackMgr && trackMgr->hasCurrentTrack()) {
        m_aiManager->setTrackBounds(trackMgr->getCurrentTrack()->getBounds());
    }

    QList<Waypoint> aiWaypoints;
    const QList<QVector2D> trackWaypoints = trackMgr ? trackMgr->getWaypoints() : QList<QVector2D>();
    for (int i = 0; i < trackWaypoints.size(); ++i) {
        aiWaypoints.append(Waypoint(trackWaypoints.at(i), 110.0, false, 0, i));
    }

    if (aiWaypoints.isEmpty()) {
        aiWaypoints.append(Waypoint(QVector2D(200, 200), 90, false, 0, 0));
        aiWaypoints.append(Waypoint(QVector2D(800, 200), 115, false, 0, 1));
        aiWaypoints.append(Waypoint(QVector2D(800, 800), 70, true, 2, 2));
        aiWaypoints.append(Waypoint(QVector2D(200, 800), 100, true, 1, 3));
    }

    const QString difficulty = m_aiDifficultyCombo
        ? m_aiDifficultyCombo->currentData().toString()
        : QStringLiteral("medium");

    AIStyle primaryStyle = AIStyle::Normal;
    AIStyle secondaryStyle = AIStyle::Defensive;
    if (difficulty == QStringLiteral("easy")) {
        primaryStyle = AIStyle::Conservative;
        secondaryStyle = AIStyle::Normal;
    } else if (difficulty == QStringLiteral("hard")) {
        primaryStyle = AIStyle::Aggressive;
        secondaryStyle = AIStyle::Defensive;
    } else if (difficulty == QStringLiteral("adaptive")) {
        primaryStyle = AIStyle::Normal;
        secondaryStyle = AIStyle::Aggressive;
    }

    AIOpponent* ai1 = m_aiManager->createOpponent("ai_1", primaryStyle);
    AIOpponent* ai2 = m_aiManager->createOpponent("ai_2", secondaryStyle);
    const QList<AIOpponent*> opponents = {ai1, ai2};

    for (int i = 0; i < opponents.size(); ++i) {
        AIOpponent* ai = opponents.at(i);
        if (!ai) {
            continue;
        }
        ai->setWaypoints(aiWaypoints);
        ai->setPosition(aiWaypoints.first().position + QVector2D(35.0f * i, 42.0f * i));
        ai->setRotation(0.0);
        ai->setVelocity(QVector2D(0.0, 0.0));
        ai->setCurrentLap(0);
        ai->setCheckpointsPassed(0);
        ai->setRacePosition(i + 2);
        ai->setFinished(false);
        ai->setState(AIState::Racing);
    }

    if (m_gameView) {
        for (AIOpponent* ai : opponents) {
            if (ai) {
                m_gameView->updateAICar(ai->getId(), ai->getPosition(), ai->getRotation(), ai->getSpeed());
            }
        }
    }
}

void MainWindow::applyAIDifficultySelection()
{
    initializeAIOpponents();
}

void MainWindow::loadCustomTrack()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("Load PhantomDrive Track"),
        QString(),
        QStringLiteral("PhantomDrive Track (*.pdtrack *.json)"));

    if (filePath.isEmpty()) {
        return;
    }

    TrackManager* trackMgr = TrackManager::instance(this);
    if (!trackMgr || !trackMgr->loadTrackFromFile(filePath)) {
        QMessageBox::warning(this,
                             QStringLiteral("Track Load Failed"),
                             QStringLiteral("Could not load the selected track. Please choose a valid .pdtrack or JSON track file."));
        return;
    }

    m_customTrackPath = filePath;
    if (m_gameView) {
        m_gameView->setTrackData(trackMgr->getCurrentTrack());
    }
    initializeAIOpponents();
    statusBar()->showMessage(QStringLiteral("Loaded custom track: %1").arg(filePath), 5000);
}

QString MainWindow::formatRaceTime(qint64 milliseconds) const
{
    const qint64 minutes = milliseconds / 60000;
    const qint64 seconds = (milliseconds % 60000) / 1000;
    const qint64 millis = milliseconds % 1000;
    return QStringLiteral("%1:%2.%3")
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 2, 10, QLatin1Char('0'))
        .arg(millis, 3, 10, QLatin1Char('0'));
}

qreal MainWindow::estimatePlayerProgress() const
{
    constexpr qint64 DemoLapDurationMs = 15000;
    const qint64 lapElapsed = qMax<qint64>(0, m_sessionElapsedMs - m_currentLapStartMs);
    return qBound(0.0, (static_cast<qreal>(lapElapsed) / DemoLapDurationMs) * 100.0, 100.0);
}

void MainWindow::updateRaceHud()
{
    if (!m_arcadeHUD) {
        return;
    }

    m_arcadeHUD->updateSpeed(m_playerSpeed);
    m_arcadeHUD->updateLap(m_currentLap, m_totalLaps);
    m_arcadeHUD->updateTotalTime(formatRaceTime(m_sessionElapsedMs));
    m_arcadeHUD->updateLapTime(formatRaceTime(qMax<qint64>(0, m_sessionElapsedMs - m_currentLapStartMs)));
    if (m_bestLapMs > 0) {
        m_arcadeHUD->updateBestLapTime(formatRaceTime(m_bestLapMs));
    }

    const int totalRacers = m_aiManager ? (m_aiManager->getOpponentCount() + 1) : 1;
    const int playerPosition = m_aiManager ? m_aiManager->getPlayerRacePosition() : 1;
    m_arcadeHUD->updatePosition(playerPosition, totalRacers);
}

void MainWindow::simulateGameLoop()
{
    m_simTimer = new QTimer(this);

    connect(m_simTimer, &QTimer::timeout, this, [this]() {
        if (!m_driveActive) {
            return;
        }

        ++m_simTick;
        m_sessionElapsedMs += 50;
        m_playerSpeed = 45.0 + 35.0 * qSin(m_simTick / 18.0);
        if (m_simTick % 220 > 160) {
            m_playerSpeed += 35.0;
        }

        if (m_simTick % 70 == 0) {
            m_playerRotation += 22.5;
        }

        const qreal radians = qDegreesToRadians(m_playerRotation);
        const QVector2D velocity(qCos(radians) * m_playerSpeed, qSin(radians) * m_playerSpeed);
        m_playerPosition += velocity * 0.018;

        if (m_playerPosition.x() < 80 || m_playerPosition.x() > 900
            || m_playerPosition.y() < 80 || m_playerPosition.y() > 900) {
            m_playerRotation += 120.0;
            m_playerPosition.setX(qBound(90.0f, m_playerPosition.x(), 890.0f));
            m_playerPosition.setY(qBound(90.0f, m_playerPosition.y(), 890.0f));
            if (m_scoreManager) {
                m_scoreManager->recordCollision(m_playerPosition.toPointF(), m_playerSpeed, QStringLiteral("Wall Hit"));
            }
            onCollision();
        }

        updateTrafficAndHud(m_simTick);

        constexpr qint64 DemoLapDurationMs = 15000;
        const qint64 lapElapsed = m_sessionElapsedMs - m_currentLapStartMs;
        if (m_currentMode == "Arcade" && !m_arcadeRaceFinished && lapElapsed >= DemoLapDurationMs) {
            if (m_bestLapMs <= 0 || lapElapsed < m_bestLapMs) {
                m_bestLapMs = lapElapsed;
            }
            onLapCompleted(m_currentLap);
            ++m_currentLap;
            m_currentLapStartMs = m_sessionElapsedMs;

            if (m_currentLap > m_totalLaps) {
                m_arcadeRaceFinished = true;
                m_currentLap = m_totalLaps;
                if (m_aiManager) {
                    m_aiManager->setPlayerRaceProgress(m_totalLaps, 100, 100.0, true, m_sessionElapsedMs / 1000.0);
                }
                if (m_arcadeHUD) {
                    m_arcadeHUD->showRaceFinished(m_aiManager ? m_aiManager->getPlayerRacePosition() : 1,
                                                  formatRaceTime(m_sessionElapsedMs));
                }
                finishDrivingSession();
                return;
            }
        }

        if (m_aiManager)
        {
            const qreal playerProgress = estimatePlayerProgress();
            m_aiManager->setPlayerPosition(m_playerPosition);
            m_aiManager->setPlayerRaceProgress(m_currentLap - 1,
                                               static_cast<int>(playerProgress),
                                               playerProgress,
                                               false,
                                               m_sessionElapsedMs / 1000.0);
            m_aiManager->update(50);

            QList<AIOpponent*> opponents = m_aiManager->getAllOpponents();

            for (AIOpponent* ai : opponents)
            {
                m_gameView->updateAICar(
                    ai->getId(),
                    ai->getPosition(),
                    ai->getRotation(),
                    ai->getSpeed()
                    );
            }
        }

        if (m_drivingDataCollector && m_drivingDataCollector->vehicleSensor()) {
            VehicleSensor* sensor = m_drivingDataCollector->vehicleSensor();
            sensor->updatePosition(m_playerPosition);
            sensor->updateVelocity(velocity);
            sensor->updateRotation(m_playerRotation);
            sensor->updateSpeedLimit(m_currentSpeedLimit, "main_route_speed_zone");
            sensor->updateAcceleratorState(m_playerSpeed >= m_drivingDataCollector->getCurrentData().speed);
            sensor->updateBrakeState(m_playerSpeed < m_drivingDataCollector->getCurrentData().speed);
        }

        if (m_reportWidget && m_reportWidget->isVisible()) {
            m_reportWidget->addSpeedData(m_playerSpeed, m_simTick);
        }

        if (m_scoreManager) {
            m_scoreManager->recordSafeDrivingTick(QDateTime::currentMSecsSinceEpoch(), m_playerSpeed);
        }

        updateRaceHud();
    });

    m_simTimer->start(50);
}

void MainWindow::startDrivingSession(const QString& mode)
{
    m_currentMode = mode;
    m_driveActive = true;
    m_arcadeRaceFinished = false;
    m_currentLap = 1;
    m_totalLaps = 3;
    m_simTick = 0;
    m_sessionElapsedMs = 0;
    m_currentLapStartMs = 0;
    m_bestLapMs = 0;
    m_playerPosition = QVector2D(320.0, 320.0);
    m_playerRotation = -90.0;
    m_playerSpeed = 0.0;
    initializeAIOpponents();

    if (m_drivingDataCollector) {
        m_drivingDataCollector->stopCollection();
        m_drivingDataCollector->clearData();
        m_drivingDataCollector->setCurrentSpeedLimit(m_currentSpeedLimit, "main_route_speed_zone");
        m_drivingDataCollector->startCollection();
    }
    if (m_scoreManager) {
        m_scoreManager->startSession(QStringLiteral("player"));
    }

    ui->stackedWidget->setCurrentIndex(1);
    if (m_gameView) {
        m_gameView->show();
    }
    if (m_learningHUD) {
        if (m_currentMode == "Learning") {
            m_learningHUD->show();
            if (m_arcadeHUD) m_arcadeHUD->hide();
        } else {
            m_learningHUD->hide();
            if (m_arcadeHUD) {
                m_arcadeHUD->updateLap(m_currentLap, m_totalLaps);
                updateRaceHud();
                m_arcadeHUD->show();
            }
        }
        m_learningHUD->updateGameMode(m_currentMode);
    }
    if (m_btnFinishDrive) {
        m_btnFinishDrive->setEnabled(true);
    }

    showCountdown();

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
    m_scoreManager->finishSession(m_drivingDataCollector);
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

    showInteractiveFeedback(violation.description, PhantomDrive::FeedbackType::Warning);
    playSound(PhantomDrive::SoundEffect::Violation);

    statusBar()->showMessage(violation.description, 3000);
}

void MainWindow::onScoreReady(const ScoreReport& report)
{
    SaveLoadManager::instance().saveReport(report);
    showReportWindow(&report);
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

void MainWindow::showInteractiveFeedback(const QString& message, PhantomDrive::FeedbackType type)
{
    PhantomDrive::InteractiveFeedback::instance(this).showFeedback(message, type);
}

void MainWindow::playSound(PhantomDrive::SoundEffect effect)
{
    PhantomDrive::SoundManager::instance(this).play(effect);
}

void MainWindow::showCountdown()
{
    PhantomDrive::InteractiveFeedback::instance(this).showCountdown(3);
    if (m_arcadeHUD && m_currentMode == QStringLiteral("Arcade")) {
        m_arcadeHUD->showCountdown(3);
    }
    playSound(PhantomDrive::SoundEffect::CountdownBeep);
    QTimer::singleShot(3000, this, &MainWindow::onRaceStart);
}

void MainWindow::onRaceStart()
{
    PhantomDrive::InteractiveFeedback::instance(this).showGo();
    playSound(PhantomDrive::SoundEffect::CountdownGo);
}

void MainWindow::onLapCompleted(int lapNumber)
{
    if (m_arcadeHUD) {
        m_arcadeHUD->showLapCompleted(lapNumber);
    }
    PhantomDrive::InteractiveFeedback::instance(this).showFeedback(
        QString("Lap %1 Complete!").arg(lapNumber),
        PhantomDrive::FeedbackType::Milestone
    );
    playSound(PhantomDrive::SoundEffect::LapComplete);
}

void MainWindow::onCheckpointReached(int checkpointNumber)
{
    PhantomDrive::InteractiveFeedback::instance(this).showFeedback(
        QString("Checkpoint %1").arg(checkpointNumber),
        PhantomDrive::FeedbackType::Milestone
    );
    playSound(PhantomDrive::SoundEffect::Checkpoint);
}

void MainWindow::onCollision()
{
    PhantomDrive::InteractiveFeedback::instance(this).showFeedback(
        "Wall Hit!",
        PhantomDrive::FeedbackType::Critical
    );
    playSound(PhantomDrive::SoundEffect::Collision);
}

void MainWindow::onPowerupCollected(const QString& powerupType)
{
    QString displayText;
    PhantomDrive::FeedbackType type = PhantomDrive::FeedbackType::Powerup;

    if (powerupType.toLower().contains("boost")) {
        displayText = "Speed Boost!";
    } else if (powerupType.toLower().contains("shield")) {
        displayText = "Shield Active!";
    } else {
        displayText = QString("%1 Collected!").arg(powerupType);
    }

    PhantomDrive::InteractiveFeedback::instance(this).showFeedback(displayText, type);
    playSound(PhantomDrive::SoundEffect::PowerupCollect);
}

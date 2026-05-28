#include "UI/mainwindow.h"
#include "ui_mainwindow.h"

#include "UI/DrivingReportWidget.h"
#include "UI/ThemeManager.h"
#include "gamemode/VehicleSensor.h"
#include "gamemode/PowerupBox.h"
#include "gamemode/TrafficLightObject.h"
#include "gamemode/TrafficObjectManager.h"
#include "gamemode/SpeedLimitSignObject.h"
#include "gamemode/PedestrianCrossingObject.h"
#include "track/TrackData.h"
#include "track/TrackManager.h"
#include "track/TrackTile.h"
#include "track/Checkpoint.h"

#include <QRectF>
#include "scoring/ScoreReport.h"
#include "core/saveloadmanager.h"
#include "core/VehiclePhysics.h"

#include <QApplication>
#include <QComboBox>
#include <QDateTime>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QVariant>
#include <QtMath>

#include "UI/SoundGenerator.h"

using namespace PhantomDrive;

namespace {

bool isDrivableSurface(TileType type)
{
    switch (type) {
    case TileType::Road:
    case TileType::Asphalt:
    case TileType::StartLine:
    case TileType::FinishLine:
        return true;
    default:
        return false;
    }
}

struct GateSpec {
    QVector2D center;
    qreal width = 0.0;
    qreal height = 0.0;
    bool valid = false;
};

// 在赛道直道段收集可行驶瓦片，生成横穿路面宽度的条状检查门
// gateSpansX=true：门沿 X 铺满（用于西/东竖直直道）；false：门沿 Y 铺满（用于北/南横向弧段）
GateSpec computeGateAcrossTrack(TrackData* track, int rowMin, int rowMax, int colMin, int colMax,
                                qreal tileSize, qreal gateThickness, bool gateSpansX)
{
    GateSpec gate;
    int minRow = 999;
    int maxRow = -1;
    int minCol = 999;
    int maxCol = -1;
    qreal sumX = 0.0;
    qreal sumY = 0.0;
    int count = 0;

    for (int row = rowMin; row <= rowMax; ++row) {
        for (int col = colMin; col <= colMax; ++col) {
            TrackTile* tile = track->getTileAt(row, col);
            if (!tile || !isDrivableSurface(tile->getType())) {
                continue;
            }
            minRow = qMin(minRow, row);
            maxRow = qMax(maxRow, row);
            minCol = qMin(minCol, col);
            maxCol = qMax(maxCol, col);
            sumX += col * tileSize + tileSize / 2.0;
            sumY += row * tileSize + tileSize / 2.0;
            ++count;
        }
    }

    if (count == 0) {
        return gate;
    }

    const qreal spanX = (maxCol - minCol + 1) * tileSize;
    const qreal spanY = (maxRow - minRow + 1) * tileSize;
    gate.center = QVector2D(sumX / count, sumY / count);
    gate.valid = true;

    if (gateSpansX) {
        gate.width = spanX;
        gate.height = gateThickness;
    } else {
        gate.width = gateThickness;
        gate.height = spanY;
    }
    return gate;
}

void addGateCheckpoint(TrackData* track, int id, int routeIndex,
                     const QVector2D& center, qreal width, qreal height)
{
    Checkpoint* cp = new Checkpoint(id, center, track);
    cp->setIndexInRoute(routeIndex);
    cp->setWidth(width);
    cp->setHeight(height);
    track->addCheckpoint(cp);
}

constexpr qreal kPhysicsMaxSpeed = 300.0;
constexpr qreal kDisplayMaxSpeedKmh = 120.0;
constexpr qreal kTileSize = 64.0;

bool tileAtIsStartFinish(PhantomDrive::TrackData* track, const QVector2D& position)
{
    if (!track) {
        return false;
    }
    const int row = static_cast<int>(position.y() / kTileSize);
    const int col = static_cast<int>(position.x() / kTileSize);
    PhantomDrive::TrackTile* tile = track->getTileAt(row, col);
    if (!tile) {
        return false;
    }
    const auto type = tile->getType();
    return type == PhantomDrive::TileType::StartLine || type == PhantomDrive::TileType::FinishLine;
}

bool positionInNorthGate(PhantomDrive::TrackData* track, const QVector2D& position)
{
    if (!track) {
        return false;
    }
    const QList<PhantomDrive::Checkpoint*> checkpoints = track->getCheckpointsInOrder();
    if (checkpoints.isEmpty() || !checkpoints.first()) {
        return false;
    }
    return checkpoints.first()->containsPoint(position);
}

bool crossedCheckpointGate(const PhantomDrive::Checkpoint* cp,
                           const QVector2D& from,
                           const QVector2D& to)
{
    if (!cp) {
        return false;
    }
    const QRectF bounds = cp->getBounds();
    if (bounds.contains(from.toPointF()) || bounds.contains(to.toPointF())) {
        return true;
    }

    constexpr int kSamples = 8;
    for (int i = 1; i < kSamples; ++i) {
        const qreal t = static_cast<qreal>(i) / static_cast<qreal>(kSamples);
        const QVector2D sample = from * (1.0 - t) + to * t;
        if (bounds.contains(sample.toPointF())) {
            return true;
        }
    }
    return false;
}

QString lightColorToString(PhantomDrive::TrafficLightObject::LightColor color)
{
    switch (color) {
    case PhantomDrive::TrafficLightObject::LightColor::Red:
        return QStringLiteral("red");
    case PhantomDrive::TrafficLightObject::LightColor::Yellow:
        return QStringLiteral("yellow");
    case PhantomDrive::TrafficLightObject::LightColor::Green:
        return QStringLiteral("green");
    default:
        return QStringLiteral("green");
    }
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_learningHUD(nullptr)
    , m_arcadeHUD(nullptr)
    , m_gameView(nullptr)
    , m_vehiclePhysics(nullptr)
    , m_drivingDataCollector(new DrivingDataCollector(this))
    , m_scoreManager(new ScoreManager(this))
    , m_aiManager(new AIOpponentManager(this))
    , m_trafficObjectManager(new TrafficObjectManager(this))
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
    , m_countdownActive(false)
    , m_arcadeRaceFinished(false)
    , m_lapsCompleted(0)
    , m_totalLaps(3)
    , m_simTick(0)
    , m_sessionElapsedMs(0)
    , m_currentLapStartMs(0)
    , m_bestLapMs(0)
    , m_playerPosition(320.0, 320.0)
    , m_previousPlayerPosition(320.0, 320.0)
    , m_playerRotation(-90.0)
    , m_playerSpeed(0.0)
    , m_arcadeRaceLogicActive(false)
    , m_nextCheckpointIndex(0)
    , m_raceCheckpointTotal(0)
    , m_hasLeftNorthSector(false)
    , m_wasOnStartLine(false)
    , m_wasInNorthGate(false)
    , m_blockCheckpointsUntilLeaveNorth(false)
    , m_wasInsideNextGate(false)
{
    ui->setupUi(this);

    PhantomDrive::InteractiveFeedback::instance(this);
    PhantomDrive::SoundManager::instance(this);
    PhantomDrive::SoundGenerator::instance().generateAllSounds();

    m_arcadeHUD = new PhantomDrive::ArcadeHUD(this);
    m_arcadeHUD->setWindowFlags(Qt::Widget);
    m_arcadeHUD->hide();

    m_learningHUD = new LearningHUD(this);
    m_learningHUD->hide();

    setupGameView();
    setupVehiclePhysics();
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
            if (m_arcadeHUD) {
                m_arcadeHUD->hide();
            }
            statusBar()->clearMessage();
        });
    }

    m_drivingDataCollector->setVehicleId("player");
    m_drivingDataCollector->setSamplingInterval(50);
    m_drivingDataCollector->setCurrentSpeedLimit(m_currentSpeedLimit, "main_route_speed_zone");
    if (m_drivingDataCollector->vehicleSensor()) {
        m_drivingDataCollector->vehicleSensor()->setSpeedLimitViolationEnabled(false);
    }

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

    if (m_drivingDataCollector->vehicleSensor()) {
        m_drivingDataCollector->vehicleSensor()->setSpeedLimitViolationEnabled(false);
    }

    connect(m_drivingDataCollector, &IDrivingDataCollector::dataCollected,
            this, &MainWindow::onDrivingDataCollected, Qt::UniqueConnection);
    connect(m_drivingDataCollector, &IDrivingDataCollector::violationDetected,
            this, &MainWindow::onViolationDetected, Qt::UniqueConnection);
    connect(m_drivingDataCollector, &IDrivingDataCollector::violationDetected,
            m_scoreManager, &ScoreManager::onViolationDetected, Qt::UniqueConnection);

    if (m_trafficObjectManager) {
        connect(m_trafficObjectManager, &TrafficObjectManager::violationDetected,
                this, &MainWindow::handleTrafficViolation, Qt::UniqueConnection);
    }

    if (m_scoreManager) {
        m_scoreManager->setTrafficObjectManager(m_trafficObjectManager);
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
            QHBoxLayout* playLayout = new QHBoxLayout();
            playLayout->setContentsMargins(0, 0, 0, 0);
            playLayout->setSpacing(8);
            playLayout->addWidget(m_gameView, 1);

            if (m_arcadeHUD) {
                m_arcadeHUD->setFixedSize(300, 280);
                playLayout->addWidget(m_arcadeHUD, 0, Qt::AlignTop | Qt::AlignRight);
            }

            layout->addLayout(playLayout, 1);

            gamePage->setFocusPolicy(Qt::StrongFocus);
            gamePage->setFocus();
        }
    }

    TrackManager* trackMgr = TrackManager::instance(this);
    if (trackMgr && trackMgr->hasCurrentTrack()) {
        TrackData* loadedTrack = trackMgr->getCurrentTrack();
        if (!loadedTrack->getCheckpointsInOrder().isEmpty()) {
            m_gameView->setTrackData(loadedTrack);
            if (loadedTrack->getStartPosition() == QVector2D(0, 0)) {
                const qreal tileSize = 64.0;
                const QVector2D fallback(15 * tileSize + tileSize / 2.0, 3 * tileSize + tileSize / 2.0);
                loadedTrack->setStartPosition(fallback);
                loadedTrack->setStartRotation(0.0);
            }
            syncRaceTrackToManager();
            setupEBRuntimeObjects();
            return;
        }
    }

    TrackData* testTrack = new TrackData();
    testTrack->setId("main_city_training_route");
    testTrack->setName("Main City Training Route");
    testTrack->setSize(30, 30);

    const int trackCenterRow = 15;
    const int trackCenterCol = 15;
    const int trackOuterRadius = 12;
    const int trackInnerRadius = 8;

    for (int row = 0; row < 30; ++row) {
        for (int col = 0; col < 30; ++col) {
            TrackTile* tile = new TrackTile();
            tile->setPosition(row, col);

            double distFromCenter = std::sqrt(
                std::pow(row - trackCenterRow, 2) + std::pow(col - trackCenterCol, 2)
            );

            const bool isOnTrack = (distFromCenter >= trackInnerRadius && distFromCenter <= trackOuterRadius);
            const bool isOuterWall = (distFromCenter > trackOuterRadius && distFromCenter <= trackOuterRadius + 1);
            const bool isInnerWall = (distFromCenter < trackInnerRadius && distFromCenter >= trackInnerRadius - 1);

            // 起跑线：环道最北侧一行（row=3），横向 5 格，与 FinishLine 重合
            const bool isStartLine = (row == trackCenterRow - trackOuterRadius
                                      && col >= trackCenterCol - 2
                                      && col <= trackCenterCol + 2);

            if (isStartLine) {
                tile->setType(TileType::StartLine);
            } else if (isOnTrack) {
                tile->setType(TileType::Road);
            } else if (isOuterWall || isInnerWall) {
                tile->setType(TileType::Wall);
            } else {
                tile->setType(TileType::Grass);
            }

            testTrack->setTileAt(row, col, tile);
        }
    }

    const qreal tileSize = 64.0;
    const int startRow = trackCenterRow - trackOuterRadius;
    const int startCol = trackCenterCol;
    const QVector2D startPos(startCol * tileSize + tileSize / 2.0,
                             startRow * tileSize + tileSize / 2.0);
    testTrack->setStartPosition(startPos);
    // rotation=0：沿 +Y 驶入环道（与当前 VehiclePhysics 前进方向一致）
    testTrack->setStartRotation(0.0);
    testTrack->setMaxLaps(m_totalLaps > 0 ? m_totalLaps : 3);

    const qreal gateThickness = tileSize * 0.75;
    const qreal gateSpan = tileSize * 5.0;
    const int band = trackOuterRadius - trackInnerRadius + 1;

    // 北/南弧段：行驶方向大致沿 X，检查门沿 Y 横穿；西/东直道：沿 Y 行驶，门沿 X 横穿
    const GateSpec northGate = computeGateAcrossTrack(
        testTrack,
        trackCenterRow - trackOuterRadius,
        trackCenterRow - trackInnerRadius,
        trackCenterCol - band,
        trackCenterCol + band,
        tileSize,
        gateThickness,
        false);
    const GateSpec eastGate = computeGateAcrossTrack(
        testTrack,
        trackCenterRow - band,
        trackCenterRow + band,
        trackCenterCol + trackInnerRadius,
        trackCenterCol + trackOuterRadius,
        tileSize,
        gateThickness,
        true);
    const GateSpec southGate = computeGateAcrossTrack(
        testTrack,
        trackCenterRow + trackInnerRadius,
        trackCenterRow + trackOuterRadius,
        trackCenterCol - band,
        trackCenterCol + band,
        tileSize,
        gateThickness,
        false);
    const GateSpec westGate = computeGateAcrossTrack(
        testTrack,
        trackCenterRow - band,
        trackCenterRow + band,
        trackCenterCol - trackOuterRadius,
        trackCenterCol - trackInnerRadius,
        tileSize,
        gateThickness,
        true);

    const GateSpec gates[] = {northGate, eastGate, southGate, westGate};
    QList<QVector2D> checkpointPositions;
    for (int i = 0; i < 4; ++i) {
        if (!gates[i].valid) {
            continue;
        }
        qreal gateW = gates[i].width;
        qreal gateH = gates[i].height;
        const qreal minSpan = gateSpan;
        const qreal minThickness = gateThickness * 2.5;
        // 所有门加宽薄边，避免高速/贴边漏检
        if (gateW >= gateH) {
            gateW = qMax(gateW, minSpan);
            gateH = qMax(gateH, minThickness);
        } else {
            gateW = qMax(gateW, minThickness);
            gateH = qMax(gateH, minSpan);
        }
        addGateCheckpoint(testTrack, i, i, gates[i].center, gateW, gateH);
        checkpointPositions.append(gates[i].center);
    }

    if (trackMgr) {
        QList<QVector2D> waypoints = checkpointPositions;
        waypoints.append(startPos);
        trackMgr->setWaypoints(waypoints);
    }

    m_gameView->setTrackData(testTrack);
    if (trackMgr) {
        trackMgr->setCurrentTrack(testTrack);
    }
    syncRaceTrackToManager();

    setupEBRuntimeObjects();
}

QString MainWindow::powerupTypeToString(PhantomDrive::PowerupType type) const
{
    switch (type) {
    case PhantomDrive::PowerupType::Boost:
        return QStringLiteral("Boost");
    case PhantomDrive::PowerupType::Shield:
        return QStringLiteral("Shield");
    case PhantomDrive::PowerupType::Missile:
        return QStringLiteral("Missile");
    case PhantomDrive::PowerupType::OilSlick:
        return QStringLiteral("Oil Slick");
    case PhantomDrive::PowerupType::EMP:
        return QStringLiteral("EMP");
    case PhantomDrive::PowerupType::Invisibility:
        return QStringLiteral("Invisibility");
    case PhantomDrive::PowerupType::Repair:
        return QStringLiteral("Repair");
    case PhantomDrive::PowerupType::Teleport:
        return QStringLiteral("Teleport");
    case PhantomDrive::PowerupType::Magnet:
        return QStringLiteral("Magnet");
    default:
        return QStringLiteral("Powerup");
    }
}

void MainWindow::clearEBRuntimeObjects()
{
    qDeleteAll(m_powerupBoxes);
    m_powerupBoxes.clear();

    if (m_trafficObjectManager) {
        m_trafficObjectManager->clear();
    }

    if (m_gameView) {
        m_gameView->clearScenarioObjects();
        m_gameView->setPlayerEffectState(false, false);
    }

    if (m_learningHUD) {
        m_learningHUD->updatePowerupState(QStringLiteral("eb_boost"), QStringLiteral("Boost"), false);
        m_learningHUD->updatePowerupState(QStringLiteral("eb_shield"), QStringLiteral("Shield"), false);
        m_learningHUD->updatePowerupState(QStringLiteral("eb_emp"), QStringLiteral("EMP"), false);
        m_learningHUD->updatePowerupState(QStringLiteral("eb_repair"), QStringLiteral("Repair"), false);
    }

    m_currentSpeedLimit = 60;
    m_currentTrafficLightState = QStringLiteral("green");
}

void MainWindow::setupEBRuntimeObjects()
{
    clearEBRuntimeObjects();

    if (!m_gameView || !m_gameView->trackData() || !m_trafficObjectManager) {
        return;
    }

    TrackData* track = m_gameView->trackData();
    const QList<Checkpoint*> checkpoints = track->getCheckpointsInOrder();
    auto checkpointPosition = [&](int index, const QVector2D& fallback) {
        if (index >= 0 && index < checkpoints.size() && checkpoints.at(index)) {
            return checkpoints.at(index)->getPosition();
        }
        return fallback;
    };

    const QVector2D start = track->getStartPosition();
    const QVector2D north = checkpointPosition(0, start + QVector2D(0.0f, 120.0f));
    const QVector2D east = checkpointPosition(1, start + QVector2D(520.0f, 520.0f));
    const QVector2D south = checkpointPosition(2, start + QVector2D(0.0f, 1040.0f));
    const QVector2D west = checkpointPosition(3, start + QVector2D(-520.0f, 520.0f));

    auto addPowerupBox = [this](const QString& id, const QVector2D& position, PowerupType type) {
        auto* box = new PowerupBox(position, 72.0f, this);
        box->setFixedPowerupType(type);
        box->setRespawnTime(8.0f);
        m_powerupBoxes.append(box);

        const QString typeName = powerupTypeToString(type);
        if (m_gameView) {
            m_gameView->addPowerupBox(id, position, typeName);
        }

        connect(box, &PowerupBox::collected,
                this, [this, id](const QString&, const PowerupType& collectedType) {
                    if (m_gameView) {
                        m_gameView->removePowerupBox(id);
                    }
                    handlePowerupCollected(collectedType);
                });
        connect(box, &PowerupBox::respawned,
                this, [this, id, position, typeName]() {
                    if (m_gameView) {
                        m_gameView->addPowerupBox(id, position, typeName);
                    }
                });
    };

    addPowerupBox(QStringLiteral("eb_boost_box"), north + QVector2D(0.0f, 120.0f), PowerupType::Boost);
    addPowerupBox(QStringLiteral("eb_shield_box"), east, PowerupType::Shield);
    addPowerupBox(QStringLiteral("eb_emp_box"), west, PowerupType::EMP);
    addPowerupBox(QStringLiteral("eb_repair_box"), south + QVector2D(140.0f, 0.0f), PowerupType::Repair);

    auto* light = new TrafficLightObject(QStringLiteral("eb_light_1"), m_trafficObjectManager);
    light->setPosition(north + QVector2D(0.0f, 190.0f));
    light->setBounds(QRectF(light->getPosition().x() - 95.0,
                            light->getPosition().y() - 95.0,
                            190.0,
                            190.0));
    light->setCurrentColor(TrafficLightObject::LightColor::Red);
    light->setRedDurationMs(7000);
    light->setGreenDurationMs(6000);
    light->setYellowDurationMs(2000);
    m_trafficObjectManager->registerTrafficObject(light);
    light->start();
    m_currentTrafficLightState = QStringLiteral("red");
    m_gameView->addTrafficLight(light->getId(), light->getPosition(), m_currentTrafficLightState);
    connect(light, &TrafficLightObject::colorChanged,
            this, [this, light](TrafficLightObject::LightColor, TrafficLightObject::LightColor color) {
                m_currentTrafficLightState = lightColorToString(color);
                if (m_gameView) {
                    m_gameView->updateTrafficLight(light->getId(), m_currentTrafficLightState);
                }
            });

    auto* sign = new SpeedLimitSignObject(QStringLiteral("eb_speed_zone_1"), m_trafficObjectManager);
    sign->setPosition(east + QVector2D(-70.0f, 0.0f));
    sign->setSpeedLimit(45.0);
    sign->setDetectionRadius(180.0);
    sign->setZoneId(QStringLiteral("eb_east_speed_zone"));
    m_trafficObjectManager->registerTrafficObject(sign);
    m_gameView->addSpeedLimitSign(sign->getId(), sign->getPosition(), static_cast<int>(sign->getSpeedLimit()));

    auto* crossing = new PedestrianCrossingObject(QStringLiteral("eb_crossing_1"), m_trafficObjectManager);
    const QSizeF crossingSize(240.0, 120.0);
    crossing->setPosition(south);
    crossing->setBounds(QRectF(south.x() - crossingSize.width() / 2.0,
                               south.y() - crossingSize.height() / 2.0,
                               crossingSize.width(),
                               crossingSize.height()));
    crossing->spawnPedestrian();
    m_trafficObjectManager->registerTrafficObject(crossing);
    m_gameView->addPedestrianCrossing(crossing->getId(), south, crossingSize);
}

void MainWindow::setupVehiclePhysics()
{
    m_vehiclePhysics = new VehiclePhysics(this);

    TrackManager* trackMgr = TrackManager::instance(this);
    if (trackMgr) {
        m_vehiclePhysics->initialize(trackMgr);
    }

    connect(m_gameView, &GameViewWidget::keyInputReceived,
            m_vehiclePhysics, &VehiclePhysics::handleKeyPress);
    connect(m_gameView, &GameViewWidget::keyReleased,
            m_vehiclePhysics, &VehiclePhysics::handleKeyRelease);

    connect(m_vehiclePhysics, &VehiclePhysics::positionUpdated,
            this, [this](const QVector2D& position) {
                m_playerPosition = position;
                m_playerRotation = m_vehiclePhysics->getRotation();
                m_playerSpeed = m_vehiclePhysics->getSpeed();

                if (m_gameView && m_driveActive) {
                    m_gameView->setPlayerEffectState(m_vehiclePhysics->isSpeedBoostActive(),
                                                     m_vehiclePhysics->isShieldActive());
                    m_gameView->updatePlayerCar(m_playerPosition, m_playerRotation, displaySpeedKmh());
                    m_gameView->setCameraPosition(m_playerPosition);
                }

                if (m_drivingDataCollector && m_drivingDataCollector->vehicleSensor()) {
                    VehicleSensor* sensor = m_drivingDataCollector->vehicleSensor();
                    sensor->updatePosition(m_playerPosition);
                    sensor->updateRotation(m_playerRotation);
                    qreal radians = qDegreesToRadians(m_playerRotation);
                    sensor->updateVelocity(QVector2D(qCos(radians) * m_playerSpeed,
                                                     qSin(radians) * m_playerSpeed));
                    sensor->updateSpeedLimit(m_currentSpeedLimit, "main_route_speed_zone");
                }
            });

    connect(m_vehiclePhysics, &VehiclePhysics::collisionOccurred,
            this, [this](const QString& objectType, const QVector2D& position, qreal impactForce) {
                Q_UNUSED(position);
                Q_UNUSED(impactForce);
                onCollision();
                if (m_scoreManager) {
                    m_scoreManager->recordCollision(position.toPointF(), m_playerSpeed, objectType);
                }
            });

    qDebug() << "VehiclePhysics initialized and connected to keyboard input";
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

void MainWindow::applyPlayerSpawnAtStartLine()
{
    TrackData* track = nullptr;
    if (m_gameView) {
        track = m_gameView->trackData();
    }
    if (!track) {
        TrackManager* trackMgr = TrackManager::instance(this);
        if (trackMgr && trackMgr->hasCurrentTrack()) {
            track = trackMgr->getCurrentTrack();
        }
    }

    QVector2D spawnPos(320.0, 320.0);
    qreal spawnRotation = 0.0;
    if (track) {
        spawnPos = track->getStartPosition();
        spawnRotation = track->getStartRotation();
    }

    m_playerPosition = spawnPos;
    m_playerRotation = spawnRotation;
    m_playerSpeed = 0.0;

    if (m_vehiclePhysics) {
        m_vehiclePhysics->setPosition(spawnPos);
        m_vehiclePhysics->setRotation(spawnRotation);
    }

    if (m_gameView) {
        m_gameView->updatePlayerCar(m_playerPosition, m_playerRotation, m_playerSpeed);
        m_gameView->setCameraPosition(m_playerPosition);
    }
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
    TrackData* loadedTrack = trackMgr->getCurrentTrack();
    if (m_gameView) {
        m_gameView->setTrackData(loadedTrack);
    }
    syncRaceTrackToManager();
    if (loadedTrack && loadedTrack->getStartPosition() == QVector2D(0, 0)) {
        const qreal tileSize = 64.0;
        loadedTrack->setStartPosition(QVector2D(15 * tileSize + tileSize / 2.0, 3 * tileSize + tileSize / 2.0));
        loadedTrack->setStartRotation(0.0);
    }
    setupEBRuntimeObjects();
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

int MainWindow::displaySpeedKmh() const
{
    return qBound(0, qRound(qAbs(m_playerSpeed) * kDisplayMaxSpeedKmh / kPhysicsMaxSpeed), 999);
}

void MainWindow::updateEBRuntime(qreal deltaSeconds)
{
    for (PowerupBox* box : m_powerupBoxes) {
        if (!box) {
            continue;
        }
        box->update(static_cast<float>(deltaSeconds));
        if (box->isActive()) {
            box->tryCollect(m_playerPosition, QStringLiteral("player"));
        }
    }

    if (!m_trafficObjectManager) {
        return;
    }

    if (m_currentMode != QStringLiteral("Learning")) {
        return;
    }

    const qreal speedKmh = displaySpeedKmh();
    m_trafficObjectManager->onVehicleSpeedChanged(speedKmh);
    m_trafficObjectManager->onVehiclePositionChanged(m_playerPosition);

    for (PedestrianCrossingObject* crossing : m_trafficObjectManager->getPedestrianCrossings()) {
        if (crossing && crossing->getPedestrianCount() == 0) {
            crossing->spawnPedestrian();
        }
    }

    m_trafficObjectManager->update(static_cast<qint64>(deltaSeconds * 1000.0));

    const qreal zoneLimit = m_trafficObjectManager->getCurrentSpeedLimit(m_playerPosition);
    if (zoneLimit > 0.0) {
        m_currentSpeedLimit = qRound(zoneLimit);
    } else {
        m_currentSpeedLimit = 80;
    }
}

void MainWindow::handlePowerupCollected(PhantomDrive::PowerupType type)
{
    const QString typeName = powerupTypeToString(type);
    const QString powerupId = QStringLiteral("eb_%1").arg(typeName.toLower().replace(QStringLiteral(" "), QStringLiteral("_")));

    onPowerupCollected(typeName);

    if (type == PowerupType::Boost && m_vehiclePhysics) {
        m_vehiclePhysics->activateSpeedBoost(1.35, 4000);
        playSound(SoundEffect::SpeedBoost);
        if (m_learningHUD) {
            m_learningHUD->updatePowerupState(powerupId, QStringLiteral("Boost"), true);
        }
        QTimer::singleShot(4000, this, [this, powerupId]() {
            if (m_learningHUD) {
                m_learningHUD->updatePowerupState(powerupId, QStringLiteral("Boost"), false);
            }
        });
    } else if (type == PowerupType::Shield && m_vehiclePhysics) {
        m_vehiclePhysics->activateShield(6000);
        if (m_learningHUD) {
            m_learningHUD->updatePowerupState(powerupId, QStringLiteral("Shield"), true);
        }
        QTimer::singleShot(6000, this, [this, powerupId]() {
            if (m_learningHUD) {
                m_learningHUD->updatePowerupState(powerupId, QStringLiteral("Shield"), false);
            }
        });
    } else if (type == PowerupType::EMP) {
        QList<QPair<QPointer<AIOpponent>, qreal>> affectedOpponents;
        if (m_aiManager) {
            for (AIOpponent* opponent : m_aiManager->getAllOpponents()) {
                if (!opponent || opponent->hasFinished()) {
                    continue;
                }
                const qreal originalSpeed = opponent->getMaxSpeed();
                affectedOpponents.append(qMakePair(QPointer<AIOpponent>(opponent), originalSpeed));
                opponent->setMaxSpeed(qMax<qreal>(35.0, originalSpeed * 0.45));
            }
        }
        if (m_learningHUD) {
            m_learningHUD->updatePowerupState(powerupId, QStringLiteral("EMP"), true);
        }
        statusBar()->showMessage(QStringLiteral("EMP Pulse! AI opponents slowed for 3 seconds."), 3000);
        QTimer::singleShot(3000, this, [this, powerupId, affectedOpponents]() {
            for (const auto& entry : affectedOpponents) {
                if (entry.first) {
                    entry.first->setMaxSpeed(entry.second);
                }
            }
            if (m_learningHUD) {
                m_learningHUD->updatePowerupState(powerupId, QStringLiteral("EMP"), false);
            }
        });
    } else if (type == PowerupType::Repair && m_vehiclePhysics) {
        m_vehiclePhysics->activateRepair();
        if (m_learningHUD) {
            m_learningHUD->updatePowerupState(powerupId, QStringLiteral("Repair"), true);
        }
        statusBar()->showMessage(QStringLiteral("Repair Kit! Vehicle stability restored."), 2500);
        QTimer::singleShot(1200, this, [this, powerupId]() {
            if (m_learningHUD) {
                m_learningHUD->updatePowerupState(powerupId, QStringLiteral("Repair"), false);
            }
        });
    }
}

void MainWindow::handleTrafficViolation(const PhantomDrive::ViolationEvent& violation)
{
    if (m_learningHUD && ui->stackedWidget->currentIndex() == 1 && m_currentMode == QStringLiteral("Learning")) {
        m_learningHUD->showPenaltyMessage(violation.description, violation.penaltyPoints);
        m_learningHUD->showViolationWarning(violation.description);
    }

    playSound(SoundEffect::Violation);
    statusBar()->showMessage(violation.description, 3000);
}

void MainWindow::updateRaceHud()
{
    if (m_learningHUD) {
        m_learningHUD->updateLapInfo(m_lapsCompleted + 1, m_totalLaps);
    }

    if (!m_arcadeHUD) {
        return;
    }

    m_arcadeHUD->updateSpeed(displaySpeedKmh());
    m_arcadeHUD->updateLap(m_lapsCompleted, m_totalLaps);
    m_arcadeHUD->updateTotalTime(formatRaceTime(m_sessionElapsedMs));
    m_arcadeHUD->updateLapTime(formatRaceTime(qMax<qint64>(0, m_sessionElapsedMs - m_currentLapStartMs)));
    if (m_bestLapMs > 0) {
        m_arcadeHUD->updateBestLapTime(formatRaceTime(m_bestLapMs));
    }

    const int totalRacers = m_aiManager ? (m_aiManager->getOpponentCount() + 1) : 1;
    const int playerPosition = m_aiManager ? m_aiManager->getPlayerRacePosition() : 1;
    m_arcadeHUD->updatePosition(playerPosition, totalRacers);
}

void MainWindow::syncRaceTrackToManager()
{
    PhantomDrive::TrackManager* trackMgr = PhantomDrive::TrackManager::instance(this);
    if (trackMgr && m_gameView && m_gameView->trackData()) {
        trackMgr->setCurrentTrack(m_gameView->trackData());
    }
}

void MainWindow::resetArcadeRaceProgress()
{
    m_nextCheckpointIndex = 0;
    m_hasLeftNorthSector = false;
    m_blockCheckpointsUntilLeaveNorth = false;
    m_previousPlayerPosition = m_playerPosition;

    PhantomDrive::TrackData* track = m_gameView ? m_gameView->trackData() : nullptr;
    m_raceCheckpointTotal = track ? track->getCheckpointsInOrder().size() : 0;

    m_wasOnStartLine = tileAtIsStartFinish(track, m_playerPosition);
    m_wasInNorthGate = positionInNorthGate(track, m_playerPosition);
    m_wasInsideNextGate = false;
    if (m_nextCheckpointIndex < m_raceCheckpointTotal && track) {
        const QList<PhantomDrive::Checkpoint*> checkpoints = track->getCheckpointsInOrder();
        if (m_nextCheckpointIndex < checkpoints.size() && checkpoints.at(m_nextCheckpointIndex)) {
            m_wasInsideNextGate = checkpoints.at(m_nextCheckpointIndex)->containsPoint(m_playerPosition);
        }
    }
}

void MainWindow::updateArcadeRaceProgress(const QVector2D& positionBefore)
{
    if (!m_arcadeRaceLogicActive || !m_driveActive || m_countdownActive || m_arcadeRaceFinished) {
        return;
    }

    PhantomDrive::TrackData* track = m_gameView ? m_gameView->trackData() : nullptr;
    if (!track) {
        return;
    }

    const QList<PhantomDrive::Checkpoint*> checkpoints = track->getCheckpointsInOrder();
    if (m_raceCheckpointTotal <= 0) {
        m_raceCheckpointTotal = checkpoints.size();
    }
    if (m_raceCheckpointTotal <= 0) {
        return;
    }

    const QVector2D& pos = m_playerPosition;
    const bool onStartLine = tileAtIsStartFinish(track, pos);
    const bool inNorthGate = positionInNorthGate(track, pos);
    const bool inNorthSector = onStartLine || inNorthGate;
    const bool wasInNorthSector = m_wasOnStartLine || m_wasInNorthGate;

    if (!inNorthSector) {
        m_hasLeftNorthSector = true;
        m_blockCheckpointsUntilLeaveNorth = false;
    }

    const bool allCheckpointsCollected = m_nextCheckpointIndex >= m_raceCheckpointTotal;

    if (allCheckpointsCollected && m_hasLeftNorthSector) {
        const bool enteredStartLine = onStartLine && !m_wasOnStartLine;
        const bool enteredNorthGate = inNorthGate && !m_wasInNorthGate;
        if (enteredStartLine || enteredNorthGate) {
            const int completedLap = m_lapsCompleted + 1;
            const qint64 lapMs = qMax<qint64>(0, m_sessionElapsedMs - m_currentLapStartMs);

            onLapCompleted(completedLap);
            m_lapsCompleted = completedLap;

            if (lapMs > 0 && (m_bestLapMs == 0 || lapMs < m_bestLapMs)) {
                m_bestLapMs = lapMs;
            }

            if (m_lapsCompleted < m_totalLaps) {
                m_currentLapStartMs = m_sessionElapsedMs;
                updateRaceHud();
            }

            m_nextCheckpointIndex = 0;
            m_hasLeftNorthSector = false;
            m_blockCheckpointsUntilLeaveNorth = true;
        }
    }

    if (!m_blockCheckpointsUntilLeaveNorth && m_nextCheckpointIndex < checkpoints.size()) {
        if (m_nextCheckpointIndex == 0) {
            const bool leavingNorth = !inNorthSector && wasInNorthSector;
            PhantomDrive::Checkpoint* cp0 = checkpoints.first();
            const bool crossedCp0 = m_hasLeftNorthSector
                && cp0
                && crossedCheckpointGate(cp0, positionBefore, pos);
            if (leavingNorth || crossedCp0) {
                onCheckpointReached(0);
                m_nextCheckpointIndex = 1;
            }
        } else {
            PhantomDrive::Checkpoint* nextCp = checkpoints.at(m_nextCheckpointIndex);
            const bool insideNext = nextCp && nextCp->containsPoint(pos);
            const bool enteredNext = insideNext && !m_wasInsideNextGate;
            const bool crossedNext = crossedCheckpointGate(nextCp, positionBefore, pos);
            if (enteredNext || crossedNext) {
                onCheckpointReached(m_nextCheckpointIndex);
                ++m_nextCheckpointIndex;
                m_wasInsideNextGate = false;
            }
        }
    }

    m_wasOnStartLine = onStartLine;
    m_wasInNorthGate = inNorthGate;

    if (m_nextCheckpointIndex < checkpoints.size()) {
        PhantomDrive::Checkpoint* nextCp = checkpoints.at(m_nextCheckpointIndex);
        m_wasInsideNextGate = nextCp && nextCp->containsPoint(pos);
    } else {
        m_wasInsideNextGate = false;
    }
}

void MainWindow::simulateGameLoop()
{
    m_simTimer = new QTimer(this);

    connect(m_simTimer, &QTimer::timeout, this, [this]() {
        if (!m_driveActive || m_countdownActive) {
            return;
        }

        ++m_simTick;
        m_sessionElapsedMs += 50;

        const QVector2D positionBeforeUpdate = m_playerPosition;

        if (m_vehiclePhysics) {
            m_vehiclePhysics->update(50);
        }

        if (m_arcadeRaceLogicActive) {
            updateArcadeRaceProgress(positionBeforeUpdate);
        }

        m_previousPlayerPosition = m_playerPosition;

        updateEBRuntime(0.05);
        updateTrafficAndHud(m_simTick);

        if (m_aiManager && m_driveActive) {
            m_aiManager->update(50);

            QList<AIOpponent*> opponents = m_aiManager->getAllOpponents();

            for (AIOpponent* ai : opponents) {
                if (ai) {
                    QVector2D dist = m_playerPosition - ai->getPosition();
                    if (dist.length() < 30.0) {
                        if (!m_vehiclePhysics->isColliding()) {
                            qreal impactForce = qAbs(m_playerSpeed - ai->getSpeed());
                            m_vehiclePhysics->handleCollision(
                                dist.normalized(),
                                impactForce
                            );
                            m_aiManager->onPlayerCollision(ai->getId(), m_playerPosition);
                        }
                    }

                    m_gameView->updateAICar(
                        ai->getId(),
                        ai->getPosition(),
                        ai->getRotation(),
                        ai->getSpeed()
                    );
                }
            }
        }

        if (m_drivingDataCollector && m_drivingDataCollector->vehicleSensor()) {
            VehicleSensor* sensor = m_drivingDataCollector->vehicleSensor();
            sensor->updatePosition(m_playerPosition);
            qreal rad = m_playerRotation * 3.14159265 / 180.0;
            QVector2D velocity(std::cos(rad) * m_playerSpeed, std::sin(rad) * m_playerSpeed);
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
    if (m_driveActive) {
        finishDrivingSession();
    }

    m_currentMode = mode;
    m_arcadeRaceLogicActive = (mode == QStringLiteral("Arcade"));
    m_driveActive = true;
    m_arcadeRaceFinished = false;
    m_lapsCompleted = 0;
    m_totalLaps = 3;
    m_simTick = 0;
    m_sessionElapsedMs = 0;
    m_currentLapStartMs = 0;
    m_bestLapMs = 0;
    m_playerSpeed = 0.0;

    if (m_gameView) {
        m_gameView->clearAllAICars();
    }

    if (m_drivingDataCollector) {
        m_drivingDataCollector->stopCollection();
        m_drivingDataCollector->clearData();
        m_drivingDataCollector->setCurrentSpeedLimit(m_currentSpeedLimit, "main_route_speed_zone");
        if (m_drivingDataCollector->vehicleSensor()) {
            m_drivingDataCollector->vehicleSensor()->setSpeedLimitViolationEnabled(false);
        }
        m_drivingDataCollector->startCollection();
    }
    if (m_scoreManager) {
        m_scoreManager->startSession(QStringLiteral("player"));
    }

    ui->stackedWidget->setCurrentIndex(1);
    if (m_gameView) {
        m_gameView->show();
        m_gameView->setFocus();
    }
    if (m_arcadeHUD && m_currentMode == QStringLiteral("Arcade")) {
        m_arcadeHUD->show();
        updateRaceHud();
    } else if (m_arcadeHUD) {
        m_arcadeHUD->hide();
    }
    syncRaceTrackToManager();
    setupEBRuntimeObjects();

    if (m_vehiclePhysics) {
        m_vehiclePhysics->reset();
        m_vehiclePhysics->resetRaceProgress();
        m_vehiclePhysics->setRaceLogicEnabled(false);
    }
    applyPlayerSpawnAtStartLine();
    resetArcadeRaceProgress();

    initializeAIOpponents();

    m_countdownActive = true;
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
    const int remainingSeconds = 9 - ((tick / 20) % 10);
    if (m_learningHUD) {
        const qreal currentSpeed = displaySpeedKmh();

        m_learningHUD->updateCurrentSpeed(currentSpeed);
        m_learningHUD->updateSpeedLimit(m_currentSpeedLimit);
        m_learningHUD->updateSpeedStatus(currentSpeed > m_currentSpeedLimit);
        m_learningHUD->updateTrafficLight(m_currentTrafficLightState, remainingSeconds);
        m_learningHUD->updateGameMode(m_currentMode);
    }
}

void MainWindow::onDrivingDataCollected(const DrivingData& data)
{
    if (m_gameView && m_driveActive) {
        m_gameView->updatePlayerCar(m_playerPosition, m_playerRotation, displaySpeedKmh());
        m_gameView->setCameraPosition(m_playerPosition);
    }
    updateHUD(displaySpeedKmh(), data.isBraking ? QStringLiteral("Braking") : QStringLiteral("Driving"));
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
    PhantomDrive::InteractiveFeedback& feedback = PhantomDrive::InteractiveFeedback::instance(this);
    feedback.show();
    feedback.raise();
    feedback.showCountdown(3);

    if (m_arcadeHUD && m_currentMode == QStringLiteral("Arcade")) {
        m_arcadeHUD->show();
        m_arcadeHUD->showCountdown(3);
        updateRaceHud();
    } else if (m_arcadeHUD) {
        m_arcadeHUD->hide();
    }

    playSound(PhantomDrive::SoundEffect::CountdownBeep);
    QTimer::singleShot(3200, this, &MainWindow::onRaceStart);
}

void MainWindow::onRaceStart()
{
    m_countdownActive = false;
    m_lapsCompleted = 0;
    m_currentLapStartMs = m_sessionElapsedMs;
    m_bestLapMs = 0;
    m_arcadeRaceFinished = false;

    syncRaceTrackToManager();
    resetArcadeRaceProgress();

    if (m_vehiclePhysics) {
        m_vehiclePhysics->resetRaceProgress();
        m_vehiclePhysics->setRaceLogicEnabled(false);
    }

    const int checkpointCount = m_gameView && m_gameView->trackData()
        ? m_gameView->trackData()->getCheckpointsInOrder().size()
        : 0;
    if (m_arcadeRaceLogicActive) {
        statusBar()->showMessage(
            QStringLiteral("GO! 检查点 %1 个已就绪 — 请先向南驶出北门/起跑线").arg(checkpointCount),
            6000);
    }

    PhantomDrive::InteractiveFeedback::instance(this).showGo();
    playSound(PhantomDrive::SoundEffect::CountdownGo);

    if (m_learningHUD) {
        if (m_currentMode == QStringLiteral("Learning")) {
            m_learningHUD->show();
            if (m_arcadeHUD) {
                m_arcadeHUD->hide();
            }
        } else {
            m_learningHUD->hide();
            if (m_arcadeHUD) {
                m_arcadeHUD->show();
            }
        }
        m_learningHUD->updateGameMode(m_currentMode);
    }

    if (m_arcadeHUD && m_currentMode == QStringLiteral("Arcade")) {
        m_arcadeHUD->updateLap(m_lapsCompleted, m_totalLaps);
        updateRaceHud();
    } else {
        updateRaceHud();
    }

    if (m_btnFinishDrive) {
        m_btnFinishDrive->setEnabled(true);
    }

    statusBar()->showMessage(
        QStringLiteral("%1 mode | GO! 第 %2 / %3 圈")
            .arg(m_currentMode)
            .arg(m_lapsCompleted + 1)
            .arg(m_totalLaps),
        4000);
}

void MainWindow::onLapCompleted(int lapNumber)
{
    if (m_arcadeHUD) {
        m_arcadeHUD->showLapCompleted(lapNumber);
    }

    PhantomDrive::InteractiveFeedback& feedback = PhantomDrive::InteractiveFeedback::instance(this);
    feedback.show();
    feedback.raise();
    feedback.showLapCompleted(lapNumber);
    playSound(PhantomDrive::SoundEffect::LapComplete);

    statusBar()->showMessage(
        QStringLiteral("第 %1 / %2 圈完成").arg(lapNumber).arg(m_totalLaps),
        3000);

    if (lapNumber < m_totalLaps) {
        return;
    }

    m_arcadeRaceFinished = true;

    if (m_aiManager) {
        m_aiManager->setPlayerRaceProgress(m_totalLaps, 0, 100.0, true, m_sessionElapsedMs / 1000.0);
    }

    if (m_arcadeHUD) {
        m_arcadeHUD->showRaceFinished(m_aiManager ? m_aiManager->getPlayerRacePosition() : 1,
                                      formatRaceTime(m_sessionElapsedMs));
    }

    showInteractiveFeedback(QStringLiteral("Race Finished!"), FeedbackType::Milestone);
    playSound(PhantomDrive::SoundEffect::RaceFinish);

    QTimer::singleShot(2000, this, [this]() {
        if (m_driveActive) {
            finishDrivingSession();
        }
    });
}

void MainWindow::onCheckpointReached(int checkpointNumber)
{
    const int displayIndex = checkpointNumber + 1;
    const int totalGates = m_raceCheckpointTotal > 0 ? m_raceCheckpointTotal : 4;

    if (m_arcadeHUD) {
        m_arcadeHUD->showRaceBanner(
            QStringLiteral("检查点 %1/%2 已通过").arg(displayIndex).arg(totalGates));
    }

    PhantomDrive::InteractiveFeedback& feedback = PhantomDrive::InteractiveFeedback::instance(this);
    feedback.show();
    feedback.raise();
    feedback.showCheckpoint(displayIndex);
    playSound(PhantomDrive::SoundEffect::Checkpoint);

    statusBar()->showMessage(
        QStringLiteral("检查点 %1/%2 已通过 — 请继续下一门").arg(displayIndex).arg(totalGates),
        5000);
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
        displayText = "Boost Collected!";
    } else if (powerupType.toLower().contains("shield")) {
        displayText = "Shield Active!";
    } else if (powerupType.toLower().contains("emp")) {
        displayText = "EMP Pulse!";
    } else if (powerupType.toLower().contains("repair")) {
        displayText = "Repair Kit!";
    } else {
        displayText = QString("%1 Collected!").arg(powerupType);
    }

    PhantomDrive::InteractiveFeedback::instance(this).showFeedback(displayText, type);
    playSound(PhantomDrive::SoundEffect::PowerupCollect);
}

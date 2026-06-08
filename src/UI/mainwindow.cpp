#include "UI/mainwindow.h"
#include "ui_mainwindow.h"

#include "UI/DrivingReportWidget.h"
#include "UI/CustomTrackEditorWidget.h"
#include "UI/ThemeManager.h"
#include "gamemode/CustomTrackMode.h"
#include "gamemode/VehicleSensor.h"
#include "gamemode/PowerupBox.h"
#include "gamemode/PowerupManager.h"
#include "gamemode/PowerupWorldRuntime.h"
#include "gamemode/TrafficLightObject.h"
#include "gamemode/TrafficObjectManager.h"
#include "gamemode/SpeedLimitSignObject.h"
#include "gamemode/PedestrianCrossingObject.h"
#include "gamemode/AIOpponent.h"
#include "gamemode/SimpleAIOpponent.h"
#include "track/TrackData.h"
#include "track/TrackManager.h"
#include "track/TrackTile.h"
#include "track/Checkpoint.h"
#include "track/TrackIO.h"
#include "track/TrackValidator.h"
#include "track/BuiltInTrackFactory.h"

#include <QRectF>
#include "scoring/ScoreReport.h"
#include "core/saveloadmanager.h"
#include "core/VehiclePhysics.h"

#include <QApplication>
#include <QComboBox>
#include <QDateTime>
#include <QDebug>
#include <QDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <QHash>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QSpacerItem>
#include <QStatusBar>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QVariant>
#include <QRandomGenerator>
#include <QtMath>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

#include "UI/SoundGenerator.h"

using namespace PhantomDrive;

namespace {

constexpr qreal kVehicleSeparationDistance = 48.0;
constexpr qreal kVehicleImpactDistance = 34.0;
constexpr qreal kVehicleContactReleaseDistance = 52.0;
constexpr qint64 kFinishedAiContactCooldownMs = 1000;

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
    const QPoint tileCoord = PhantomDrive::TrackData::worldToTile(position, kTileSize);
    const int row = tileCoord.y();
    const int col = tileCoord.x();
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

void dumpCustomTrackLayoutForDebug(PhantomDrive::TrackData* track, const QString& label)
{
    if (!track) {
        qDebug() << "[CustomTrackDebug]" << label << "track=null";
        return;
    }

    int roadCount = 0;
    int grassCount = 0;
    int wallCount = 0;
    QPoint finishTile(-1, -1);
    for (int row = 0; row < track->getRowCount(); ++row) {
        for (int col = 0; col < track->getColCount(); ++col) {
            const PhantomDrive::TrackTile* tile = track->getTileAt(row, col);
            if (!tile) {
                continue;
            }
            switch (tile->getType()) {
            case PhantomDrive::TileType::Road:
            case PhantomDrive::TileType::Asphalt:
                ++roadCount;
                break;
            case PhantomDrive::TileType::Grass:
                ++grassCount;
                break;
            case PhantomDrive::TileType::Wall:
            case PhantomDrive::TileType::Barrier:
                ++wallCount;
                break;
            case PhantomDrive::TileType::FinishLine:
            case PhantomDrive::TileType::StartLine:
                finishTile = QPoint(col, row);
                ++roadCount;
                break;
            default:
                break;
            }
        }
    }

    const QList<QVector2D> starts = track->getStartPositions();
    const QPoint startTile = starts.isEmpty() ? QPoint(-1, -1) : PhantomDrive::TrackData::worldToTile(starts.first());
    qDebug().noquote()
        << QStringLiteral("[CustomTrackDebug] %1 rows=%2 cols=%3 start(row=%4,col=%5) finish(row=%6,col=%7) road=%8 grass=%9 wall=%10")
               .arg(label)
               .arg(track->getRowCount())
               .arg(track->getColCount())
               .arg(startTile.y())
               .arg(startTile.x())
               .arg(finishTile.y())
               .arg(finishTile.x())
               .arg(roadCount)
               .arg(grassCount)
               .arg(wallCount);

    const QList<PhantomDrive::Checkpoint*> checkpoints = track->getCheckpointsInOrder();
    for (int i = 0; i < checkpoints.size(); ++i) {
        const PhantomDrive::Checkpoint* cp = checkpoints.at(i);
        if (!cp) {
            continue;
        }
        const QPoint cpTile = PhantomDrive::TrackData::worldToTile(cp->getPosition());
        qDebug().noquote()
            << QStringLiteral("[CustomTrackDebug] %1 CP%2 row=%3 col=%4")
                   .arg(label)
                   .arg(i + 1)
                   .arg(cpTile.y())
                   .arg(cpTile.x());
    }

    const QList<QVector2D> items = track->getItemBoxPositions();
    for (int i = 0; i < items.size(); ++i) {
        const QPoint itemTile = PhantomDrive::TrackData::worldToTile(items.at(i));
        qDebug().noquote()
            << QStringLiteral("[CustomTrackDebug] %1 Item%2 row=%3 col=%4")
                   .arg(label)
                   .arg(i + 1)
                   .arg(itemTile.y())
                   .arg(itemTile.x());
    }
}

PhantomDrive::TrackData* cloneCustomTrackSnapshot(PhantomDrive::TrackData* source, QObject* parent)
{
    if (!source) {
        return nullptr;
    }

    auto* snapshot = new PhantomDrive::TrackData(parent);
    const int rows = source->getRowCount();
    const int cols = source->getColCount();

    snapshot->setName(source->getName());
    snapshot->setId(source->getId());
    snapshot->setAuthor(source->getAuthor());
    snapshot->setDescription(source->getDescription());
    snapshot->setDifficulty(source->getDifficulty());
    snapshot->setEstimatedLapTime(source->getEstimatedLapTime());
    snapshot->setTrackLength(source->getTrackLength());
    snapshot->setMaxLaps(1);
    snapshot->setSize(rows, cols);
    snapshot->setStartRotation(source->getStartRotation());

    // Editor rows grow downward. The existing driving renderer/physics path is y-up,
    // so flip rows once in the runtime snapshot instead of adding a custom input mode.
    for (int row = 0; row < rows; ++row) {
        const int engineRow = rows - 1 - row;
        for (int col = 0; col < cols; ++col) {
            const PhantomDrive::TrackTile* srcTile = source->getTileAt(row, col);
            PhantomDrive::TrackTile* dstTile = snapshot->getTileAt(engineRow, col);
            if (srcTile && dstTile) {
                dstTile->setType(srcTile->getType());
                dstTile->setDirection(srcTile->getDirection());
                dstTile->setFriction(srcTile->getFriction());
                dstTile->setDrivable(srcTile->isDrivable());
                dstTile->setCollisionTile(srcTile->isCollisionTile());
                dstTile->setAssetId(srcTile->getAssetId());
            }
        }
    }

    const qreal trackHeight = rows * PhantomDrive::TrackData::DefaultTileSize;
    auto flipWorldY = [trackHeight](const QVector2D& pos) {
        return QVector2D(pos.x(), trackHeight - pos.y());
    };

    snapshot->clearStartPositions();
    const QList<QVector2D> starts = source->getStartPositions();
    for (const QVector2D& start : starts) {
        snapshot->addStartPosition(flipWorldY(start));
    }
    snapshot->setStartPosition(starts.isEmpty() ? flipWorldY(source->getStartPosition()) : flipWorldY(starts.first()));

    const QList<PhantomDrive::Checkpoint*> checkpoints = source->getCheckpointsInOrder();
    for (int i = 0; i < checkpoints.size(); ++i) {
        const PhantomDrive::Checkpoint* srcCp = checkpoints.at(i);
        if (!srcCp) {
            continue;
        }
        auto* cp = new PhantomDrive::Checkpoint(srcCp->getId(), flipWorldY(srcCp->getPosition()), snapshot);
        cp->setIndexInRoute(srcCp->getIndexInRoute());
        cp->setWidth(srcCp->getWidth());
        cp->setHeight(srcCp->getHeight());
        cp->setActive(srcCp->isActive());
        cp->setMandatory(srcCp->isMandatory());
        cp->setRequiredLap(srcCp->getRequiredLap());
        snapshot->addCheckpoint(cp);
    }

    snapshot->clearItemBoxPositions();
    for (const QVector2D& item : source->getItemBoxPositions()) {
        snapshot->addItemBoxPosition(flipWorldY(item));
    }

    return snapshot;
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
    , m_player2Physics(nullptr)
    , m_drivingDataCollector(new DrivingDataCollector(this))
    , m_player2DataCollector(new DrivingDataCollector(this))
    , m_scoreManager(new ScoreManager(this))
    , m_player2ScoreManager(new ScoreManager(this))
    , m_aiManager(new AIOpponentManager(this))
    , m_trafficObjectManager(new TrafficObjectManager(this))
    , m_powerupWorld(new PowerupWorldRuntime(this))
    , m_reportWidget(nullptr)
    , m_reportPage(nullptr)
    , m_btnFinishDrive(nullptr)
    , m_aiDifficultyCombo(nullptr)
    , m_btnLoadCustomTrack(nullptr)
    , m_btnCustomTrackMode(nullptr)
    , m_btnGuide(nullptr)
    , m_btnPlayCustomTrack(nullptr)
    , m_btnSaveCustomTrack(nullptr)
    , m_btnLoadCustomTrackForEdit(nullptr)
    , m_btnExportCustomTrackJson(nullptr)
    , m_customTrackEditor(nullptr)
    , m_customTrackMode(new CustomTrackMode(this))
    , m_defaultRaceTrack(nullptr)
    , m_selectedBuiltInTrack(nullptr)
    , m_runtimeCustomTrack(nullptr)
    , m_simTimer(nullptr)
    , m_learningSessionTimer(nullptr)
    , m_currentMode("Arcade")
    , m_customTrackPath()
    , m_trackSelectCombo(nullptr)
    , m_playerCountCombo(nullptr)
    , m_selectedTrackId(QStringLiteral("neon_loop"))
    , m_twoPlayerMode(false)
    , m_currentSpeedLimit(60)
    , m_currentTrafficLightState("green")
    , m_driveActive(false)
    , m_countdownActive(false)
    , m_arcadeRaceFinished(false)
    , m_customTrackPlaying(false)
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
    , m_player2Position(360.0, 320.0)
    , m_previousPlayer2Position(360.0, 320.0)
    , m_player2Rotation(-90.0)
    , m_player2Speed(0.0)
    , m_player2LapsCompleted(0)
    , m_player2NextCheckpointIndex(0)
    , m_player2WasInsideNextGate(false)
    , m_twoPlayerFinishHandled(false)
    , m_player1PendingReport()
    , m_player2PendingReport()
    , m_player1PendingSamples()
    , m_player2PendingSamples()
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
    resize(1280, 900);

    // Apply consistent styles to the top HUD buttons
    if (ui->btn_Back) {
        ui->btn_Back->setFixedHeight(52);
        ui->btn_Back->setStyleSheet(R"(
            QPushButton {
                background: rgba(15, 40, 90, 220);
                color: #00CCFF;
                border: 2px solid #0088CC;
                border-radius: 8px;
                padding: 4px 16px;
                font-size: 12px;
                font-weight: bold;
                letter-spacing: 1px;
            }
            QPushButton:hover {
                background: rgba(20, 55, 120, 240);
                border-color: #00AAEE;
            }
            QPushButton:pressed {
                background: rgba(10, 30, 80, 240);
            }
        )");
    }
    if (ui->btn_FinishDrive_Top) {
        ui->btn_FinishDrive_Top->setFixedHeight(52);
        ui->btn_FinishDrive_Top->setStyleSheet(R"(
            QPushButton {
                background: rgba(180, 20, 40, 200);
                color: #FF6688;
                border: 2px solid #FF2255;
                border-radius: 8px;
                padding: 4px 14px;
                font-size: 12px;
                font-weight: bold;
                letter-spacing: 1px;
            }
            QPushButton:hover {
                background: rgba(220, 30, 50, 230);
                border-color: #FF4466;
            }
            QPushButton:pressed {
                background: rgba(140, 15, 30, 230);
            }
        )");
    }
    if (ui->btn_ExitGame_Top) {
        ui->btn_ExitGame_Top->setFixedHeight(52);
        ui->btn_ExitGame_Top->setStyleSheet(R"(
            QPushButton {
                background: rgba(140, 10, 25, 200);
                color: #FF9999;
                border: 2px solid #CC1133;
                border-radius: 8px;
                padding: 4px 14px;
                font-size: 12px;
                font-weight: bold;
                letter-spacing: 1px;
            }
            QPushButton:hover {
                background: rgba(180, 20, 35, 230);
                border-color: #FF2255;
            }
            QPushButton:pressed {
                background: rgba(100, 5, 15, 230);
            }
        )");
    }

    PhantomDrive::InteractiveFeedback::instance(this);
    PhantomDrive::SoundManager::instance(this);
    PhantomDrive::SoundGenerator::instance().generateAllSounds();

    QWidget* gamePage = ui->stackedWidget && ui->stackedWidget->count() > 1
        ? ui->stackedWidget->widget(1)
        : nullptr;

    m_arcadeHUD = new PhantomDrive::ArcadeHUD(gamePage ? gamePage : this);
    m_arcadeHUD->setWindowFlags(Qt::Widget);
    m_arcadeHUD->hide();

    m_learningHUD = new LearningHUD(gamePage ? gamePage : this);
    m_learningHUD->hide();

    setupGameView();

    // Tell InteractiveFeedback exactly where the game-map widget is so all
    // notifications are centred inside it and never overlap the HUD panel.
    if (m_gameView) {
        PhantomDrive::InteractiveFeedback::instance(this).setGameView(m_gameView);
    }

    // Connect ArcadeHUD banner signal → InteractiveFeedback (map-area display).
    if (m_arcadeHUD) {
        connect(m_arcadeHUD, &PhantomDrive::ArcadeHUD::centerNotificationRequested,
                this, [this](const QString& text, int durationMs) {
                    showInteractiveFeedback(text, PhantomDrive::FeedbackType::Milestone);
                    Q_UNUSED(durationMs);
                });
    }

    // Connect LearningHUD floating-message signals → InteractiveFeedback.
    if (m_learningHUD) {
        connect(m_learningHUD, &LearningHUD::penaltyMessageRequested,
                this, [this](const QString& message, int points) {
                    const QString text = points != 0
                        ? QStringLiteral("⚠ %1  %2").arg(message).arg(points)
                        : QStringLiteral("⚠ %1").arg(message);
                    showInteractiveFeedback(text, PhantomDrive::FeedbackType::Warning);
                });
        connect(m_learningHUD, &LearningHUD::violationWarningRequested,
                this, [this](const QString& violationType) {
                    PhantomDrive::InteractiveFeedback::instance(this)
                        .showFeedback(QStringLiteral("⚠ %1").arg(violationType),
                                      PhantomDrive::FeedbackType::Critical);
                });
    }

    setupVehiclePhysics();
    setupDemoControls();
    setupCustomTrackControls();
    setupRaceSetupControls();
    styleMainMenu();
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
                showInteractiveFeedback(QString("%1 Finished Rank #%2").arg(opponentId).arg(finalPosition),
                                        FeedbackType::Milestone);
            });

    qDebug() << "=== QLearning Connected ===";

    qApp->setStyleSheet(ThemeManager::getStyleSheet("dark") + ThemeManager::mainMenuNeonQss());

    // Build the report page (stackedWidget index 2) and embed the report widget.
    // We do this programmatically so it works regardless of whether the .ui
    // file has been regenerated after adding pageReport.
    {
        QWidget* pageReport = (ui->stackedWidget->count() > 2)
            ? ui->stackedWidget->widget(2)
            : nullptr;

        if (!pageReport) {
            pageReport = new QWidget();
            QVBoxLayout* lay = new QVBoxLayout(pageReport);
            lay->setContentsMargins(0, 0, 0, 0);
            lay->setSpacing(0);
            ui->stackedWidget->addWidget(pageReport);  // becomes index 2
        } else if (!pageReport->layout()) {
            QVBoxLayout* lay = new QVBoxLayout(pageReport);
            lay->setContentsMargins(0, 0, 0, 0);
            lay->setSpacing(0);
        }

        // Create the report widget with pageReport as parent so Qt's layout
        // system never places it outside the stackedWidget.
        m_reportWidget = new DrivingReportWidget(pageReport);
        m_reportWidget->setMockDataEnabled(false);
        pageReport->layout()->addWidget(m_reportWidget);
        m_reportPage = pageReport;   // cache for use in onGameFinished()

        qDebug() << "[setup] stackedWidget count=" << ui->stackedWidget->count()
                 << "pageReport=" << pageReport
                 << "m_reportWidget=" << m_reportWidget;
    }

    connect(m_reportWidget, &DrivingReportWidget::backToMenuRequested,
            this, &MainWindow::onReportBackToMenu);
    connect(m_reportWidget, &DrivingReportWidget::newDriveRequested,
            this, &MainWindow::onReportNewDrive);

    m_scoreManager->setVehicleId("player_1");
    if (m_player2ScoreManager) {
        m_player2ScoreManager->setVehicleId(QStringLiteral("player_2"));
    }

    if (ui->btn_Arcade) {
        connect(ui->btn_Arcade, &QPushButton::clicked, this, [this]() {
            showArcadeSetupDialog();
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
            // If the report page is currently shown, delegate to the report's
            // own "Back to Menu" handler so the user must explicitly exit there.
            QWidget* reportPage = m_reportPage ? m_reportPage
                                               : (ui->pageReport ? static_cast<QWidget*>(ui->pageReport)
                                                                  : (ui->stackedWidget->count() > 2 ? ui->stackedWidget->widget(2) : nullptr));
            if (reportPage && ui->stackedWidget->currentWidget() == reportPage) {
                onReportBackToMenu();
                return;
            }

            if (m_currentMode == QStringLiteral("Custom Track") && !m_customTrackPlaying) {
                hideCustomTrackEditor();
                ui->stackedWidget->setCurrentIndex(0);
                statusBar()->clearMessage();
                return;
            }
            if (m_driveActive) {
                // Session is still running: end it and show the report.
                // The report window will handle navigation back to menu.
                onGameFinished();
                return;
            }
            // Not driving and not on report page: just go back to the menu.
            m_customTrackPlaying = false;
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

    m_drivingDataCollector->setVehicleId("player_1");
    m_drivingDataCollector->setSamplingInterval(50);
    m_drivingDataCollector->setCurrentSpeedLimit(m_currentSpeedLimit, "main_route_speed_zone");
    if (m_drivingDataCollector->vehicleSensor()) {
        m_drivingDataCollector->vehicleSensor()->setSpeedLimitViolationEnabled(false);
    }
    if (m_player2DataCollector) {
        m_player2DataCollector->setVehicleId(QStringLiteral("player_2"));
        m_player2DataCollector->setSamplingInterval(50);
        m_player2DataCollector->setCurrentSpeedLimit(m_currentSpeedLimit, "main_route_speed_zone");
        if (m_player2DataCollector->vehicleSensor()) {
            m_player2DataCollector->vehicleSensor()->setSpeedLimitViolationEnabled(false);
        }
    }

    simulateGameLoop();
}

MainWindow::~MainWindow()
{
    if (m_drivingDataCollector) {
        m_drivingDataCollector->stopCollection();
    }
    if (m_player2DataCollector) {
        m_player2DataCollector->stopCollection();
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

    if (!m_btnLoadCustomTrack && ui->verticalLayout) {
        m_btnLoadCustomTrack = new QPushButton(QStringLiteral("Load Custom Track"), this);
        m_btnLoadCustomTrack->setObjectName(QStringLiteral("btn_LoadCustomTrack"));

        const int historyIndex = ui->btn_History ? ui->verticalLayout->indexOf(ui->btn_History) : -1;
        ui->verticalLayout->insertWidget(historyIndex >= 0 ? historyIndex : ui->verticalLayout->count(),
                                        m_btnLoadCustomTrack);

        connect(m_btnLoadCustomTrack, &QPushButton::clicked,
                this, &MainWindow::loadCustomTrack);
    }

    // m_btnFinishDrive (menu bar) → end game
    connect(m_btnFinishDrive, &QPushButton::clicked,
            this, &MainWindow::onGameFinished);

    // Top HUD buttons (from .ui hudLayout): Back, Finish Drive, Exit Game
    if (ui->btn_Back) {
        // Back is already wired to handleReportBackToMenu in the btn_Back click handler above
    }
    if (ui->btn_FinishDrive_Top) {
        connect(ui->btn_FinishDrive_Top, &QPushButton::clicked,
                this, &MainWindow::onGameFinished);
    }
    if (ui->btn_ExitGame_Top) {
        connect(ui->btn_ExitGame_Top, &QPushButton::clicked,
                this, &MainWindow::onGameFinished);
    }
}

void MainWindow::setupCustomTrackControls()
{
    if (!m_btnCustomTrackMode && ui->verticalLayout) {
        m_btnCustomTrackMode = new QPushButton(QStringLiteral("Custom Track Mode"), this);
        m_btnCustomTrackMode->setObjectName(QStringLiteral("btn_CustomTrackMode"));

        const int learningIndex = ui->btn_Learn ? ui->verticalLayout->indexOf(ui->btn_Learn) : -1;
        const int insertIndex = learningIndex >= 0 ? learningIndex + 1 : ui->verticalLayout->count();
        ui->verticalLayout->insertWidget(insertIndex, m_btnCustomTrackMode);

        connect(m_btnCustomTrackMode, &QPushButton::clicked,
                this, &MainWindow::showCustomTrackEditor);
    }

    if (!m_customTrackEditor && ui->stackedWidget && ui->stackedWidget->count() > 1) {
        QWidget* gamePage = ui->stackedWidget->widget(1);
        QVBoxLayout* pageLayout = gamePage ? qobject_cast<QVBoxLayout*>(gamePage->layout()) : nullptr;
        if (pageLayout) {
            m_customTrackEditor = new CustomTrackEditorWidget(gamePage);
            m_customTrackEditor->hide();
            pageLayout->addWidget(m_customTrackEditor, 1);

            connect(m_customTrackEditor, &CustomTrackEditorWidget::playRequested,
                    this, &MainWindow::playCurrentCustomTrack);
            connect(m_customTrackEditor, &CustomTrackEditorWidget::saveRequested,
                    this, &MainWindow::saveCurrentCustomTrack);
            connect(m_customTrackEditor, &CustomTrackEditorWidget::loadRequested,
                    this, &MainWindow::loadCustomTrackIntoEditor);
            connect(m_customTrackEditor, &CustomTrackEditorWidget::exportJsonRequested,
                    this, &MainWindow::exportCurrentCustomTrackJson);
            connect(m_customTrackEditor, &CustomTrackEditorWidget::backRequested,
                    this, [this]() {
                        hideCustomTrackEditor();
                        if (ui->stackedWidget) {
                            ui->stackedWidget->setCurrentIndex(0);
                        }
                        statusBar()->clearMessage();
                    });
            connect(m_customTrackEditor, &CustomTrackEditorWidget::trackChanged,
                    this, [this](TrackData* track) {
                        if (m_customTrackMode) {
                            m_customTrackMode->setTrack(track);
                        }
                    });
        }
    }
}

void MainWindow::setupRaceSetupControls()
{
    if (!m_btnGuide && ui->verticalLayout) {
        m_btnGuide = new QPushButton(QStringLiteral("Guide / Powerups"), this);
        m_btnGuide->setObjectName(QStringLiteral("btn_Guide"));

        const int historyIndex = ui->btn_History ? ui->verticalLayout->indexOf(ui->btn_History) : -1;
        ui->verticalLayout->insertWidget(historyIndex >= 0 ? historyIndex : ui->verticalLayout->count(), m_btnGuide);

        connect(m_btnGuide, &QPushButton::clicked, this, &MainWindow::showGuideDialog);
    }
}

void MainWindow::showArcadeSetupDialog()
{
    if (!ui->stackedWidget) {
        startBuiltInTrackSession(QStringLiteral("Arcade"));
        return;
    }

    QWidget* setupPage = ui->stackedWidget->findChild<QWidget*>(QStringLiteral("pageArcadeSetup"));
    if (!setupPage) {
        setupPage = new QWidget(ui->stackedWidget);
        setupPage->setObjectName(QStringLiteral("pageArcadeSetup"));
        setupPage->setStyleSheet(QStringLiteral(
            "QWidget#pageArcadeSetup{background:#06101F;color:#EAFBFF;}"
            "QLabel#label_ArcadeSetupTitle,QLabel.arcadeSetupSection{"
            "color:#38F6FF;font-size:20px;font-weight:800;letter-spacing:7px;}"
            "QComboBox{background:#071126;color:#F3FBFF;border:2px solid #00CFE8;"
            "border-radius:10px;padding:0 28px;font-size:26px;font-weight:700;min-height:78px;}"
            "QComboBox::drop-down{width:74px;border-left:1px solid #008FB0;}"
            "QPushButton{background:#071126;color:#F3FBFF;border:2px solid #00CFE8;"
            "border-radius:10px;min-height:64px;font-size:22px;font-weight:800;}"
            "QPushButton:hover{background:#0A1C38;border-color:#38F6FF;}"
            "QPushButton#btn_StartArcade{background:#0B2734;border-color:#35F6FF;color:#FFFFFF;}"));

        auto* outer = new QVBoxLayout(setupPage);
        outer->setContentsMargins(18, 22, 18, 22);
        outer->setSpacing(18);

        auto* title = new QLabel(QStringLiteral("Race Setup"), setupPage);
        title->setObjectName(QStringLiteral("label_ArcadeSetupTitle"));
        title->setAlignment(Qt::AlignCenter);
        outer->addWidget(title);

        m_trackSelectCombo = new QComboBox(setupPage);
        m_trackSelectCombo->setObjectName(QStringLiteral("combo_BuiltInTrack"));
        for (const BuiltInTrackInfo& info : BuiltInTrackFactory::tracks()) {
            m_trackSelectCombo->addItem(info.name, info.id);
        }
        outer->addWidget(m_trackSelectCombo);

        m_playerCountCombo = new QComboBox(setupPage);
        m_playerCountCombo->setObjectName(QStringLiteral("combo_PlayerCount"));
        m_playerCountCombo->addItem(QStringLiteral("1 Player"), 1);
        m_playerCountCombo->addItem(QStringLiteral("2 Players"), 2);
        outer->addWidget(m_playerCountCombo);

        auto* difficultyLabel = new QLabel(QStringLiteral("AI Difficulty"), setupPage);
        difficultyLabel->setProperty("class", QStringLiteral("arcadeSetupSection"));
        difficultyLabel->setAlignment(Qt::AlignCenter);
        outer->addWidget(difficultyLabel);

        m_aiDifficultyCombo = new QComboBox(setupPage);
        m_aiDifficultyCombo->setObjectName(QStringLiteral("combo_AIDifficulty"));
        m_aiDifficultyCombo->addItem(QStringLiteral("Easy"), QStringLiteral("easy"));
        m_aiDifficultyCombo->addItem(QStringLiteral("Medium"), QStringLiteral("medium"));
        m_aiDifficultyCombo->addItem(QStringLiteral("Hard"), QStringLiteral("hard"));
        m_aiDifficultyCombo->addItem(QStringLiteral("Adaptive"), QStringLiteral("adaptive"));
        m_aiDifficultyCombo->setCurrentIndex(1);
        outer->addWidget(m_aiDifficultyCombo);

        auto* buttonRow = new QHBoxLayout();
        buttonRow->setContentsMargins(0, 8, 0, 0);
        buttonRow->setSpacing(14);

        auto* backButton = new QPushButton(QStringLiteral("Back"), setupPage);
        backButton->setObjectName(QStringLiteral("btn_BackArcadeSetup"));
        auto* startButton = new QPushButton(QStringLiteral("Start Game"), setupPage);
        startButton->setObjectName(QStringLiteral("btn_StartArcade"));
        buttonRow->addWidget(backButton);
        buttonRow->addWidget(startButton);
        outer->addLayout(buttonRow);
        outer->addStretch(1);

        connect(backButton, &QPushButton::clicked, this, [this]() {
            if (ui->stackedWidget) {
                ui->stackedWidget->setCurrentIndex(0);
            }
        });
        connect(startButton, &QPushButton::clicked, this, [this]() {
            if (m_trackSelectCombo) {
                m_selectedTrackId = m_trackSelectCombo->currentData().toString();
            }
            m_twoPlayerMode = selectedPlayerCount() == 2;
            startBuiltInTrackSession(QStringLiteral("Arcade"));
        });

        ui->stackedWidget->addWidget(setupPage);
    }

    if (m_trackSelectCombo) {
        const int trackIndex = m_trackSelectCombo->findData(m_selectedTrackId);
        m_trackSelectCombo->setCurrentIndex(trackIndex >= 0 ? trackIndex : 0);
    }
    if (m_playerCountCombo) {
        m_playerCountCombo->setCurrentIndex(m_twoPlayerMode ? 1 : 0);
    }
    ui->stackedWidget->setCurrentWidget(setupPage);
    statusBar()->clearMessage();
}

void MainWindow::styleMainMenu()
{
    if (ui->label) {
        ui->label->setObjectName(QStringLiteral("label_MainLogo"));
        ui->label->setText(QStringLiteral("PHANTOMDRIVE"));
        ui->label->setMinimumHeight(120);
    }

    if (ui->menuGridLayout) {
        ui->menuGridLayout->setContentsMargins(96, 74, 96, 74);
        ui->menuGridLayout->setVerticalSpacing(20);
        ui->menuGridLayout->setRowStretch(0, 0);
        ui->menuGridLayout->setRowStretch(1, 1);
    }

    if (ui->verticalLayout) {
        ui->verticalLayout->setSpacing(14);
        ui->verticalLayout->setContentsMargins(0, 0, 0, 0);
        ui->verticalLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    }

    const QList<QPushButton*> menuButtons = {
        ui->btn_Arcade,
        ui->btn_Learn,
        m_btnCustomTrackMode,
        ui->btn_History,
        m_btnLoadCustomTrack,
        m_btnGuide,
        ui->btn_Exit
    };
    for (QPushButton* button : menuButtons) {
        if (!button) {
            continue;
        }
        button->setCursor(Qt::PointingHandCursor);
        button->setMinimumSize(620, 58);
        button->setMaximumSize(860, 66);
    }

    if (m_aiDifficultyCombo) {
        m_aiDifficultyCombo->setMinimumSize(620, 56);
        m_aiDifficultyCombo->setMaximumSize(860, 62);
        m_aiDifficultyCombo->setCursor(Qt::PointingHandCursor);
    }
    if (m_trackSelectCombo) {
        m_trackSelectCombo->setMinimumSize(620, 56);
        m_trackSelectCombo->setMaximumSize(860, 62);
        m_trackSelectCombo->setCursor(Qt::PointingHandCursor);
    }
    if (m_playerCountCombo) {
        m_playerCountCombo->setMinimumSize(620, 56);
        m_playerCountCombo->setMaximumSize(860, 62);
        m_playerCountCombo->setCursor(Qt::PointingHandCursor);
    }
}

void MainWindow::setGameHeaderVisible(bool visible)
{
    if (ui->label_Speed) {
        ui->label_Speed->setVisible(visible);
    }
    if (ui->label_Limit) {
        ui->label_Limit->setVisible(visible);
    }
    if (ui->label_) {
        ui->label_->setVisible(visible);
    }
    if (ui->label_ModeTitle) {
        ui->label_ModeTitle->setVisible(visible);
    }
    if (ui->btn_Back) {
        ui->btn_Back->setVisible(visible);
    }
    if (ui->btn_FinishDrive_Top) {
        ui->btn_FinishDrive_Top->setVisible(visible);
    }
    if (ui->btn_ExitGame_Top) {
        ui->btn_ExitGame_Top->setVisible(visible);
    }

    QWidget* gamePage = ui->stackedWidget && ui->stackedWidget->count() > 1
        ? ui->stackedWidget->widget(1)
        : nullptr;
    QVBoxLayout* pageLayout = gamePage ? qobject_cast<QVBoxLayout*>(gamePage->layout()) : nullptr;
    if (!pageLayout || pageLayout->count() == 0) {
        return;
    }

    pageLayout->setContentsMargins(visible ? 8 : 0,
                                   visible ? 8 : 0,
                                   visible ? 8 : 0,
                                   visible ? 8 : 0);
    pageLayout->setSpacing(visible ? 4 : 0);

    QLayoutItem* headerItem = pageLayout->itemAt(0);
    QHBoxLayout* hudLayout = headerItem ? qobject_cast<QHBoxLayout*>(headerItem->layout()) : nullptr;
    if (!hudLayout) {
        return;
    }

    hudLayout->setSpacing(visible ? 12 : 0);
    for (int i = 0; i < hudLayout->count(); ++i) {
        QLayoutItem* item = hudLayout->itemAt(i);
        QSpacerItem* spacer = item ? item->spacerItem() : nullptr;
        if (spacer) {
            spacer->changeSize(visible ? 20 : 0,
                               visible ? 20 : 0,
                               visible ? QSizePolicy::Expanding : QSizePolicy::Fixed,
                               QSizePolicy::Fixed);
        }
    }

    hudLayout->invalidate();
    pageLayout->invalidate();
}

void MainWindow::showCustomTrackEditor()
{
    if (m_driveActive) {
        silentFinishSession();
    }

    m_currentMode = QStringLiteral("Custom Track");
    m_customTrackPlaying = false;
    m_arcadeRaceLogicActive = false;
    m_countdownActive = false;

    if (m_customTrackMode) {
        m_customTrackMode->onEnter();
        m_customTrackMode->setCustomState(CustomTrackModeState::Editing);
        if (m_customTrackEditor) {
            m_customTrackMode->setTrack(m_customTrackEditor->trackData());
        }
    }

    if (ui->stackedWidget) {
        ui->stackedWidget->setCurrentIndex(1);
    }
    if (m_gameView) {
        m_gameView->hide();
    }
    if (m_arcadeHUD) {
        m_arcadeHUD->hide();
    }
    if (m_learningHUD) {
        m_learningHUD->hide();
    }
    if (m_btnFinishDrive) {
        m_btnFinishDrive->setEnabled(false);
    }
    setGameHeaderVisible(false);
    if (m_customTrackEditor) {
        m_customTrackEditor->show();
        m_customTrackEditor->setFocus();
    }

    clearEBRuntimeObjects();
    statusBar()->showMessage(QStringLiteral("Custom Track Mode: edit a 24 x 18 tile track"));
}

void MainWindow::hideCustomTrackEditor()
{
    if (m_customTrackEditor) {
        m_customTrackEditor->hide();
        m_customTrackEditor->clearFocus();
    }
    if (m_btnFinishDrive) {
        m_btnFinishDrive->show();
    }
    setGameHeaderVisible(true);
    if (m_customTrackMode) {
        m_customTrackMode->setCustomState(CustomTrackModeState::Editing);
    }
}

void MainWindow::playCurrentCustomTrack()
{
    TrackData* track = m_customTrackEditor ? m_customTrackEditor->trackData() : nullptr;
    if (!track) {
        QMessageBox::warning(this,
                             QStringLiteral("Custom Track"),
                             QStringLiteral("No custom track is available to play."));
        return;
    }

    const TrackValidationResult validation = TrackValidator::validateCustomTrack(*track);
    if (!validation.ok) {
        QMessageBox::warning(this,
                             QStringLiteral("Track Is Not Playable"),
                             validation.errors.join(QStringLiteral("\n")));
        return;
    }

    dumpCustomTrackLayoutForDebug(track, QStringLiteral("Editor before Play snapshot"));
    if (m_runtimeCustomTrack) {
        m_runtimeCustomTrack->deleteLater();
        m_runtimeCustomTrack = nullptr;
    }
    m_runtimeCustomTrack = cloneCustomTrackSnapshot(track, this);
    dumpCustomTrackLayoutForDebug(m_runtimeCustomTrack, QStringLiteral("Runtime Play snapshot"));

    startCustomTrackSession(m_runtimeCustomTrack);
}

void MainWindow::saveCurrentCustomTrack()
{
    TrackData* track = m_customTrackEditor ? m_customTrackEditor->trackData() : nullptr;
    if (!track) {
        return;
    }

    const QString filePath = QFileDialog::getSaveFileName(
        this,
        QStringLiteral("Save Custom Track"),
        QString(),
        QStringLiteral("PhantomDrive Track (*.pdtrack)"));
    if (filePath.isEmpty()) {
        return;
    }

    QString finalPath = filePath;
    if (QFileInfo(finalPath).suffix().isEmpty()) {
        finalPath += QStringLiteral(".pdtrack");
    }

    TrackIO io;
    if (!io.saveTrack(track, finalPath)) {
        QMessageBox::warning(this,
                             QStringLiteral("Track Save Failed"),
                             io.getLastError());
        return;
    }

    statusBar()->showMessage(QStringLiteral("Saved custom track: %1").arg(finalPath), 5000);
}

void MainWindow::loadCustomTrackIntoEditor()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("Load Custom Track Into Editor"),
        QString(),
        QStringLiteral("PhantomDrive Track (*.pdtrack *.json)"));
    if (filePath.isEmpty()) {
        return;
    }

    TrackIO io;
    TrackData* track = io.loadTrack(filePath);
    if (!track) {
        QMessageBox::warning(this,
                             QStringLiteral("Track Load Failed"),
                             io.getLastError());
        return;
    }

    track->setMaxLaps(1);
    if (m_customTrackEditor) {
        m_customTrackEditor->setTrackData(track);
    }
    if (m_customTrackMode) {
        m_customTrackMode->setTrack(track);
    }
    statusBar()->showMessage(QStringLiteral("Loaded custom track into editor: %1").arg(filePath), 5000);
}

void MainWindow::exportCurrentCustomTrackJson()
{
    TrackData* track = m_customTrackEditor ? m_customTrackEditor->trackData() : nullptr;
    if (!track) {
        return;
    }

    const QString filePath = QFileDialog::getSaveFileName(
        this,
        QStringLiteral("Export Custom Track JSON"),
        QString(),
        QStringLiteral("JSON (*.json)"));
    if (filePath.isEmpty()) {
        return;
    }

    QString finalPath = filePath;
    if (QFileInfo(finalPath).suffix().isEmpty()) {
        finalPath += QStringLiteral(".json");
    }

    TrackIO io;
    if (!io.saveTrack(track, finalPath)) {
        QMessageBox::warning(this,
                             QStringLiteral("JSON Export Failed"),
                             io.getLastError());
        return;
    }

    statusBar()->showMessage(QStringLiteral("Exported custom track JSON: %1").arg(finalPath), 5000);
}

void MainWindow::restoreDefaultRaceTrack()
{
    if (!m_defaultRaceTrack || !m_gameView) {
        return;
    }

    m_gameView->setTrackData(m_defaultRaceTrack);
    TrackManager* trackMgr = TrackManager::instance(this);
    if (trackMgr) {
        trackMgr->setCurrentTrack(m_defaultRaceTrack);
        QList<QVector2D> waypoints;
        for (Checkpoint* cp : m_defaultRaceTrack->getCheckpointsInOrder()) {
            if (cp) {
                waypoints.append(cp->getPosition());
            }
        }
        waypoints.append(m_defaultRaceTrack->getStartPosition());
        trackMgr->setWaypoints(waypoints);
    }
}

void MainWindow::focusGameViewForDriving()
{
    if (!m_gameView) {
        return;
    }

    m_gameView->show();
    m_gameView->raise();
    m_gameView->activateWindow();
    m_gameView->setFocus(Qt::OtherFocusReason);
}

int MainWindow::selectedPlayerCount() const
{
    return m_playerCountCombo ? m_playerCountCombo->currentData().toInt() : 1;
}

bool MainWindow::isTwoPlayerSelected() const
{
    return selectedPlayerCount() == 2;
}

void MainWindow::preparePlayerReportSystems()
{
    if (m_drivingDataCollector) {
        m_drivingDataCollector->stopCollection();
        m_drivingDataCollector->clearData();
        m_drivingDataCollector->setVehicleId(QStringLiteral("player_1"));
        m_drivingDataCollector->setCurrentSpeedLimit(m_currentSpeedLimit, "main_route_speed_zone");
        if (m_drivingDataCollector->vehicleSensor()) {
            m_drivingDataCollector->vehicleSensor()->setSpeedLimitViolationEnabled(false);
        }
        m_drivingDataCollector->startCollection();
    }
    if (m_scoreManager) {
        m_scoreManager->startSession(QStringLiteral("player_1"));
    }

    if (!m_twoPlayerMode) {
        if (m_player2DataCollector) {
            m_player2DataCollector->stopCollection();
            m_player2DataCollector->clearData();
        }
        return;
    }

    if (m_player2DataCollector) {
        m_player2DataCollector->stopCollection();
        m_player2DataCollector->clearData();
        m_player2DataCollector->setVehicleId(QStringLiteral("player_2"));
        m_player2DataCollector->setCurrentSpeedLimit(m_currentSpeedLimit, "main_route_speed_zone");
        if (m_player2DataCollector->vehicleSensor()) {
            m_player2DataCollector->vehicleSensor()->setSpeedLimitViolationEnabled(false);
        }
        m_player2DataCollector->startCollection();
    }
    if (m_player2ScoreManager) {
        m_player2ScoreManager->startSession(QStringLiteral("player_2"));
    }
}

void MainWindow::applyPlayer2SpawnAtStartLine()
{
    if (!m_player2Physics) {
        return;
    }

    const QVector2D base = m_playerPosition;
    const qreal radians = qDegreesToRadians(m_playerRotation);
    const QVector2D right(qCos(radians), -qSin(radians));
    const QVector2D back(-qSin(radians), -qCos(radians));
    const QVector2D candidates[] = {
        base + right * 52.0,
        base - right * 52.0,
        base + back * 64.0,
        base - back * 64.0
    };

    QVector2D spawn = base + right * 52.0;
    for (const QVector2D& candidate : candidates) {
        if (m_player2Physics->isPositionFree(candidate)) {
            spawn = candidate;
            break;
        }
    }

    m_player2Physics->setPosition(spawn);
    m_player2Physics->setRotation(m_playerRotation);
    m_player2Physics->clearDriveInput();
    m_player2Position = spawn;
    m_previousPlayer2Position = spawn;
    m_player2Rotation = m_playerRotation;
    m_player2Speed = 0.0;
    m_player2LapsCompleted = 0;
    m_player2NextCheckpointIndex = 0;
    m_player2WasInsideNextGate = false;
}

void MainWindow::updateTwoPlayerCamera()
{
    if (!m_gameView || !m_twoPlayerMode) {
        return;
    }
    const QVector2D midpoint = (m_playerPosition + m_player2Position) * 0.5;
    const qreal distance = (m_playerPosition - m_player2Position).length();
    const qreal zoom = qBound<qreal>(0.55, 1.1 - distance / 1600.0, 1.15);
    m_gameView->setCameraPosition(midpoint);
    m_gameView->setCameraZoom(zoom);
}

void MainWindow::showGuideDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("PhantomDrive Guide"));
    dialog.resize(760, 620);
    dialog.setStyleSheet(QStringLiteral(
        "QDialog{background:#06101F;color:#EAFBFF;}"
        "QTextEdit{background:#0B1830;color:#EAFBFF;border:1px solid #29E6FF;"
        "font-size:14px;padding:14px;selection-background-color:#FF2D75;}"));

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    auto* text = new QTextEdit(&dialog);
    text->setReadOnly(true);
    text->setHtml(QStringLiteral(
        "<h1 style='color:#4DFBFF'>PHANTOMDRIVE GUIDE</h1>"
        "<h2>Controls</h2>"
        "<p><b>P1</b>: W accelerate, S brake/reverse, A/D steer.</p>"
        "<p><b>P2</b>: Arrow Up accelerate, Down brake/reverse, Left/Right steer.</p>"
        "<h2>Race Goal</h2>"
        "<p>Pass checkpoints in order, then cross the start/finish gate to complete the route.</p>"
        "<h2>Powerups</h2>"
        "<ul>"
        "<li><b>Boost</b>: short speed burst.</li>"
        "<li><b>Shield</b>: reduces collision penalty and protects momentum.</li>"
        "<li><b>Missile</b>: launches at the best target ahead.</li>"
        "<li><b>Oil Slick</b>: drops a slipping hazard behind the car.</li>"
        "<li><b>EMP</b>: slows AI opponents in range.</li>"
        "<li><b>Invisibility</b>: temporary no-collision phase.</li>"
        "<li><b>Repair</b>: restores stability and speed.</li>"
        "<li><b>Teleport</b>: jumps forward along the current direction.</li>"
        "<li><b>Magnet</b>: increases nearby item collection radius.</li>"
        "<li><b>Random</b>: rolls one of the active powerups.</li>"
        "</ul>"));
    layout->addWidget(text);

    QPushButton* closeButton = new QPushButton(QStringLiteral("Close"), &dialog);
    closeButton->setMinimumHeight(44);
    closeButton->setStyleSheet(QStringLiteral(
        "QPushButton{background:#10264A;color:#EAFBFF;border:1px solid #29E6FF;"
        "border-radius:7px;font-weight:bold;} QPushButton:hover{background:#173B6C;}"));
    layout->addWidget(closeButton);
    connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    dialog.exec();
}

void MainWindow::setupGameView()
{
    QWidget* gamePage = ui->stackedWidget && ui->stackedWidget->count() > 1
        ? ui->stackedWidget->widget(1)
        : nullptr;

    m_gameView = new GameViewWidget(gamePage ? gamePage : this);
    m_gameView->hide();

    if (gamePage) {
        QWidget* page = gamePage;
        if (page) {
            QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(page->layout());
            if (!layout) {
                layout = new QVBoxLayout(page);
                layout->setContentsMargins(0, 32, 0, 0);
                layout->setSpacing(0);
                page->setLayout(layout);
            }
            QHBoxLayout* playLayout = new QHBoxLayout();
            playLayout->setContentsMargins(0, 0, 0, 0);
            playLayout->setSpacing(8);
            playLayout->addWidget(m_gameView, 1);

            if (m_arcadeHUD) {
                // Fixed-width right panel — always 300px, no stretch.
                playLayout->addWidget(m_arcadeHUD, 0, Qt::AlignTop);
            }

            layout->addLayout(playLayout, 1);

            page->setFocusPolicy(Qt::StrongFocus);
            page->setFocus();
        }
    }

    TrackManager* trackMgr = TrackManager::instance(this);
    if (trackMgr && trackMgr->hasCurrentTrack()) {
        TrackData* loadedTrack = trackMgr->getCurrentTrack();
        if (!loadedTrack->getCheckpointsInOrder().isEmpty()) {
            m_defaultRaceTrack = loadedTrack;
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
    m_defaultRaceTrack = testTrack;
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
    case PhantomDrive::PowerupType::Custom:
        return QStringLiteral("Custom");
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
        m_gameView->setPlayerEffectState(false, false, false, false);
        m_gameView->setWorldEffects({}, {}, {});
    }

    if (m_powerupWorld) {
        m_powerupWorld->clear();
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
        auto* box = new PowerupBox(position, 54.0f, this);
        box->setObjectName(id);
        box->setFixedPowerupType(type);
        box->setRespawnTime(8.0f);
        m_powerupBoxes.append(box);

        const QString typeName = powerupTypeToString(type);
        if (m_gameView) {
            m_gameView->addPowerupBox(id, position, typeName);
        }

        connect(box, &PowerupBox::collected,
                this, [this, id](const QString& playerId, const PowerupType& collectedType) {
                    if (m_gameView) {
                        m_gameView->removePowerupBox(id);
                    }
                    handlePowerupCollectedForPlayer(collectedType, playerId == QStringLiteral("player_2") ? 2 : 1);
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
    addPowerupBox(QStringLiteral("eb_missile_box"), north + QVector2D(180.0f, 0.0f), PowerupType::Missile);
    addPowerupBox(QStringLiteral("eb_oil_box"), south + QVector2D(-160.0f, 0.0f), PowerupType::OilSlick);
    addPowerupBox(QStringLiteral("eb_invis_box"), east + QVector2D(0.0f, 180.0f), PowerupType::Invisibility);
    addPowerupBox(QStringLiteral("eb_teleport_box"), west + QVector2D(0.0f, -180.0f), PowerupType::Teleport);
    addPowerupBox(QStringLiteral("eb_magnet_box"), start + QVector2D(260.0f, 260.0f), PowerupType::Magnet);
    addPowerupBox(QStringLiteral("eb_custom_box"), south + QVector2D(0.0f, -120.0f), PowerupType::Custom);

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
    m_player2Physics = new VehiclePhysics(this);
    m_vehiclePhysics->setControlScheme(VehiclePhysics::ControlScheme::Wasd);
    m_player2Physics->setControlScheme(VehiclePhysics::ControlScheme::Arrows);

    TrackManager* trackMgr = TrackManager::instance(this);
    if (trackMgr) {
        m_vehiclePhysics->initialize(trackMgr);
        m_player2Physics->initialize(trackMgr);
    }

    if (m_gameView) {
        connect(m_gameView, &GameViewWidget::keyInputReceived,
                m_vehiclePhysics, &VehiclePhysics::handleKeyPress);
        connect(m_gameView, &GameViewWidget::keyReleased,
                m_vehiclePhysics, &VehiclePhysics::handleKeyRelease);
        connect(m_gameView, &GameViewWidget::keyInputReceived,
                m_player2Physics, &VehiclePhysics::handleKeyPress);
        connect(m_gameView, &GameViewWidget::keyReleased,
                m_player2Physics, &VehiclePhysics::handleKeyRelease);
    } else {
        qWarning() << "MainWindow: game view is null; keyboard input connections skipped";
    }

    connect(m_vehiclePhysics, &VehiclePhysics::positionUpdated,
            this, [this](const QVector2D& position) {
                m_playerPosition = position;
                m_playerRotation = m_vehiclePhysics->getRotation();
                m_playerSpeed = m_vehiclePhysics->getSpeed();

                if (m_gameView && m_driveActive) {
                    m_gameView->setPlayerEffectState(m_vehiclePhysics->isSpeedBoostActive(),
                                                     m_vehiclePhysics->isShieldActive(),
                                                     m_vehiclePhysics->isInvisible(),
                                                     m_vehiclePhysics->isMagnetActive());
                    if (m_twoPlayerMode) {
                        m_gameView->updatePlayerCar(QStringLiteral("P1"),
                                                    m_playerPosition,
                                                    m_playerRotation,
                                                    displaySpeedKmh(),
                                                    QColor(255, 48, 118),
                                                    m_vehiclePhysics->isSpeedBoostActive(),
                                                    m_vehiclePhysics->isShieldActive(),
                                                    m_vehiclePhysics->isInvisible(),
                                                    m_vehiclePhysics->isMagnetActive());
                        updateTwoPlayerCamera();
                    } else {
                        m_gameView->updatePlayerCar(m_playerPosition, m_playerRotation, displaySpeedKmh());
                        m_gameView->setCameraPosition(m_playerPosition);
                    }
                }

                if (m_drivingDataCollector && m_drivingDataCollector->vehicleSensor()) {
                    VehicleSensor* sensor = m_drivingDataCollector->vehicleSensor();
                    sensor->updatePosition(m_playerPosition);
                    sensor->updateRotation(m_playerRotation);
                    qreal radians = qDegreesToRadians(m_playerRotation);
                    const qreal displaySpeed = displaySpeedKmh();
                    sensor->updateVelocity(QVector2D(qCos(radians) * displaySpeed,
                                                     qSin(radians) * displaySpeed));
                    sensor->updateSpeedLimit(m_currentSpeedLimit, "main_route_speed_zone");
                }
            });

    connect(m_player2Physics, &VehiclePhysics::positionUpdated,
            this, [this](const QVector2D& position) {
                m_player2Position = position;
                m_player2Rotation = m_player2Physics->getRotation();
                m_player2Speed = m_player2Physics->getSpeed();

                if (m_gameView && m_driveActive && m_twoPlayerMode) {
                    m_gameView->updatePlayerCar(QStringLiteral("P2"),
                                                m_player2Position,
                                                m_player2Rotation,
                                                speedToDisplayKmh(m_player2Speed),
                                                QColor(40, 220, 255));
                    updateTwoPlayerCamera();
                }

                if (m_player2DataCollector && m_player2DataCollector->vehicleSensor()) {
                    VehicleSensor* sensor = m_player2DataCollector->vehicleSensor();
                    sensor->updatePosition(m_player2Position);
                    sensor->updateRotation(m_player2Rotation);
                    qreal radians = qDegreesToRadians(m_player2Rotation);
                    const qreal displaySpeed = speedToDisplayKmh(m_player2Speed);
                    sensor->updateVelocity(QVector2D(qCos(radians) * displaySpeed,
                                                     qSin(radians) * displaySpeed));
                    sensor->updateSpeedLimit(m_currentSpeedLimit, "main_route_speed_zone");
                }
            });

    connect(m_vehiclePhysics, &VehiclePhysics::collisionOccurred,
            this, [this](const QString& objectType, const QVector2D& position, qreal impactForce) {
                Q_UNUSED(position);
                Q_UNUSED(impactForce);
                onCollision();
                if (m_scoreManager) {
                    m_scoreManager->recordCollision(position.toPointF(), displaySpeedKmh(), objectType);
                }
            });

    connect(m_player2Physics, &VehiclePhysics::collisionOccurred,
            this, [this](const QString& objectType, const QVector2D& position, qreal impactForce) {
                if (!m_twoPlayerMode) {
                    return;
                }
                Q_UNUSED(impactForce);
                showInteractiveFeedback(QStringLiteral("P2 Wall Hit!"), PhantomDrive::FeedbackType::Critical);
                playSound(PhantomDrive::SoundEffect::Collision);
                if (m_player2ScoreManager) {
                    m_player2ScoreManager->recordCollision(position.toPointF(),
                                                           speedToDisplayKmh(m_player2Speed),
                                                           objectType);
                }
            });

    connect(m_vehiclePhysics, &VehiclePhysics::invisibilityRecoveryApplied,
            this, [this](const QVector2D& position) {
                m_playerPosition = position;
                if (m_gameView && m_driveActive) {
                    m_gameView->updatePlayerCar(m_playerPosition, m_vehiclePhysics->getRotation(), displaySpeedKmh());
                    m_gameView->setCameraPosition(m_playerPosition);
                }
                statusBar()->showMessage(
                    QStringLiteral("隐身结束：你不在赛道上，已送回最后一次拾取道具时的位置。"),
                    3500);
            });

    qDebug() << "VehiclePhysics initialized and connected to keyboard input";
}

void MainWindow::initializeAIOpponents()
{
    if (!m_aiManager) {
        return;
    }

    m_aiManager->destroyAllOpponents();
    m_playerVehicleContacts.clear();
    m_aiManager->setRaceTotalLaps(m_totalLaps);
    m_aiManager->setPlayerPosition(m_playerPosition);
    m_aiManager->setPlayerRaceProgress(0, 0, 0.0, false, 0.0);
    m_aiManager->setTrackBounds(QRectF(0.0, 0.0, 1280.0, 1920.0));

    TrackManager* trackMgr = TrackManager::instance(this);
    if (trackMgr) {
        m_aiManager->setTrackManager(trackMgr);
    }
    if (trackMgr && trackMgr->hasCurrentTrack()) {
        m_aiManager->setTrackBounds(trackMgr->getCurrentTrack()->getBounds());
    }

    QList<Waypoint> aiWaypoints;
    QList<QVector2D> trackWaypoints;
    if (trackMgr && trackMgr->hasCurrentTrack()) {
        const QString trackId = trackMgr->getCurrentTrack()->getId();
        if (BuiltInTrackFactory::isBuiltInTrackId(trackId)) {
            trackWaypoints = BuiltInTrackFactory::getAIDrivingWaypoints(trackId);
        }
    }
    if (trackWaypoints.isEmpty() && trackMgr) {
        trackWaypoints = trackMgr->getWaypoints();
    }

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
        const QVector2D spawnPos = aiWaypoints.first().position + QVector2D(35.0f * i, 42.0f * i);
        ai->setPosition(spawnPos);
        if (aiWaypoints.size() > 1) {
            const QVector2D toNext = aiWaypoints.at(1).position - aiWaypoints.first().position;
            ai->setRotation(qRadiansToDegrees(std::atan2(toNext.x(), toNext.y())));
        } else {
            ai->setRotation(0.0);
        }
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
                m_gameView->updateAICar(ai->getId(),
                                        ai->getPosition(),
                                        ai->getRotation(),
                                        speedToDisplayKmh(ai->getSpeed()));
            }
        }
    }
}

void MainWindow::applyAIDifficultySelection()
{
    if (m_currentMode == QStringLiteral("Arcade") && m_driveActive && !m_twoPlayerMode) {
        initializeAIOpponents();
    } else if (m_aiManager) {
        m_aiManager->destroyAllOpponents();
        if (m_gameView) {
            m_gameView->clearAllAICars();
        }
    }
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
        const QList<QVector2D> startPositions = track->getStartPositions();
        spawnPos = startPositions.isEmpty() ? track->getStartPosition() : startPositions.first();
        spawnRotation = track->getStartRotation();
    }
    const QPoint spawnTile = TrackData::worldToTile(spawnPos);
    qDebug().noquote()
        << QStringLiteral("[CustomTrackDebug] applyPlayerSpawnAtStartLine row=%1 col=%2 world=(%3,%4)")
               .arg(spawnTile.y())
               .arg(spawnTile.x())
               .arg(spawnPos.x())
               .arg(spawnPos.y());

    m_playerPosition = spawnPos;
    m_playerRotation = spawnRotation;
    m_playerSpeed = 0.0;

    if (m_vehiclePhysics) {
        m_vehiclePhysics->setPosition(spawnPos);
        m_vehiclePhysics->setRotation(spawnRotation);
    }

    if (m_gameView) {
        m_gameView->updatePlayerCar(m_playerPosition, m_playerRotation, displaySpeedKmh());
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

void MainWindow::setupCustomTrackRuntimeObjects(TrackData* track)
{
    clearEBRuntimeObjects();

    if (!track || !m_gameView) {
        return;
    }

    const QList<QVector2D> itemBoxes = track->getItemBoxPositions();
    for (int i = 0; i < itemBoxes.size(); ++i) {
        const QString id = QStringLiteral("custom_item_box_%1").arg(i + 1);
        auto* box = new PowerupBox(itemBoxes.at(i), 54.0f, this);
        box->setObjectName(id);
        box->setRespawnTime(8.0f);
        m_powerupBoxes.append(box);

        m_gameView->addPowerupBox(id, itemBoxes.at(i), QStringLiteral("Random"));

        connect(box, &PowerupBox::collected,
                this, [this, id](const QString& playerId, const PowerupType& collectedType) {
                    if (m_gameView) {
                        m_gameView->removePowerupBox(id);
                    }
                    handlePowerupCollectedForPlayer(collectedType, playerId == QStringLiteral("player_2") ? 2 : 1);
                });
        connect(box, &PowerupBox::respawned,
                this, [this, id, itemBoxes, i]() {
                    if (m_gameView) {
                        m_gameView->addPowerupBox(id, itemBoxes.at(i), QStringLiteral("Random"));
                    }
                });
    }
}

void MainWindow::startCustomTrackSession(TrackData* track)
{
    if (!track) {
        return;
    }

    if (m_driveActive) {
        silentFinishSession();
    }

    // Stop any previous learning session timer.
    if (m_learningSessionTimer) {
        m_learningSessionTimer->stop();
        m_learningSessionTimer->deleteLater();
        m_learningSessionTimer = nullptr;
    }

    m_currentMode = QStringLiteral("Custom Track");
    m_twoPlayerMode = isTwoPlayerSelected();
    m_twoPlayerFinishHandled = false;
    m_customTrackPlaying = true;
    m_arcadeRaceLogicActive = true;
    m_driveActive = true;
    m_arcadeRaceFinished = false;
    m_lapsCompleted = 0;
    m_totalLaps = 1;
    m_simTick = 0;
    m_sessionElapsedMs = 0;
    m_currentLapStartMs = 0;
    m_bestLapMs = 0;
    m_playerSpeed = 0.0;
    m_player2Speed = 0.0;
    track->setMaxLaps(1);

    if (m_customTrackMode) {
        m_customTrackMode->setTrack(track);
        m_customTrackMode->setCustomState(CustomTrackModeState::Playing);
    }

    if (m_customTrackEditor) {
        m_customTrackEditor->hide();
    }
    setGameHeaderVisible(true);
    if (ui->stackedWidget) {
        ui->stackedWidget->setCurrentIndex(1);
    }
    if (m_gameView) {
        m_gameView->show();
        m_gameView->setTrackData(track);
        m_gameView->clearAllAICars();
    }

    TrackManager* trackMgr = TrackManager::instance(this);
    if (trackMgr) {
        trackMgr->setCurrentTrack(track);

        QList<QVector2D> waypoints;

        waypoints.append(track->getStartPosition());

        for (Checkpoint* cp : track->getCheckpointsInOrder()) {
            if (cp) {
                waypoints.append(cp->getPosition());
            }
        }

        // ===== 找 FinishLine =====
        for (int row = 0; row < track->getRowCount(); ++row) {
            for (int col = 0; col < track->getColCount(); ++col) {

                TrackTile* tile = track->getTileAt(row, col);

                if (tile &&
                    tile->getType() == TileType::FinishLine)
                {
                    QVector2D finishPos =
                        TrackData::tileToWorldCenter(row, col);

                    qDebug() << "FINISH =" << finishPos;

                    waypoints.append(finishPos);
                }
            }
        }

        QVector2D finishPos;
        bool finishFound = false;

        for (int row = 0; row < track->getRowCount(); ++row)
        {
            for (int col = 0; col < track->getColCount(); ++col)
            {
                TrackTile* tile = track->getTileAt(row, col);

                if (tile &&
                    tile->getType() == TileType::FinishLine)
                {
                    finishPos =
                        TrackData::tileToWorldCenter(row, col);

                    finishFound = true;
                    break;
                }
            }

            if (finishFound)
                break;
        }

        if (finishFound)
        {
            waypoints.append(finishPos);

            qDebug()
                << "FINISH ="
                << finishPos;
        }

        qDebug() << "WAYPOINT COUNT =" << waypoints.size();

        trackMgr->setWaypoints(waypoints);
    }

    preparePlayerReportSystems();
    if (m_aiManager) {
        m_aiManager->destroyAllOpponents();
        m_aiManager->setRaceTotalLaps(1);
        m_aiManager->setPlayerRaceProgress(0, 0, 0.0, false, 0.0);
        m_aiManager->setTrackBounds(track->getBounds());

        //补充
        initializeAIOpponents();
    }

    if (m_learningHUD) {
        m_learningHUD->hide();
    }
    if (m_arcadeHUD) {
        m_arcadeHUD->setGameMode("Custom Track");
        m_arcadeHUD->reset();
        m_arcadeHUD->show();
    }
    if (ui->label_ModeTitle) {
        ui->label_ModeTitle->setText("CUSTOM TRACK");
        ui->label_ModeTitle->setStyleSheet(
            "QLabel{color:#59F7FF;font-size:13px;font-weight:bold;letter-spacing:3px;}");
    }
    if (m_btnFinishDrive) {
        m_btnFinishDrive->show();
        m_btnFinishDrive->setEnabled(false);
    }

    setupCustomTrackRuntimeObjects(track);

    if (m_vehiclePhysics) {
        m_vehiclePhysics->reset();
        m_vehiclePhysics->resetRaceProgress();
        m_vehiclePhysics->setRaceLogicEnabled(false);
    }
    applyPlayerSpawnAtStartLine();
    if (m_twoPlayerMode) {
        applyPlayer2SpawnAtStartLine();
        if (m_gameView) {
            m_gameView->updatePlayerCar(QStringLiteral("P2"),
                                        m_player2Position,
                                        m_player2Rotation,
                                        speedToDisplayKmh(m_player2Speed),
                                        QColor(40, 220, 255));
            updateTwoPlayerCamera();
        }
    }
    resetArcadeRaceProgress();
    focusGameViewForDriving();

    m_countdownActive = true;
    showCountdown();

    statusBar()->showMessage(QStringLiteral("Custom Track Mode running"));
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
    return speedToDisplayKmh(m_playerSpeed);
}

int MainWindow::speedToDisplayKmh(qreal physicsSpeed) const
{
    return qBound(0, qRound(qAbs(physicsSpeed) * kDisplayMaxSpeedKmh / kPhysicsMaxSpeed), 999);
}

void MainWindow::updateEBRuntime(qreal deltaSeconds)
{
    const qint64 deltaMs = static_cast<qint64>(deltaSeconds * 1000.0);
    const float collectRadius = (m_vehiclePhysics && m_vehiclePhysics->isMagnetActive()) ? 96.0f : 54.0f;
    const float player2CollectRadius = (m_player2Physics && m_player2Physics->isMagnetActive()) ? 96.0f : 54.0f;

    for (PowerupBox* box : m_powerupBoxes) {
        if (!box) {
            continue;
        }
        box->update(static_cast<float>(deltaSeconds));
        if (box->isActive()) {
            box->tryCollect(m_playerPosition, QStringLiteral("player_1"), collectRadius);
        }
        if (m_twoPlayerMode && box->isActive()) {
            box->tryCollect(m_player2Position, QStringLiteral("player_2"), player2CollectRadius);
        }
    }

    if (m_powerupWorld) {
        m_powerupWorld->update(deltaMs, m_vehiclePhysics, m_aiManager, m_powerupBoxes, m_gameView);
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
    if (m_arcadeHUD) {
        m_arcadeHUD->updateSpeedLimit(m_currentSpeedLimit);
    }
}

void MainWindow::handlePowerupCollected(PhantomDrive::PowerupType type)
{
    handlePowerupCollectedForPlayer(type, 1);
}

void MainWindow::handlePowerupCollectedForPlayer(PhantomDrive::PowerupType type, int playerIndex)
{
    PowerupType effectiveType = type;
    if (type == PowerupType::Custom) {
        static const PowerupType kCustomPool[] = {
            PowerupType::Boost,
            PowerupType::Shield,
            PowerupType::Repair,
            PowerupType::Missile,
            PowerupType::EMP
        };
        const int index = QRandomGenerator::global()->bounded(static_cast<int>(sizeof(kCustomPool) / sizeof(kCustomPool[0])));
        effectiveType = kCustomPool[index];
    }

    const QString typeName = powerupTypeToString(effectiveType);
    const QString powerupId = QStringLiteral("eb_%1").arg(typeName.toLower().replace(QStringLiteral(" "), QStringLiteral("_")));
    VehiclePhysics* targetPhysics = (playerIndex == 2) ? m_player2Physics : m_vehiclePhysics;
    QVector2D* targetPosition = (playerIndex == 2) ? &m_player2Position : &m_playerPosition;
    qreal* targetRotation = (playerIndex == 2) ? &m_player2Rotation : &m_playerRotation;
    qreal* targetSpeed = (playerIndex == 2) ? &m_player2Speed : &m_playerSpeed;
    const QString playerLabel = playerIndex == 2 ? QStringLiteral("P2") : QStringLiteral("P1");

    onPowerupCollected(playerIndex == 2 ? QStringLiteral("P2 %1").arg(typeName) : typeName);

    auto notifyHud = [this](const QString& id, const QString& label, bool active, int durationMs) {
        if (!m_learningHUD) {
            return;
        }
        m_learningHUD->updatePowerupState(id, label, active);
        if (durationMs > 0) {
            QTimer::singleShot(durationMs, this, [this, id, label]() {
                if (m_learningHUD) {
                    m_learningHUD->updatePowerupState(id, label, false);
                }
            });
        }
    };

    if (effectiveType == PowerupType::Boost && targetPhysics) {
        targetPhysics->activateSpeedBoost(1.35, 4000);
        playSound(SoundEffect::SpeedBoost);
        notifyHud(powerupId, QStringLiteral("Boost"), true, 4000);
    } else if (effectiveType == PowerupType::Shield && targetPhysics) {
        targetPhysics->activateShield(6000);
        notifyHud(powerupId, QStringLiteral("Shield"), true, 6000);
    } else if (effectiveType == PowerupType::EMP) {
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
        notifyHud(powerupId, QStringLiteral("EMP"), true, 3000);
        statusBar()->showMessage(QStringLiteral("%1 EMP Pulse! AI opponents slowed for 3 seconds.").arg(playerLabel), 3000);
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
    } else if (effectiveType == PowerupType::Repair && targetPhysics) {
        targetPhysics->activateRepair();
        notifyHud(powerupId, QStringLiteral("Repair"), true, 1200);
        statusBar()->showMessage(QStringLiteral("%1 Repair Kit! Vehicle stability restored.").arg(playerLabel), 2500);
    } else if (effectiveType == PowerupType::Missile && m_powerupWorld && targetPhysics) {
        const QString targetId = m_powerupWorld->findBestMissileTarget(
            *targetPosition, targetPhysics->getRotation(), m_aiManager);
        if (targetId.isEmpty()) {
            statusBar()->showMessage(QStringLiteral("Missile launched but no target in range."), 2500);
        } else {
            const qreal radians = qDegreesToRadians(targetPhysics->getRotation());
            const QVector2D launchPos = *targetPosition
                + QVector2D(qSin(radians), qCos(radians)) * 42.0;
            m_powerupWorld->spawnMissile(launchPos, targetId);
            statusBar()->showMessage(QStringLiteral("Missile locked on %1!").arg(targetId), 2500);
        }
        notifyHud(powerupId, QStringLiteral("Missile"), true, 1500);
    } else if (effectiveType == PowerupType::OilSlick && m_powerupWorld && targetPhysics) {
        const qreal radians = qDegreesToRadians(targetPhysics->getRotation());
        const QVector2D dropPos = *targetPosition
            - QVector2D(qSin(radians), qCos(radians)) * 36.0;
        m_powerupWorld->spawnOilPuddle(dropPos, 72.0, 12000);
        statusBar()->showMessage(QStringLiteral("Oil slick dropped behind your car."), 2500);
        notifyHud(powerupId, QStringLiteral("Oil Slick"), true, 1500);
    } else if (effectiveType == PowerupType::Invisibility && targetPhysics) {
        targetPhysics->activateInvisibility(8000);
        statusBar()->showMessage(QStringLiteral("%1 Invisibility active! You can pass through walls and AI.").arg(playerLabel), 3000);
        notifyHud(powerupId, QStringLiteral("Invisibility"), true, 8000);
    } else if (effectiveType == PowerupType::Teleport && targetPhysics) {
        if (targetPhysics->teleportForward(280.0)) {
            *targetPosition = targetPhysics->getPosition();
            *targetRotation = targetPhysics->getRotation();
            *targetSpeed = targetPhysics->getSpeed();
            if (m_gameView) {
                if (playerIndex == 2) {
                    m_gameView->updatePlayerCar(QStringLiteral("P2"), m_player2Position, m_player2Rotation,
                                                speedToDisplayKmh(m_player2Speed), QColor(40, 220, 255));
                    updateTwoPlayerCamera();
                } else if (m_twoPlayerMode) {
                    m_gameView->updatePlayerCar(QStringLiteral("P1"), m_playerPosition, m_playerRotation,
                                                displaySpeedKmh(), QColor(255, 48, 118));
                    updateTwoPlayerCamera();
                } else {
                    m_gameView->updatePlayerCar(m_playerPosition, m_playerRotation, displaySpeedKmh());
                    m_gameView->setCameraPosition(m_playerPosition);
                }
            }
            statusBar()->showMessage(QStringLiteral("%1 Teleport! Jumped forward along the track.").arg(playerLabel), 2500);
        } else {
            statusBar()->showMessage(QStringLiteral("%1 Teleport blocked - no free space ahead.").arg(playerLabel), 2500);
        }
        notifyHud(powerupId, QStringLiteral("Teleport"), true, 1200);
    } else if (effectiveType == PowerupType::Magnet && targetPhysics) {
        targetPhysics->activateMagnet(6000);
        statusBar()->showMessage(QStringLiteral("%1 Magnet active! Nearby powerups are pulled toward you.").arg(playerLabel), 3000);
        notifyHud(powerupId, QStringLiteral("Magnet"), true, 6000);
    }
}

void MainWindow::handleTrafficViolation(const PhantomDrive::ViolationEvent& violation)
{
    if (m_learningHUD && ui->stackedWidget->currentIndex() == 1 && m_currentMode == QStringLiteral("Learning")) {
        // showViolationWarning emits violationWarningRequested → InteractiveFeedback (Critical).
        m_learningHUD->showViolationWarning(violation.description);
    } else {
        showInteractiveFeedback(violation.description, PhantomDrive::FeedbackType::Critical);
    }

    playSound(SoundEffect::Violation);
    statusBar()->showMessage(violation.description, 3000);
}

void MainWindow::updateRaceHud()
{
    if (m_aiManager && m_driveActive && !m_countdownActive) {
        const int totalCheckpoints = qMax(1, m_raceCheckpointTotal);
        const int checkpointIndex = qBound(0, m_nextCheckpointIndex, totalCheckpoints);
        const qreal progressPercent = qBound(0.0,
                                             (static_cast<qreal>(checkpointIndex) / totalCheckpoints) * 100.0,
                                             100.0);
        const int lap = m_customTrackPlaying
            ? (m_arcadeRaceFinished ? 1 : 0)
            : m_lapsCompleted;
        m_aiManager->setPlayerRaceProgress(lap,
                                           checkpointIndex,
                                           progressPercent,
                                           m_arcadeRaceFinished,
                                           m_sessionElapsedMs / 1000.0);
    }

    if (m_learningHUD) {
        m_learningHUD->updateLapInfo(m_lapsCompleted + 1, m_totalLaps);
    }

    if (!m_arcadeHUD) {
        return;
    }

    m_arcadeHUD->updateSpeed(displaySpeedKmh());
    if (m_customTrackPlaying) {
        const int total = qMax(0, m_raceCheckpointTotal);
        const int passed = qBound(0, m_nextCheckpointIndex, total);
        const QString nextTarget = passed >= total
            ? QStringLiteral("FINISH")
            : QStringLiteral("CP%1").arg(passed + 1);
        m_arcadeHUD->updateRouteProgress(passed, total, nextTarget);
    } else {
        m_arcadeHUD->updateLap(m_lapsCompleted, m_totalLaps);
    }
    m_arcadeHUD->updateTotalTime(formatRaceTime(m_sessionElapsedMs));
    m_arcadeHUD->updateLapTime(formatRaceTime(qMax<qint64>(0, m_sessionElapsedMs - m_currentLapStartMs)));
    if (m_bestLapMs > 0) {
        m_arcadeHUD->updateBestLapTime(formatRaceTime(m_bestLapMs));
    }

    const int totalRacers = m_aiManager ? (m_aiManager->getOpponentCount() + 1) : 1;
    const int playerPosition = m_aiManager ? m_aiManager->getPlayerRacePosition() : 1;
    m_arcadeHUD->updatePosition(playerPosition, totalRacers);

    // Sync traffic light state to ArcadeHUD so the signal-dot and red-blink work.
    m_arcadeHUD->updateTrafficLight(m_currentTrafficLightState);

    // Sync speed limit so the speedometer turns red when speeding.
    m_arcadeHUD->updateSpeedLimit(m_currentSpeedLimit);

    // Show boost bar: 100% when boost is active, 0% otherwise.
    if (m_vehiclePhysics) {
        m_arcadeHUD->updateBoost(m_vehiclePhysics->isSpeedBoostActive() ? 100.0 : 0.0);
    }
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
    const bool inNorthGate = !m_customTrackPlaying && positionInNorthGate(track, pos);
    const bool inNorthSector = m_customTrackPlaying ? onStartLine : (onStartLine || inNorthGate);
    const bool wasInNorthSector = m_customTrackPlaying ? m_wasOnStartLine : (m_wasOnStartLine || m_wasInNorthGate);

    if (!inNorthSector) {
        m_hasLeftNorthSector = true;
        m_blockCheckpointsUntilLeaveNorth = false;
    }

    const bool allCheckpointsCollected = m_nextCheckpointIndex >= m_raceCheckpointTotal;

    if (allCheckpointsCollected && m_hasLeftNorthSector) {
        const bool enteredStartLine = onStartLine && !m_wasOnStartLine;
        const bool enteredNorthGate = !m_customTrackPlaying && inNorthGate && !m_wasInNorthGate;
        if (enteredStartLine || enteredNorthGate) {
            if (m_customTrackPlaying) {
                finishCustomTrackRoute();
            } else {
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
    }

    if (!m_blockCheckpointsUntilLeaveNorth && m_nextCheckpointIndex < checkpoints.size()) {
        if (m_nextCheckpointIndex == 0) {
            PhantomDrive::Checkpoint* cp0 = checkpoints.first();
            const bool leavingNorth = !m_customTrackPlaying && !inNorthSector && wasInNorthSector;
            const bool insideCp0 = cp0 && cp0->containsPoint(pos);
            const bool enteredCp0 = insideCp0 && !m_wasInsideNextGate;
            const bool crossedCp0 = (m_customTrackPlaying || m_hasLeftNorthSector)
                && cp0
                && crossedCheckpointGate(cp0, positionBefore, pos);
            if (leavingNorth || enteredCp0 || crossedCp0) {
                onCheckpointReached(0);
                m_nextCheckpointIndex = 1;
                m_wasInsideNextGate = false;
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

void MainWindow::finishCustomTrackRoute()
{
    if (m_arcadeRaceFinished) {
        return;
    }
    if (m_twoPlayerMode) {
        finishTwoPlayerRace(1);
        return;
    }

    m_arcadeRaceFinished = true;
    m_lapsCompleted = 1;
    m_nextCheckpointIndex = m_raceCheckpointTotal;
    const qint64 elapsedMs = qMax<qint64>(0, m_sessionElapsedMs - m_currentLapStartMs);
    if (elapsedMs > 0 && (m_bestLapMs == 0 || elapsedMs < m_bestLapMs)) {
        m_bestLapMs = elapsedMs;
    }

    if (m_aiManager) {
        m_aiManager->setPlayerRaceProgress(1, m_raceCheckpointTotal, 100.0, true, m_sessionElapsedMs / 1000.0);
    }

    if (m_arcadeHUD) {
        m_arcadeHUD->updateRouteProgress(m_raceCheckpointTotal, m_raceCheckpointTotal, QStringLiteral("FINISH"));
        m_arcadeHUD->showRaceBanner(QStringLiteral("Custom Track Finished"));
        m_arcadeHUD->showRaceFinished(1, formatRaceTime(m_sessionElapsedMs));
    }

    playSound(PhantomDrive::SoundEffect::RaceFinish);
    statusBar()->showMessage(QStringLiteral("Custom Track Finished"), 4000);

    // Wait 2 s so the player can see the finish banner, then show report.
    QTimer::singleShot(2000, this, [this]() {
        if (m_driveActive) {
            onGameFinished();
        }
    });
}

void MainWindow::updatePlayer2RaceProgress(const QVector2D& positionBefore)
{
    if (!m_twoPlayerMode || !m_arcadeRaceLogicActive || !m_driveActive || m_countdownActive
        || m_arcadeRaceFinished || m_twoPlayerFinishHandled) {
        return;
    }

    TrackData* track = m_gameView ? m_gameView->trackData() : nullptr;
    if (!track) {
        return;
    }

    const QList<Checkpoint*> checkpoints = track->getCheckpointsInOrder();
    const int total = checkpoints.size();
    if (total <= 0) {
        return;
    }

    const QVector2D pos = m_player2Position;
    if (m_player2NextCheckpointIndex < total) {
        Checkpoint* nextCp = checkpoints.at(m_player2NextCheckpointIndex);
        const bool insideNext = nextCp && nextCp->containsPoint(pos);
        const bool enteredNext = insideNext && !m_player2WasInsideNextGate;
        const bool crossedNext = crossedCheckpointGate(nextCp, positionBefore, pos);
        if (enteredNext || crossedNext) {
            if (m_arcadeHUD) {
                m_arcadeHUD->showRaceBanner(
                    QStringLiteral("P2 CP %1/%2 passed")
                        .arg(m_player2NextCheckpointIndex + 1)
                        .arg(total));
            }
            playSound(PhantomDrive::SoundEffect::Checkpoint);
            ++m_player2NextCheckpointIndex;
            m_player2WasInsideNextGate = false;
        }
    }

    if (m_player2NextCheckpointIndex >= total && tileAtIsStartFinish(track, pos)) {
        if (m_customTrackPlaying) {
            finishTwoPlayerRace(2);
            return;
        }
        ++m_player2LapsCompleted;
        if (m_player2LapsCompleted >= m_totalLaps) {
            finishTwoPlayerRace(2);
            return;
        }
        m_player2NextCheckpointIndex = 0;
    }

    if (m_player2NextCheckpointIndex < checkpoints.size()) {
        Checkpoint* nextCp = checkpoints.at(m_player2NextCheckpointIndex);
        m_player2WasInsideNextGate = nextCp && nextCp->containsPoint(pos);
    } else {
        m_player2WasInsideNextGate = false;
    }
}

void MainWindow::finishTwoPlayerRace(int winnerIndex)
{
    if (m_twoPlayerFinishHandled) {
        return;
    }
    m_twoPlayerFinishHandled = true;
    m_arcadeRaceFinished = true;

    const QString winner = winnerIndex == 2 ? QStringLiteral("Player 2") : QStringLiteral("Player 1");
    if (m_arcadeHUD) {
        m_arcadeHUD->showRaceBanner(QStringLiteral("%1 Wins").arg(winner));
        m_arcadeHUD->showRaceFinished(winnerIndex, formatRaceTime(m_sessionElapsedMs));
    }
    playSound(PhantomDrive::SoundEffect::RaceFinish);
    statusBar()->showMessage(QStringLiteral("%1 wins. Generating both reports...").arg(winner), 4000);

    QTimer::singleShot(2000, this, [this]() {
        if (m_driveActive) {
            onGameFinished();
        }
    });
}

void MainWindow::resolvePlayerAiVehicleContact(AIOpponent* ai)
{
    if (!ai || !m_vehiclePhysics) {
        return;
    }

    const QString aiId = ai->getId();

    if (m_vehiclePhysics->isInvisible()) {
        m_playerVehicleContacts.remove(aiId);
        return;
    }

    if (ai->hasFinished()) {
        static QHash<QString, qint64> lastFinishedAiContactFeedbackMs;

        QVector2D playerPos = m_vehiclePhysics->getPosition();
        const QVector2D aiPos = ai->getPosition();
        QVector2D delta = playerPos - aiPos;
        qreal dist = delta.length();

        if (dist >= kVehicleContactReleaseDistance) {
            m_playerVehicleContacts.remove(aiId);
            return;
        }

        QVector2D normal = dist > 0.001 ? delta / dist : QVector2D(1.0, 0.0);
        if (dist < kVehicleSeparationDistance) {
            const qreal overlap = kVehicleSeparationDistance - dist;
            const QVector2D playerNew = playerPos + normal * qMin<qreal>(overlap, 12.0);
            if (m_vehiclePhysics->isPositionFree(playerNew)) {
                m_vehiclePhysics->setPosition(playerNew);
                m_playerPosition = playerNew;
                playerPos = playerNew;
                delta = playerPos - aiPos;
                dist = delta.length();
                normal = dist > 0.001 ? delta / dist : normal;
            }
        }

        const bool wasInContact = m_playerVehicleContacts.contains(aiId);
        const bool inContact = dist < kVehicleImpactDistance;
        const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
        const qint64 lastFeedbackMs = lastFinishedAiContactFeedbackMs.value(aiId, -kFinishedAiContactCooldownMs);

        if (inContact) {
            m_playerVehicleContacts.insert(aiId);
        }

        if (inContact
            && !wasInContact
            && nowMs - lastFeedbackMs >= kFinishedAiContactCooldownMs
            && !m_vehiclePhysics->isColliding()) {
            const qreal impactForce = qMax<qreal>(5.0, qAbs(m_playerSpeed - ai->getSpeed()) * 0.25);
            m_vehiclePhysics->handleCollision(normal, impactForce);
            lastFinishedAiContactFeedbackMs.insert(aiId, nowMs);
        }
        return;
    }

    auto* simpleAi = qobject_cast<SimpleAIOpponent*>(ai);

    QVector2D playerPos = m_vehiclePhysics->getPosition();
    QVector2D aiPos = ai->getPosition();
    QVector2D delta = playerPos - aiPos;
    qreal dist = delta.length();

    if (dist >= kVehicleContactReleaseDistance) {
        m_playerVehicleContacts.remove(aiId);
    }

    if (dist < kVehicleSeparationDistance) {
        QVector2D dir = dist > 0.001 ? delta / dist : QVector2D(1.0, 0.0);
        const qreal overlap = kVehicleSeparationDistance - dist;
        const QVector2D playerNew = playerPos + dir * (overlap * 0.5);
        const QVector2D aiNew = aiPos - dir * (overlap * 0.5);

        const bool playerFree = m_vehiclePhysics->isPositionFree(playerNew);
        const bool aiFree = !simpleAi || simpleAi->isPositionFree(aiNew);

        if (playerFree && aiFree) {
            m_vehiclePhysics->setPosition(playerNew);
            m_playerPosition = playerNew;
            ai->setPosition(aiNew);
        } else if (playerFree) {
            const QVector2D pushedPlayer = playerPos + dir * overlap;
            if (m_vehiclePhysics->isPositionFree(pushedPlayer)) {
                m_vehiclePhysics->setPosition(pushedPlayer);
                m_playerPosition = pushedPlayer;
            }
        } else if (aiFree) {
            const QVector2D pushedAi = aiPos - dir * overlap;
            ai->setPosition(pushedAi);
        }
    }

    playerPos = m_vehiclePhysics->getPosition();
    aiPos = ai->getPosition();
    delta = playerPos - aiPos;
    dist = delta.length();

    const bool wasInContact = m_playerVehicleContacts.contains(aiId);
    const bool inContact = dist < kVehicleImpactDistance;

    if (inContact && !wasInContact && dist > 0.001) {
        const QVector2D normal = delta / dist;
        const qreal impactForce = qAbs(m_playerSpeed - ai->getSpeed());

        if (!m_vehiclePhysics->isColliding()) {
            m_vehiclePhysics->handleCollision(normal, impactForce);
        }
        if (simpleAi && simpleAi->usesVehiclePhysics() && !simpleAi->isPhysicsColliding()) {
            simpleAi->applyExternalCollision(-normal, impactForce);
        }
        m_aiManager->onPlayerCollision(aiId, playerPos);
        m_playerVehicleContacts.insert(aiId);
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
        const QVector2D player2PositionBeforeUpdate = m_player2Position;

        if (m_vehiclePhysics) {
            m_vehiclePhysics->update(50);
        }
        if (m_twoPlayerMode && m_player2Physics) {
            m_player2Physics->update(50);
        }

        if (m_arcadeRaceLogicActive) {
            updateArcadeRaceProgress(positionBeforeUpdate);
            if (m_twoPlayerMode) {
                updatePlayer2RaceProgress(player2PositionBeforeUpdate);
            }
        }

        m_previousPlayerPosition = m_playerPosition;
        m_previousPlayer2Position = m_player2Position;

        updateEBRuntime(0.05);
        updateTrafficAndHud(m_simTick);

        if (m_aiManager && m_driveActive) {
            m_aiManager->setPlayerPosition(m_playerPosition);
            m_aiManager->update(50);

            QList<AIOpponent*> opponents = m_aiManager->getAllOpponents();

            for (AIOpponent* ai : opponents) {
                if (ai) {
                    resolvePlayerAiVehicleContact(ai);

                    m_gameView->updateAICar(
                        ai->getId(),
                        ai->getPosition(),
                        ai->getRotation(),
                        speedToDisplayKmh(ai->getSpeed())
                    );
                }
            }
        }

        if (m_drivingDataCollector && m_drivingDataCollector->vehicleSensor()) {
            VehicleSensor* sensor = m_drivingDataCollector->vehicleSensor();
            sensor->updatePosition(m_playerPosition);
            qreal rad = m_playerRotation * 3.14159265 / 180.0;
            const qreal displaySpeed = displaySpeedKmh();
            QVector2D velocity(std::cos(rad) * displaySpeed, std::sin(rad) * displaySpeed);
            sensor->updateVelocity(velocity);
            sensor->updateRotation(m_playerRotation);
            sensor->updateSpeedLimit(m_currentSpeedLimit, "main_route_speed_zone");
            sensor->updateAcceleratorState(displaySpeed >= m_drivingDataCollector->getCurrentData().speed);
            sensor->updateBrakeState(displaySpeed < m_drivingDataCollector->getCurrentData().speed);
        }
        if (m_twoPlayerMode && m_player2DataCollector && m_player2DataCollector->vehicleSensor()) {
            VehicleSensor* sensor = m_player2DataCollector->vehicleSensor();
            sensor->updatePosition(m_player2Position);
            qreal rad = m_player2Rotation * 3.14159265 / 180.0;
            const qreal displaySpeed = speedToDisplayKmh(m_player2Speed);
            QVector2D velocity(std::cos(rad) * displaySpeed, std::sin(rad) * displaySpeed);
            sensor->updateVelocity(velocity);
            sensor->updateRotation(m_player2Rotation);
            sensor->updateSpeedLimit(m_currentSpeedLimit, "main_route_speed_zone");
            sensor->updateAcceleratorState(displaySpeed >= m_player2DataCollector->getCurrentData().speed);
            sensor->updateBrakeState(displaySpeed < m_player2DataCollector->getCurrentData().speed);
        }

        if (m_reportWidget && m_reportWidget->isVisible()) {
            m_reportWidget->addSpeedData(displaySpeedKmh(), m_simTick);
        }

        if (m_scoreManager) {
            m_scoreManager->recordSafeDrivingTick(QDateTime::currentMSecsSinceEpoch(), displaySpeedKmh());
        }

        updateRaceHud();
    });

    m_simTimer->start(50);
}

void MainWindow::startDrivingSession(const QString& mode)
{
    if (m_driveActive) {
        silentFinishSession();
    }

    // Stop any previous learning session timer.
    if (m_learningSessionTimer) {
        m_learningSessionTimer->stop();
        m_learningSessionTimer->deleteLater();
        m_learningSessionTimer = nullptr;
    }

    m_currentMode = mode;
    m_twoPlayerMode = (mode == QStringLiteral("Arcade")) && isTwoPlayerSelected();
    m_twoPlayerFinishHandled = false;
    m_arcadeRaceLogicActive = (mode == QStringLiteral("Arcade"));
    m_customTrackPlaying = false;
    m_driveActive = true;
    m_arcadeRaceFinished = false;
    m_lapsCompleted = 0;
    m_totalLaps = 3;
    m_simTick = 0;
    m_sessionElapsedMs = 0;
    m_currentLapStartMs = 0;
    m_bestLapMs = 0;
    m_playerSpeed = 0.0;
    m_player2Speed = 0.0;
    m_player2LapsCompleted = 0;
    m_player2NextCheckpointIndex = 0;
    m_player2WasInsideNextGate = false;

    if (m_gameView) {
        m_gameView->clearAllAICars();
    }

    preparePlayerReportSystems();

    ui->stackedWidget->setCurrentIndex(1);
    if (m_customTrackEditor) {
        m_customTrackEditor->hide();
    }
    setGameHeaderVisible(true);
    if (m_btnFinishDrive) {
        m_btnFinishDrive->show();
        m_btnFinishDrive->setEnabled(false);
    }
    if (m_gameView) {
        m_gameView->show();
    }

    restoreDefaultRaceTrack();
    focusGameViewForDriving();

    // Update the top HUD mode title label
    if (ui->label_ModeTitle) {
        if (mode == "Learning") {
            ui->label_ModeTitle->setText("LEARNING MODE");
            ui->label_ModeTitle->setStyleSheet(
                "QLabel{color:#00FFA0;font-size:13px;font-weight:bold;letter-spacing:3px;}");
        } else if (mode == "Custom Track") {
            ui->label_ModeTitle->setText("CUSTOM TRACK");
            ui->label_ModeTitle->setStyleSheet(
                "QLabel{color:#59F7FF;font-size:13px;font-weight:bold;letter-spacing:3px;}");
        } else {
            ui->label_ModeTitle->setText("ARCADE MODE");
            ui->label_ModeTitle->setStyleSheet(
                "QLabel{color:#FF3366;font-size:13px;font-weight:bold;letter-spacing:3px;}");
        }
    }

    // Show the right-side HUD in ALL driving modes (Arcade, Learning, Custom Track)
    if (m_arcadeHUD) {
        m_arcadeHUD->setGameMode(mode);
        m_arcadeHUD->reset();
        m_arcadeHUD->show();

        // Position the HUD panel on the right side of the game canvas
        QWidget* gamePage = ui->stackedWidget ? ui->stackedWidget->widget(1) : nullptr;
        if (gamePage && m_gameView) {
            const int hw = 300;
            const int hudBarHeight = 42;
            const int hx = gamePage->width() - hw - 8;
            const int hy = hudBarHeight + 8;
            m_arcadeHUD->setFixedHeight(gamePage->height() - hy - 8);
            m_arcadeHUD->move(hx, hy);
            m_arcadeHUD->raise();
        }
    }
    // Hide the old LearningHUD (replaced by ArcadeHUD)
    if (m_learningHUD) {
        m_learningHUD->hide();
    }
    updateRaceHud();
    setupEBRuntimeObjects();

    if (m_vehiclePhysics) {
        m_vehiclePhysics->reset();
        m_vehiclePhysics->resetRaceProgress();
        m_vehiclePhysics->setRaceLogicEnabled(false);
    }
    if (m_player2Physics) {
        m_player2Physics->reset();
        m_player2Physics->resetRaceProgress();
        m_player2Physics->setRaceLogicEnabled(false);
    }
    applyPlayerSpawnAtStartLine();
    if (m_twoPlayerMode) {
        applyPlayer2SpawnAtStartLine();
        if (m_gameView) {
            m_gameView->updatePlayerCar(QStringLiteral("P2"),
                                        m_player2Position,
                                        m_player2Rotation,
                                        speedToDisplayKmh(m_player2Speed),
                                        QColor(40, 220, 255));
            updateTwoPlayerCamera();
        }
    }
    resetArcadeRaceProgress();

    initializeAIOpponents();

    m_countdownActive = true;
    showCountdown();

    // Learning Mode: auto-finish after 5 minutes so the report always appears.
    if (mode == QStringLiteral("Learning")) {
        constexpr int kLearningMaxMs = 5 * 60 * 1000;
        m_learningSessionTimer = new QTimer(this);
        m_learningSessionTimer->setSingleShot(true);
        connect(m_learningSessionTimer, &QTimer::timeout, this, [this]() {
            if (m_driveActive && m_currentMode == QStringLiteral("Learning")) {
                showInteractiveFeedback(QStringLiteral("Learning Session Complete!"), FeedbackType::Milestone);
                playSound(PhantomDrive::SoundEffect::RaceFinish);
                QTimer::singleShot(1500, this, [this]() {
                    if (m_driveActive) {
                        onGameFinished();
                    }
                });
            }
        });
        m_learningSessionTimer->start(kLearningMaxMs);
    }

    statusBar()->showMessage(QStringLiteral("%1 mode running").arg(m_currentMode));
}

void MainWindow::startBuiltInTrackSession(const QString& mode)
{
    const QString trackId = m_trackSelectCombo
        ? m_trackSelectCombo->currentData().toString()
        : m_selectedTrackId;
    m_selectedTrackId = trackId.isEmpty() ? QStringLiteral("neon_loop") : trackId;

    if (m_selectedBuiltInTrack) {
        m_selectedBuiltInTrack->deleteLater();
        m_selectedBuiltInTrack = nullptr;
    }
    m_selectedBuiltInTrack = BuiltInTrackFactory::createTrack(m_selectedTrackId, this);
    m_defaultRaceTrack = m_selectedBuiltInTrack;
    TrackManager* trackMgr = TrackManager::instance(this);
    if (trackMgr && m_defaultRaceTrack) {
        trackMgr->setCurrentTrack(m_defaultRaceTrack);
    }

    startDrivingSession(mode);
}

void MainWindow::onGameFinished()
{
    if (!m_driveActive || !m_scoreManager || !m_drivingDataCollector) {
        return;
    }

    m_driveActive = false;

    // Cancel the learning session auto-end timer if it is still running.
    if (m_learningSessionTimer) {
        m_learningSessionTimer->stop();
        m_learningSessionTimer->deleteLater();
        m_learningSessionTimer = nullptr;
    }

    m_drivingDataCollector->stopCollection();
    if (m_player2DataCollector) {
        m_player2DataCollector->stopCollection();
    }
    if (m_btnFinishDrive) {
        m_btnFinishDrive->setEnabled(false);
    }
    // Disable btn_Back while the report page is shown so it cannot accidentally
    // navigate away before the user has seen the report.
    if (ui->btn_Back) {
        ui->btn_Back->setEnabled(false);
    }

    if (m_learningHUD) {
        m_learningHUD->hide();
    }
    if (m_arcadeHUD) {
        m_arcadeHUD->hide();
    }
    if (m_gameView) {
        m_gameView->hide();
    }
    setGameHeaderVisible(false);

    // Grab all data synchronously before anything can mutate the collector.
    const QList<DrivingData> speedSamples = m_drivingDataCollector->getCollectedData();
    const ScoreReport report = m_scoreManager->finishSession(m_drivingDataCollector);
    QList<DrivingData> player2SpeedSamples;
    ScoreReport player2Report;
    if (m_twoPlayerMode && m_player2DataCollector && m_player2ScoreManager) {
        player2SpeedSamples = m_player2DataCollector->getCollectedData();
        player2Report = m_player2ScoreManager->finishSession(m_player2DataCollector);
    }

    qDebug() << "[onGameFinished] score=" << report.totalScore
             << "grade=" << report.grade
             << "samples=" << speedSamples.size()
             << "violations=" << report.violations.size()
             << "sessionId=" << report.sessionId
             << "stackedWidget count=" << ui->stackedWidget->count();

    qDebug() << "[onGameFinished] step 1 saveReport";
    SaveLoadManager::instance().saveReport(report);
    if (m_twoPlayerMode && !player2Report.sessionId.isEmpty()) {
        SaveLoadManager::instance().saveReport(player2Report);
    }

    // Populate the report widget and switch to the report page.
    if (!m_reportWidget || !m_reportPage) {
        qWarning() << "[onGameFinished] m_reportWidget or m_reportPage is null – report page not set up"
                   << "m_reportWidget=" << m_reportWidget
                   << "m_reportPage=" << m_reportPage;
        // Still clean up and return to menu to avoid getting stuck
        m_driveActive = false;
        if (ui->stackedWidget && ui->stackedWidget->count() > 0) {
            ui->stackedWidget->setCurrentIndex(0);
        }
        if (ui->btn_Back) {
            ui->btn_Back->setEnabled(true);
        }
        return;
    }

    qDebug() << "[onGameFinished] step 2 hideLoading";
    m_reportWidget->hideLoading();

    qDebug() << "[onGameFinished] step 3 loadHistory";
    m_reportWidget->loadHistoryFromSaveLoadManager();

    if (m_twoPlayerMode && !player2Report.sessionId.isEmpty()) {
        qDebug() << "[onGameFinished] step 4 setPlayerReports p1=" << speedSamples.size()
                 << "p2=" << player2SpeedSamples.size();
        m_reportWidget->setPlayerReports(report, speedSamples, player2Report, player2SpeedSamples);
    } else {
        qDebug() << "[onGameFinished] step 4 setSpeedSamples count=" << speedSamples.size();
        m_reportWidget->setSessionSpeedSamples(speedSamples);

        qDebug() << "[onGameFinished] step 5 setCurrentReport";
        m_reportWidget->setCurrentReport(report);
    }

    qDebug() << "[onGameFinished] step 6 switch to report page";
    ui->stackedWidget->setCurrentWidget(m_reportPage);
    m_reportPage->show();
    m_reportPage->raise();
    m_reportWidget->show();
    m_reportWidget->raise();
    ui->stackedWidget->update();
    qDebug() << "[onGameFinished] step 7 done – report page visible";

    // Re-enable btn_Back so the user can return to menu from the report page.
    if (ui->btn_Back) {
        ui->btn_Back->setEnabled(true);
    }

    statusBar()->showMessage(
        QStringLiteral("Driving report ready: %1 (%2)")
            .arg(report.totalScore, 0, 'f', 1)
            .arg(report.grade),
        5000);
}

void MainWindow::finishDrivingSession()
{
    onGameFinished();
}

void MainWindow::silentFinishSession()
{
    // End the current session without showing the report panel.
    // Used when the player starts a new game while one is already running.
    if (!m_driveActive) {
        return;
    }
    m_driveActive = false;

    if (m_learningSessionTimer) {
        m_learningSessionTimer->stop();
        m_learningSessionTimer->deleteLater();
        m_learningSessionTimer = nullptr;
    }

    if (m_drivingDataCollector) {
        m_drivingDataCollector->stopCollection();
    }
    if (m_btnFinishDrive) {
        m_btnFinishDrive->setEnabled(false);
    }
    // Restore btn_Back so the next session can use it normally.
    if (ui->btn_Back) {
        ui->btn_Back->setEnabled(true);
    }
    if (m_learningHUD) {
        m_learningHUD->hide();
    }
    if (m_arcadeHUD) {
        m_arcadeHUD->hide();
    }
    if (m_gameView) {
        m_gameView->hide();
    }
    setGameHeaderVisible(false);
    if (m_scoreManager && m_drivingDataCollector) {
        m_scoreManager->finishSession(m_drivingDataCollector);
    }
    if (m_twoPlayerMode && m_player2ScoreManager && m_player2DataCollector) {
        m_player2ScoreManager->finishSession(m_player2DataCollector);
    }
}

void MainWindow::showReportWindow(const ScoreReport* report)
{
    if (!m_reportWidget) {
        qWarning() << "[showReportWindow] m_reportWidget is null";
        return;
    }
    m_reportWidget->hideLoading();
    m_reportWidget->loadHistoryFromSaveLoadManager();

    if (report) {
        if (m_drivingDataCollector) {
            m_reportWidget->setSessionSpeedSamples(m_drivingDataCollector->getCollectedData());
        }
        m_reportWidget->setCurrentReport(*report);
    } else {
        const ScoreReport latest = m_scoreManager ? m_scoreManager->latestReport() : ScoreReport();
        if (!latest.sessionId.isEmpty()) {
            if (m_drivingDataCollector) {
                m_reportWidget->setSessionSpeedSamples(m_drivingDataCollector->getCollectedData());
            }
            m_reportWidget->setCurrentReport(latest);
        }
    }

    QWidget* reportPage = m_reportPage ? m_reportPage
                                       : (ui->pageReport ? static_cast<QWidget*>(ui->pageReport)
                                                         : ui->stackedWidget->widget(2));
    ui->stackedWidget->setCurrentWidget(reportPage);
    reportPage->show();
    reportPage->raise();
    m_reportWidget->show();
    m_reportWidget->raise();
}

void MainWindow::fadeInReportWindow()
{
    QWidget* reportPage = m_reportPage ? m_reportPage
                                       : (ui->pageReport ? static_cast<QWidget*>(ui->pageReport)
                                                         : ui->stackedWidget->widget(2));
    ui->stackedWidget->setCurrentWidget(reportPage);
    reportPage->show();
    reportPage->raise();
    if (m_reportWidget) {
        m_reportWidget->show();
        m_reportWidget->raise();
    }
}

void MainWindow::onReportBackToMenu()
{
    if (m_gameView) {
        m_gameView->hide();
    }
    if (m_learningHUD) {
        m_learningHUD->hide();
    }
    if (m_arcadeHUD) {
        m_arcadeHUD->hide();
    }
    setGameHeaderVisible(false);
    // Re-enable btn_Back now that we are returning to the menu.
    if (ui->btn_Back) {
        ui->btn_Back->setEnabled(true);
    }
    ui->stackedWidget->setCurrentIndex(0);
    statusBar()->clearMessage();
}

void MainWindow::onReportNewDrive()
{
    // Re-enable btn_Back before starting a new session.
    if (ui->btn_Back) {
        ui->btn_Back->setEnabled(true);
    }
    const QString mode = m_currentMode.isEmpty() ? QStringLiteral("Arcade") : m_currentMode;
    if (mode == QStringLiteral("Custom Track") && m_runtimeCustomTrack) {
        startCustomTrackSession(m_runtimeCustomTrack);
    } else {
        startDrivingSession(mode);
    }
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
    } else {
        // For non-Learning modes, show directly via InteractiveFeedback.
        showInteractiveFeedback(violation.description, PhantomDrive::FeedbackType::Warning);
    }

    playSound(PhantomDrive::SoundEffect::Violation);
    statusBar()->showMessage(violation.description, 3000);
}

void MainWindow::onScoreReady(const ScoreReport& report)
{
    Q_UNUSED(report);
    // finishDrivingSession() now handles saving and opening the report window
    // directly for every mode. Keep this slot as a compatibility hook only.
}

void MainWindow::onCoachReportReady(const QString& markdown)
{
    if (m_reportWidget) {
        m_reportWidget->setCoachReportMarkdown(markdown);
        // Coach report is the last async piece – ensure skeleton is dismissed.
        m_reportWidget->hideLoading();
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
    // Manual history open should immediately show the latest saved report
    // and history trend, never stay in a loading state.
    showReportWindow(nullptr);
}

void MainWindow::showInteractiveFeedback(const QString& message, PhantomDrive::FeedbackType type)
{
    PhantomDrive::InteractiveFeedback& feedback = PhantomDrive::InteractiveFeedback::instance(this);
    if (m_gameView) {
        feedback.setGameView(m_gameView);
    }
    feedback.showFeedback(message, type);
}

void MainWindow::playSound(PhantomDrive::SoundEffect effect)
{
    PhantomDrive::SoundManager::instance(this).play(effect);
}

void MainWindow::showCountdown()
{
    // Show ArcadeHUD in ALL modes during countdown
    if (m_arcadeHUD) {
        m_arcadeHUD->show();
        m_arcadeHUD->showCountdown(3);
        updateRaceHud();
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
    focusGameViewForDriving();
    QTimer::singleShot(50, this, [this]() {
        focusGameViewForDriving();
    });

    const int checkpointCount = m_gameView && m_gameView->trackData()
        ? m_gameView->trackData()->getCheckpointsInOrder().size()
        : 0;
    if (m_arcadeRaceLogicActive) {
        statusBar()->showMessage(
            QStringLiteral("GO! 检查点 %1 个已就绪 — 请先向南驶出北门/起跑线").arg(checkpointCount),
            6000);
    }

    playSound(PhantomDrive::SoundEffect::CountdownGo);
    QTimer::singleShot(0, this, [this]() {
        focusGameViewForDriving();
    });
    QTimer::singleShot(100, this, [this]() {
        focusGameViewForDriving();
    });

    // Unified: ArcadeHUD is shown in ALL modes; LearningHUD is no longer used.
    if (m_learningHUD) {
        m_learningHUD->hide();
    }
    if (m_arcadeHUD) {
        m_arcadeHUD->show();
    }

    updateRaceHud();

    if (m_btnFinishDrive) {
        m_btnFinishDrive->setEnabled(true);
    }

    if (m_customTrackPlaying) {
        statusBar()->showMessage(QStringLiteral("Custom Track | GO! Next: CP1"), 4000);
        return;
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

    playSound(PhantomDrive::SoundEffect::LapComplete);

    statusBar()->showMessage(
        QStringLiteral("第 %1 / %2 圈完成").arg(lapNumber).arg(m_totalLaps),
        3000);

    if (lapNumber < m_totalLaps) {
        return;
    }

    if (m_twoPlayerMode) {
        finishTwoPlayerRace(1);
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

    playSound(PhantomDrive::SoundEffect::RaceFinish);

    // Wait 2 s so the player can see the "Race Finished" banner, then show report.
    QTimer::singleShot(2000, this, [this]() {
        if (m_driveActive) {
            onGameFinished();
        }
    });
}

void MainWindow::onCheckpointReached(int checkpointNumber)
{
    const int displayIndex = checkpointNumber + 1;
    const int totalGates = m_raceCheckpointTotal > 0 ? m_raceCheckpointTotal : 4;

    if (m_customTrackPlaying) {
        const QString nextTarget = displayIndex >= totalGates
            ? QStringLiteral("FINISH")
            : QStringLiteral("CP%1").arg(displayIndex + 1);
        if (m_arcadeHUD) {
            m_arcadeHUD->showRaceBanner(
                QStringLiteral("CP %1/%2 passed | Next: %3")
                    .arg(displayIndex)
                    .arg(totalGates)
                    .arg(nextTarget));
        }
        playSound(PhantomDrive::SoundEffect::Checkpoint);
        statusBar()->showMessage(
            QStringLiteral("Checkpoint %1/%2 passed. Next: %3")
                .arg(displayIndex)
                .arg(totalGates)
                .arg(nextTarget),
            5000);
        return;
    }

    if (m_arcadeHUD) {
        m_arcadeHUD->showRaceBanner(
            QStringLiteral("检查点 %1/%2 已通过").arg(displayIndex).arg(totalGates));
    }

    playSound(PhantomDrive::SoundEffect::Checkpoint);

    statusBar()->showMessage(
        QStringLiteral("检查点 %1/%2 已通过 — 请继续下一门").arg(displayIndex).arg(totalGates),
        5000);
}

void MainWindow::onCollision()
{
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

    showInteractiveFeedback(displayText, type);
    playSound(PhantomDrive::SoundEffect::PowerupCollect);
}

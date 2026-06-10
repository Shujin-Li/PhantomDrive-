#include "UI/TestVisualizationWindow.h"
#include "UI/GameViewWidget.h"
#include "track/TrackData.h"
#include "track/TrackTile.h"

#include <QApplication>
#include <QScreen>
#include <QtMath>
#include <QRandomGenerator>

TestVisualizationWindow::TestVisualizationWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_gameView(nullptr)
    , m_animateBtn(nullptr)
    , m_resetCameraBtn(nullptr)
    , m_zoomInBtn(nullptr)
    , m_zoomOutBtn(nullptr)
    , m_addPowerupBtn(nullptr)
    , m_addTrafficLightBtn(nullptr)
    , m_cycleTrafficLightBtn(nullptr)
    , m_clearAllBtn(nullptr)
    , m_statusLabel(nullptr)
    , m_fpsLabel(nullptr)
    , m_simulationTimer(nullptr)
    , m_fpsTimer(nullptr)
    , m_isAnimating(false)
    , m_playerAngle(0)
    , m_playerX(0)
    , m_playerY(0)
    , m_trafficLightState(0)
    , m_powerupCount(1)
    , m_frameCount(0)
{
    setWindowTitle("PhantomDrive - E-A 可视化测试程序");
    resize(1200, 800);

    setupUI();
    setupConnections();
    startSimulation();
}

TestVisualizationWindow::~TestVisualizationWindow()
{
    if (m_simulationTimer) {
        m_simulationTimer->stop();
    }
    if (m_fpsTimer) {
        m_fpsTimer->stop();
    }
}

void TestVisualizationWindow::setupUI()
{
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    m_gameView = new PhantomDrive::GameViewWidget(this);
    m_gameView->setMinimumSize(800, 600);
    mainLayout->addWidget(m_gameView);

    PhantomDrive::TrackData* testTrack = new PhantomDrive::TrackData(this);
    testTrack->setId("test_demo_track");
    testTrack->setName("Demo Track");
    testTrack->setSize(26, 11);

    for (int row = 0; row < 26; ++row) {
        for (int col = 0; col < 11; ++col) {
            PhantomDrive::TrackTile* tile = new PhantomDrive::TrackTile();
            tile->setPosition(row, col);

            bool isRoad = (col >= 4 && col <= 6);
            bool isIntersection = (row >= 10 && row <= 12 && col >= 3 && col <= 7);

            if (isIntersection) {
                tile->setType(PhantomDrive::TileType::Asphalt);
            } else if (isRoad) {
                tile->setType(PhantomDrive::TileType::Road);
            } else {
                tile->setType(PhantomDrive::TileType::Grass);
            }

            testTrack->setTileAt(row, col, tile);
        }
    }

    m_gameView->setTrackData(testTrack);

    m_gameView->updatePlayerCar(QVector2D(0, 0), 0, 0);
    m_gameView->addPowerupBox("powerup_demo_1", QVector2D(150, 50), "Shield");
    m_gameView->addTrafficLight("traffic_light_demo_1", QVector2D(200, 150), "green");
    m_gameView->addSpeedLimitSign("speed_sign_demo_1", QVector2D(100, 200), 60);

    QHBoxLayout* controlLayout = new QHBoxLayout();

    m_animateBtn = new QPushButton("开始动画");
    m_animateBtn->setCheckable(true);
    controlLayout->addWidget(m_animateBtn);

    m_resetCameraBtn = new QPushButton("重置相机");
    controlLayout->addWidget(m_resetCameraBtn);

    m_zoomInBtn = new QPushButton("放大");
    controlLayout->addWidget(m_zoomInBtn);

    m_zoomOutBtn = new QPushButton("缩小");
    controlLayout->addWidget(m_zoomOutBtn);

    m_addPowerupBtn = new QPushButton("添加道具盒");
    controlLayout->addWidget(m_addPowerupBtn);

    m_addTrafficLightBtn = new QPushButton("添加红绿灯");
    controlLayout->addWidget(m_addTrafficLightBtn);

    m_cycleTrafficLightBtn = new QPushButton("切换红绿灯");
    controlLayout->addWidget(m_cycleTrafficLightBtn);

    m_clearAllBtn = new QPushButton("清空");
    controlLayout->addWidget(m_clearAllBtn);

    controlLayout->addStretch();

    m_statusLabel = new QLabel("状态: 就绪");
    controlLayout->addWidget(m_statusLabel);

    m_fpsLabel = new QLabel("FPS: 0");
    controlLayout->addWidget(m_fpsLabel);

    mainLayout->addLayout(controlLayout);
}

void TestVisualizationWindow::setupConnections()
{
    connect(m_animateBtn, &QPushButton::clicked, this, &TestVisualizationWindow::onAnimateToggle);
    connect(m_resetCameraBtn, &QPushButton::clicked, this, &TestVisualizationWindow::onResetCamera);
    connect(m_zoomInBtn, &QPushButton::clicked, this, &TestVisualizationWindow::onZoomIn);
    connect(m_zoomOutBtn, &QPushButton::clicked, this, &TestVisualizationWindow::onZoomOut);
    connect(m_addPowerupBtn, &QPushButton::clicked, this, &TestVisualizationWindow::onAddPowerup);
    connect(m_addTrafficLightBtn, &QPushButton::clicked, this, &TestVisualizationWindow::onAddTrafficLight);
    connect(m_cycleTrafficLightBtn, &QPushButton::clicked, this, &TestVisualizationWindow::onCycleTrafficLight);
    connect(m_clearAllBtn, &QPushButton::clicked, this, &TestVisualizationWindow::onClearAll);

    m_simulationTimer = new QTimer(this);
    connect(m_simulationTimer, &QTimer::timeout, this, &TestVisualizationWindow::onSimulationTick);

    m_fpsTimer = new QTimer(this);
    connect(m_fpsTimer, &QTimer::timeout, this, [this]() {
        m_fpsLabel->setText(QString("FPS: %1").arg(m_frameCount));
        m_frameCount = 0;
    });
    m_fpsTimer->start(1000);
}

void TestVisualizationWindow::startSimulation()
{
    m_simulationTimer->start(50);
}

void TestVisualizationWindow::onAnimateToggle()
{
    m_isAnimating = m_animateBtn->isChecked();
    m_animateBtn->setText(m_isAnimating ? "停止动画" : "开始动画");
    m_statusLabel->setText(m_isAnimating ? "状态: 动画运行中" : "状态: 已停止");
}

void TestVisualizationWindow::onResetCamera()
{
    m_gameView->resetCamera();
    m_statusLabel->setText("状态: 相机已重置");
}

void TestVisualizationWindow::onZoomIn()
{
    m_gameView->setCameraZoom(m_gameView->getRenderState().cameraZoom * 1.2);
    m_statusLabel->setText("状态: 放大");
}

void TestVisualizationWindow::onZoomOut()
{
    m_gameView->setCameraZoom(m_gameView->getRenderState().cameraZoom / 1.2);
    m_statusLabel->setText("状态: 缩小");
}

void TestVisualizationWindow::onSimulationTick()
{
    m_frameCount++;

    if (m_isAnimating) {
        m_playerAngle += 2.0;
        if (m_playerAngle >= 360.0) {
            m_playerAngle -= 360.0;
        }

        qreal radius = 100.0;
        m_playerX = qCos(m_playerAngle * M_PI / 180.0) * radius;
        m_playerY = qSin(m_playerAngle * M_PI / 180.0) * radius;

        m_gameView->updatePlayerCar(QVector2D(m_playerX, m_playerY), m_playerAngle, 50.0);
        m_gameView->setCameraPosition(QVector2D(m_playerX, m_playerY));
    }

    m_gameView->refresh();
}

void TestVisualizationWindow::onAddPowerup()
{
    QString id = QString("powerup_%1").arg(++m_powerupCount);
    qreal x = (QRandomGenerator::global()->bounded(400)) - 200;
    qreal y = (QRandomGenerator::global()->bounded(400)) - 200;
    QStringList types = {"Shield", "Speed", "Nitro", "Magnet"};
    QString type = types[QRandomGenerator::global()->bounded(types.size())];

    m_gameView->addPowerupBox(id, QVector2D(x, y), type);
    m_statusLabel->setText(QString("状态: 已添加道具 %1 (%2)").arg(id).arg(type));
}

void TestVisualizationWindow::onAddTrafficLight()
{
    QString id = QString("traffic_light_%1").arg(QRandomGenerator::global()->bounded(1000));
    qreal x = (QRandomGenerator::global()->bounded(400)) - 200;
    qreal y = (QRandomGenerator::global()->bounded(400)) - 200;

    m_gameView->addTrafficLight(id, QVector2D(x, y), "green");
    m_statusLabel->setText(QString("状态: 已添加红绿灯 %1").arg(id));
}

void TestVisualizationWindow::onCycleTrafficLight()
{
    QStringList states = {"green", "yellow", "red"};
    m_trafficLightState = (m_trafficLightState + 1) % states.size();
    QString newState = states[m_trafficLightState];

    m_gameView->updateTrafficLight("traffic_light_demo_1", newState);
    m_statusLabel->setText(QString("状态: 红绿灯 -> %1").arg(newState));
}

void TestVisualizationWindow::onClearAll()
{
    m_gameView->clearAll();
    m_gameView->updatePlayerCar(QVector2D(0, 0), 0, 0);
    m_gameView->addPowerupBox("powerup_demo_1", QVector2D(150, 50), "Shield");
    m_gameView->addTrafficLight("traffic_light_demo_1", QVector2D(200, 150), "green");
    m_statusLabel->setText("状态: 已清空并重置");
}

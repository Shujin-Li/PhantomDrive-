#pragma once

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QTimer>

namespace PhantomDrive {
    class GameViewWidget;
}

class TestVisualizationWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TestVisualizationWindow(QWidget* parent = nullptr);
    ~TestVisualizationWindow() override;

private slots:
    void onAnimateToggle();
    void onResetCamera();
    void onZoomIn();
    void onZoomOut();
    void onSimulationTick();
    void onAddPowerup();
    void onAddTrafficLight();
    void onCycleTrafficLight();
    void onClearAll();

private:
    void setupUI();
    void setupConnections();
    void startSimulation();
    void updateDemoObjects();

    PhantomDrive::GameViewWidget* m_gameView;

    QPushButton* m_animateBtn;
    QPushButton* m_resetCameraBtn;
    QPushButton* m_zoomInBtn;
    QPushButton* m_zoomOutBtn;
    QPushButton* m_addPowerupBtn;
    QPushButton* m_addTrafficLightBtn;
    QPushButton* m_cycleTrafficLightBtn;
    QPushButton* m_clearAllBtn;

    QLabel* m_statusLabel;
    QLabel* m_fpsLabel;

    QTimer* m_simulationTimer;
    QTimer* m_fpsTimer;

    bool m_isAnimating;
    qreal m_playerAngle;
    qreal m_playerX;
    qreal m_playerY;
    int m_trafficLightState;
    int m_powerupCount;
    int m_frameCount;
};

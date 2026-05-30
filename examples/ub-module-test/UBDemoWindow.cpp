#include "UBDemoWindow.h"

#include <QPushButton>
#include <QLabel>
#include <QApplication>

#include "UI/InteractiveFeedback.h"
#include "UI/SoundManager.h"
#include "UI/ArcadeHUD.h"
#include "UI/ThemeManager.h"
#include "UI/SoundGenerator.h"

using namespace PhantomDrive;

UBDemoWindow::UBDemoWindow()
    : m_arcadeHUD(nullptr)
{
    setupUI();
}

UBDemoWindow::~UBDemoWindow()
{
}

void UBDemoWindow::setupUI()
{
    setWindowTitle("U-B Module Demo");
    resize(800, 600);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* titleLabel = new QLabel("U-B Module Test - InteractiveFeedback & SoundManager", this);
    titleLabel->setStyleSheet("QLabel { font-size: 18px; font-weight: bold; padding: 10px; }");
    mainLayout->addWidget(titleLabel);

    QPushButton* btnPositive = new QPushButton("Show Positive Feedback (Great!)", this);
    mainLayout->addWidget(btnPositive);
    connect(btnPositive, &QPushButton::clicked, this, &UBDemoWindow::onPositiveClicked);

    QPushButton* btnWarning = new QPushButton("Show Warning (Speeding!)", this);
    mainLayout->addWidget(btnWarning);
    connect(btnWarning, &QPushButton::clicked, this, &UBDemoWindow::onWarningClicked);

    QPushButton* btnCritical = new QPushButton("Show Critical (Collision!)", this);
    mainLayout->addWidget(btnCritical);
    connect(btnCritical, &QPushButton::clicked, this, &UBDemoWindow::onCriticalClicked);

    QPushButton* btnPowerup = new QPushButton("Show Powerup (Boost!)", this);
    mainLayout->addWidget(btnPowerup);
    connect(btnPowerup, &QPushButton::clicked, this, &UBDemoWindow::onPowerupClicked);

    QPushButton* btnMilestone = new QPushButton("Show Milestone (Lap Complete!)", this);
    mainLayout->addWidget(btnMilestone);
    connect(btnMilestone, &QPushButton::clicked, this, &UBDemoWindow::onMilestoneClicked);

    QPushButton* btnCountdown = new QPushButton("Show Countdown (3-2-1-GO!)", this);
    mainLayout->addWidget(btnCountdown);
    connect(btnCountdown, &QPushButton::clicked, this, &UBDemoWindow::onCountdownClicked);

    mainLayout->addSpacing(20);

    QLabel* soundLabel = new QLabel("Sound Tests:", this);
    soundLabel->setStyleSheet("QLabel { font-size: 14px; font-weight: bold; }");
    mainLayout->addWidget(soundLabel);

    QPushButton* btnBeep = new QPushButton("Play Countdown Beep", this);
    mainLayout->addWidget(btnBeep);
    connect(btnBeep, &QPushButton::clicked, this, &UBDemoWindow::onBeepClicked);

    QPushButton* btnCollision = new QPushButton("Play Collision Sound", this);
    mainLayout->addWidget(btnCollision);
    connect(btnCollision, &QPushButton::clicked, this, &UBDemoWindow::onCollisionClicked);

    QPushButton* btnViolation = new QPushButton("Play Violation Sound", this);
    mainLayout->addWidget(btnViolation);
    connect(btnViolation, &QPushButton::clicked, this, &UBDemoWindow::onViolationClicked);

    QPushButton* btnLapComplete = new QPushButton("Play Lap Complete", this);
    mainLayout->addWidget(btnLapComplete);
    connect(btnLapComplete, &QPushButton::clicked, this, &UBDemoWindow::onLapCompleteClicked);

    mainLayout->addSpacing(20);

    QLabel* hudLabel = new QLabel("Arcade HUD Test:", this);
    hudLabel->setStyleSheet("QLabel { font-size: 14px; font-weight: bold; }");
    mainLayout->addWidget(hudLabel);

    QPushButton* btnShowHUD = new QPushButton("Show Arcade HUD", this);
    mainLayout->addWidget(btnShowHUD);
    connect(btnShowHUD, &QPushButton::clicked, this, &UBDemoWindow::onShowHUDClicked);

    QPushButton* btnTestHUD = new QPushButton("Test HUD Updates", this);
    mainLayout->addWidget(btnTestHUD);
    connect(btnTestHUD, &QPushButton::clicked, this, &UBDemoWindow::onTestHUDClicked);

    mainLayout->addStretch();

    m_arcadeHUD = new ArcadeHUD(this);
}

void UBDemoWindow::onPositiveClicked()
{
    InteractiveFeedback::instance().showFeedback("Great!", FeedbackType::Positive);
}

void UBDemoWindow::onWarningClicked()
{
    InteractiveFeedback::instance().showFeedback("Speeding! -5", FeedbackType::Warning);
}

void UBDemoWindow::onCriticalClicked()
{
    InteractiveFeedback::instance().showFeedback("Wall Hit!", FeedbackType::Critical);
}

void UBDemoWindow::onPowerupClicked()
{
    InteractiveFeedback::instance().showFeedback("Boost Collected!", FeedbackType::Powerup);
}

void UBDemoWindow::onMilestoneClicked()
{
    InteractiveFeedback::instance().showFeedback("Lap 1 Completed!", FeedbackType::Milestone);
}

void UBDemoWindow::onCountdownClicked()
{
    InteractiveFeedback::instance().showCountdown(3);
}

void UBDemoWindow::onBeepClicked()
{
    SoundManager::instance().play(SoundEffect::CountdownBeep);
}

void UBDemoWindow::onCollisionClicked()
{
    SoundManager::instance().play(SoundEffect::Collision);
}

void UBDemoWindow::onViolationClicked()
{
    SoundManager::instance().play(SoundEffect::Violation);
}

void UBDemoWindow::onLapCompleteClicked()
{
    SoundManager::instance().play(SoundEffect::LapComplete);
}

void UBDemoWindow::onShowHUDClicked()
{
    m_arcadeHUD->show();
}

void UBDemoWindow::onTestHUDClicked()
{
    m_arcadeHUD->updateSpeed(120);
    m_arcadeHUD->updateLap(2, 3);
    m_arcadeHUD->updateLapTime("01:23.456");
    m_arcadeHUD->updateTotalTime("02:45.678");
    m_arcadeHUD->updatePosition(2, 3);
    m_arcadeHUD->updateBestLapTime("01:20.123");
    m_arcadeHUD->show();
}

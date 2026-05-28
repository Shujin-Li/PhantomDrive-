#include "UI/ArcadeHUD.h"

#include <QApplication>
#include <QScreen>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QTimer>

using namespace PhantomDrive;

ArcadeHUD::ArcadeHUD(QWidget* parent)
    : QWidget(parent)
    , m_speedLabel(nullptr)
    , m_speedUnitLabel(nullptr)
    , m_lapLabel(nullptr)
    , m_lapTimeLabel(nullptr)
    , m_totalTimeLabel(nullptr)
    , m_positionLabel(nullptr)
    , m_bestLapLabel(nullptr)
    , m_countdownLabel(nullptr)
    , m_centerMessageWidget(nullptr)
    , m_centerMessageLabel(nullptr)
    , m_countdownTimer(new QTimer(this))
    , m_countdownValue(0)
    , m_currentLap(1)
    , m_totalLaps(3)
    , m_currentSpeed(0.0)
{
    setupUI();
    setFloatingStyle();

    connect(m_countdownTimer, &QTimer::timeout, this, &ArcadeHUD::onCountdownTick);
}

ArcadeHUD::~ArcadeHUD()
{
}

void ArcadeHUD::setupUI()
{
    setFixedSize(300, 280);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    QLabel* titleLabel = new QLabel("Arcade Mode", this);
    titleLabel->setStyleSheet(R"(
        QLabel {
            color: #E74C3C;
            font-size: 18px;
            font-weight: bold;
            padding: 5px;
        }
    )");
    mainLayout->addWidget(titleLabel, 0, Qt::AlignCenter);

    QHBoxLayout* speedLayout = new QHBoxLayout();
    m_speedLabel = new QLabel("0", this);
    m_speedLabel->setStyleSheet(R"(
        QLabel {
            color: #2ECC71;
            font-size: 48px;
            font-weight: bold;
            font-family: 'Consolas', 'Courier New', monospace;
        }
    )");
    speedLayout->addWidget(m_speedLabel);
    speedLayout->addStretch();

    m_speedUnitLabel = new QLabel("km/h", this);
    m_speedUnitLabel->setStyleSheet(R"(
        QLabel {
            color: #95A5A6;
            font-size: 14px;
        }
    )");
    speedLayout->addWidget(m_speedUnitLabel);
    mainLayout->addLayout(speedLayout);

    QHBoxLayout* lapLayout = new QHBoxLayout();
    QLabel* lapTitle = new QLabel("Lap", this);
    lapTitle->setStyleSheet("QLabel { color: #BDC3C7; font-size: 12px; }");
    lapLayout->addWidget(lapTitle);

    m_lapLabel = new QLabel("1 / 3", this);
    m_lapLabel->setStyleSheet(R"(
        QLabel {
            color: #FFFFFF;
            font-size: 16px;
            font-weight: bold;
        }
    )");
    lapLayout->addWidget(m_lapLabel);
    lapLayout->addStretch();
    mainLayout->addLayout(lapLayout);

    QHBoxLayout* timeLayout = new QHBoxLayout();
    QLabel* timeTitle = new QLabel("Lap Time", this);
    timeTitle->setStyleSheet("QLabel { color: #BDC3C7; font-size: 12px; }");
    timeLayout->addWidget(timeTitle);

    m_lapTimeLabel = new QLabel("00:00.000", this);
    m_lapTimeLabel->setStyleSheet(R"(
        QLabel {
            color: #3498DB;
            font-size: 14px;
            font-family: 'Consolas', monospace;
        }
    )");
    timeLayout->addWidget(m_lapTimeLabel);
    timeLayout->addStretch();
    mainLayout->addLayout(timeLayout);

    QHBoxLayout* totalLayout = new QHBoxLayout();
    QLabel* totalTitle = new QLabel("Total", this);
    totalTitle->setStyleSheet("QLabel { color: #BDC3C7; font-size: 12px; }");
    totalLayout->addWidget(totalTitle);

    m_totalTimeLabel = new QLabel("00:00.000", this);
    m_totalTimeLabel->setStyleSheet(R"(
        QLabel {
            color: #9B59B6;
            font-size: 14px;
            font-family: 'Consolas', monospace;
        }
    )");
    totalLayout->addWidget(m_totalTimeLabel);
    totalLayout->addStretch();
    mainLayout->addLayout(totalLayout);

    QHBoxLayout* posLayout = new QHBoxLayout();
    QLabel* posTitle = new QLabel("Position", this);
    posTitle->setStyleSheet("QLabel { color: #BDC3C7; font-size: 12px; }");
    posLayout->addWidget(posTitle);

    m_positionLabel = new QLabel("1st", this);
    m_positionLabel->setStyleSheet(R"(
        QLabel {
            color: #F1C40F;
            font-size: 16px;
            font-weight: bold;
        }
    )");
    posLayout->addWidget(m_positionLabel);
    posLayout->addStretch();

    QLabel* bestTitle = new QLabel("Best", this);
    bestTitle->setStyleSheet("QLabel { color: #BDC3C7; font-size: 12px; }");
    posLayout->addWidget(bestTitle);

    m_bestLapLabel = new QLabel("--:--.---", this);
    m_bestLapLabel->setStyleSheet(R"(
        QLabel {
            color: #E74C3C;
            font-size: 12px;
            font-family: 'Consolas', monospace;
        }
    )");
    posLayout->addWidget(m_bestLapLabel);
    mainLayout->addLayout(posLayout);

    mainLayout->addStretch();

    m_countdownLabel = new QLabel("", this);
    m_countdownLabel->setAlignment(Qt::AlignCenter);
    m_countdownLabel->setStyleSheet(R"(
        QLabel {
            color: #F1C40F;
            font-size: 72px;
            font-weight: bold;
            background: transparent;
        }
    )");
    mainLayout->addWidget(m_countdownLabel, 0, Qt::AlignCenter);

    m_centerMessageWidget = new QWidget(this);
    m_centerMessageWidget->setVisible(false);
    QVBoxLayout* centerLayout = new QVBoxLayout(m_centerMessageWidget);
    centerLayout->setContentsMargins(0, 0, 0, 0);

    m_centerMessageLabel = new QLabel("", m_centerMessageWidget);
    m_centerMessageLabel->setAlignment(Qt::AlignCenter);
    centerLayout->addWidget(m_centerMessageLabel);
}

void ArcadeHUD::setFloatingStyle()
{
    setStyleSheet(R"(
        ArcadeHUD {
            background-color: rgba(20, 20, 30, 235);
            border: 2px solid #3498DB;
            border-radius: 10px;
        }
    )");
}

void ArcadeHUD::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
}

void ArcadeHUD::updateSpeed(qreal speed)
{
    m_currentSpeed = speed;
    m_speedLabel->setText(QString::number(static_cast<int>(speed)));
}

void ArcadeHUD::updateLap(int lapsCompleted, int totalLaps)
{
    m_currentLap = lapsCompleted + 1;
    m_totalLaps = totalLaps;
    m_lapLabel->setText(
        QStringLiteral("完成 %1/%2 · 第 %3 圈")
            .arg(lapsCompleted)
            .arg(totalLaps)
            .arg(lapsCompleted + 1));
}

void ArcadeHUD::updateLapTime(const QString& time)
{
    m_lapTimeLabel->setText(time);
}

void ArcadeHUD::updateTotalTime(const QString& time)
{
    m_totalTimeLabel->setText(time);
}

void ArcadeHUD::updatePosition(int position, int totalRacers)
{
    QString posText;
    switch (position) {
    case 1: posText = "1st"; break;
    case 2: posText = "2nd"; break;
    case 3: posText = "3rd"; break;
    default: posText = QString("%1th").arg(position); break;
    }
    m_positionLabel->setText(posText);
}

void ArcadeHUD::updateBestLapTime(const QString& time)
{
    m_bestLapLabel->setText(time);
}

void ArcadeHUD::showCountdown(int seconds)
{
    m_countdownValue = seconds;
    m_countdownLabel->setText(QString::number(m_countdownValue));
    m_countdownLabel->setVisible(true);

    QPropertyAnimation* anim = new QPropertyAnimation(m_countdownLabel, "styleSheet");
    anim->setDuration(300);
    anim->setStartValue(m_countdownLabel->styleSheet());
    anim->setEndValue(R"(
        QLabel {
            color: #F1C40F;
            font-size: 72px;
            font-weight: bold;
            background: transparent;
        }
    )");
    anim->start();
    connect(anim, &QPropertyAnimation::finished, anim, &QPropertyAnimation::deleteLater);

    m_countdownTimer->start(1000);
}

void ArcadeHUD::onCountdownTick()
{
    m_countdownValue--;

    if (m_countdownValue > 0) {
        m_countdownLabel->setText(QString::number(m_countdownValue));
    } else {
        m_countdownTimer->stop();
        showGo();
    }
}

void ArcadeHUD::showGo()
{
    m_countdownLabel->setText("GO!");
    m_countdownLabel->setStyleSheet(R"(
        QLabel {
            color: #2ECC71;
            font-size: 72px;
            font-weight: bold;
            background: transparent;
        }
    )");

    QTimer::singleShot(1500, this, [this]() {
        m_countdownLabel->setVisible(false);
    });
}

void ArcadeHUD::showRaceBanner(const QString& message)
{
    if (!m_countdownLabel) {
        return;
    }

    m_countdownLabel->setText(message);
    m_countdownLabel->setStyleSheet(R"(
        QLabel {
            color: #2ECC71;
            font-size: 20px;
            font-weight: bold;
            background: transparent;
            padding: 8px;
        }
    )");
    m_countdownLabel->setVisible(true);

    QTimer::singleShot(3500, this, [this]() {
        if (m_countdownLabel) {
            m_countdownLabel->setVisible(false);
            m_countdownLabel->setText(QString());
        }
    });
}

void ArcadeHUD::showLapCompleted(int lapNumber)
{
    showRaceBanner(QStringLiteral("Lap %1 Complete!").arg(lapNumber));
}

void ArcadeHUD::showRaceFinished(int finalPosition, const QString& totalTime)
{
    QString posText;
    switch (finalPosition) {
    case 1: posText = "1st Place!"; break;
    case 2: posText = "2nd Place"; break;
    case 3: posText = "3rd Place"; break;
    default: posText = QString("%1th Place").arg(finalPosition); break;
    }

    showFloatingMessage(QString("%1\nTotal: %2").arg(posText).arg(totalTime), R"(
        QLabel {
            color: #F1C40F;
            font-size: 32px;
            font-weight: bold;
            background: transparent;
            padding: 20px;
        }
    )");
}

void ArcadeHUD::reset()
{
    m_currentLap = 1;
    m_totalLaps = 3;
    m_currentSpeed = 0;
    m_countdownValue = COUNTDOWN_START;

    if (m_speedLabel) m_speedLabel->setText("0");
    if (m_lapLabel) m_lapLabel->setText(QStringLiteral("完成 0/3 · 第 1 圈"));
    if (m_lapTimeLabel) m_lapTimeLabel->setText("0:00.000");
    if (m_totalTimeLabel) m_totalTimeLabel->setText("0:00.000");
    if (m_positionLabel) m_positionLabel->setText("P1");
    if (m_bestLapLabel) m_bestLapLabel->setText("-");
    if (m_countdownLabel) m_countdownLabel->setText("");
    if (m_centerMessageWidget) m_centerMessageWidget->setVisible(false);
    if (m_countdownTimer) m_countdownTimer->stop();
}

void ArcadeHUD::showFloatingMessage(const QString& message, const QString& style)
{
    m_centerMessageLabel->setText(message);
    m_centerMessageLabel->setStyleSheet(style);
    m_centerMessageWidget->setVisible(true);

    QParallelAnimationGroup* group = new QParallelAnimationGroup(this);

    QPropertyAnimation* fadeIn = new QPropertyAnimation(m_centerMessageWidget, "windowOpacity");
    fadeIn->setDuration(300);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    group->addAnimation(fadeIn);

    QPropertyAnimation* scaleUp = new QPropertyAnimation(m_centerMessageWidget, "size");
    scaleUp->setDuration(300);
    scaleUp->setStartValue(QSize(100, 50));
    scaleUp->setEndValue(m_centerMessageWidget->sizeHint());
    scaleUp->setEasingCurve(QEasingCurve::OutBack);
    group->addAnimation(scaleUp);

    group->start();

    QTimer::singleShot(3000, this, [this, group]() {
        QParallelAnimationGroup* fadeOut = new QParallelAnimationGroup(this);

        QPropertyAnimation* fade = new QPropertyAnimation(m_centerMessageWidget, "windowOpacity");
        fade->setDuration(500);
        fade->setStartValue(1.0);
        fade->setEndValue(0.0);
        fadeOut->addAnimation(fade);

        fadeOut->start();
        connect(fadeOut, &QParallelAnimationGroup::finished, this, [this]() {
            m_centerMessageWidget->setVisible(false);
            m_centerMessageWidget->setWindowOpacity(1.0);
        });
        delete fadeOut;
    });
}

void ArcadeHUD::setVisible(bool visible)
{
    QWidget::setVisible(visible);
}

void ArcadeHUD::onMessageTimeout()
{
    m_centerMessageWidget->setVisible(false);
}

#include "UI/learninghud.h"

#include <QApplication>
#include <QScreen>
#include <QTimer>
#include <QFrame>

LearningHUD::LearningHUD(QWidget *parent)
    : QWidget(parent)
    , m_modeLabel(nullptr)
    , m_trafficDot(nullptr)
    , m_stopLabel(nullptr)
    , m_speedLabel(nullptr)
    , m_speedUnitLabel(nullptr)
    , m_speedLimitLabel(nullptr)
    , m_trafficLightLabel(nullptr)
    , m_lapLabel(nullptr)
    , m_lapTimeLabel(nullptr)
    , m_powerupLabel(nullptr)
    , m_blinkTimer(new QTimer(this))
{
    setupUI();
    connect(m_blinkTimer, &QTimer::timeout, this, &LearningHUD::onRedLightBlink);
}

LearningHUD::~LearningHUD()
{
    qDeleteAll(m_floatingMessages);
}

void LearningHUD::setupUI()
{
    setFixedWidth(300);
    setMinimumHeight(480);
    setStyleSheet(R"(
        LearningHUD {
            background-color: rgba(8, 14, 26, 245);
            border-left: 2px solid rgba(52, 152, 219, 120);
        }
    )");

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(12, 10, 12, 10);
    root->setSpacing(0);

    auto makeDivider = [&]() -> QFrame* {
        QFrame *d = new QFrame(this);
        d->setFrameShape(QFrame::HLine);
        d->setStyleSheet("QFrame{color:rgba(80,120,180,60);}");
        d->setFixedHeight(1);
        return d;
    };

    auto makeInfoRow = [&](const QString& title, QLabel*& valueOut,
                           const QString& initVal = "--") -> QWidget* {
        QWidget* row = new QWidget(this);
        QHBoxLayout* hl = new QHBoxLayout(row);
        hl->setContentsMargins(0, 0, 0, 0);
        hl->setSpacing(4);
        QLabel* t = new QLabel(title, row);
        t->setStyleSheet("QLabel{color:rgba(160,190,230,160);font-size:10px;font-weight:bold;}");
        t->setFixedWidth(72);
        hl->addWidget(t);
        valueOut = new QLabel(initVal, row);
        valueOut->setStyleSheet(
            "QLabel{color:#D0E8FF;font-size:12px;font-weight:bold;"
            "font-family:'Consolas','Courier New',monospace;}");
        hl->addWidget(valueOut);
        hl->addStretch();
        return row;
    };

    // ---- TOP BAR ----
    {
        QHBoxLayout *topBar = new QHBoxLayout();
        topBar->setSpacing(6);
        m_modeLabel = new QLabel("LEARNING MODE", this);
        m_modeLabel->setStyleSheet(
            "QLabel{color:#3498DB;font-size:12px;font-weight:900;letter-spacing:3px;}");
        topBar->addWidget(m_modeLabel);
        topBar->addStretch();
        root->addLayout(topBar);
        root->addSpacing(6);
    }

    root->addWidget(makeDivider());
    root->addSpacing(8);

    // ---- SPEED ROW ----
    {
        QHBoxLayout *speedRow = new QHBoxLayout();
        speedRow->setSpacing(10);

        QVBoxLayout *speedCol = new QVBoxLayout();
        speedCol->setSpacing(0);
        QLabel *speedTitle = new QLabel("SPEED", this);
        speedTitle->setStyleSheet(
            "QLabel{color:rgba(160,190,230,160);font-size:9px;font-weight:bold;letter-spacing:2px;}");
        speedCol->addWidget(speedTitle);
        QHBoxLayout *speedNumRow = new QHBoxLayout();
        speedNumRow->setSpacing(3);
        m_speedLabel = new QLabel("0", this);
        m_speedLabel->setStyleSheet(
            "QLabel{color:#00E5FF;font-size:38px;font-weight:bold;"
            "font-family:'Consolas','Courier New',monospace;}");
        speedNumRow->addWidget(m_speedLabel);
        m_speedUnitLabel = new QLabel("km/h", this);
        m_speedUnitLabel->setStyleSheet(
            "QLabel{color:rgba(160,190,230,140);font-size:11px;margin-top:18px;}");
        speedNumRow->addWidget(m_speedUnitLabel);
        speedNumRow->addStretch();
        speedCol->addLayout(speedNumRow);
        speedRow->addLayout(speedCol);

        QVBoxLayout *limitCol = new QVBoxLayout();
        limitCol->setSpacing(0);
        QLabel *limitTitle = new QLabel("LIMIT", this);
        limitTitle->setStyleSheet(
            "QLabel{color:rgba(160,190,230,160);font-size:9px;font-weight:bold;letter-spacing:2px;}");
        limitCol->addWidget(limitTitle);
        m_speedLimitLabel = new QLabel("60", this);
        m_speedLimitLabel->setStyleSheet(
            "QLabel{color:#F39C12;font-size:22px;font-weight:bold;"
            "font-family:'Consolas','Courier New',monospace;}");
        limitCol->addWidget(m_speedLimitLabel);
        speedRow->addLayout(limitCol);
        speedRow->addStretch();

        // Traffic light block
        QVBoxLayout *lightCol = new QVBoxLayout();
        lightCol->setSpacing(3);
        lightCol->setAlignment(Qt::AlignCenter);
        QLabel *lightTitle = new QLabel("LIGHT", this);
        lightTitle->setStyleSheet(
            "QLabel{color:rgba(160,190,230,160);font-size:9px;font-weight:bold;letter-spacing:2px;}");
        lightTitle->setAlignment(Qt::AlignCenter);
        lightCol->addWidget(lightTitle);
        m_trafficDot = new QLabel(this);
        m_trafficDot->setFixedSize(22, 22);
        m_trafficDot->setStyleSheet(
            "QLabel{background:#2ECC71;border-radius:11px;border:2px solid rgba(255,255,255,80);}");
        lightCol->addWidget(m_trafficDot, 0, Qt::AlignCenter);
        m_trafficLightLabel = new QLabel("GREEN", this);
        m_trafficLightLabel->setStyleSheet(
            "QLabel{color:#2ECC71;font-size:9px;font-weight:bold;}");
        m_trafficLightLabel->setAlignment(Qt::AlignCenter);
        lightCol->addWidget(m_trafficLightLabel);
        speedRow->addLayout(lightCol);

        root->addLayout(speedRow);
        root->addSpacing(8);
    }

    root->addWidget(makeDivider());
    root->addSpacing(8);

    // ---- LAP / LAP TIME ----
    root->addWidget(makeInfoRow("LAP",      m_lapLabel,      "1 / 3"));
    root->addSpacing(4);
    root->addWidget(makeInfoRow("LAP TIME", m_lapTimeLabel,  "0:00.000"));
    root->addSpacing(6);
    root->addWidget(makeDivider());
    root->addSpacing(6);

    // ---- POWERUP ----
    root->addWidget(makeInfoRow("ITEM",     m_powerupLabel,  "None"));
    root->addSpacing(6);
    root->addWidget(makeDivider());
    root->addSpacing(6);

    // ---- STOP label (absolute overlay, shown on red) ----
    m_stopLabel = new QLabel("STOP", this);
    m_stopLabel->setStyleSheet(
        "QLabel{color:#E74C3C;font-size:11px;font-weight:bold;"
        "background:rgba(231,76,60,40);border:1px solid #E74C3C;"
        "border-radius:4px;padding:1px 6px;}");
    m_stopLabel->setVisible(false);
    m_stopLabel->setGeometry(220, 10, 50, 20);
    m_stopLabel->raise();

    root->addStretch();
}
void LearningHUD::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
}

// ==================== Data update ====================

void LearningHUD::updateCurrentSpeed(qreal speed)
{
    m_currentSpeed = speed;
    if (m_speedLabel) m_speedLabel->setText(QString::number(static_cast<int>(speed)));
    updateSpeedColor();

    if (m_speedLimit > 0 && speed > m_speedLimit && !m_isOverLimit) {
        m_isOverLimit = true;
        showViolationWarning("SPEEDING");
        emit violationTriggered("SpeedOverLimit");
    } else if (speed <= m_speedLimit) {
        m_isOverLimit = false;
    }
}

void LearningHUD::updateSpeedLimit(int limit)
{
    m_speedLimit = limit;
    if (limit > 0) {
        if (m_speedLimitLabel)
            m_speedLimitLabel->setText(QString::number(limit));
        QString color = limit <= 30 ? "#E74C3C" : limit <= 50 ? "#F39C12" : "#F1C40F";
        if (m_speedLimitLabel)
            m_speedLimitLabel->setStyleSheet(
                QString("QLabel{color:%1;font-size:22px;font-weight:bold;"
                        "font-family:'Consolas','Courier New',monospace;}").arg(color));
        emit speedLimitChanged(limit);
    } else {
        if (m_speedLimitLabel) {
            m_speedLimitLabel->setText("--");
            m_speedLimitLabel->setStyleSheet(
                "QLabel{color:#95A5A6;font-size:22px;font-weight:bold;"
                "font-family:'Consolas','Courier New',monospace;}");
        }
    }
}

void LearningHUD::updateSpeedStatus(bool isOverLimit)
{
    m_isOverLimit = isOverLimit;
    updateSpeedColor();
}

void LearningHUD::updateTrafficLight(const QString& state, int remainingSeconds)
{
    m_trafficState = state.toLower();
    const bool red    = (m_trafficState == "red");
    const bool yellow = (m_trafficState == "yellow" || m_trafficState == "amber");

    if (m_trafficDot) {
        QString dotColor = red ? "#E74C3C" : yellow ? "#F39C12" : "#2ECC71";
        m_trafficDot->setStyleSheet(
            QString("QLabel{background:%1;border-radius:11px;"
                    "border:2px solid rgba(255,255,255,80);}").arg(dotColor));
    }
    if (m_stopLabel) m_stopLabel->setVisible(red);

    QString stateText;
    QString stateColor;
    if (red) {
        stateText  = remainingSeconds > 0 ? QString("RED %1s").arg(remainingSeconds) : "RED";
        stateColor = "#E74C3C";
        m_blinkOn = true;
        m_blinkTimer->start(500);
    } else if (yellow) {
        stateText  = remainingSeconds > 0 ? QString("YLW %1s").arg(remainingSeconds) : "YELLOW";
        stateColor = "#F39C12";
        m_blinkTimer->stop();
        m_blinkOn = false;
    } else {
        stateText  = "GREEN";
        stateColor = "#2ECC71";
        m_blinkTimer->stop();
        m_blinkOn = false;
    }

    if (m_trafficLightLabel) {
        m_trafficLightLabel->setText(stateText);
        m_trafficLightLabel->setStyleSheet(
            QString("QLabel{color:%1;font-size:9px;font-weight:bold;}").arg(stateColor));
    }
    updateSpeedColor();
}


void LearningHUD::onRedLightBlink()
{
    m_blinkOn = !m_blinkOn;
    updateSpeedColor();
}

void LearningHUD::showPenaltyMessage(const QString& message, int points)
{
    emit penaltyMessageRequested(message, points);
}

void LearningHUD::showViolationWarning(const QString& violationType)
{
    emit violationWarningRequested(violationType);
}

void LearningHUD::updatePowerupState(const QString& powerupId, const QString& name, bool active)
{
    if (active) {
        if (m_powerupLabel) {
            m_powerupLabel->setText(name);
            m_powerupLabel->setStyleSheet(
                "QLabel{color:#9B59B6;font-size:13px;font-weight:bold;}");
        }
    } else {
        bool hasActive = false;
        for (auto it = m_powerupIcons.constBegin(); it != m_powerupIcons.constEnd(); ++it) {
            if (it.key() != powerupId && it.value() && it.value()->isVisible()) {
                hasActive = true;
                break;
            }
        }
        if (!hasActive && m_powerupLabel) {
            m_powerupLabel->setText("None");
            m_powerupLabel->setStyleSheet("QLabel{color:#BDC3C7;font-size:13px;}");
        }
    }
}

void LearningHUD::updateGameMode(const QString& mode)
{
    m_currentMode = mode;
    if (m_modeLabel) {
        if (mode.toLower().contains("arcade")) {
            m_modeLabel->setText("ARCADE MODE");
            m_modeLabel->setStyleSheet(
                "QLabel{color:#E74C3C;font-size:12px;font-weight:900;letter-spacing:3px;}");
        } else {
            m_modeLabel->setText("LEARNING MODE");
            m_modeLabel->setStyleSheet(
                "QLabel{color:#3498DB;font-size:12px;font-weight:900;letter-spacing:3px;}");
        }
    }
}

void LearningHUD::updateLapInfo(int currentLap, int totalLaps)
{
    if (m_lapLabel)
        m_lapLabel->setText(QString("%1 / %2").arg(currentLap).arg(totalLaps));
}

void LearningHUD::updateLapTime(const QString& time)
{
    if (m_lapTimeLabel) m_lapTimeLabel->setText(time);
}

void LearningHUD::updateSpeedColor()
{
    const bool redBlink = (m_trafficState == "red") && m_blinkOn;
    QString color;
    if (redBlink) {
        color = "#E74C3C";
    } else if (m_speedLimit > 0 && m_currentSpeed > m_speedLimit) {
        color = "#E74C3C";
    } else if (m_speedLimit > 0 && m_currentSpeed > m_speedLimit * 0.9) {
        color = "#F39C12";
    } else if (m_currentSpeed > 0) {
        color = "#00E5FF";
    } else {
        color = "#95A5A6";
    }
    if (m_speedLabel)
        m_speedLabel->setStyleSheet(
            QString("QLabel{color:%1;font-size:38px;font-weight:bold;"
                    "font-family:'Consolas','Courier New',monospace;}").arg(color));
}

void LearningHUD::clearAllWarnings()
{
    for (QLabel *label : m_floatingMessages) label->deleteLater();
    m_floatingMessages.clear();
}

void LearningHUD::setVisible(bool visible)
{
    QWidget::setVisible(visible);
}

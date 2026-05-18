#include "UI/learninghud.h"

#include <QApplication>
#include <QDebug>
#include <QScreen>
#include <QTimer>
#include <QPainter>
#include <QFont>
#include <QPainterPath>

LearningHUD::LearningHUD(QWidget *parent)
    : QWidget(parent)
    , m_currentSpeed(0.0)
    , m_speedLimit(60)
    , m_isOverLimit(false)
{
    setupUI();
    setFloatingStyle();
}

LearningHUD::~LearningHUD()
{
    qDeleteAll(m_floatingMessages);
}

void LearningHUD::setupUI()
{
    setFixedSize(280, 320);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // ========== 速度面板 ==========
    QWidget *speedPanel = new QWidget(this);
    speedPanel->setObjectName("speedPanel");
    speedPanel->setStyleSheet(R"(
        #speedPanel {
            background-color: rgba(20, 25, 35, 220);
            border-radius: 12px;
            border: 1px solid rgba(255, 255, 255, 30);
        }
    )");

    QVBoxLayout *speedLayout = new QVBoxLayout(speedPanel);
    speedLayout->setSpacing(5);
    speedLayout->setContentsMargins(15, 10, 15, 10);

    // 模式标签
    m_modeLabel = new QLabel("Learning Mode", speedPanel);
    m_modeLabel->setStyleSheet(
        "QLabel { color: #3498DB; font-size: 11px; font-weight: bold; }"
    );
    m_modeLabel->setAlignment(Qt::AlignCenter);
    speedLayout->addWidget(m_modeLabel);

    // 速度数字（主显示）
    m_speedLabel = new QLabel("0", speedPanel);
    m_speedLabel->setAlignment(Qt::AlignCenter);
    m_speedLabel->setStyleSheet(R"(
        QLabel {
            color: #2ECC71;
            font-size: 56px;
            font-weight: bold;
            font-family: 'Segoe UI', 'Arial', sans-serif;
        }
    )");
    speedLayout->addWidget(m_speedLabel);

    // 速度和限速横向布局
    QHBoxLayout *speedInfoLayout = new QHBoxLayout();

    // 当前速度单位
    m_speedUnitLabel = new QLabel("km/h", speedPanel);
    m_speedUnitLabel->setStyleSheet(
        "QLabel { color: rgba(255,255,255,150); font-size: 14px; }"
    );
    speedInfoLayout->addWidget(m_speedUnitLabel);
    speedInfoLayout->addStretch();

    // 限速标签
    m_speedLimitLabel = new QLabel("Limit: -- km/h", speedPanel);
    m_speedLimitLabel->setStyleSheet(
        "QLabel { color: #F39C12; font-size: 14px; font-weight: bold; }"
    );
    speedInfoLayout->addWidget(m_speedLimitLabel);

    speedLayout->addLayout(speedInfoLayout);
    mainLayout->addWidget(speedPanel);

    // ========== 交通信号面板 ==========
    QWidget *trafficPanel = new QWidget(this);
    trafficPanel->setObjectName("trafficPanel");
    trafficPanel->setStyleSheet(R"(
        #trafficPanel {
            background-color: rgba(20, 25, 35, 220);
            border-radius: 8px;
            border: 1px solid rgba(255, 255, 255, 30);
        }
    )");

    QHBoxLayout *trafficLayout = new QHBoxLayout(trafficPanel);
    trafficLayout->setContentsMargins(10, 8, 10, 8);

    QLabel *trafficTitle = new QLabel("信号:", trafficPanel);
    trafficTitle->setStyleSheet("QLabel { color: rgba(255,255,255,150); font-size: 12px; }");

    m_trafficLightLabel = new QLabel("等待中...", trafficPanel);
    m_trafficLightLabel->setStyleSheet(
        "QLabel { color: #95A5A6; font-size: 14px; font-weight: bold; }"
    );

    trafficLayout->addWidget(trafficTitle);
    trafficLayout->addWidget(m_trafficLightLabel);
    trafficLayout->addStretch();

    mainLayout->addWidget(trafficPanel);

    // ========== 圈数面板 ==========
    QWidget *lapPanel = new QWidget(this);
    lapPanel->setObjectName("lapPanel");
    lapPanel->setStyleSheet(R"(
        #lapPanel {
            background-color: rgba(20, 25, 35, 220);
            border-radius: 8px;
            border: 1px solid rgba(255, 255, 255, 30);
        }
    )");

    QHBoxLayout *lapLayout = new QHBoxLayout(lapPanel);
    lapLayout->setContentsMargins(10, 8, 10, 8);

    QLabel *lapTitle = new QLabel("圈数:", lapPanel);
    lapTitle->setStyleSheet("QLabel { color: rgba(255,255,255,150); font-size: 12px; }");

    m_lapLabel = new QLabel("1/3", lapPanel);
    m_lapLabel->setStyleSheet(
        "QLabel { color: #ECF0F1; font-size: 14px; font-weight: bold; }"
    );

    lapLayout->addWidget(lapTitle);
    lapLayout->addWidget(m_lapLabel);
    lapLayout->addStretch();

    mainLayout->addWidget(lapPanel);

    // ========== 道具状态面板 ==========
    QWidget *powerupPanel = new QWidget(this);
    powerupPanel->setObjectName("powerupPanel");
    powerupPanel->setStyleSheet(R"(
        #powerupPanel {
            background-color: rgba(20, 25, 35, 220);
            border-radius: 8px;
            border: 1px solid rgba(255, 255, 255, 30);
        }
    )");

    QHBoxLayout *powerupLayout = new QHBoxLayout(powerupPanel);
    powerupLayout->setContentsMargins(10, 8, 10, 8);

    QLabel *powerupTitle = new QLabel("道具:", powerupPanel);
    powerupTitle->setStyleSheet("QLabel { color: rgba(255,255,255,150); font-size: 12px; }");

    m_powerupLabel = new QLabel("无", powerupPanel);
    m_powerupLabel->setStyleSheet(
        "QLabel { color: #BDC3C7; font-size: 14px; }"
    );

    powerupLayout->addWidget(powerupTitle);
    powerupLayout->addWidget(m_powerupLabel);
    powerupLayout->addStretch();

    mainLayout->addWidget(powerupPanel);

    mainLayout->addStretch();
}

void LearningHUD::setFloatingStyle()
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
}

void LearningHUD::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    // 移动到右上角
    const QRect screenGeometry = QApplication::primaryScreen()->geometry();
    move(screenGeometry.width() - width() - 20, 20);
}

// ==================== 数据更新实现 ====================

void LearningHUD::updateCurrentSpeed(qreal speed)
{
    m_currentSpeed = speed;
    m_speedLabel->setText(QString::number(static_cast<int>(speed)));

    updateSpeedColor();

    // 发送超速警告
    if (m_speedLimit > 0 && speed > m_speedLimit && !m_isOverLimit) {
        m_isOverLimit = true;
        showViolationWarning("超速!");
        emit violationTriggered("SpeedOverLimit");
    } else if (speed <= m_speedLimit) {
        m_isOverLimit = false;
    }
}

void LearningHUD::updateSpeedLimit(int limit)
{
    m_speedLimit = limit;
    if (limit > 0) {
        m_speedLimitLabel->setText(QString("限速: %1 km/h").arg(limit));

        // 根据限速设置颜色
        QString color;
        if (limit <= 30) {
            color = "#E74C3C";  // 红色 - 学校区域等
        } else if (limit <= 50) {
            color = "#F39C12";  // 橙色 - 一般道路
        } else {
            color = "#F1C40F";  // 黄色 - 高速公路
        }
        m_speedLimitLabel->setStyleSheet(QString(
            "QLabel { color: %1; font-size: 14px; font-weight: bold; }").arg(color));

        emit speedLimitChanged(limit);
    } else {
        m_speedLimitLabel->setText("限速: --");
        m_speedLimitLabel->setStyleSheet(
            "QLabel { color: #95A5A6; font-size: 14px; }"
        );
    }
}

void LearningHUD::updateSpeedStatus(bool isOverLimit)
{
    m_isOverLimit = isOverLimit;
    updateSpeedColor();
}

void LearningHUD::updateTrafficLight(const QString& state, int remainingSeconds)
{
    QString displayText;
    QString colorStyle;

    if (state.toLower() == "red") {
        displayText = QString("红灯 %1s").arg(remainingSeconds);
        colorStyle = "QLabel { color: #E74C3C; font-size: 14px; font-weight: bold; background-color: rgba(231, 76, 60, 50); padding: 5px 10px; border-radius: 4px; }";
    } else if (state.toLower() == "yellow" || state.toLower() == "amber") {
        displayText = QString("黄灯 %1s").arg(remainingSeconds);
        colorStyle = "QLabel { color: #F39C12; font-size: 14px; font-weight: bold; background-color: rgba(243, 156, 18, 50); padding: 5px 10px; border-radius: 4px; }";
    } else if (state.toLower() == "green") {
        displayText = "绿灯 - 可通行";
        colorStyle = "QLabel { color: #2ECC71; font-size: 14px; font-weight: bold; background-color: rgba(46, 204, 113, 50); padding: 5px 10px; border-radius: 4px; }";
    } else {
        displayText = "无信号";
        colorStyle = "QLabel { color: #95A5A6; font-size: 14px; }";
    }

    m_trafficLightLabel->setText(displayText);
    m_trafficLightLabel->setStyleSheet(colorStyle);

    qDebug() << "HUD traffic light updated:" << displayText;
}

void LearningHUD::showPenaltyMessage(const QString& message, int points)
{
    // 创建飘字标签
    QLabel *msgLabel = new QLabel(QString("⚠ %1 -%2分").arg(message).arg(points), this);
    msgLabel->setStyleSheet(
        "QLabel { color: #E74C3C; font-size: 14px; font-weight: bold; "
        "background-color: rgba(0,0,0,200); padding: 8px 12px; border-radius: 8px; "
        "border: 1px solid #E74C3C; }"
    );
    msgLabel->setAlignment(Qt::AlignCenter);
    msgLabel->resize(250, 35);

    // 位置在速度标签下方
    const int x = (width() - msgLabel->width()) / 2;
    const int y = 130;
    msgLabel->move(x, y);
    msgLabel->show();

    m_floatingMessages.append(msgLabel);

    // 向上飘动动画
    QPropertyAnimation *moveAnim = new QPropertyAnimation(msgLabel, "pos");
    moveAnim->setDuration(2500);
    moveAnim->setStartValue(QPoint(x, y));
    moveAnim->setEndValue(QPoint(x, y - 100));
    moveAnim->setEasingCurve(QEasingCurve::OutQuad);

    // 淡出动画
    QPropertyAnimation *opacityAnim = new QPropertyAnimation(msgLabel, "windowOpacity");
    opacityAnim->setDuration(2500);
    opacityAnim->setStartValue(1.0);
    opacityAnim->setEndValue(0.0);

    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(moveAnim);
    group->addAnimation(opacityAnim);

    connect(group, &QParallelAnimationGroup::finished, this, [this, msgLabel]() {
        m_floatingMessages.removeOne(msgLabel);
        msgLabel->deleteLater();
    });

    group->start(QAbstractAnimation::DeleteWhenStopped);

    qDebug() << "HUD penalty:" << message << points;
}

void LearningHUD::showViolationWarning(const QString& violationType)
{
    // 创建警告标签
    QLabel *warnLabel = new QLabel(QString("⚠ %1").arg(violationType), this);
    warnLabel->setStyleSheet(
        "QLabel { color: #FFFFFF; font-size: 16px; font-weight: bold; "
        "background-color: rgba(231, 76, 60, 220); padding: 10px 15px; border-radius: 8px; }"
    );
    warnLabel->setAlignment(Qt::AlignCenter);
    warnLabel->resize(200, 40);
    warnLabel->move((width() - warnLabel->width()) / 2, 175);
    warnLabel->show();

    m_floatingMessages.append(warnLabel);

    // 闪烁动画
    QPropertyAnimation *blinkAnim = new QPropertyAnimation(warnLabel, "windowOpacity");
    blinkAnim->setDuration(500);
    blinkAnim->setStartValue(1.0);
    blinkAnim->setEndValue(0.3);
    blinkAnim->setLoopCount(4);

    // 淡出并删除
    QTimer::singleShot(3000, this, [this, warnLabel]() {
        if (m_floatingMessages.contains(warnLabel)) {
            m_floatingMessages.removeOne(warnLabel);
            warnLabel->deleteLater();
        }
    });

    blinkAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void LearningHUD::updatePowerupState(const QString& powerupId, const QString& name, bool active)
{
    if (active) {
        m_powerupLabel->setText(name);
        m_powerupLabel->setStyleSheet(
            "QLabel { color: #9B59B6; font-size: 14px; font-weight: bold; }"
        );
    } else {
        // 如果没有激活的道具，显示"无"
        bool hasActive = false;
        for (auto it = m_powerupIcons.constBegin(); it != m_powerupIcons.constEnd(); ++it) {
            if (it.key() != powerupId && it.value()->isVisible()) {
                hasActive = true;
                break;
            }
        }
        if (!hasActive) {
            m_powerupLabel->setText("无");
            m_powerupLabel->setStyleSheet(
                "QLabel { color: #BDC3C7; font-size: 14px; }"
            );
        }
    }
}

void LearningHUD::updateGameMode(const QString& mode)
{
    m_currentMode = mode;
    if (mode.toLower().contains("arcade")) {
        m_modeLabel->setText("Arcade Mode");
        m_modeLabel->setStyleSheet(
            "QLabel { color: #E74C3C; font-size: 11px; font-weight: bold; }"
        );
    } else {
        m_modeLabel->setText("Learning Mode");
        m_modeLabel->setStyleSheet(
            "QLabel { color: #3498DB; font-size: 11px; font-weight: bold; }"
        );
    }
}

void LearningHUD::updateLapInfo(int currentLap, int totalLaps)
{
    m_lapLabel->setText(QString("%1/%2").arg(currentLap).arg(totalLaps));
}

void LearningHUD::updateLapTime(const QString& time)
{
    // 可以添加时间显示，暂用圈数显示
    Q_UNUSED(time);
}

void LearningHUD::updateSpeedColor()
{
    QString color;
    if (m_speedLimit > 0 && m_currentSpeed > m_speedLimit) {
        color = "#E74C3C";  // 超速 - 红色
    } else if (m_speedLimit > 0 && m_currentSpeed > m_speedLimit * 0.9) {
        color = "#F39C12";  // 接近限速 - 橙色
    } else if (m_currentSpeed > 0) {
        color = "#2ECC71";  // 正常 - 绿色
    } else {
        color = "#95A5A6";  // 静止 - 灰色
    }

    m_speedLabel->setStyleSheet(QString(
        "QLabel { color: %1; font-size: 56px; font-weight: bold; "
        "font-family: 'Segoe UI', 'Arial', sans-serif; }").arg(color));
}

void LearningHUD::clearAllWarnings()
{
    for (QLabel *label : m_floatingMessages) {
        label->deleteLater();
    }
    m_floatingMessages.clear();
}

void LearningHUD::setVisible(bool visible)
{
    QWidget::setVisible(visible);
}

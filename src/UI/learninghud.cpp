#include "UI/learninghud.h"
#include <QTimer>
#include <QApplication>
#include <QScreen>

LearningHUD::LearningHUD(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    setFloatingStyle();
}

void LearningHUD::setupUI()
{
    // 设置窗口大小和位置（右上角）
    setFixedSize(250, 100);

    // 创建布局
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(8);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);

    // 创建限速标签
    m_speedLimitLabel = new QLabel("限速: -- km/h", this);
    m_speedLimitLabel->setStyleSheet(
        "QLabel { color: white; font-size: 16px; font-weight: bold; background-color: rgba(0,0,0,150); padding: 5px; border-radius: 5px; }"
        );
    m_speedLimitLabel->setAlignment(Qt::AlignCenter);

    // 创建红绿灯标签
    m_trafficLightLabel = new QLabel("🚦 等待信号", this);
    m_trafficLightLabel->setStyleSheet(
        "QLabel { color: white; font-size: 14px; background-color: rgba(0,0,0,150); padding: 5px; border-radius: 5px; }"
        );
    m_trafficLightLabel->setAlignment(Qt::AlignCenter);

    // 添加到布局
    m_mainLayout->addWidget(m_speedLimitLabel);
    m_mainLayout->addWidget(m_trafficLightLabel);
}

void LearningHUD::setFloatingStyle()
{
    // 设置窗口为悬浮、无边框、半透明
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // 设置整体样式
    setStyleSheet(
        "QWidget { background-color: transparent; }"
        );
}

void LearningHUD::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    // 移动到屏幕右上角（距离边缘20像素）
    QRect screenGeometry = QApplication::primaryScreen()->geometry();
    int x = screenGeometry.width() - width() - 20;
    int y = 20;
    move(x, y);
}

void LearningHUD::updateSpeedLimit(int limit)
{
    m_speedLimitLabel->setText(QString("限速: %1 km/h").arg(limit));

    // 如果限速值较高，可以改变颜色预警
    if (limit > 60) {
        m_speedLimitLabel->setStyleSheet(
            "QLabel { color: yellow; font-size: 16px; font-weight: bold; background-color: rgba(0,0,0,150); padding: 5px; border-radius: 5px; }"
            );
    } else {
        m_speedLimitLabel->setStyleSheet(
            "QLabel { color: white; font-size: 16px; font-weight: bold; background-color: rgba(0,0,0,150); padding: 5px; border-radius: 5px; }"
            );
    }

    qDebug() << "HUD: 限速更新为" << limit;
}

void LearningHUD::updateTrafficLight(const QString& state, int remainingSeconds)
{
    QString displayText;

    if (state == "red") {
        displayText = QString("🔴 红灯 %1 秒").arg(remainingSeconds);
    } else if (state == "yellow") {
        displayText = QString("🟡 黄灯 %1 秒").arg(remainingSeconds);
    } else if (state == "green") {
        displayText = QString("🟢 绿灯 %1 秒").arg(remainingSeconds);
    } else {
        displayText = "🚦 无信号";
    }

    m_trafficLightLabel->setText(displayText);
    qDebug() << "HUD: 红绿灯更新 -" << displayText;
}

void LearningHUD::showPenaltyMessage(const QString& message, int points)
{
    // 创建飘字标签
    QLabel *msgLabel = new QLabel(QString("%1 -%2分").arg(message).arg(points), this);
    msgLabel->setStyleSheet(
        "QLabel { color: red; font-size: 14px; font-weight: bold; background-color: rgba(0,0,0,180); padding: 8px; border-radius: 8px; }"
        );
    msgLabel->setAlignment(Qt::AlignCenter);

    // 设置初始位置（在 HUD 下方）
    msgLabel->setParent(this);
    msgLabel->resize(200, 40);

    // 计算位置：HUD 正下方
    int x = (width() - msgLabel->width()) / 2;
    int y = height();
    msgLabel->move(x, y);
    msgLabel->show();

    // 添加到列表
    m_floatingMessages.append(msgLabel);

    // 创建动画：向上飘移 + 淡出
    QPropertyAnimation *moveAnim = new QPropertyAnimation(msgLabel, "pos");
    moveAnim->setDuration(2000);
    moveAnim->setStartValue(QPoint(x, y));
    moveAnim->setEndValue(QPoint(x, y - 80));

    QPropertyAnimation *opacityAnim = new QPropertyAnimation(msgLabel, "windowOpacity");
    opacityAnim->setDuration(2000);
    opacityAnim->setStartValue(1.0);
    opacityAnim->setEndValue(0.0);

    // 动画结束后删除标签
    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(moveAnim);
    group->addAnimation(opacityAnim);

    connect(group, &QParallelAnimationGroup::finished, [this, msgLabel]() {
        m_floatingMessages.removeOne(msgLabel);
        msgLabel->deleteLater();
    });

    group->start();

    qDebug() << "HUD: 显示扣分提示 -" << message << points << "分";
}
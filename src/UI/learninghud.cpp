#include "UI/learninghud.h"

#include <QApplication>
#include <QDebug>
#include <QScreen>
#include <QTimer>

LearningHUD::LearningHUD(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    setFloatingStyle();
}

void LearningHUD::setupUI()
{
    setFixedSize(250, 100);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(8);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);

    m_speedLimitLabel = new QLabel("Limit: -- km/h", this);
    m_speedLimitLabel->setStyleSheet(
        "QLabel { color: white; font-size: 16px; font-weight: bold; "
        "background-color: rgba(0,0,0,150); padding: 5px; border-radius: 5px; }");
    m_speedLimitLabel->setAlignment(Qt::AlignCenter);

    m_trafficLightLabel = new QLabel("Light: waiting", this);
    m_trafficLightLabel->setStyleSheet(
        "QLabel { color: white; font-size: 14px; "
        "background-color: rgba(0,0,0,150); padding: 5px; border-radius: 5px; }");
    m_trafficLightLabel->setAlignment(Qt::AlignCenter);

    m_mainLayout->addWidget(m_speedLimitLabel);
    m_mainLayout->addWidget(m_trafficLightLabel);
}

void LearningHUD::setFloatingStyle()
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("QWidget { background-color: transparent; }");
}

void LearningHUD::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    const QRect screenGeometry = QApplication::primaryScreen()->geometry();
    move(screenGeometry.width() - width() - 20, 20);
}

void LearningHUD::updateSpeedLimit(int limit)
{
    m_speedLimitLabel->setText(QString("Limit: %1 km/h").arg(limit));

    const QString color = limit > 60 ? "yellow" : "white";
    m_speedLimitLabel->setStyleSheet(QString(
        "QLabel { color: %1; font-size: 16px; font-weight: bold; "
        "background-color: rgba(0,0,0,150); padding: 5px; border-radius: 5px; }").arg(color));

    qDebug() << "HUD speed limit updated:" << limit;
}

void LearningHUD::updateTrafficLight(const QString& state, int remainingSeconds)
{
    QString displayText;
    if (state == "red") {
        displayText = QString("Light: red %1s").arg(remainingSeconds);
    } else if (state == "yellow") {
        displayText = QString("Light: yellow %1s").arg(remainingSeconds);
    } else if (state == "green") {
        displayText = QString("Light: green %1s").arg(remainingSeconds);
    } else {
        displayText = "Light: none";
    }

    m_trafficLightLabel->setText(displayText);
    qDebug() << "HUD traffic light updated:" << displayText;
}

void LearningHUD::showPenaltyMessage(const QString& message, int points)
{
    QLabel *msgLabel = new QLabel(QString("%1 -%2").arg(message).arg(points), this);
    msgLabel->setStyleSheet(
        "QLabel { color: red; font-size: 14px; font-weight: bold; "
        "background-color: rgba(0,0,0,180); padding: 8px; border-radius: 8px; }");
    msgLabel->setAlignment(Qt::AlignCenter);
    msgLabel->resize(220, 40);

    const int x = (width() - msgLabel->width()) / 2;
    const int y = height();
    msgLabel->move(x, y);
    msgLabel->show();

    m_floatingMessages.append(msgLabel);

    QPropertyAnimation *moveAnim = new QPropertyAnimation(msgLabel, "pos");
    moveAnim->setDuration(2000);
    moveAnim->setStartValue(QPoint(x, y));
    moveAnim->setEndValue(QPoint(x, y - 80));

    QPropertyAnimation *opacityAnim = new QPropertyAnimation(msgLabel, "windowOpacity");
    opacityAnim->setDuration(2000);
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

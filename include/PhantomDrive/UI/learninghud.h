#ifndef LEARNINGHUD_H
#define LEARNINGHUD_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

class LearningHUD : public QWidget
{
    Q_OBJECT
public:
    explicit LearningHUD(QWidget *parent = nullptr);

    // 供外部调用的接口（暂时自己模拟数据，等引擎组信号来了再改）
    void updateSpeedLimit(int limit);           // 更新限速
    void updateTrafficLight(const QString& state, int remainingSeconds);  // 更新红绿灯
    void showPenaltyMessage(const QString& message, int points);  // 显示扣分飘字

protected:
    void showEvent(QShowEvent *event) override;  // 窗口显示时的设置

private:
    void setupUI();           // 初始化界面
    void setFloatingStyle();  // 设置悬浮样式

    QLabel *m_speedLimitLabel;      // 限速显示
    QLabel *m_trafficLightLabel;    // 红绿灯显示
    QVBoxLayout *m_mainLayout;      // 主布局
    QList<QLabel*> m_floatingMessages;  // 正在飘动的消息列表
};

#endif // LEARNINGHUD_H
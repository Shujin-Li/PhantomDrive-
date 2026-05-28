#ifndef LEARNINGHUD_H
#define LEARNINGHUD_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QMap>

#include "PhantomDrive_global.h"

class PHANTOMDRIVE_EXPORT LearningHUD : public QWidget
{
    Q_OBJECT
public slots:
    // ==================== 真实数据更新接口 ====================
    // 速度相关
    void updateCurrentSpeed(qreal speed);                      // 更新当前速度
    void updateSpeedLimit(int limit);                         // 更新限速
    void updateSpeedStatus(bool isOverLimit);                 // 更新是否超速

    // 交通信号
    void updateTrafficLight(const QString& state, int remainingSeconds = 0);  // 更新红绿灯

    // 违规提示
    void showPenaltyMessage(const QString& message, int points);  // 显示扣分飘字
    void showViolationWarning(const QString& violationType);      // 显示违规警告

    // 道具状态
    void updatePowerupState(const QString& powerupId, const QString& name, bool active);

    // 当前模式
    void updateGameMode(const QString& mode);                  // Arcade / Learning

    // 比赛信息
    void updateLapInfo(int currentLap, int totalLaps);         // 圈数信息
    void updateLapTime(const QString& time);                    // 当前圈时间

    // ==================== 控制接口 ====================
    void setVisible(bool visible) override;
    void clearAllWarnings();

signals:
    void speedLimitChanged(int limit);
    void violationTriggered(const QString& type);
    void powerupActivated(const QString& powerupId, const QString& name);
    void powerupDeactivated(const QString& powerupId);

public:
    explicit LearningHUD(QWidget *parent = nullptr);
    ~LearningHUD();

private:
    void setupUI();                    // 初始化界面
    void setFloatingStyle();           // 设置悬浮样式
    void updateSpeedColor();            // 根据速度更新颜色
    void showEvent(QShowEvent *event) override;  // 悬浮样式

    // UI 元素
    QLabel *m_speedLabel;               // 当前速度显示
    QLabel *m_speedLimitLabel;          // 限速显示
    QLabel *m_speedUnitLabel;           // 速度单位
    QLabel *m_trafficLightLabel;        // 红绿灯显示
    QLabel *m_modeLabel;                // 当前模式
    QLabel *m_lapLabel;                 // 圈数显示
    QLabel *m_powerupLabel;             // 道具状态

    // 速度指示器（用于超速警告）
    QWidget *m_speedIndicator;

    // 飘动消息列表
    QList<QLabel*> m_floatingMessages;

    // 道具图标映射
    QMap<QString, QLabel*> m_powerupIcons;

    // 当前状态缓存
    qreal m_currentSpeed;
    int m_speedLimit;
    bool m_isOverLimit;
    QString m_currentMode;
};

#endif // LEARNINGHUD_H

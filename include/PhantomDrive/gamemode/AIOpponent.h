#pragma once

#include "PhantomDrive_global.h"

#include <QObject>
#include <QString>
#include <QVector2D>
#include <QList>
#include <QMap>
#include <QVariantMap>
#include <QJsonObject>
#include <QJsonArray>

namespace PhantomDrive {

/**
 * @brief AI 状态枚举
 */
enum class AIState {
    Idle = 0,
    Racing,
    Overtaking,
    Defending,
    Recovering,
    Finished,
    Count
};

/**
 * @brief AI 驾驶风格枚举
 */
enum class AIStyle {
    Unknown = -1,
    Conservative = 0,  // 保守型 - 注重稳定
    Normal = 1,         // 正常型 - 平衡速度与安全
    Aggressive = 2,    // 激进型 - 追求最快圈速
    Defensive = 3,     // 防守型 - 保持位置
    Count
};

/**
 * @brief 路径点结构
 */
struct Waypoint {
    QVector2D position;
    qreal preferredSpeed;
    bool isCorner;
    int cornerSeverity;  // 0-3, 弯道激烈程度
    int index;

    Waypoint()
        : preferredSpeed(0.0)
        , isCorner(false)
        , cornerSeverity(0)
        , index(-1)
    {}

    Waypoint(const QVector2D& pos, qreal speed = 0.0, bool corner = false, int severity = 0, int idx = -1)
        : position(pos)
        , preferredSpeed(speed)
        , isCorner(corner)
        , cornerSeverity(severity)
        , index(idx)
    {}
};

/**
 * @brief AI 决策结果
 */
struct AIDecision {
    qreal throttle;         // 油门 [0.0, 1.0]
    qreal brake;            // 刹车 [0.0, 1.0]
    qreal steering;          // 转向 [-1.0, 1.0]
    bool usePowerup;         // 是否使用道具
    int powerupSlot;        // 道具槽位
    AIState suggestedState;  // 建议的状态

    AIDecision()
        : throttle(0.0)
        , brake(0.0)
        , steering(0.0)
        , usePowerup(false)
        , powerupSlot(-1)
        , suggestedState(AIState::Racing)
    {}
};

/**
 * @brief AI 配置结构
 */
struct AIConfig {
    QString name;
    AIStyle style;
    qreal maxSpeed;
    qreal acceleration;
    qreal handling;
    qreal reactionTime;      // 反应时间(ms)
    qreal aggressionLevel;    // 侵略等级 [0.0, 1.0]
    qreal riskTolerance;     // 风险容忍度 [0.0, 1.0]
    int skillLevel;          // 技能等级 1-10

    AIConfig()
        : style(AIStyle::Normal)
        , maxSpeed(200.0)
        , acceleration(150.0)
        , handling(100.0)
        , reactionTime(100.0)
        , aggressionLevel(0.5)
        , riskTolerance(0.5)
        , skillLevel(5)
    {}
};

/**
 * @brief AI 对手基类接口
 *
 * 定义 AI 对手的抽象接口，包括：
 * - 路径点管理
 * - 状态更新
 * - 决策生成
 * - 与游戏对象交互
 */
class AIOpponent : public QObject
{
    Q_OBJECT

public:
    explicit AIOpponent(const QString& id, QObject* parent = nullptr);
    ~AIOpponent() override;

    // ==================== 标识与配置 ====================
    QString getId() const { return m_id; }
    void setId(const QString& id) { m_id = id; }

    virtual QString getName() const = 0;
    virtual AIStyle getStyle() const = 0;
    virtual AIConfig getConfig() const = 0;
    virtual void setConfig(const AIConfig& config) = 0;

    // ==================== 路径点系统 ====================
    virtual void setWaypoints(const QList<Waypoint>& waypoints) = 0;
    virtual QList<Waypoint> getWaypoints() const = 0;
    virtual void addWaypoint(const Waypoint& waypoint) = 0;
    virtual void clearWaypoints() = 0;

    virtual int getCurrentWaypointIndex() const = 0;
    virtual void setCurrentWaypointIndex(int index) = 0;
    virtual Waypoint getCurrentWaypoint() const = 0;
    virtual Waypoint getNextWaypoint() const = 0;
    virtual Waypoint getWaypointAt(int index) const = 0;

    virtual qreal getTotalPathLength() const = 0;
    virtual qreal getRemainingPathLength() const = 0;
    virtual qreal getProgressPercentage() const = 0;

    // ==================== 位置与运动 ====================
    virtual QVector2D getPosition() const = 0;
    virtual void setPosition(const QVector2D& position) = 0;

    virtual qreal getRotation() const = 0;
    virtual void setRotation(qreal rotation) = 0;

    virtual QVector2D getVelocity() const = 0;
    virtual void setVelocity(const QVector2D& velocity) = 0;

    virtual qreal getSpeed() const = 0;
    virtual qreal getMaxSpeed() const = 0;
    virtual void setMaxSpeed(qreal speed) = 0;

    virtual qreal getSteeringAngle() const = 0;
    virtual void setSteeringAngle(qreal angle) = 0;

    // ==================== 状态管理 ====================
    virtual AIState getState() const = 0;
    virtual void setState(AIState state) = 0;

    virtual qreal getStateDuration() const = 0;
    virtual void resetStateDuration() = 0;

    virtual bool isActive() const { return m_active; }
    virtual void setActive(bool active) { m_active = active; }

    virtual bool hasFinished() const { return m_finished; }
    virtual void setFinished(bool finished) { m_finished = finished; }

    // ==================== 游戏数据 ====================
    virtual int getCurrentLap() const = 0;
    virtual void setCurrentLap(int lap) = 0;

    virtual qreal getLapTime() const = 0;
    virtual void setLapTime(qreal time) = 0;

    virtual qreal getBestLapTime() const = 0;
    virtual void setBestLapTime(qreal time) = 0;

    virtual int getRacePosition() const = 0;  // 当前排名
    virtual void setRacePosition(int pos) = 0;

    virtual int getCheckpointsPassed() const = 0;
    virtual void setCheckpointsPassed(int count) = 0;

    // ==================== 决策与更新 ====================
    virtual AIDecision makeDecision() = 0;
    virtual void update(qint64 elapsedMs) = 0;

    virtual QVector2D calculateSteering() = 0;
    virtual qreal calculateThrottle() const = 0;
    virtual qreal calculateBraking() const = 0;

    // ==================== 道具系统 ====================
    virtual bool hasPowerup() const = 0;
    virtual int getPowerupCount() const = 0;
    virtual void addPowerup(int powerupType) = 0;
    virtual bool usePowerup(int slot) = 0;
    virtual void clearPowerups() = 0;

    // ==================== 碰撞与交互 ====================
    virtual void onCollision(const QString& objectId, const QVector2D& point) = 0;
    virtual void onNearMiss(const QString& opponentId, qreal distance) = 0;
    virtual void onOvertaken(const QString& opponentId) = 0;
    virtual void onOvertake(const QString& opponentId) = 0;

    // ==================== 环境感知 ====================
    virtual void setPlayerPosition(const QVector2D& position);
    virtual QVector2D getPlayerPosition() const { return m_playerPosition; }

    virtual void setOtherOpponentPositions(const QMap<QString, QVector2D>& positions);
    virtual QVector2D getNearestOpponentPosition() const;
    virtual qreal getDistanceToPlayer() const;
    virtual bool isPlayerAhead() const;

    // ==================== 辅助工具 ====================
    static QList<Waypoint> generateWaypointsFromTrack(const QString& trackDataPath);
    static qreal normalizeAngle(qreal angle);
    static qreal calculateLateralOffset(const QVector2D& from, const QVector2D& to, qreal heading);

    // ==================== 序列化 ====================
    virtual QJsonObject toJson() const = 0;
    virtual void fromJson(const QJsonObject& json) = 0;
    virtual QVariantMap getStateData() const = 0;
    virtual void loadStateData(const QVariantMap& data) = 0;

signals:
    void stateChanged(AIState oldState, AIState newState);
    void waypointReached(int index);
    void lapCompleted(int lapNumber, qreal lapTime);
    void bestLapTimeUpdated(qreal newBestTime);
    void positionChanged(int oldPosition, int newPosition);
    void collisionOccurred(const QString& objectId, const QVector2D& point);
    void powerupUsed(int slot, int powerupType);
    void powerupCollected(int powerupType);
    void overtakeAttempted(const QString& targetId);
    void overtakeCompleted(const QString& targetId);
    void raceFinished(int finalPosition, qreal totalTime);
    void decisionMade(const PhantomDrive::AIDecision& decision);
    void updated(qint64 elapsedMs);

protected:
    virtual void onStateEnter(AIState newState) = 0;
    virtual void onStateExit(AIState oldState) = 0;

    virtual qreal calculateDistanceToWaypoint(const Waypoint& waypoint) const = 0;
    virtual qreal calculateAngleToWaypoint(const Waypoint& waypoint) const = 0;
    virtual int findNearestWaypointIndex() const = 0;
    virtual int findNextRelevantWaypoint() const = 0;

    QString m_id;
    bool m_active;
    bool m_finished;

    QVector2D m_playerPosition;
    QMap<QString, QVector2D> m_otherOpponentPositions;
};

} // namespace PhantomDrive

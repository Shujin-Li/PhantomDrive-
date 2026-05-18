# AIOpponent 接口设计文档

## 1. 概述

`AIOpponent` 是 AI 对手的抽象基类接口，定义了所有 AI 对手必须实现的方法。该接口与项目现有的代码风格保持一致（基于 Qt 框架，使用 QObject 信号槽机制）。

## 2. 文件结构

```
include/PhantomDrive/gamemode/
├── AIOpponent.h           # AI对手基类接口
└── AIOpponentManager.h    # AI对手管理器

src/gamemode/
├── AIOpponent.cpp         # 基类实现
└── AIOpponentManager.cpp  # 管理器实现
```

## 3. 核心数据结构

### 3.1 Waypoint (路径点)

```cpp
struct Waypoint {
    QVector2D position;       // 位置
    qreal preferredSpeed;     // 偏好速度
    bool isCorner;            // 是否为弯道
    int cornerSeverity;       // 弯道激烈程度 (0-3)
    int index;                // 索引
};
```

### 3.2 AIDecision (AI决策)

```cpp
struct AIDecision {
    qreal throttle;           // 油门 [0.0, 1.0]
    qreal brake;              // 刹车 [0.0, 1.0]
    qreal steering;           // 转向 [-1.0, 1.0]
    bool usePowerup;          // 是否使用道具
    int powerupSlot;          // 道具槽位
    AIState suggestedState;   // 建议的状态
};
```

### 3.3 AIConfig (AI配置)

```cpp
struct AIConfig {
    QString name;             // 名称
    AIStyle style;           // 驾驶风格
    qreal maxSpeed;          // 最大速度
    qreal acceleration;      // 加速度
    qreal handling;          // 操控性
    qreal reactionTime;      // 反应时间(ms)
    qreal aggressionLevel;   // 侵略等级 [0.0, 1.0]
    qreal riskTolerance;     // 风险容忍度 [0.0, 1.0]
    int skillLevel;          // 技能等级 1-10
};
```

### 3.4 AIState (AI状态)

```cpp
enum class AIState {
    Idle = 0,
    Racing,       // 正常竞速
    Overtaking,  // 超车中
    Defending,   // 防守中
    Recovering,   // 恢复中
    Finished,     // 已完成
    Count
};
```

### 3.5 AIStyle (AI风格)

```cpp
enum class AIStyle {
    Unknown = -1,
    Conservative = 0,  // 保守型
    Normal = 1,        // 正常型
    Aggressive = 2,   // 激进型
    Defensive = 3,     // 防守型
    Count
};
```

## 4. 接口方法说明

### 4.1 必须实现的方法

#### 配置相关
- `getName() const` - 返回 AI 名称
- `getStyle() const` - 返回驾驶风格
- `getConfig() const` - 返回配置
- `setConfig(const AIConfig&)` - 设置配置

#### 路径点系统
- `setWaypoints(const QList<Waypoint>&)` - 设置路径点列表
- `getWaypoints() const` - 获取路径点列表
- `getCurrentWaypoint() const` - 获取当前路径点
- `getNextWaypoint() const` - 获取下一个路径点

#### 位置与运动
- `getPosition() const` / `setPosition(const QVector2D&)` - 位置
- `getRotation() const` / `setRotation(qreal)` - 旋转角度
- `getVelocity() const` / `setVelocity(const QVector2D&)` - 速度向量
- `getSpeed() const` - 当前速度标量
- `getMaxSpeed() const` / `setMaxSpeed(qreal)` - 最大速度
- `getSteeringAngle() const` / `setSteeringAngle(qreal)` - 转向角

#### 状态管理
- `getState() const` / `setState(AIState)` - 当前状态
- `getCurrentLap() const` / `setCurrentLap(int)` - 当前圈数
- `getLapTime() const` / `setLapTime(qreal)` - 圈速时间
- `getBestLapTime() const` / `setBestLapTime(qreal)` - 最快圈速
- `getPosition() const` / `setPosition(int)` - 当前排名
- `getCheckpointsPassed() const` / `setCheckpointsPassed(int)` - 通过检查点数量

#### 决策与更新
- `makeDecision()` - 生成驾驶决策
- `update(qint64 elapsedMs)` - 更新 AI 状态
- `calculateSteering()` - 计算转向量
- `calculateThrottle()` - 计算油门值
- `calculateBraking()` - 计算刹车值

#### 道具系统
- `hasPowerup() const` - 是否有道具
- `getPowerupCount() const` - 道具数量
- `addPowerup(int powerupType)` - 添加道具
- `usePowerup(int slot)` - 使用道具
- `clearPowerups()` - 清除所有道具

#### 碰撞与交互
- `onCollision(const QString& objectId, const QVector2D& point)` - 碰撞处理
- `onNearMiss(const QString& opponentId, qreal distance)` - 擦身而过
- `onOvertaken(const QString& opponentId)` - 被超越
- `onOvertake(const QString& opponentId)` - 超越其他对手

#### 序列化
- `toJson() const` - 序列化为 JSON
- `fromJson(const QJsonObject&)` - 从 JSON 反序列化
- `getStateData() const` - 获取状态数据
- `loadStateData(const QVariantMap&)` - 加载状态数据

### 4.2 纯虚方法 (必须实现)

```cpp
// 状态进入/退出回调
virtual void onStateEnter(AIState newState) = 0;
virtual void onStateExit(AIState oldState) = 0;

// 辅助计算方法
virtual qreal calculateDistanceToWaypoint(const Waypoint& waypoint) const = 0;
virtual qreal calculateAngleToWaypoint(const Waypoint& waypoint) const = 0;
virtual int findNearestWaypointIndex() const = 0;
virtual int findNextRelevantWaypoint() const = 0;
```

### 4.3 信号 (Signals)

```cpp
void stateChanged(AIState oldState, AIState newState);     // 状态改变
void waypointReached(int index);                             // 到达路径点
void lapCompleted(int lapNumber, qreal lapTime);            // 完成一圈
void bestLapTimeUpdated(qreal newBestTime);                  // 刷新最快圈速
void positionChanged(int oldPosition, int newPosition);      // 排名变化
void collisionOccurred(const QString& objectId, const QVector2D& point);  // 碰撞
void powerupUsed(int slot, int powerupType);                // 使用道具
void powerupCollected(int powerupType);                      // 获取道具
void overtakeAttempted(const QString& targetId);            // 尝试超越
void overtakeCompleted(const QString& targetId);            // 完成超越
void raceFinished(int finalPosition, qreal totalTime);      // 比赛完成
void decisionMade(const AIDecision& decision);               // 做出决策
void updated(qint64 elapsedMs);                              // 更新信号
```

## 5. 实现建议

### 5.1 FSM 状态机

建议在 `update()` 中实现有限状态机：

```cpp
void update(qint64 elapsedMs) override {
    switch (m_state) {
        case AIState::Idle:
            // 等待发车
            break;
        case AIState::Racing:
            // 正常竞速逻辑
            break;
        case AIState::Overtaking:
            // 超车逻辑
            break;
        case AIState::Defending:
            // 防守逻辑
            break;
        case AIState::Recovering:
            // 恢复/救车逻辑
            break;
        case AIState::Finished:
            // 完成比赛
            break;
    }
}
```

### 5.2 路径点跟随算法

建议使用 Pure Pursuit 或类似算法：

```cpp
QVector2D calculateSteering() {
    Waypoint target = getNextWaypoint();
    QVector2D toTarget = target.position - getPosition();
    qreal targetAngle = qAtan2(toTarget.y(), toTarget.x());
    qreal angleDiff = normalizeAngle(targetAngle - getRotation());
    return QVector2D(qSin(angleDiff), 0);
}
```

### 5.3 速度控制

```cpp
qreal calculateThrottle() {
    Waypoint next = getNextWaypoint();
    qreal targetSpeed = next.preferredSpeed;

    if (next.isCorner) {
        // 弯道减速
        targetSpeed *= (1.0 - next.cornerSeverity * 0.2);
    }

    qreal speedDiff = targetSpeed - getSpeed();
    return qBound(0.0, speedDiff / getConfig().acceleration, 1.0);
}
```

## 6. 与其他模块的集成

### 6.1 TrackData 集成

```cpp
// 从 TrackData 获取路径点
QList<Waypoint> extractWaypointsFromTrack(TrackData* track) {
    QList<Waypoint> waypoints;
    // 根据赛道数据生成路径点
    return waypoints;
}
```

### 6.2 ArcadeMode 集成

```cpp
// 在 ArcadeMode 中使用 AI 对手
class ArcadeMode : public GameMode {
    AIOpponentManager* m_aiManager;
    void update(qint64 elapsedMs) override {
        m_aiManager->update(elapsedMs);
    }
};
```

### 6.3 Powerup 集成

```cpp
// AI 使用道具的逻辑
bool usePowerup(int slot) override {
    if (slot < 0 || slot >= m_powerups.size()) return false;

    // 根据当前状态决定是否使用
    if (getState() == AIState::Overtaking && m_powerups[slot] == SpeedBoost) {
        // 加速超车
        activatePowerup(slot);
        return true;
    }
    return false;
}
```

## 7. 测试建议

### 7.1 单元测试

- 测试路径点跟随精度
- 测试不同风格 AI 的行为差异
- 测试状态转换逻辑
- 测试道具使用逻辑

### 7.2 集成测试

- 与 ArcadeMode 集成测试
- 与物理引擎集成测试
- 与道具系统集成测试
- 多 AI 同时运行测试

## 8. 注意事项

1. **线程安全**: 如果使用多线程，注意线程同步
2. **性能**: update() 被每帧调用，注意算法效率
3. **可扩展性**: 预留接口以便后续添加新的 AI 风格
4. **一致性**: 保持与现有代码风格一致

# E-A 模块交付文档

日期：2026-05-28（含验收标准对照更新）

***

## 零、验收标准完成情况（对照 `验收标准.md`）

E-A 负责内容：车辆物理、键盘输入、赛道边界与碰撞、比赛流程（倒计时 / 检查点 / 圈数 / 冲线）、向 UI 提供可渲染的车辆状态。

以下按 `验收标准.md` 两条交付线逐条说明**如何完成**、**由谁实现**、**如何自测**。

### 0.1 交付一：最小可玩赛车主循环

| 验收标准 | 是否完成 | 实现说明 |
| -------- | -------- | -------- |
| 主程序中玩家车可以用 WASD 或方向键控制 | ✅ | `GameViewWidget::keyPressEvent` 捕获按键，发射 `keyInputReceived` / `keyReleased`；`MainWindow::setupVehiclePhysics()` 连接到 `VehiclePhysics::handleKeyPress/Release`；`GameViewWidget` 设 `Qt::StrongFocus`，`startDrivingSession()` 中 `m_gameView->setFocus()`。 |
| 车辆不是固定模拟路径，而是根据输入运动 | ✅ | 已移除“按 tick 写死位移”的模拟逻辑；`simulateGameLoop()` 每 50ms 调用 `VehiclePhysics::update(50)`，位移由加速度、转向、摩擦积分得到。 |
| 车速、方向、位置会随物理参数变化 | ✅ | `applyAcceleration` / `applySteering` / `applyFriction` / `applyMovement` 更新 `m_speed`、`m_rotation`、`m_position`；`positionUpdated` 驱动 `GameViewWidget::updatePlayerCar` 与相机跟随。 |
| 车辆不能随意穿出赛道边界 | ✅ | `TrackManager::setCurrentTrack()` 与程序化环道同步；`applyMovement()` 分步移动 + `canOccupyPosition()` 多点采样；`isSolidAt()` 识别 `Wall`/`Barrier`；`checkTrackBounds()` 对赛道包围盒回退。 |
| 草地或非道路区域有减速效果 | ✅ | `checkCollisions()` 中 `Grass`/`Sand` 使用 `m_grassFrictionMultiplier`（0.6）降低 `m_speed`，低速归零。 |
| 撞墙或撞边界时有明显反馈：减速、反弹、停顿或提示 | ✅ | `handleCollisionResponse()`：速度 ×0.3、沿法线反射、500ms `m_collisionCooldown`；`emit collisionOccurred` → `MainWindow::onCollision()` → `InteractiveFeedback`「Wall Hit!」+ 碰撞音效。 |
| 能跑至少 1 圈，并触发 lap completed | ✅ | Arcade 模式：`MainWindow::updateArcadeRaceProgress()` 顺序通过北门→东→南→西 4 个检查门后，再次进入北门/起跑线区域触发 `onLapCompleted()`；`ArcadeHUD::showRaceBanner("Lap X Complete!")` + `InteractiveFeedback::showLapCompleted`。 |

**交付一自测要点：** 运行 `build/bin/Release/test_main_window.exe` → Arcade → Start Drive → GO 后 WASD 驾驶；故意上草地、撞墙看减速与提示；跑完 4 门回北门应出现 Lap Complete。

### 0.2 交付二：可集成最终演示版游戏主循环

| 验收标准 | 是否完成 | 实现说明 |
| -------- | -------- | -------- |
| 竞技模式：倒计时 → 起跑 → 跑圈 → 冲线 → 成绩统计 | ✅ | **倒计时**：`showCountdown()` → `InteractiveFeedback::showCountdown(3)` + `ArcadeHUD::showCountdown`。**起跑**：`onRaceStart()` → `showGo()`、启用 `m_arcadeRaceLogicActive`。**跑圈**：`updateArcadeRaceProgress()` 检查点 0–3 + 圈数。**冲线**：完成 `m_totalLaps`（默认 3）圈后 `showRaceFinished`。**成绩**：`finishDrivingSession()` → `ScoreManager::finishSession` → 驾驶报告。 |
| 学习模式复用同一套车辆运动和赛道边界逻辑 | ✅ | Learning / Arcade 共用 `VehiclePhysics` 与 `TrackManager`；Learning 不启用 `m_arcadeRaceLogicActive`（无倒计时门阵、不计圈），仅复用驾驶与碰撞。 |
| 碰撞事件能传给 U-B 的互动提示层 | ✅ | `collisionOccurred` → `onCollision()` → `InteractiveFeedback::showFeedback`；检查点/圈数另走 `showCheckpoint` / `showLapCompleted`。 |
| 撞墙、撞 AI、撞障碍物不会导致程序崩溃 | ✅ | 碰撞前判空；AI 距离阈值 + `isColliding()` 冷却；`handleCollision` 法线归一化与速度夹紧。 |
| 与 U-A 主界面集成后，模式切换不会导致旧状态残留 | ✅ | `startDrivingSession()` 先 `finishDrivingSession()`；`clearAllAICars()`、`reset()` / `resetArcadeRaceProgress()`、`ArcadeHUD::reset()`；Back 隐藏 HUD 与 gameView。 |
| 最终版本运行时帧率基本稳定，没有明显卡顿 | ✅ | 固定 50ms 物理步长；单定时器驱动物理 + AI + HUD；无重复 `update` 调用。 |

**交付二自测要点：** Arcade 完整跑 3 圈；Learning 可驾驶且不受圈数逻辑干扰；Back 切换模式后重新进入无旧 AI/HUD/圈数残留。

### 0.3 比赛流程与 HUD 的最终实现（补充说明）

与早期设计文档不同，当前**检查点与计圈在 MainWindow 中实现**，并**强制使用与画面一致的赛道数据**（避免 TrackManager 与 GameView 不同步导致“有四扇门但不计数”）：

```
simulateGameLoop (50ms)
  → VehiclePhysics::update()        // 运动、地形、撞墙
  → MainWindow::updateArcadeRaceProgress()  // 仅 Arcade，读 m_gameView->trackData()
       ├ 检查点 0：驶离北门/起跑线区域（或穿过北门条）
       ├ 检查点 1–3：进入东/南/西门（边沿触发 + 路径采样，门条加宽）
       └ 4 门完成后再次进入北门/起跑线 → onLapCompleted
  → updateRaceHud()                 // 速度、完成 0/3·第 1 圈、圈时、排名
```

| 模块 | 作用 |
| ---- | ---- |
| `setupGameView()` | 生成 30×30 环道、起跑线、4 个条状检查门（`computeGateAcrossTrack`）、`setCurrentTrack` + `syncRaceTrackToManager()` |
| `ArcadeHUD` | 嵌入游戏页右侧：速度、圈数、圈时、总时、排名；`showRaceBanner` 显示检查点/Lap 提示 |
| `InteractiveFeedback` | 屏幕中央倒计时、GO、检查点、Lap Complete（`raise()` 保证可见） |
| `statusBar` | 底部「检查点 x/4 已通过」「GO! 检查点 4 个已就绪」等 |

**说明：** 终端若出现 `Failed to initialize QMediaPlayer "Not available"`，为 Qt 多媒体后端未安装，**不影响**驾驶、检查点与 HUD 文字；仅音效可能无声。

***

## 一、功能概述

### 1.1 键盘控制系统

实现了完整的键盘输入控制系统，玩家可以使用 WASD 或方向键控制赛车，空格键用于手刹。键盘事件通过 Qt 事件系统传递到 VehiclePhysics 物理引擎，实现真实的车辆运动响应。

#### 键盘映射

| 按键        | 功能    | 物理效果                    |
| --------- | ----- | ----------------------- |
| **W / ↑** | 加速    | 增加车辆速度，最大速度受赛道限速和车辆性能限制 |
| **S / ↓** | 刹车/倒车 | 减速，速度为0时可倒车             |
| **A / ←** | 左转    | 逆时针旋转车辆，转向角度随速度变化       |
| **D / →** | 右转    | 顺时针旋转车辆，转向角度随速度变化       |
| **空格**    | 手刹    | 快速减速，用于漂移过弯             |

### 1.2 物理约束与赛道边界

- **车速、方向、位置随物理参数变化**：加速度、摩擦力、转向灵敏度等物理参数实时影响车辆运动
- **赛道边界限制**：车辆不能穿出赛道边界，边界处设置安全边距
- **草地/非道路区域减速**：Grass/Sand 区域使用 0.6 摩擦系数 multiplier，明显降低车速
- **撞墙/撞边界反馈**：
  - 减速：碰撞时速度降至 30%
  - 反弹：根据碰撞法线反射速度向量
  - 停顿：500ms 碰撞冷却期，期间无法加速
  - 提示事件：发射 `collisionOccurred` 信号

### 1.3 竞技模式完整流程

实现了完整的比赛流程：**倒计时 → 起跑 → 跑圈（4 检查门）→ 冲线 → 成绩统计**

- **倒计时**：3 秒倒计时（`InteractiveFeedback` + `ArcadeHUD`），提示音（依赖本机 QMediaPlayer）
- **起跑**：`onRaceStart()` 显示 GO、重置 `m_lapsCompleted=0`、启用 `m_arcadeRaceLogicActive`
- **跑圈**：按顺序通过北(1/4)→东(2/4)→南(3/4)→西(4/4) 条状门；状态栏与右侧 HUD 同步提示
- **冲线**：再次进入北门/起跑线且 4 门已齐 → `Lap X Complete`；满 3 圈 → Race Finished
- **成绩统计**：`finishDrivingSession()` → 评分与驾驶报告

### 1.4 学习模式复用

学习模式和竞技模式共用同一套：
- VehiclePhysics 物理引擎
- 赛道边界检测逻辑
- 碰撞响应系统
- 检查点和圈数检测

### 1.5 碰撞事件传递

碰撞事件通过信号槽机制传递给 U-B 的互动提示层：
- `VehiclePhysics::collisionOccurred` → `MainWindow::onCollision()`
- 调用 `InteractiveFeedback::showFeedback()` 显示碰撞提示
- 播放碰撞音效

### 1.6 崩溃防护

- 空指针检查（AI对象、TrackManager等）
- 碰撞响应使用速度方向计算法线，避免除零
- 草地减速添加速度阈值检查（< 0.5 时归零）
- AI碰撞检测添加距离判断和碰撞冷却

### 1.7 模式切换状态清理

- `startDrivingSession()` 开始时自动结束旧会话
- `clearAllAICars()` 清除旧AI车辆
- `ArcadeHUD::reset()` 重置HUD状态
- 清理所有计数器、计时器、状态标志

### 1.8 性能优化

- 固定 50ms 时间步长（20 FPS 物理更新）
- 移除模拟移动逻辑，完全依赖真实物理
- AI更新与玩家物理更新同步进行

***

## 二、架构设计

### 2.1 键盘输入事件传递路径

```
┌─────────────────────────────────────────────────────────────┐
│                    键盘输入流程                               │
│                                                             │
│  ┌──────────────┐    keyPressEvent     ┌──────────────────┐ │
│  │   用户按键    │ ──────────────────→ │  GameViewWidget  │ │
│  └──────────────┘                      │                  │ │
│                                        │  setFocusPolicy  │ │
│                                        │  (Qt::StrongFocus)│ │
│                                        └────────┬─────────┘ │
│                                                 │           │
│                                        emit     │           │
│                                        keyInputReceived     │
│                                                 │           │
│                                        connect  │           │
│                                                 ▼           │
│                                        ┌──────────────────┐ │
│                                        │ VehiclePhysics   │ │
│                                        │                  │ │
│                                        │ handleKeyPress() │ │
│                                        │ handleKeyRelease()│ │
│                                        │                  │ │
│                                        │  m_keyStates     │ │
│                                        │  (加速/刹车/转向) │ │
│                                        └────────┬─────────┘ │
│                                                 │           │
│                                        update() │ 每50ms    │
│                                                 ▼           │
│                                        ┌──────────────────┐ │
│                                        │ 物理计算          │ │
│                                        │ - 加速度计算      │ │
│                                        │ - 摩擦力应用      │ │
│                                        │ - 转向角度更新    │ │
│                                        │ - 位置积分        │ │
│                                        └────────┬─────────┘ │
│                                                 │           │
│                                        emit     │           │
│                                        positionUpdated      │
│                                                 │           │
│                                        connect  │           │
│                                                 ▼           │
│                                        ┌──────────────────┐ │
│                                        │ MainWindow       │ │
│                                        │                  │ │
│                                        │ updatePlayerCar()│ │
│                                        │ setCameraPosition()│
│                                        └────────┬─────────┘ │
│                                                 │           │
│                                                 ▼           │
│                                        ┌──────────────────┐ │
│                                        │ GameViewWidget   │ │
│                                        │                  │ │
│                                        │ paintEvent()     │ │
│                                        │ drawPlayerCar()  │ │
│                                        └──────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 游戏循环集成

```cpp
// MainWindow::simulateGameLoop() 中的主循环（节选）
connect(m_simTimer, &QTimer::timeout, this, [this]() {
    if (!m_driveActive || m_countdownActive) return;

    const QVector2D positionBeforeUpdate = m_playerPosition;
    if (m_vehiclePhysics) {
        m_vehiclePhysics->update(50);
    }
    if (m_arcadeRaceLogicActive) {
        updateArcadeRaceProgress(positionBeforeUpdate);  // 检查点 + 计圈（用 GameView 赛道）
    }
    // AI 更新、updateRaceHud() ...
});
```

### 2.3 物理约束系统架构

```
┌─────────────────────────────────────────────────────────────┐
│                    物理约束系统                               │
│                                                             │
│  ┌───────────────────────────────────────────────────────┐ │
│  │              VehiclePhysics::update()                  │ │
│  │                                                       │ │
│  │  1. applyAcceleration(dt)  ← 加速度/刹车计算           │ │
│  │  2. applySteering(dt)      ← 转向计算（速度相关）       │ │
│  │  3. applyFriction(dt)      ← 摩擦力（地形相关）         │ │
│  │  4. applyMovement()       ← 分步移动+占格碰撞检测       │ │
│  │  5. checkTrackBounds()    ← 包围盒+瓦片碰撞回退         │ │
│  │  6. checkCollisions()     ← 草地减速+撞墙（不计圈）     │ │
│  │                                                       │ │
│  └───────────────────────────────────────────────────────┘ │
│                                                             │
│  ┌───────────────────────────────────────────────────────┐ │
│  │              地形摩擦系数                              │ │
│  │                                                       │ │
│  │  Road/Asphalt/StartLine/FinishLine: 0.95 (正常)       │ │
│  │  Grass/Sand: 0.95 × 0.6 = 0.57 (明显减速)             │ │
│  │  Wall/Barrier: 0.5 (碰撞后快速减速)                    │ │
│  └───────────────────────────────────────────────────────┘ │
│                                                             │
│  ┌───────────────────────────────────────────────────────┐ │
│  │              碰撞响应流程                              │ │
│  │                                                       │ │
│  │  检测碰撞 → handleCollisionResponse()                 │ │
│  │     ↓                                                 │ │
│  │  1. m_speed *= 0.3          (减速至30%)               │ │
│  │  2. 计算反射向量             (反弹效果)                │ │
│  │  3. emit collisionOccurred  (通知UI层)                │ │
│  │  4. m_isColliding = true    (碰撞状态)                │ │
│  │  5. m_collisionCooldown = 500ms (冷却期)              │ │
│  └───────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

### 2.4 竞技模式流程图

```
┌─────────────────────────────────────────────────────────────┐
│                    竞技模式完整流程                           │
│                                                             │
│  startDrivingSession("Arcade")                              │
│         ↓                                                   │
│  ┌─────────────────┐                                        │
│  │  showCountdown() │  3秒倒计时，播放提示音                  │
│  └────────┬────────┘                                        │
│           │ QTimer::singleShot(3000)                        │
│           ▼                                                 │
│  ┌─────────────────┐                                        │
│  │  onRaceStart()   │  显示"GO!"，播放起跑音效                │
│  └────────┬────────┘                                        │
│           │ m_driveActive = true                             │
│           ▼                                                 │
│  ┌─────────────────────────────────────────────────────┐   │
│  │              游戏循环 (50ms/次)                       │   │
│  │                                                     │   │
│  │  - VehiclePhysics::update(50)                       │   │
│  │  - AIOpponentManager::update(50)                    │   │
│  │  - 碰撞检测 (玩家 vs AI)                             │   │
│  │  - 更新HUD (速度、圈数、时间、排名)                   │   │
│  │                                                     │   │
│  │  ┌─────────────────────────────────────────────┐   │   │
│  │  │  updateArcadeRaceProgress()（MainWindow）   │   │   │
│  │  │  - 驶离北门 → 检查点 1/4                      │   │   │
│  │  │  - 进入东/南/西门 → 2/4、3/4、4/4             │   │   │
│  │  │  - 4 门齐后再进北门 → onLapCompleted          │   │   │
│  │  └─────────────────────────────────────────────┘   │   │
│  └─────────────────────┬───────────────────────────────┘   │
│                        │ 直接调用 onLapCompleted           │
│                        ▼                                   │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  onLapCompleted(lapNumber)                          │   │
│  │                                                     │   │
│  │  if (lapNumber >= m_totalLaps):                     │   │
│  │      - m_arcadeRaceFinished = true                  │   │
│  │      - showRaceFinished(排名, 总时间)                │   │
│  │      - QTimer::singleShot(2000) → finishDrivingSession()│
│  └─────────────────────┬───────────────────────────────┘   │
│                        │                                   │
│                        ▼                                   │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  finishDrivingSession()                             │   │
│  │                                                     │   │
│  │  - m_driveActive = false                            │   │
│  │  - m_scoreManager->finishSession()                  │   │
│  │  - 触发 onScoreReady → 显示驾驶报告                  │   │
│  └─────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

### 2.5 检查点与圈数检测逻辑（当前实现）

检查点由 `setupGameView()` 中 `computeGateAcrossTrack()` 按环道四向扫描路面生成**条状门**（与黄框绘制一致）。计圈在 `MainWindow::updateArcadeRaceProgress()` 中完成，数据源为 `m_gameView->trackData()`。

**检查点顺序（顺时针）：** 0=北门，1=东门，2=南门，3=西门。

```cpp
// 逻辑摘要（见 src/UI/mainwindow.cpp）
// 1) 检查点 0：已离开北门/起跑区域后，驶离时记 1/4（或穿过北门条）
// 2) 检查点 1–3：进入对应门区（边沿触发 enteredNext + 路径 8 点采样）
// 3) 计圈：m_nextCheckpointIndex >= 4 且再次进入北门/起跑线（边沿触发）
// 4) HUD：完成 0/3·第 1 圈；onCheckpointReached → 检查点 x/4 已通过
```

**关键状态变量：** `m_nextCheckpointIndex`、`m_hasLeftNorthSector`、`m_wasInsideNextGate`、`m_lapsCompleted`、`m_arcadeRaceLogicActive`。

### 2.6 碰撞事件传递到U-B互动提示层

```cpp
// MainWindow::setupVehiclePhysics() 中的信号连接
connect(m_vehiclePhysics, &VehiclePhysics::collisionOccurred,
        this, [this](const QString& objectType, 
                     const QVector2D& position, qreal impactForce) {
            onCollision();  // 调用互动提示层
            
            if (m_scoreManager) {
                m_scoreManager->recordCollision(
                    position.toPointF(), m_playerSpeed, objectType);
            }
        });

// onCollision() 实现
void MainWindow::onCollision()
{
    // 显示碰撞提示 (U-B 互动提示层)
    PhantomDrive::InteractiveFeedback::instance(this).showFeedback(
        "Wall Hit!",
        PhantomDrive::FeedbackType::Critical
    );
    
    // 播放碰撞音效
    playSound(PhantomDrive::SoundEffect::Collision);
}
```

***

## 三、修改的文件

### 3.1 GameViewWidget — 键盘事件接收器

**文件：** `include/PhantomDrive/UI/GameViewWidget.h` / `src/UI/GameViewWidget.cpp`

**修改内容：**

1. 添加键盘事件处理函数声明和实现
2. 添加焦点管理函数
3. 添加键盘信号
4. 添加 `clearAllAICars()` 方法（模式切换时清理）

### 3.2 MainWindow — 信号槽连接与比赛逻辑中心

**文件：** `include/PhantomDrive/UI/mainwindow.h` / `src/UI/mainwindow.cpp`

**修改内容：**

1. 添加 `VehiclePhysics`、程序化环道、`computeGateAcrossTrack` 四向检查门
2. `setupVehiclePhysics()`：键盘 → 物理；`positionUpdated` → 渲染
3. **`updateArcadeRaceProgress()`**：Arcade 专用检查点/计圈（使用 `GameView` 赛道，避免与 TrackManager 不同步）
4. **`syncRaceTrackToManager()`**：开局/GO/加载赛道时同步碰撞用赛道
5. `ArcadeHUD` 嵌入游戏页右侧；`displaySpeedKmh()` 统一速度显示
6. `onLapCompleted()` / `onCheckpointReached()`：HUD + InteractiveFeedback + statusBar
7. AI 碰撞、`startDrivingSession()` / `onRaceStart()` 状态清理

### 3.3 VehiclePhysics — 物理引擎核心

**文件：** `include/PhantomDrive/core/VehiclePhysics.h` / `src/core/VehiclePhysics.cpp`

**功能：**

- 键盘状态管理（m_keyStates）
- 物理运动计算（加速度、摩擦力、转向）
- 赛道边界检测（checkTrackBounds）
- 碰撞检测与响应（checkCollisions）
- `applyMovement()` / `canOccupyPosition()` / `isSolidAt()` — 分步移动与穿墙防护
- 地形摩擦（草地/沙地）
- 圈数/检查点逻辑已迁至 `MainWindow::updateArcadeRaceProgress()`（物理层仅做地形与撞墙）

**新增/保留接口：**

- `handleCollision()` — 供 AI 碰撞调用
- `setRaceLogicEnabled()` — 保留；Arcade 计圈以 MainWindow 为准

### 3.4 ArcadeHUD — 竞技模式 HUD

**文件：** `include/PhantomDrive/UI/ArcadeHUD.h` / `src/UI/ArcadeHUD.cpp`

**新增/调整：**

- 嵌入 `MainWindow` 游戏页右侧（非浮动 Tool 窗），避免被遮挡
- `updateLap(lapsCompleted, totalLaps)` — 显示「完成 0/3 · 第 1 圈」
- `showRaceBanner()` — 检查点 x/4、Lap Complete 大字提示
- `reset()` — 模式切换时重置

### 3.5 SoundEffects — 音效枚举

**文件：** `include/PhantomDrive/UI/SoundEffects.h`

**新增音效：**

- `RaceFinish` — 比赛完成音效

### 3.6 CMakeLists.txt — 构建系统

**修改内容：**

添加 VehiclePhysics 到构建系统：

***

## 四、焦点管理

键盘事件需要 GameViewWidget 获得焦点才能接收。在开始驾驶会话时自动设置焦点：

```cpp
void MainWindow::startDrivingSession(const QString& trackId)
{
    // ...
    
    // 重置物理状态
    if (m_vehiclePhysics) {
        m_vehiclePhysics->reset();
    }
    
    // 确保游戏画面获得键盘焦点
    if (m_gameView) {
        m_gameView->setFocus();
    }
    
    m_driveActive = true;
}
```

***

## 五、测试说明

### 5.1 运行测试程序

```powershell
$env:Path = "C:\Users\win11\Desktop\anaconda\envs\dustracing\Library\bin;" + $env:Path
$env:QT_QPA_PLATFORM_PLUGIN_PATH = "C:\Users\win11\Desktop\anaconda\envs\dustracing\Library\lib\qt6\plugins\platforms"
cd d:\c++大作业\PhantomDrive528\build\bin\Release
.\test_main_window.exe
```

### 5.2 键盘控制测试

1. 启动测试程序
2. 点击 Arcade 或 Learn 按钮进入游戏场景
3. 点击"开始驾驶"按钮
4. 使用 WASD 或方向键控制车辆
5. 验证以下功能：
   - W/↑ 加速，车辆向前移动
   - S/↓ 刹车，车辆减速
   - A/← 左转，车辆逆时针旋转
   - D/→ 右转，车辆顺时针旋转
   - 空格 手刹，车辆快速减速
   - 相机跟随车辆移动
   - HUD 显示速度信息

### 5.3 键盘控制预期行为

| 操作      | 预期结果                     |
| ------- | ------------------------ |
| 按住 W    | 车辆持续加速，速度表数值增加           |
| 松开 W    | 车辆因摩擦力逐渐减速               |
| 按住 S    | 车辆减速，速度为0后开始倒车           |
| 按住 A/D  | 车辆转向，转向角度随速度变化（高速时转向更敏感） |
| 按空格     | 车辆快速减速，模拟手刹效果            |
| 同时按 W+A | 车辆加速并左转，沿弧线运动            |

### 5.4 物理约束测试

1. **赛道边界测试**：
   - 驾驶车辆靠近赛道边界
   - 验证车辆不能穿出边界
   - 验证碰撞时有减速、反弹、停顿效果

2. **草地减速测试**：
   - 驾驶车辆离开道路进入草地区域
   - 验证车速明显降低
   - 返回道路后恢复正常速度

3. **撞墙反馈测试**：
   - 故意驾驶车辆撞向墙壁
   - 验证以下效果：
     - 速度降至 30%
     - 车辆反弹
     - 500ms 碰撞冷却期
     - 显示 "Wall Hit!" 提示
     - 播放碰撞音效

4. **AI碰撞测试**：
   - 驾驶车辆靠近AI车辆
   - 验证碰撞时有减速和反弹效果
   - 验证程序不会崩溃

### 5.5 竞技模式流程测试

1. **倒计时测试**：
   - 点击 Arcade 按钮
   - 验证显示 3、2、1 倒计时
   - 验证每秒播放提示音

2. **起跑测试**：
   - 倒计时结束后
   - 验证显示 "GO!" 提示
   - 验证播放起跑音效

3. **跑圈测试**：
   - GO 后先向南驶离北门黄条
   - 依次确认状态栏/右侧 HUD：**检查点 1/4 → 2/4 → 3/4 → 4/4**
   - 回到北门/起跑线，验证 **Lap 1 Complete!** 与 **完成 1/3·第 2 圈**
   - 满 3 圈后 Race Finished 与驾驶报告

4. **冲线测试**：
   - 完成所有圈数（默认3圈）
   - 验证显示 "Race Finished!" 和排名
   - 验证显示总时间
   - 验证播放比赛完成音效

5. **成绩统计测试**：
   - 比赛结束后
   - 验证自动弹出驾驶报告窗口
   - 验证报告包含分数和等级

### 5.6 模式切换测试

1. **Arcade → Learning 切换**：
   - 在 Arcade 模式中开始驾驶
   - 点击 Back 返回主菜单
   - 点击 Learning 按钮
   - 验证：
     - AI车辆被清除
     - HUD状态重置
     - 无旧状态残留

2. **Learning → Arcade 切换**：
   - 在 Learning 模式中开始驾驶
   - 点击 Back 返回主菜单
   - 点击 Arcade 按钮
   - 验证：
     - HUD正确显示竞技模式信息
     - AI车辆正常生成
     - 无旧状态残留

### 5.7 性能测试

1. **帧率稳定性**：
   - 运行游戏 1 分钟
   - 验证帧率基本稳定（20 FPS 物理更新）
   - 验证无明显卡顿

2. **长时间运行**：
   - 连续运行游戏 5 分钟
   - 验证内存无泄漏
   - 验证性能无下降

***

## 六、文件清单

### 键盘控制相关文件

| 文件                                           | 说明                      |
| -------------------------------------------- | ----------------------- |
| `include/PhantomDrive/UI/GameViewWidget.h`   | 添加键盘事件处理接口              |
| `src/UI/GameViewWidget.cpp`                  | 实现键盘事件处理和信号发射           |
| `include/PhantomDrive/UI/mainwindow.h`       | 添加 VehiclePhysics 成员    |
| `src/UI/mainwindow.cpp`                      | 连接键盘事件到物理引擎             |
| `include/PhantomDrive/core/VehiclePhysics.h` | 车辆物理引擎头文件               |
| `src/core/VehiclePhysics.cpp`                | 车辆物理引擎实现                |
| `src/track/TrackManager.cpp`                 | `setCurrentTrack`、与视图赛道同步 |
| `CMakeLists.txt`                             | 添加 VehiclePhysics 到构建系统 |

### 物理约束相关文件

| 文件                                           | 说明                      |
| -------------------------------------------- | ----------------------- |
| `include/PhantomDrive/core/VehiclePhysics.h` | 添加 handleCollision 公共接口 |
| `src/core/VehiclePhysics.cpp`                | 增强碰撞响应、圈数检测、边界检测     |
| `include/PhantomDrive/UI/ArcadeHUD.h`        | 添加 reset 方法声明           |
| `src/UI/ArcadeHUD.cpp`                       | 实现 reset 方法             |
| `include/PhantomDrive/UI/GameViewWidget.h`   | 添加 clearAllAICars 方法声明 |
| `src/UI/GameViewWidget.cpp`                  | 实现 clearAllAICars 方法    |
| `include/PhantomDrive/UI/SoundEffects.h`     | 添加 RaceFinish 音效        |

### 额外修复的文件

| 文件                                                | 修复内容                                   |
| ------------------------------------------------- | -------------------------------------- |
| `include/PhantomDrive/track/TrackManager.h`       | 移除 instance() 的 PHANTOMDRIVE\_EXPORT 宏 |
| `include/PhantomDrive/track/TrackDatabase.h`      | 移除 instance() 的 PHANTOMDRIVE\_EXPORT 宏 |
| `include/PhantomDrive/UI/InteractiveFeedback.h`   | 移除 instance() 的 PHANTOMDRIVE\_EXPORT 宏 |
| `include/PhantomDrive/UI/SoundManager.h`          | 移除 instance() 的 PHANTOMDRIVE\_EXPORT 宏 |
| `include/PhantomDrive/UI/SoundGenerator.h`        | 移除 instance() 的 PHANTOMDRIVE\_EXPORT 宏 |
| `include/PhantomDrive/core/GameEngine.h`          | 移除 instance() 的 PHANTOMDRIVE\_EXPORT 宏 |
| `include/PhantomDrive/core/saveloadmanager.h`     | 移除 instance() 的 PHANTOMDRIVE\_EXPORT 宏 |
| `include/PhantomDrive/gamemode/GameModeManager.h` | 移除 instance() 的 PHANTOMDRIVE\_EXPORT 宏 |

***

## 七、架构总结

```
┌─────────────────────────────────────────────────────────────┐
│                      MainWindow (U-A)                        │
│                                                             │
│  ┌───────────────────────────────────────────────────────┐ │
│  │                  GameViewWidget (E-A)                  │ │
│  │                                                       │ │
│  │  ┌─────────────┐         ┌─────────────────────────┐ │ │
│  │  │  键盘事件    │ ──────→ │  emit keyInputReceived  │ │ │
│  │  │  keyPress   │         │  emit keyReleased       │ │ │
│  │  └─────────────┘         └────────────┬────────────┘ │ │
│  │                                       │              │ │
│  │  ┌────────────────────────────────────▼──────────┐  │ │
│  │  │              paintEvent()                      │  │ │
│  │  │  - drawTrack()                                 │  │ │
│  │  │  - drawPlayerCar() ← m_playerPosition          │  │ │
│  │  │  - drawAICar()                                 │  │ │
│  │  │  - setCameraPosition() ← m_playerPosition      │  │ │
│  │  └────────────────────────────────────────────────┘  │ │
│  └───────────────────────────────────────────────────────┘ │
│                           ↕ connect                        │
│  ┌───────────────────────────────────────────────────────┐ │
│  │               VehiclePhysics (E-A)                     │ │
│  │                                                       │ │
│  │  ┌─────────────┐         ┌─────────────────────────┐ │ │
│  │  │ handleKey   │ ──────→ │  m_keyStates            │ │ │
│  │  │ Press/Release│        │  - m_accelerating       │ │ │
│  │  └─────────────┘         │  - m_braking            │ │ │
│  │                          │  - m_steeringLeft       │ │ │
│  │                          │  - m_steeringRight      │ │ │
│  │                          │  - m_handbrake          │ │ │
│  │                          └────────────┬────────────┘ │ │
│  │                                       │              │ │
│  │                          ┌────────────▼────────────┐ │ │
│  │                          │     update(50ms)        │ │ │
│  │                          │  - 计算加速度            │ │ │
│  │                          │  - 应用摩擦力            │ │ │
│  │                          │  - 更新转向角度          │ │ │
│  │                          │  - 积分计算位置          │ │ │
│  │                          │  - 碰撞检测              │ │ │
│  │                          │  - 检查点检测            │ │ │
│  │                          └────────────┬────────────┘ │ │
│  │                                       │              │ │
│  │                          emit         │              │ │
│  │                          positionUpdated             │ │
│  │                          collisionOccurred           │ │
│  │                          lapCompleted                │ │
│  │                          checkpointReached           │ │
│  └───────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

**键盘控制系统通过 Qt 事件系统接收键盘输入，经信号槽传递到 VehiclePhysics 物理引擎，再通过游戏循环驱动车辆运动，最终将位置更新反馈到 GameViewWidget 进行渲染。**

**物理约束系统确保车辆在赛道边界内行驶，草地/非道路区域有明显减速效果，撞墙/撞边界时有减速、反弹、停顿和提示事件反馈。**

**竞技模式完整流程：倒计时 → 起跑 → 四检查门顺序计圈 → 冲线 → 成绩统计；学习模式复用物理与边界，不参与计圈。**

**碰撞与圈数/检查点事件进入 U-B 互动层（InteractiveFeedback）与 ArcadeHUD、状态栏；`验收标准.md` 所列条目均已按上表实现并可按第五节步骤复测。**

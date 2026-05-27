# E-A 模块交付

日期：2026-05-17

***

相关文件架构

```
PhantomDrive517/
├── include/PhantomDrive/                    
│   ├── PhantomDrive_global.h               
│   ├── core/                               ← 核心引擎（新增）
│   │   └── GameEngine.h                    ← 统一引擎接口
│   ├── UI/                                 ← 用户界面（新增）
│   │   ├── GameViewWidget.h                ← 游戏画面控件
│   │   ├── GameRenderState.h               ← 渲染状态数据结构
│   │   ├── mainwindow.h                    ← 主窗口
│   │   ├── LearningHUD.h                   ← HUD 控件
│   │   ├── DrivingReportWidget.h           ← 数据报表窗口
│   │   └── TestVisualizationWindow.h       ← 测试窗口
│   ├── gamemode/                           ← 游戏模式框架（新增）
│   │   ├── AIOpponent.h                    ← AI 对手抽象接口
│   │   ├── SimpleAIOpponent.h              ← AI 对手基础实现
│   │   ├── AIOpponentManager.h             ← AI 管理器
│   │   ├── TrafficObjectManager.h          ← 交通对象管理器
│   │   ├── PowerupManager.h                ← 道具管理器
│   │   ├── VehicleSensor.h                 ← 车辆传感器
│   │   └── CollisionDetector.h             ← 碰撞检测器
│   └── track/                              
│       ├── TrackData.h                     
│       ├── TrackTile.h                     
│       ├── TrackManager.h                  
│       └── TrackIO.h                       
├── src/                                    
│   ├── core/
│   │   └── GameEngine.cpp
│   ├── UI/
│   │   ├── GameViewWidget.cpp
│   │   ├── mainwindow.cpp
│   │   ├── LearningHUD.cpp
│   │   ├── DrivingReportWidget.cpp
│   │   └── TestVisualizationWindow.cpp
│   ├── gamemode/
│   │   ├── AIOpponent.cpp
│   │   ├── SimpleAIOpponent.cpp
│   │   ├── AIOpponentManager.cpp
│   │   ├── TrafficObjectManager.cpp
│   │   └── PowerupManager.cpp
│   └── track/
│       ├── TrackData.cpp
│       ├── TrackTile.cpp
│       └── TrackManager.cpp
├── tests/                                  
│   ├── test_main_window.cpp                ← 完整游戏界面（新增）
│   └── test_visualization_simple.cpp       ← 简化可视化（新增）
├── assets/                                 
│   └── mainwindow.ui                                          
└── build/                                  ← 编译输出
    ├── Release/
    │   └── PhantomDrive.dll                
    └── bin/Release/
        ├── test_main_window.exe            ← 完整测试程序
        └── test_visualization.exe          ← 简化测试程序
```

***

## 一、明确问题

### 1.1 渲染责任划分

| 游戏元素                   | 负责模块                 | 具体实现位置                                                      | 说明                                        |
| ---------------------- | -------------------- | ----------------------------------------------------------- | ----------------------------------------- |
| **赛道/道路（Track Tiles）** | **E-A**              | `src/UI/GameViewWidget.cpp` → `drawTrack()`                 | 根据 `TrackData` 逐格绘制草地、道路、沥青、起点线、终点线等      |
| **玩家赛车**               | **E-A**              | `src/UI/GameViewWidget.cpp` → `drawPlayerCar()`             | 红色多边形，带速度标签                               |
| **AI 赛车**              | **E-A（渲染）/ A-A（状态）** | `src/UI/GameViewWidget.cpp` → `drawAICar()`                 | 蓝色多边形，A-A 提供位置/旋转/速度，E-A 负责画              |
| **道具盒**                | **E-A（渲染）/ E-B（状态）** | `src/UI/GameViewWidget.cpp` → `drawPowerupBox()`            | 黄色圆角方块，E-B 管理道具生成/拾取，E-A 负责画              |
| **红绿灯**                | **E-A（渲染）/ E-B（状态）** | `src/UI/GameViewWidget.cpp` → `drawTrafficLight()`          | 灯柱+圆形信号灯，红/黄/绿三色切换                        |
| **限速牌**                | **E-A**              | `src/UI/GameViewWidget.cpp` → `drawSpeedLimitSign()`        | 圆形标志，显示限速数值                               |
| **行人区域（斑马线）**          | **E-A**              | `src/UI/GameViewWidget.cpp` → `drawPedestrianCrossing()`    | 白色半透明矩形+条纹                                |
| **HUD（速度、圈数、模式等）**     | **U-B**              | `src/UI/LearningHUD.cpp` / `src/UI/DrivingReportWidget.cpp` | E-A 提供数据信号，U-B 负责样式和布局                    |
| **主窗口框架（菜单、按钮、布局）**    | **U-A**              | `src/UI/mainwindow.cpp`                                     | U-A 负责将 `GameViewWidget` 嵌入 stackedWidget |

***

## 二、对接关系

### 3.1 E-A → U-A：交付可嵌入主窗口的游戏画面控件

**交付物：** `GameViewWidget` 类

**嵌入方式（已在 MainWindow 中实现）：**

```cpp
// src/UI/mainwindow.cpp 中的 setupGameView()
m_gameView = new GameViewWidget(this);
m_gameView->hide();

// 将 GameViewWidget 嵌入 stackedWidget 的第 1 页（游戏页面）
QWidget* gamePage = ui->stackedWidget->widget(1);
QVBoxLayout* layout = new QVBoxLayout(gamePage);
layout->setContentsMargins(0, 0, 0, 0);
layout->setSpacing(0);
layout->addWidget(m_gameView);
```

**U-A 下一步工作需要注意：**

1. `GameViewWidget` 是一个标准 `QWidget`，可以直接添加到任何布局中
2. 建议设置 `setContentsMargins(0, 0, 0, 0)` 避免边框留白
3. 游戏页面切换时，调用 `m_gameView->show()` / `m_gameView->hide()` 控制显示
4. 不需要 U-A 手动调用 `update()` 或 `paintEvent()`，`GameViewWidget` 会自动重绘
5. 如果需要响应鼠标点击游戏对象，可以连接 `objectClicked` 信号

**相关文件：**

- `include/PhantomDrive/UI/GameViewWidget.h` — 控件头文件
- `src/UI/GameViewWidget.cpp` — 控件实现
- `include/PhantomDrive/UI/GameRenderState.h` — 渲染状态数据结构
- `src/UI/mainwindow.cpp` — 嵌入示例（`setupGameView()` 函数）

### 3.2 E-A → A-A：交付赛道路径、waypoints、车辆更新接口

**交付物：** `GameEngine` 中的赛道查询接口 + `updateAICar()` 渲染接口

**A-A 需要使用的接口：**

```cpp
// 定义于 include/PhantomDrive/core/GameEngine.h

// 获取赛道路径点（用于 AI 导航）
QList<QVector2D> getTrackWaypoints() const;

// 获取检查点（用于 AI 判断进度）
QList<QVector2D> getTrackCheckpoints() const;
int getTrackCheckpointCount() const;

// 获取起始位置和方向
QVector2D getTrackStartPosition() const;
qreal getTrackStartRotation() const;

// 查询赛道边界和路面类型
QRectF getTrackBounds() const;
bool isPositionOnTrack(const QVector2D& position) const;
TileType getTileTypeAt(const QVector2D& position) const;

// 更新 AI 车辆在画面中的显示
void updateAIVehicle(const QString& aiId, const QVector2D& position, qreal rotation, qreal speed,
                    const QString& aiStyle = "", const QString& aiState = "");
```

**A-A 下一步工作需要注意：**

1. AI 决策逻辑完全由 A-A 负责，E-A 只提供赛道数据和渲染接口
2. A-A 需要在自己的更新循环中调用 `GameEngine::updateAIVehicle()` 来刷新 AI 车辆画面
3. 5 月 21 日目标：AI 在自定义赛道上完成完整一圈，需要 E-A 提供完整的 waypoints 列表
4. AI 车辆 ID 需要全局唯一，建议使用 `"ai_1"`, `"ai_2"` 等命名

**相关文件：**

- `include/PhantomDrive/core/GameEngine.h` — 引擎接口（第 87-97 行）
- `src/core/GameEngine.cpp` — 引擎实现
- `include/PhantomDrive/gamemode/AIOpponent.h` — AI 对手接口
- `src/gamemode/AIOpponent.cpp` — AI 对手实现
- `include/PhantomDrive/gamemode/SimpleAIOpponent.h` — AI 对手基础实现
- `src/gamemode/SimpleAIOpponent.cpp` — AI 对手基础实现

### 3.3 E-A → E-B：提供车辆位置、碰撞、赛道区域等底层状态

**交付物：** `GameEngine` 信号 + `VehicleSensor` / `CollisionDetector` 组件

**E-B 可以连接的信号：**

```cpp
// 定义于 include/PhantomDrive/core/GameEngine.h

// 车辆状态更新
void playerVehicleUpdated(const VehicleState& state);
void aiVehicleUpdated(const QString& aiId, const VehicleState& state);

// 碰撞事件
void collisionOccurred(const QString& vehicleId, const QString& objectId);

// 赛道事件
void trackBoundsExceeded(const QString& vehicleId);
void checkpointReached(int checkpointId, int index);
```

**E-B 下一步工作需要注意：**

1. E-B 负责道具盒和交通对象的状态管理（生成、拾取、状态切换）
2. E-B 在道具/交通对象状态变化时，需要调用 `GameViewWidget` 的对应接口更新画面
3. 碰撞检测由 E-A 的 `CollisionDetector` 处理，结果通过信号通知 E-B

**相关文件：**

- `include/PhantomDrive/core/GameEngine.h` — 引擎信号定义
- `src/core/GameEngine.cpp` — 信号发射实现
- `include/PhantomDrive/gamemode/VehicleSensor.h` — 车辆传感器
- `include/PhantomDrive/gamemode/CollisionDetector.h` — 碰撞检测器
- `include/PhantomDrive/gamemode/TrafficObjectManager.h` — 交通对象管理器
- `include/PhantomDrive/gamemode/PowerupManager.h` — 道具管理器

### 3.4 E-A → U-B：提供 HUD 需要的实时状态

**交付物：** `GameEngine` 信号 + `LearningHUD` 数据接口

**U-B 可以连接的信号：**

```cpp
// 定义于 include/PhantomDrive/core/GameEngine.h

void speedDataReady(qreal speed);
void lapDataReady(int currentLap, int totalLaps);
void modeDataReady(const QString& mode);
void speedLimitZoneEntered(qreal limit, const QString& zoneId);
void speedLimitZoneExited();
void lapCompleted(int lapNumber, qreal lapTime);
void raceFinished();
```

**U-B 下一步工作需要注意：**

1. `LearningHUD` 已经实现了基本 HUD 显示（速度、圈数、模式）
2. U-B 负责样式美化和数据看板（`DrivingReportWidget`）
3. E-A 通过 `simulateGameLoop()` 模拟数据，后续需要替换为真实游戏数据

**相关文件：**

- `include/PhantomDrive/core/GameEngine.h` — 引擎信号定义
- `src/core/GameEngine.cpp` — 信号发射实现
- `include/PhantomDrive/UI/LearningHUD.h` — HUD 控件头文件
- `src/UI/LearningHUD.cpp` — HUD 控件实现
- `include/PhantomDrive/UI/DrivingReportWidget.h` — 数据报表窗口头文件
- `src/UI/DrivingReportWidget.cpp` — 数据报表窗口实现

***

## 四、测试程序说明

### 4.1 test\_main\_window\.exe — 完整游戏界面测试

**源码位置：** `tests/test_main_window.cpp`\
**可执行文件位置：** `build/bin/Release/test_main_window.exe`

**功能：**

- 主菜单界面（Arcade、Learn、History、Exit 按钮）
- 点击 Arcade/Learn 进入游戏场景
- 自动模拟玩家车辆运动、AI 车辆跟随
- 道具盒、红绿灯、限速牌、人行横道占位可视化
- HUD 速度显示
- History 按钮打开驾驶数据报表窗口

**运行方式：**

```powershell
$env:Path = "C:\Users\win11\Desktop\anaconda\envs\dustracing\Library\bin;" + $env:Path
$env:QT_QPA_PLATFORM_PLUGIN_PATH = "C:\Users\win11\Desktop\anaconda\envs\dustracing\Library\lib\qt6\plugins\platforms"
.\test_main_window.exe
```

### 4.2 test\_visualization.exe — 简化可视化测试

**源码位置：** `tests/test_visualization_simple.cpp`\
**可执行文件位置：** `build/bin/Release/test_visualization.exe`

**功能：** 独立窗口，直接显示赛道、玩家车、道具盒、交通灯的占位可视化，不依赖 MainWindow。

***

## 五、文件清单

### 核心交付文件

| 文件                                          | 说明                         |
| ------------------------------------------- | -------------------------- |
| `include/PhantomDrive/UI/GameViewWidget.h`  | 游戏画面控件头文件                  |
| `src/UI/GameViewWidget.cpp`                 | 游戏画面控件实现                   |
| `include/PhantomDrive/UI/GameRenderState.h` | 渲染状态数据结构                   |
| `include/PhantomDrive/core/GameEngine.h`    | 统一引擎接口                     |
| `src/core/GameEngine.cpp`                   | 引擎实现                       |
| `src/UI/mainwindow.cpp`                     | 主窗口（含 GameViewWidget 嵌入示例） |

### 辅助文件

| 文件                                                                                               | 说明          |
| ------------------------------------------------------------------------------------------------ | ----------- |
| `include/PhantomDrive/UI/LearningHUD.h` / `src/UI/LearningHUD.cpp`                               | HUD 控件      |
| `include/PhantomDrive/UI/DrivingReportWidget.h` / `src/UI/DrivingReportWidget.cpp`               | 数据报表窗口      |
| `include/PhantomDrive/track/TrackData.h` / `src/track/TrackData.cpp`                             | 赛道数据        |
| `include/PhantomDrive/track/TrackTile.h` / `src/track/TrackTile.cpp`                             | 赛道格子        |
| `include/PhantomDrive/track/TrackManager.h` / `src/track/TrackManager.cpp`                       | 赛道管理器       |
| `include/PhantomDrive/gamemode/AIOpponent.h` / `src/gamemode/AIOpponent.cpp`                     | AI 对手接口     |
| `include/PhantomDrive/gamemode/SimpleAIOpponent.h` / `src/gamemode/SimpleAIOpponent.cpp`         | AI 对手实现     |
| `include/PhantomDrive/gamemode/AIOpponentManager.h` / `src/gamemode/AIOpponentManager.cpp`       | AI 管理器      |
| `include/PhantomDrive/gamemode/VehicleSensor.h`                                                  | 车辆传感器       |
| `include/PhantomDrive/gamemode/CollisionDetector.h`                                              | 碰撞检测器       |
| `include/PhantomDrive/gamemode/TrafficObjectManager.h` / `src/gamemode/TrafficObjectManager.cpp` | 交通对象管理器     |
| `include/PhantomDrive/gamemode/PowerupManager.h` / `src/gamemode/PowerupManager.cpp`             | 道具管理器       |
| `tests/test_main_window.cpp`                                                                     | 完整游戏界面测试程序  |
| `tests/test_visualization_simple.cpp`                                                            | 简化可视化测试程序   |
| `assets/mainwindow.ui`                                                                           | 主窗口 UI 设计文件 |
| `CMakeLists.txt`                                                                                 | 构建配置文件      |

***

## 六、架构总结

```
┌─────────────────────────────────────────────────────────┐
│                        MainWindow (U-A)                  │
│  ┌─────────────┐  ┌───────────────────────────────────┐ │
│  │  主菜单页面   │  │  游戏页面 (stackedWidget[1])       │ │
│  │  Arcade/Learn│  │  ┌─────────────────────────────┐ │ │
│  │  History/Exit│  │  │    GameViewWidget (E-A)      │ │ │
│  └─────────────┘  │  │  ┌─────────────────────────┐ │ │ │
│                   │  │  │  - 赛道/道路              │ │ │ │
│                   │  │  │  - 玩家赛车               │ │ │ │
│                   │  │  │  - AI 赛车                │ │ │ │
│                   │  │  │  - 道具盒                 │ │ │ │
│                   │  │  │  - 红绿灯                 │ │ │ │
│                   │  │  │  - 限速牌                 │ │ │ │
│                   │  │  │  - 行人区域               │ │ │ │
│                   │  │  └─────────────────────────┘ │ │ │
│                   │  └───────────────────────────────┘ │ │
│                   └─────────────────────────────────────┘ │
│                                                           │
│  ┌─────────────────────────────────────────────────────┐ │
│  │  LearningHUD (U-B) — 速度、圈数、模式显示             │ │
│  └─────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│                    GameEngine (E-A)                      │
│  - 游戏循环管理                                          │
│  - 车辆状态管理                                          │
│  - 赛道信息查询                                          │
│  - 碰撞检测                                              │
│  - 信号发射（速度、圈数、碰撞、限速区等）                   │
└─────────────────────────────────────────────────────────┘
```

**一句话总结：E-A 交付的** **`GameViewWidget`** **是一个标准 QWidget，U-A 只需将其添加到布局中即可显示游戏画面；所有游戏场景内的可视化元素都由** **`GameViewWidget`** **统一绘制。**

# U-B 历史面板 & HUD 界面 — 交付说明

分支：https://github.com/AAA354544/PhantomDrive-/tree/feature/ub-history-hud
分支：https://github.com/AAA354544/PhantomDrive-/tree/feature/ub-history-hud （主开发分支）

---

## 功能概述

U-B（展示层）完成了驾驶 HUD、历史报表、图表可视化、JSON 存档、主题样式和音效系统的完整实现，涵盖 Arcade Mode、Learning Mode 和 Custom Track Mode 三个模式。

---

## 一、DrivingReportWidget — 驾驶报告面板

**文件：**
- `include/PhantomDrive/UI/DrivingReportWidget.h`
- `src/UI/DrivingReportWidget.cpp`

### 功能清单

| 功能 | 说明 |
|------|------|
| 实时速度折线图 | `QSplineSeries`，-60s 滚动窗口，驾驶过程中实时追加数据点 |
| 统计数据卡片 | 平均速度、最高速度、总分、等级（A/B/C/D/E） |
| 四项子分进度条 | 安全 40% / 规则遵守 30% / 平顺性 20% / 效率 10%，各有独立颜色 |
| AI 教练建议 | 规则化生成：碰撞≥2 → 安全警告，超速≥2 → 规则警告，急刹≥3 → 平顺提示，转向≥3 → 操控提示 |
| 历史趋势图 | 加载最近 5 条历史记录，绘制 5 条指标曲线的趋势线 |
| 违规事件列表 | 表格形式，含时间戳、类型、扣分 |
| 加载中动画 | "正在计算驾驶报告…" 带脉冲动画，完成后自动切换 |

### 对外接口

```cpp
void setCurrentReport(const ScoreReport& report);       // 填入报告数据，禁用 Mock
void setSessionSpeedSamples(const QList<DrivingData>&);  // 驾驶中追加速度曲线
void addViolationEvent(const ViolationEvent& violation); // 追加违规记录
void loadHistoryReports(const QList<ScoreReport>&);     // 加载历史趋势
void showLoading();                                      // 显示加载中动画
void hideLoading();                                      // 隐藏加载动画
```

### 与 A-B 对接

`finishSession()` 触发 `reportReady` 信号 → `DrivingReportWidget::setCurrentReport()` 填入数据并隐藏 Mock。

---

## 二、DrivingHistoryChartWidget — 历史趋势图表

**文件：**
- `include/PhantomDrive/UI/DrivingHistoryChartWidget.h`
- `src/UI/DrivingHistoryChartWidget.cpp`

### 功能清单

| 功能 | 说明 |
|------|------|
| 历史趋势折线图 | 总分、安全、规则、平顺随时间变化 |
| 分项得分柱状图 | 四项子分横向对比 |
| 违规统计饼图 | 碰撞 / 超速 / 红灯 / 行人 / 逆行 占比 |
| 报告列表 | 表格展示历史记录，支持选中查看详情 |

### 对外接口

```cpp
void updateHistory(const QList<ScoreReport>& history);
void addReport(const ScoreReport& report);
void clear();
QList<ScoreReport> getHistory() const;

signals:
    void reportSelected(int index, const ScoreReport& report);
```

---

## 三、ArcadeHUD — 竞技模式 HUD

**文件：**
- `include/PhantomDrive/UI/ArcadeHUD.h`
- `src/UI/ArcadeHUD.cpp`

### 功能清单

| 功能 | 说明 |
|------|------|
| `SpeedometerWidget` | 200×200 自绘仪表盘，霓虹弧形进度条（绿→橙→红）、刻度、阴影指针、中央速度数字 + "km/h" 单位，飙红闪烁 |
| 速度 / 限速 / 交通灯状态行 | 实时显示，限速数字颜色随限速值变化（红≤30 / 橙≤50） |
| 圈数卡片 + 单圈时间 / 总时间并排 | 格式 `MM:SS.mmm` |
| 排名卡片 | 格式 "Nth"，实时更新 |
| Boost 进度条 | 渐变填充条 |
| AI 速度卡片 | 实时显示最前 AI 速度 |
| 最快圈速卡片 | 格式 `MM:SS.mmm` |
| STOP 叠加层 | 红灯时绝对定位显示红色半透明 "STOP" 文字 |
| 倒计时叠加层 | 3-2-1-GO 序列，居中于仪表盘，文字颜色随阶段变化（红→黄→绿） |

### 对外接口

```cpp
void updateSpeed(qreal speed);                                      // 物理速度转 km/h 后更新仪表盘
void updateLap(int currentLap, int totalLaps);                       // 圈数更新
void updateLapTime(const QString& time);                             // 单圈计时
void updateTotalTime(const QString& time);                           // 总计时
void updateBestLapTime(const QString& time);                         // 最快圈速
void updatePosition(int position, int totalRacers);                   // 排名更新
void updateTrafficLight(const QString& state, int remainingSeconds);  // 交通灯状态 + 闪烁
void updateSpeedLimit(int limit);                                    // 限速更新
void updateAiSpeed(qreal aiSpeed);                                   // AI 速度
void updateBoostBar(qreal value);                                     // Boost 进度 0-1
void showCountdown(int seconds);                                     // 启动倒计时 3-2-1
void showGo();                                                       // GO 信号
void applyTheme(const QString& mode);                                // 模式颜色（Arcade=粉/Learning=绿/CustomTrack=青）
```

---

## 四、LearningHUD — 学习模式 HUD

**文件：**
- `include/PhantomDrive/UI/learninghud.h`
- `src/UI/learninghud.cpp`

### 功能清单

| 功能 | 说明 |
|------|------|
| 速度显示 | 38px 等宽数字，颜色随状态：超速→红，接近限速→黄，正常→青，零速→灰 |
| 限速标签 | 颜色随限速值变化（红≤30 / 橙≤50 / 黄>50） |
| 交通灯指示 | 圆点 + 文字 "RED 5s" / "YLW 3s" / "GREEN"，红灯时 500ms 闪烁定时器 |
| STOP 叠加层 | 红灯时绝对定位红色半透明叠加层 |
| 违规警告信号 | 发射 `violationTriggered` 信号供外部浮动文字使用 |
| 违规扣分飘字信号 | 发射 `penaltyMessageReady` 信号供 `InteractiveFeedback` 使用 |

### 对外接口

```cpp
void updateCurrentSpeed(qreal speed);                              // 更新速度显示
void updateSpeedLimit(int limit);                                  // 更新限速标签
void updateSpeedStatus(bool isOverLimit);                          // 超速状态（颜色反馈）
void updateTrafficLight(const QString& state, int remainingSeconds); // 交通灯 + 闪烁

signals:
    void penaltyMessageReady(const QString& msg, int points);
    void violationTriggered(const QString& type);
```

---

## 五、InteractiveFeedback — 浮动反馈系统

**文件：**
- `include/PhantomDrive/UI/InteractiveFeedback.h`
- `src/UI/InteractiveFeedback.cpp`

### 功能清单

| 功能 | 说明 |
|------|------|
| 非阻塞显示 | 不使用 `QMessageBox`，不卡游戏循环 |
| 消息队列 | 多条消息排队依次显示，不丢失 |
| 类型区分 | Positive / Warning / Critical / Powerup / Milestone / Countdown，样式各异 |
| 动画效果 | 缩放弹入 + 淡入 + 向上飘动，自动淡出消失 |

### 对外接口

```cpp
static InteractiveFeedback& instance(QWidget* parent = nullptr);

void showFeedback(const QString& message, FeedbackType type);
void showFeedback(const QString& message, int points, FeedbackType type);
void showCountdown(int seconds);
void showGo();
void showLapCompleted(int lapNumber);
void showCheckpoint(int checkpointNumber);
void showSpeedBoost(bool active);
void showShield(bool active);
void clearAll();
void pause();
void resume();
```

---

## 六、JSON 存档系统

**文件：**
- `include/PhantomDrive/core/saveloadmanager.h`
- `src/core/saveloadmanager.cpp`
- `include/PhantomDrive/core/datamodels.h`

### 功能清单

| 功能 | 说明 |
|------|------|
| `ScoreReport` 完整序列化 | 违规记录、教练建议、QLearning 反馈全部持久化 |
| 版本兼容 | 支持旧版 `PracticeReport` 格式 |
| 导入/导出 | `exportReport(path)` / `importReport(path)` |
| 信号通知 | `historyChanged` / `reportSaved` / `reportDeleted` / `reportUpdated` |

### 对外接口

```cpp
SaveLoadManager& instance();

bool saveReport(const ScoreReport& report);
QList<ScoreReport> loadHistory();
bool deleteReport(int index);
bool updateReport(int index, const ScoreReport& newReport);
bool exportReport(const ScoreReport& report, const QString& filePath);
bool importReport(const QString& filePath, ScoreReport& outReport);
```

---

## 七、ThemeManager — 主题系统

**文件：**
- `include/PhantomDrive/UI/ThemeManager.h`

三种主题：`dark`（深色，默认）/ `light`（浅色）/ `racing`（霓虹赛车风），覆盖按钮、输入框、表格、Tab、进度条等全部 Qt 控件。

```cpp
qApp->setStyleSheet(ThemeManager::getStyleSheet("racing"));
```

---

## 八、SoundManager & SoundGenerator — 音效系统

**文件：**
- `include/PhantomDrive/UI/SoundManager.h`
- `src/UI/SoundManager.cpp`
- `include/PhantomDrive/UI/SoundGenerator.h`
- `src/UI/SoundGenerator.cpp`

`SoundManager` 为单例，先尝试 `QMediaPlayer` 播放资源文件，资源缺失时自动 fallback 到 `SoundGenerator` 程序化生成音效，无需外部音频文件。

音效类型：`CountdownBeep` / `CountdownGo` / `Collision` / `SpeedBoost` / `Violation` / `Checkpoint` / `LapComplete` / `PowerupCollect`

---

## 九、GameTopBar 重构（2026-05-31）

**文件：**
- `assets/mainwindow.ui` — 新增 `label_ModeTitle`、`btn_ExitGame_Top`、`btn_FinishDrive_Top` 入 `hudLayout`
- `src/UI/mainwindow.cpp` — 移除 `m_gameTopBar` 浮动窗口，统一按钮样式
- `CMakeLists.txt` — 移除 `GameTopBar.h/cpp`

移除原因：浮动 `GameTopBar` 压在左侧状态文字上，且与 `.ui` 中 `hudLayout` 的 Back 按钮重复。

新布局：
```
[Speed | Limit | Light]  [spacer]  [ARCADE MODE]  [spacer]  [Back] [Finish Drive] [Exit Game]
```

三个按钮统一高度 52px、圆角 8px；`label_ModeTitle` 颜色随模式自动切换（Learning=绿 / CustomTrack=青 / Arcade=粉红）。

---

## 十、速度单位统一（2026-05-30）

所有 UI 显示统一使用 km/h，内部物理计算使用原始物理单位不变：

- `MainWindow::speedToDisplayKmh(qreal)` — 统一换算函数
- 玩家速度传感器写入 km/h
- AI 速度标签使用 `speedToDisplayKmh()`
- `DrivingReportWidget` 实时速度曲线使用 km/h
- 限速比较使用 km/h

---

## 十一、DDL 进度

| 任务 | 截止日期 | 状态 |
|------|----------|------|
| JSON 存档系统 | 5月16日 | ✅ 已完成 |
| InteractiveFeedback 浮动反馈 | 5月25日 | ✅ 已完成 |
| SoundManager 音效系统 | 5月25日 | ✅ 已完成 |
| ArcadeHUD 竞技 HUD | 5月25日 | ✅ 已完成 |
| DrivingHistoryChartWidget 历史图表 | 5月22日 | ✅ 已完成 |
| HUD 和 QSS 主题美化 | 5月27日 | ✅ 已完成 |
| DrivingReportWidget 驾驶报告 | 5月27日 | ✅ 已完成 |
| LearningHUD 学习 HUD | 5月27日 | ✅ 已完成 |
| GameTopBar 重构整合 | 5月31日 | ✅ 已完成 |
| 速度单位统一 | 5月30日 | ✅ 已完成 |

---

## 十二、与其他组对接

**A-B（评分）：**
```cpp
// 报告生成后
ScoreReport report = calculator.generateReport();
SaveLoadManager::instance().saveReport(report);
reportWidget.setCurrentReport(report);
historyWidget.addReport(report);
```

**E-B（物理/驾驶）：**
```cpp
// 驾驶循环中
arcadeHUD.updateSpeed(vehicleData.speed);
arcadeHUD.updateTrafficLight(state, remaining);
arcadeHUD.updatePosition(rank, total);
arcadeHUD.updateLapTime(currentLapTime);
learningHUD.updateCurrentSpeed(speed);
learningHUD.updateSpeedLimit(limit);
InteractiveFeedback::instance().showFeedback("Speeding! -5", Warning);
```

**UI 组（界面风格）：**
- 所有 HUD 使用统一赛博朋克暗色主题（`ThemeManager`）
- 右侧面板固定 300px，布局由 `.ui` `hudLayout` 统一管理
- `GameTopBar` 已整合入 `.ui`，后续风格修改只需改 `.ui` 和 `mainwindow.cpp` 的 `setupUi()` 样式代码
